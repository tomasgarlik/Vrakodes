#define BLENDER_BUILD
#define GL_SILENCE_DEPRECATION

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

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
bool init_chunk(int, int, int) { return false; }
void gen_chunk(int, int) {}
float gen_mountains_pixel(int, int) { return 0.0f; }
#include "src/debug.hpp"
#include "src/VKDR_2.hpp"

namespace {

struct PreviewMesh {
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;
    std::string albedo;
    std::string roughness;
};

struct PreviewFile {
    std::vector<PreviewMesh> meshes;
    float camera_position[3] = {0.0f, 1.5f, 6.0f};
    float camera_target[3] = {0.0f, 0.0f, 0.0f};
    float camera_fov = 60.0f;
    float camera_near = 0.1f;
    float camera_far = 100.0f;
    int render_width = 1280;
    int render_height = 720;
};

bool read_bytes(std::ifstream& file, void* data, std::size_t size) {
    file.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
    return file.good();
}

bool read_string(std::ifstream& file, std::string& value) {
    std::uint32_t length = 0;
    if (!read_bytes(file, &length, sizeof(length)) || length > 4096) return false;
    value.resize(length);
    return length == 0 || read_bytes(file, value.data(), length);
}

bool load_preview_file(const char* path, PreviewFile& output) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    char magic[8] = {};
    std::uint32_t mesh_count = 0;
    if (!read_bytes(file, magic, sizeof(magic)) ||
        std::string(magic, sizeof(magic)) != "VKD2MESH" ||
        !read_bytes(file, &mesh_count, sizeof(mesh_count)) ||
        mesh_count == 0 || mesh_count > MAX_VEHICLES) {
        return false;
    }
    output.meshes.resize(mesh_count);
    for (PreviewMesh& mesh : output.meshes) {
        std::uint32_t vertex_count = 0;
        std::uint32_t index_count = 0;
        if (!read_bytes(file, &vertex_count, sizeof(vertex_count)) ||
            !read_bytes(file, &index_count, sizeof(index_count)) ||
            vertex_count == 0 || vertex_count > CAR_MAX_VERTICES ||
            index_count == 0 || index_count % 3 != 0 || index_count > CAR_MAX_POLY * 3) {
            return false;
        }
        mesh.vertices.resize(static_cast<std::size_t>(vertex_count) * 8);
        mesh.indices.resize(index_count);
        if (!read_bytes(file, mesh.vertices.data(), mesh.vertices.size() * sizeof(float)) ||
            !read_bytes(file, mesh.indices.data(), mesh.indices.size() * sizeof(std::uint32_t)) ||
            !read_string(file, mesh.albedo) || !read_string(file, mesh.roughness)) {
            return false;
        }
    }
    float camera_data[9] = {};
    if (read_bytes(file, camera_data, sizeof(camera_data))) {
        for (int i = 0; i < 3; ++i) {
            output.camera_position[i] = camera_data[i];
            output.camera_target[i] = camera_data[i + 3];
        }
        output.camera_fov = camera_data[6];
        output.camera_near = camera_data[7];
        output.camera_far = camera_data[8];
    }
    std::uint32_t render_size[2] = {};
    if (read_bytes(file, render_size, sizeof(render_size))) {
        output.render_width = std::max(1u, render_size[0]);
        output.render_height = std::max(1u, render_size[1]);
    }
    return true;
}

void create_car_buffer(cardata& car) {
    std::vector<float> data;
    data.reserve(static_cast<std::size_t>(car.faces_count) * 3 * 8);
    for (int face_index = 0; face_index < car.faces_count; ++face_index) {
        for (int vertex_index = 0; vertex_index < 3; ++vertex_index) {
            const car_vertex& vertex = car.vertices[car.faces[face_index].vertices[vertex_index]];
            data.insert(data.end(), {vertex.x, vertex.y, vertex.z,
                                     vertex.nx, vertex.ny, vertex.nz,
                                     vertex.u, vertex.v});
        }
    }
    glGenVertexArrays(1, &car.vao);
    glGenBuffers(1, &car.vbo);
    glBindVertexArray(car.vao);
    glBindBuffer(GL_ARRAY_BUFFER, car.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

bool install_preview_mesh(const PreviewFile& file) {
    cars.clear();
    for (const PreviewMesh& mesh : file.meshes) {
        cars.emplace_back();
        cardata& car = cars.back();
        car.faces_count = static_cast<int>(mesh.indices.size() / 3);
        car.vertices_count = static_cast<int>(mesh.indices.size());
        for (std::size_t i = 0; i < mesh.indices.size(); ++i) {
            const std::size_t source = static_cast<std::size_t>(mesh.indices[i]) * 8;
            car_vertex& vertex = car.vertices[i];
            vertex.x = mesh.vertices[source + 0]; vertex.y = mesh.vertices[source + 1];
            vertex.z = mesh.vertices[source + 2]; vertex.nx = mesh.vertices[source + 3];
            vertex.ny = mesh.vertices[source + 4]; vertex.nz = mesh.vertices[source + 5];
            vertex.u = mesh.vertices[source + 6]; vertex.v = mesh.vertices[source + 7];
        }
        for (int face_index = 0; face_index < car.faces_count; ++face_index) {
            face& face_data = car.faces[face_index];
            face_data.vertices[0] = face_index * 3;
            face_data.vertices[1] = face_index * 3 + 1;
            face_data.vertices[2] = face_index * 3 + 2;
        }
        create_car_buffer(car);
        car.texture = load_texture_PNG(const_cast<char*>(mesh.albedo.c_str()));
        car.roughness = load_texture_PNG(const_cast<char*>(mesh.roughness.c_str()));
        std::printf("Preview mesh: %d triangles, VAO=%u, albedo=%u, roughness=%u\n",
                    car.faces_count, car.vao, car.texture, car.roughness);
    }
    std::printf("Camera: position=(%.2f, %.2f, %.2f), target=(%.2f, %.2f, %.2f), fov=%.2f\n",
                file.camera_position[0], file.camera_position[1], file.camera_position[2],
                file.camera_target[0], file.camera_target[1], file.camera_target[2], file.camera_fov);
    std::fflush(stdout);
    return true;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: vkdr2_preview path/to/preview.vkd2mesh\n");
        return 1;
    }
    if (SDL_Init(SDL_INIT_VIDEO) != 0 || (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    PreviewFile file;
    if (!load_preview_file(argv[1], file)) {
        std::fprintf(stderr, "Could not read preview mesh: %s\n", argv[1]);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    int width = file.render_width;
    int height = file.render_height;
    const float fit_scale = std::min(1.0f,
        std::min(1280.0f / static_cast<float>(width), 720.0f / static_cast<float>(height)));
    width = std::max(320, static_cast<int>(width * fit_scale));
    height = std::max(240, static_cast<int>(height * fit_scale));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_Window* window = SDL_CreateWindow(
        "VKDR2 Blender Preview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GLContext context = window ? SDL_GL_CreateContext(window) : nullptr;
    if (!window || !context) {
        std::fprintf(stderr, "OpenGL window creation failed: %s\n", SDL_GetError());
        if (context) SDL_GL_DeleteContext(context);
        if (window) SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);
    glDisable(GL_CULL_FACE);

    WIDTH = width;
    HEIGHT = height;
    x_pos = file.camera_position[0];
    y_pos = file.camera_position[1];
    z_pos = file.camera_position[2];
    VKDR2_setup(width, height);
    std::printf("OpenGL: %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::printf("Car shader program: %u\n", prog3DCar);
    std::fflush(stdout);
    if (!install_preview_mesh(file)) return 1;

    blender_external_camera = true;
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                width = std::max(320, event.window.data1);
                height = std::max(240, event.window.data2);
                WIDTH = width;
                HEIGHT = height;
            }
        }
        Mat4 view = m_lookAt(
            {file.camera_position[0], file.camera_position[1], file.camera_position[2]},
            {file.camera_target[0], file.camera_target[1], file.camera_target[2]},
            {0.0f, 1.0f, 0.0f});
        Mat4 proj = m_perspective(file.camera_fov, static_cast<float>(width) / height,
                                  file.camera_near, file.camera_far);
        VKDR2_render_blender(view, proj, width, height, 0);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
