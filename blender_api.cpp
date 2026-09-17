#define BLENDER_BUILD

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "libs/fastnoiselite.h"
#include "src/constants.h"
#include "src/opengl_include.h"
#include "src/types.hpp"
#include "src/global_vars.hpp"
#define INCLUDES
#include "src/function_definitions.h"
#include "src/general_functions.hpp"
inline int get_wrapped_chunk_idx(float pos, float chunk_size, int map_chunks_size) {
    int index = static_cast<int>(std::floor(pos / chunk_size)) % map_chunks_size;
    if (index < 0) index += map_chunks_size;
    return index;
}
bool init_chunk(int, int, int) {
    return false;
}
void gen_chunk(int, int) {}
float gen_mountains_pixel(int, int) {
    return 0.0f;
}
#include "src/debug.hpp"
#include "src/VKDR_2.hpp"

#if !defined(__APPLE__)
#include <GL/glew.h>
#endif

namespace py = pybind11;

namespace {

struct OwnedMesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    std::size_t car_index = 0;
};

struct MaterialTextures {
    GLuint albedo = 0;
    GLuint roughness = 0;
};

void require_matrix(const py::array_t<float>& matrix, const char* name) {
    if (matrix.ndim() != 2 || matrix.shape(0) != 4 || matrix.shape(1) != 4) {
        throw py::value_error(std::string(name) + " must have shape (4, 4)");
    }
}

GLuint create_roughness_texture(float roughness) {
    const float value = std::max(0.0f, std::min(1.0f, roughness));
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, &value);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

GLuint load_png_texture(const std::string& path) {
    if (path.empty()) return 0;
    std::vector<char> mutable_path(path.begin(), path.end());
    mutable_path.push_back('\0');
    const GLuint texture = load_texture_PNG(mutable_path.data());
    if (texture == 0) {
        throw std::runtime_error("VKDR2 could not load texture: " + path);
    }
    return texture;
}

void configure_mesh_attributes() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void create_car_buffer(cardata& car) {
    std::vector<float> data;
    data.reserve(static_cast<std::size_t>(car.faces_count) * 3 * 8);
    for (int face_index = 0; face_index < car.faces_count; ++face_index) {
        for (int vertex_index = 0; vertex_index < 3; ++vertex_index) {
            const car_vertex& vertex = car.vertices[car.faces[face_index].vertices[vertex_index]];
            data.push_back(vertex.x);
            data.push_back(vertex.y);
            data.push_back(vertex.z);
            data.push_back(vertex.nx);
            data.push_back(vertex.ny);
            data.push_back(vertex.nz);
            data.push_back(vertex.u);
            data.push_back(vertex.v);
        }
    }

    glGenVertexArrays(1, &car.vao);
    glGenBuffers(1, &car.vbo);
    glBindVertexArray(car.vao);
    glBindBuffer(GL_ARRAY_BUFFER, car.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    configure_mesh_attributes();
    glBindVertexArray(0);
}

void recreate_hdr_target(int width, int height) {
    if (fboHDR) glDeleteFramebuffers(1, &fboHDR);
    if (texHDR) glDeleteTextures(1, &texHDR);
    if (rboHDRDepth) glDeleteRenderbuffers(1, &rboHDRDepth);

    glGenFramebuffers(1, &fboHDR);
    glGenTextures(1, &texHDR);
    glBindTexture(GL_TEXTURE_2D, texHDR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &rboHDRDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboHDRDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHDR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHDR, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboHDRDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("VKDR2 HDR framebuffer is incomplete after resize");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void delete_renderer_resources() {
    const GLuint programs[] = {prog3DMap, prog3DCar, prog3D0, progHUD,
                               progSkybox, progDepth, progPostProcess, dbg_prog};
    for (GLuint program : programs) {
        if (program) glDeleteProgram(program);
    }
    prog3DMap = prog3DCar = prog3D0 = progHUD = 0;
    progSkybox = progDepth = progPostProcess = dbg_prog = 0;

    const GLuint vertex_arrays[] = {hudVAO, skyboxVAO, quadVAO, dbg_sphere_vao,
                                    dbg_cylinder_vao, dbg_cube_vao, dbg_mesh_vao};
    for (GLuint vao : vertex_arrays) {
        if (vao) glDeleteVertexArrays(1, &vao);
    }
    const GLuint buffers[] = {hudVBO, skyboxVBO, quadVBO, dbg_sphere_vbo,
                              dbg_cylinder_vbo, dbg_cube_vbo, dbg_mesh_vbo};
    for (GLuint buffer : buffers) {
        if (buffer) glDeleteBuffers(1, &buffer);
    }
    hudVAO = skyboxVAO = quadVAO = dbg_sphere_vao = 0;
    dbg_cylinder_vao = dbg_cube_vao = dbg_mesh_vao = 0;
    hudVBO = skyboxVBO = quadVBO = dbg_sphere_vbo = 0;
    dbg_cylinder_vbo = dbg_cube_vbo = dbg_mesh_vbo = 0;

    if (fallbackTex) glDeleteTextures(1, &fallbackTex);
    if (hud_texture_id) glDeleteTextures(1, &hud_texture_id);
    fallbackTex = hud_texture_id = 0;
    if (fboShadow) glDeleteFramebuffers(1, &fboShadow);
    if (texShadow) glDeleteTextures(1, &texShadow);
    if (fboHDR) glDeleteFramebuffers(1, &fboHDR);
    if (texHDR) glDeleteTextures(1, &texHDR);
    if (rboHDRDepth) glDeleteRenderbuffers(1, &rboHDRDepth);
    fboShadow = texShadow = fboHDR = texHDR = rboHDRDepth = 0;
    glDeleteBuffers(2, ae_pbo);
    ae_pbo[0] = ae_pbo[1] = 0;
}

} // namespace

class BlenderRenderer {
public:
    BlenderRenderer(int width = 1024, int height = 1024)
        : width_(width), height_(height) {
        if (width_ <= 0 || height_ <= 0) {
            throw py::value_error("renderer dimensions must be positive");
        }
    }

    ~BlenderRenderer() {
        if (initialized_) cleanup();
    }

    void init() {
        if (initialized_) return;

        if (SDL_Init(0) != 0) {
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
        }
        if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
            const std::string error = IMG_GetError();
            SDL_Quit();
            throw std::runtime_error("SDL_image PNG initialization failed: " + error);
        }
        sdl_initialized_ = true;

#if !defined(__APPLE__)
        glewExperimental = GL_TRUE;
        const GLenum glew_status = glewInit();
        glGetError();
        if (glew_status != GLEW_OK) {
            throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(glew_status)));
        }
#endif

        if (glGetString(GL_VERSION) == nullptr) {
            throw std::runtime_error("Blender must call init() with a current OpenGL context");
        }

        WIDTH = width_;
        HEIGHT = height_;
        blender_target_width = width_;
        blender_target_height = height_;
        VKDR2_setup(width_, height_);
        initialized_ = true;
    }

    void set_camera(const py::array_t<float>& view_matrix,
                    const py::array_t<float>& proj_matrix) {
        require_matrix(view_matrix, "view_matrix");
        require_matrix(proj_matrix, "proj_matrix");

        auto view = view_matrix.unchecked<2>();
        auto projection = proj_matrix.unchecked<2>();
        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                blender_view_matrix.m[column * 4 + row] = view(row, column);
                blender_proj_matrix.m[column * 4 + row] = projection(row, column);
            }
        }
        blender_external_camera = true;

        const Mat4 camera_world = m_inverse(blender_view_matrix);
        x_pos = camera_world.m[12];
        y_pos = camera_world.m[13];
        z_pos = camera_world.m[14];
    }

    void upload_mesh(int id, const py::array_t<float>& vertices,
                     const py::array_t<std::uint32_t>& indices) {
        ensure_initialized();
        if (vertices.ndim() != 2 || vertices.shape(1) < 3) {
            throw py::value_error("vertices must have shape (N, 3), (N, 5), (N, 6), or (N, 8)");
        }
        if (indices.ndim() != 1 || indices.size() % 3 != 0) {
            throw py::value_error("indices must be a one-dimensional triangle index array");
        }

        remove_mesh(id);
        const auto vertex_count = static_cast<std::size_t>(vertices.shape(0));
        const auto attribute_count = static_cast<std::size_t>(vertices.shape(1));
        auto vertex_data = vertices.unchecked<2>();
        auto index_data = indices.unchecked<1>();
        cars.emplace_back();
        cardata& car = cars.back();
        car.faces_count = static_cast<int>(indices.size() / 3);
        car.vertices_count = static_cast<int>(indices.size());

        for (ssize_t i = 0; i < indices.size(); ++i) {
            const std::uint32_t index = index_data(i);
            if (index >= vertex_count) {
                throw py::value_error("mesh index is outside the vertex array");
            }
            const bool has_normals = attribute_count >= 6;
            const bool has_uvs = attribute_count == 5 || attribute_count >= 8;
            const ssize_t uv_offset = attribute_count >= 8 ? 6 : 3;
            car_vertex& car_vertex_data = car.vertices[i];
            car_vertex_data.x = vertex_data(index, 0);
            car_vertex_data.y = vertex_data(index, 1);
            car_vertex_data.z = vertex_data(index, 2);
            car_vertex_data.nx = has_normals ? vertex_data(index, 3) : 0.0f;
            car_vertex_data.ny = has_normals ? vertex_data(index, 4) : 1.0f;
            car_vertex_data.nz = has_normals ? vertex_data(index, 5) : 0.0f;
            car_vertex_data.u = has_uvs ? vertex_data(index, uv_offset) : 0.0f;
            car_vertex_data.v = has_uvs ? vertex_data(index, uv_offset + 1) : 0.0f;
        }
        for (int face_index = 0; face_index < car.faces_count; ++face_index) {
            face& car_face = car.faces[face_index];
            car_face.vertices[0] = face_index * 3;
            car_face.vertices[1] = face_index * 3 + 1;
            car_face.vertices[2] = face_index * 3 + 2;
            car_face.nx = car.vertices[car_face.vertices[0]].nx;
            car_face.ny = car.vertices[car_face.vertices[0]].ny;
            car_face.nz = car.vertices[car_face.vertices[0]].nz;
        }

        create_car_buffer(car);
        owned_meshes_[id] = {cars.back().vao, cars.back().vbo, cars.size() - 1};
        apply_material(cars.back(), id);
    }

    void upload_material(int id, const std::string& albedo_path,
                         float default_roughness, const std::string& roughness_path) {
        ensure_initialized();
        MaterialTextures textures;
        textures.albedo = load_png_texture(albedo_path);
        textures.roughness = roughness_path.empty()
            ? create_roughness_texture(default_roughness)
            : load_png_texture(roughness_path);
        if (textures.roughness == 0) textures.roughness = create_roughness_texture(default_roughness);

        auto previous = materials_.find(id);
        if (previous != materials_.end()) {
            glDeleteTextures(1, &previous->second.albedo);
            glDeleteTextures(1, &previous->second.roughness);
        }
        materials_[id] = textures;
        const auto owned = owned_meshes_.find(id);
        if (owned != owned_meshes_.end()) apply_material(cars[owned->second.car_index], id);
    }

    void render_frame(int width, int height, unsigned int fbo_id) {
        ensure_initialized();
        if (width <= 0 || height <= 0) throw py::value_error("render dimensions must be positive");
        WIDTH = width;
        HEIGHT = height;
        recreate_hdr_target_if_needed(width, height);
        blender_target_width = width;
        blender_target_height = height;
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo_id));
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VKDR2_render_blender(blender_view_matrix, blender_proj_matrix,
                     width, height, static_cast<GLuint>(fbo_id));
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo_id));
    }

    void cleanup() {
        for (const auto& [id, mesh] : owned_meshes_) {
            if (mesh.car_index < cars.size()) {
                glDeleteVertexArrays(1, &cars[mesh.car_index].vao);
                glDeleteBuffers(1, &cars[mesh.car_index].vbo);
            }
        }
        owned_meshes_.clear();
        for (const auto& [id, texture] : materials_) {
            glDeleteTextures(1, &texture.albedo);
            glDeleteTextures(1, &texture.roughness);
        }
        materials_.clear();
        delete_renderer_resources();
        cars.clear();
        blender_external_camera = false;
        if (sdl_initialized_) {
            IMG_Quit();
            SDL_Quit();
            sdl_initialized_ = false;
        }
        initialized_ = false;
    }

private:
    void ensure_initialized() const {
        if (!initialized_) throw std::runtime_error("BlenderRenderer.init() must be called first");
    }

    void apply_material(MeshBuffer& buffer, int id) {
        const auto material = materials_.find(id);
        if (material != materials_.end()) {
            buffer.textureID = material->second.albedo;
            buffer.roughnessID = material->second.roughness;
        }
    }

    void apply_material(cardata& car, int id) {
        const auto material = materials_.find(id);
        if (material != materials_.end()) {
            car.texture = material->second.albedo;
            car.roughness = material->second.roughness;
        }
    }

    void remove_mesh(int id) {
        const auto owned = owned_meshes_.find(id);
        if (owned == owned_meshes_.end()) return;
        if (owned->second.car_index < cars.size()) {
            glDeleteVertexArrays(1, &cars[owned->second.car_index].vao);
            glDeleteBuffers(1, &cars[owned->second.car_index].vbo);
            cars.erase(cars.begin() + static_cast<std::ptrdiff_t>(owned->second.car_index));
            for (auto& entry : owned_meshes_) {
                if (entry.second.car_index > owned->second.car_index) --entry.second.car_index;
            }
        }
        owned_meshes_.erase(owned);
    }

    void recreate_hdr_target_if_needed(int width, int height) {
        if (width == width_ && height == height_) return;
        recreate_hdr_target(width, height);
        width_ = width;
        height_ = height;
    }

    int width_;
    int height_;
    bool initialized_ = false;
    bool sdl_initialized_ = false;
    std::map<int, OwnedMesh> owned_meshes_;
    std::map<int, MaterialTextures> materials_;
};

PYBIND11_MODULE(vkdr2_renderer, module) {
    module.doc() = "Blender bridge for the VKDR2 OpenGL renderer";
    py::class_<BlenderRenderer>(module, "BlenderRenderer")
        .def(py::init<int, int>(), py::arg("width") = 1024, py::arg("height") = 1024)
        .def("init", &BlenderRenderer::init)
        .def("set_camera", &BlenderRenderer::set_camera)
        .def("upload_mesh", &BlenderRenderer::upload_mesh)
        .def("upload_material", &BlenderRenderer::upload_material)
        .def("render_frame", &BlenderRenderer::render_frame)
        .def("cleanup", &BlenderRenderer::cleanup);
}