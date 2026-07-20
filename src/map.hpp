#ifndef INCLUDES
#define INCLUDES
#include <SDL_opengl.h>
#include <SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <SDL.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "opengl_include.h"
#include "constants.h"
#include "general_functions.hpp"
#include "types.hpp"
#include "global_vars.hpp"
#include "renderer.hpp"
#endif

void rend_tex_face(unsigned int textur,point p1,point p2,point p3,point p4)
{
    glBindTexture(GL_TEXTURE_2D, textur);
    glBegin(GL_QUADS);
        glTexCoord2i(0, 1); glVertex3f(p1.x, p1.y, p1.z);
        glTexCoord2i(1, 1); glVertex3f(p2.x, p2.y, p2.z);
        glTexCoord2i(1, 0); glVertex3f(p3.x, p3.y, p3.z);
        glTexCoord2i(0, 0); glVertex3f(p4.x, p4.y, p4.z);

    glEnd();
}
void rend_face(int tex,point p1,point p2,point p3,point p4)
{
    rend_tex_face(textures[tex],p1,p2,p3,p4);
}
void rend_test(float x,float y,float z)
{
    glColor3f(0.0,0.0,0.0);
    glBegin(GL_QUADS);
    glVertex3f(x, y, z);
    glVertex3f(x+0.5, y, z);
    glVertex3f(x, y, z+0.5);
    glEnd();
}
unsigned int rend_white(){
	unsigned int white_list=glGenLists(1);
	glNewList(white_list,GL_COMPILE);

		p1.x=0;
		p1.y=0;
		p1.z=-0.1;

		p2.x=0.1;
		p2.y=0;
		p2.z=-0.1;

		p3.x=0.1;
		p3.y=0.5;
		p3.z=-0.1;

		p4.x=0;
		p4.y=0.5;
		p4.z=-0.1;

		rend_face(20,p1,p2,p3,p4);

	glEndList();
	return white_list;
}
void load_textures()
{
	printf("loading textures...\n");
    // obj_textures[0] = load_texture_PNG("assets/objects/textures/texture.png");
    obj_textures_count=1;
    if (selected_map==1){
        log("map 1");
        log("0");
        textures[0] = load_texture("assets/mountain1.bmp");
        log("1");
        textures[1] = load_texture("assets/asphalt.bmp");
        log("2");
        textures[2] = load_texture_PNG("assets/dry_grass.png");
        log("3");
        textures[3] = load_texture("assets/beton.bmp");
        log("4");
        textures[4] = load_texture_PNG("assets/sky2.png");
        log("5");
        textures[5] = load_texture_PNG("assets/mud.png");
        log("6");
        textures[6] = load_texture_PNG("assets/gravel.png");
    } else if (selected_map==0){
        log("map 0");
textures[0] =123;

        log("0");
        textures[0] = load_texture("assets/grid.bmp");
        log("1");
        textures[1] = load_texture("assets/asphalt.bmp");
        log("2");
        textures[2] = load_texture("assets/grass.bmp");
        log("3");
        textures[3] = load_texture("assets/beton.bmp");
        log("4");
        textures[4] = load_texture_PNG("assets/sky.png");
        log("5");
        textures[5] = load_texture_PNG("assets/mud.png");
        log("6");
        textures[6] = load_texture_PNG("assets/gravel.png");
    } else if (selected_map==2){
        log("map 2");
        log("0");
        textures[0] = load_texture_PNG("assets/grass.png");
        log("1");
        textures[1] = load_texture("assets/asphalt.bmp");
        log("2");
        textures[2] = load_texture_PNG("assets/rock.png");
        log("3");
        textures[3] = load_texture("assets/beton.bmp");
        log("4");
        textures[4] = load_texture_PNG("assets/sky.png");
        log("5");
        textures[5] = load_texture_PNG("assets/mud.png");
        log("6");
        textures[6] = load_texture_PNG("assets/gravel.png");
        textures[7] = load_texture_PNG("assets/white_line.png");
    }
    normal_fallback = load_texture_PNG("assets/normal.png");
    generic_texture = load_texture_PNG("assets/roughness.png");

}
mesh load_obj(const char* obj_path,char* texture_path)
{
    std::vector<ObjVertex> vertices;
    std::vector<ObjUV> uvs;
    std::vector<ObjNormal> normals;
    std::vector<ObjFace> faces;
    mesh m;
    m.texture_type=0;
    FILE* f = fopen(obj_path, "r");
    if (!f) {
        printf("[LOAD OBJ] failed to open: %s\n", obj_path);
        return m;
    }

    char line[512];

    while (fgets(line, sizeof(line), f)) {

        // ===== vertex =====
        if (strncmp(line, "v ", 2) == 0) {
            ObjVertex v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            m.vertices.push_back(v);
        }

        // ===== uv =====
        else if (strncmp(line, "vt ", 3) == 0) {
            ObjUV uv;
            sscanf(line, "vt %f %f", &uv.u, &uv.v);
            uv.v = 1.0f - uv.v; // OpenGL flip
            m.uvs.push_back(uv);
        }

        // ===== normal =====
        else if (strncmp(line, "vn ", 3) == 0) {
            ObjNormal n;
            sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            m.normals.push_back(n);
        }

        // ===== face =====
        else if (strncmp(line, "f ", 2) == 0) {

            ObjFace fce;
            for (int i=0;i<3;i++) {
                fce.v[i]  = -1;
                fce.vt[i] = -1;
                fce.vn[i] = -1;
            }

            int c = sscanf(line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &fce.v[0], &fce.vt[0], &fce.vn[0],
                &fce.v[1], &fce.vt[1], &fce.vn[1],
                &fce.v[2], &fce.vt[2], &fce.vn[2]);

            if (c < 9) {
                c = sscanf(line,
                    "f %d//%d %d//%d %d//%d",
                    &fce.v[0], &fce.vn[0],
                    &fce.v[1], &fce.vn[1],
                    &fce.v[2], &fce.vn[2]);
            }

            if (c < 6) {
                c = sscanf(line,
                    "f %d/%d %d/%d %d/%d",
                    &fce.v[0], &fce.vt[0],
                    &fce.v[1], &fce.vt[1],
                    &fce.v[2], &fce.vt[2]);
            }

            if (c < 3) {
                c = sscanf(line,
                    "f %d %d %d",
                    &fce.v[0],
                    &fce.v[1],
                    &fce.v[2]);
            }

            if (c >= 3) {
                for (int i=0;i<3;i++) {
                    fce.v[i]--;
                    if (fce.vt[i] >= 0) fce.vt[i]--;
                    if (fce.vn[i] >= 0) fce.vn[i]--;
                }
                m.faces.push_back(fce);
            }
        }
    }

    fclose(f);

    GLuint tex = 0;
    if (texture_path) {
        tex = load_texture_PNG(texture_path);
    }

    printf("[LOAD OBJ] loaded %s | v=%zu vt=%zu vn=%zu f=%zu\n",
        obj_path,
        m.vertices.size(),
        m.uvs.size(),
        m.normals.size(),
        m.faces.size()
    );

    if (tex) {
        obj_textures[obj_textures_count]=tex;
        m.texture=obj_textures_count;
        obj_textures_count++;
        printf("[LOAD OBJ] Succesfully loaded a texture\n");
    } else {
        m.texture=0;
        printf("[LOAD OBJ] Failed to load a texture\n");
    }
    return m;
}
void rotate_mesh(mesh* m, float rx, float ry, float rz)
{
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);

    for (auto &v : m->vertices)
    {
        float x = v.x;
        float y = v.y;
        float z = v.z;

        // Y rotace
        float x1 = x * cy + z * sy;
        float z1 = -x * sy + z * cy;
        x = x1;
        z = z1;

        // X rotace
        float y1 = y * cx - z * sx;
        float z2 = y * sx + z * cx;
        y = y1;
        z = z2;

        // Z rotace
        float x2 = x * cz - y * sz;
        float y2 = x * sz + y * cz;
        x = x2;
        y = y2;

        v.x = x;
        v.y = y;
        v.z = z;
    }
}
void add_center_lines_to_road(float width)
{
    printf("adding center lines to roads...\n");
    mesh m;
    m.texture = textures[7];
    m.x = m.y = m.z = 0.0f;

    for (size_t i = 0; i < roadparts.size(); i++)
    {
        auto &p = roadparts[i];
        auto &a = roadpoints[p.p1];
        auto &b = roadpoints[p.p2];

        // ===== direction (X/Z) =====
        float dx = b.y - a.y;
        float dz = b.x - a.x;

        float len = sqrtf(dx*dx + dz*dz);
        if (len < 1e-6f) continue;

        dx /= len;
        dz /= len;

        // right vector
        float rx = -dz;
        float rz = dx;

        // ===== points =====
        float ax = a.y;
        float az = a.x;

        float bx = b.y;
        float bz = b.x;

        float ay = a.h;
        float by = b.h;

        float h = 0.02f;

        // sample terrain height (center line)
        float ha = get_heightmap_height(ax, az) + h;
        float hb = get_heightmap_height(bx, bz) + h;

        // ===== quad =====
        ObjVertex v0 = { ax - rx*width, ha, az - rz*width };
        ObjVertex v1 = { bx - rx*width, hb, bz - rz*width };
        ObjVertex v2 = { bx + rx*width, hb, bz + rz*width };
        ObjVertex v3 = { ax + rx*width, ha, az + rz*width };

        // normal (simple up-ish, or recompute if needed)
        ObjNormal n = {0,1,0};

        int base = m.vertices.size();

        m.vertices.push_back(v0);
        m.vertices.push_back(v1);
        m.vertices.push_back(v2);
        m.vertices.push_back(v3);

        m.normals.push_back(n);
        m.normals.push_back(n);
        m.normals.push_back(n);
        m.normals.push_back(n);

        m.uvs.push_back({0,1});
        m.uvs.push_back({1,1});
        m.uvs.push_back({1,0});
        m.uvs.push_back({0,0});

        ObjFace f1;
        f1.v[0]=base+0;
        f1.v[1]=base+1;
        f1.v[2]=base+2;
        f1.vn[0]=base+0;
        f1.vn[1]=base+1;
        f1.vn[2]=base+2;
        f1.vt[0]=base+0;
        f1.vt[1]=base+1;
        f1.vt[2]=base+2;

        ObjFace f2;
        f2.v[0]=base+0;
        f2.v[1]=base+2;
        f2.v[2]=base+3;
        f2.vn[0]=base+0;
        f2.vn[1]=base+2;
        f2.vn[2]=base+3;
        f2.vt[0]=base+0;
        f2.vt[1]=base+2;
        f2.vt[2]=base+3;

        m.faces.push_back(f1);
        m.faces.push_back(f2);
    }

    static_objects.push_back(m);
}
void rend_map_terrain()
{
    printf("creating terrain meshes...\n");
    int levels=render_distance/terrain_lod_level_size;
    if (levels<1){levels=1;}
    int start_dist=0;
    int end_dist;
    int now_tex_pres=tex_pres;
    for (int level=0;level<levels;level++){
        end_dist=((render_distance/levels)*(level+1))*1.05f;
        start_dist=((render_distance/levels)*level)*0.95f;
        if (level!=0){
            now_tex_pres=((float)tex_pres*((float)(level+1)*lod_factor));
        }
        int aaa;
        int bbb;
        int start_i = (int)((float)(((int)z_pos/now_tex_pres)*now_tex_pres) - render_distance);
        int end_i   = (int)((float)(((int)z_pos/now_tex_pres)*now_tex_pres) + render_distance);
        int start_j = (int)((float)(((int)x_pos/now_tex_pres)*now_tex_pres) - render_distance);
        int end_j   = (int)((float)(((int)x_pos/now_tex_pres)*now_tex_pres) + render_distance);

        // if (start_i < 0) start_i = 0;
        // if (start_j < 0) start_j = 0;
        if (end_i > MAP_SIZE - now_tex_pres) end_i = MAP_SIZE - now_tex_pres;
        if (end_j > MAP_SIZE - now_tex_pres) end_j = MAP_SIZE - now_tex_pres;

        std::vector<intVec2> special_chunks;
        if (level == 0) {
            int chunk_start_z = std::max(0, start_i / CHUNK_SIZE);
            int chunk_end_z   = std::min(CHUNKS_SIZE - 1, end_i / CHUNK_SIZE);
            int chunk_start_x = std::max(0, start_j / CHUNK_SIZE);
            int chunk_end_x   = std::min(CHUNKS_SIZE - 1, end_j / CHUNK_SIZE);
            for (int cz = chunk_start_z; cz <= chunk_end_z; cz++) {
                for (int cx = chunk_start_x; cx <= chunk_end_x; cx++) {
                    int chunk_tex_pres = chunks[cz][cx]->tex_pres;
                    if (chunk_tex_pres > 0 && chunk_tex_pres != tex_pres) {
                        special_chunks.push_back({cx, cz});
                    }
                }
            }
        }

        for (k=0;k<10;k++){
            mesh m;
            m.texture=textures[k];
            m.x=0.0f;
            m.y=0.0f;
            m.z=0.0f;
            // // progress_v_levelu je od 0 do 1 podle toho, jak daleko je k
            // float k_progress = (float)k / 10.0f; 

            // // Celkový progress: (aktuální level + drobek z k) děleno celkovým počtem levelů
            // float total_progress = ((float)level + k_progress) / (float)levels;

            // render_progress_bar(total_progress, "Generating terrain mesh");
            // run_essencials();
            // render_progress_bar(total_progress, "Generating terrain mesh");
            // run_essencials();
            if (level == 0) {
                int chunk_start_z = std::max(0, start_i / CHUNK_SIZE);
                int chunk_end_z   = std::min(CHUNKS_SIZE - 1, (end_i - 1) / CHUNK_SIZE);
                int chunk_start_x = std::max(0, start_j / CHUNK_SIZE);
                int chunk_end_x   = std::min(CHUNKS_SIZE - 1, (end_j - 1) / CHUNK_SIZE);
                for (int cz = chunk_start_z; cz <= chunk_end_z; cz++) {
                    int chunk_z0 = cz * CHUNK_SIZE;
                    int chunk_z1 = chunk_z0 + CHUNK_SIZE;
                    int render_start_i = std::max(start_i, chunk_z0);
                    int render_end_i = std::min(end_i, chunk_z1);
                    for (int cx = chunk_start_x; cx <= chunk_end_x; cx++) {
                        int chunk_x0 = cx * CHUNK_SIZE;
                        int chunk_x1 = chunk_x0 + CHUNK_SIZE;
                        int render_start_j = std::max(start_j, chunk_x0);
                        int render_end_j = std::min(end_j, chunk_x1);
                        int chunk_tex_pres = chunks[cz][cx]->tex_pres;
                        if (chunk_tex_pres <= 0) chunk_tex_pres = tex_pres;
                        for (i = render_start_i; i < render_end_i; i += chunk_tex_pres) {
                            for (j = render_start_j; j < render_end_j; j += chunk_tex_pres) {
                                float diff_i = (float)i - z_pos;
                                float diff_j = (float)j - x_pos;
                                // Použijeme "Manhattanskou vzdálenost" pro čtvercové LODy (rychlejší než sqrt)
                                // nebo klasickou Pythagorovu větu pro kruhové LODy
                                float dist = sqrtf(diff_i * diff_i + diff_j * diff_j);
                                if (dist < (float)start_dist || dist >= (float)end_dist) {
                                    continue;
                                }
                                aaa = i;
                                bbb = j;
                                if (get_heightmap_texture(bbb, aaa) != k) {
                                    continue;
                                }
                                p1.x = j + chunk_tex_pres;
                                p1.y = get_heightmap_height((float)(bbb + chunk_tex_pres), (float)(aaa + chunk_tex_pres));
                                p1.z = i + chunk_tex_pres;

                                p2.x = j;
                                p2.y = get_heightmap_height((float)(bbb), (float)(aaa + chunk_tex_pres));
                                p2.z = i + chunk_tex_pres;

                                p3.x = j;
                                p3.y = get_heightmap_height((float)(bbb), (float)(aaa));
                                p3.z = i;

                                p4.x = j + chunk_tex_pres;
                                p4.y = get_heightmap_height((float)(bbb + chunk_tex_pres), (float)(aaa));
                                p4.z = i;

                                float v1x = p4.x - p1.x; float v1y = p4.y - p1.y; float v1z = p4.z - p1.z;
                                float v2x = p2.x - p1.x; float v2y = p2.y - p1.y; float v2z = p2.z - p1.z;
                                float nx = (v1y * v2z) - (v1z * v2y);
                                float ny = (v1z * v2x) - (v1x * v2z);
                                float nz = (v1x * v2y) - (v1y * v2x);
                                float delka = sqrtf(nx * nx + ny * ny + nz * nz);
                                if (delka > 0.0f) {
                                    nx /= delka; ny /= delka; nz /= delka;
                                }
                                m.normals.push_back({nx,ny,nz});
                                m.uvs.push_back({0.0f,1.0f});
                                m.uvs.push_back({1.0f,1.0f});
                                m.uvs.push_back({1.0f,0.0f});
                                m.uvs.push_back({0.0f,0.0f});
                                m.vertices.push_back({p1.x, p1.y, p1.z});
                                m.vertices.push_back({p2.x, p2.y, p2.z});
                                m.vertices.push_back({p3.x, p3.y, p3.z});
                                m.vertices.push_back({p4.x, p4.y, p4.z});
                                ObjFace f;
                                f.vn[0]=m.normals.size()-1;
                                f.vn[1]=m.normals.size()-1;
                                f.vn[2]=m.normals.size()-1;
                                f.vt[0]=m.uvs.size()-1;
                                f.vt[1]=m.uvs.size()-2;
                                f.vt[2]=m.uvs.size()-3;
                                f.v[0]=m.vertices.size()-1;
                                f.v[1]=m.vertices.size()-2;
                                f.v[2]=m.vertices.size()-3;
                                m.faces.push_back(f);
                                ObjFace f2;
                                f2.vn[0]=m.normals.size()-1;
                                f2.vn[1]=m.normals.size()-1;
                                f2.vn[2]=m.normals.size()-1;
                                f2.vt[0]=m.uvs.size()-1;
                                f2.vt[1]=m.uvs.size()-3;
                                f2.vt[2]=m.uvs.size()-4;
                                f2.v[0]=m.vertices.size()-1;
                                f2.v[1]=m.vertices.size()-3;
                                f2.v[2]=m.vertices.size()-4;
                                m.faces.push_back(f2);
                            }
                        }
                    }
                }
            } else {
                for (i = start_i; i < end_i; i += now_tex_pres) {
                    for (j = start_j; j < end_j; j += now_tex_pres) {
                        float diff_i = (float)i - z_pos;
                        float diff_j = (float)j - x_pos;
                        float dist = sqrtf(diff_i * diff_i + diff_j * diff_j);
                        if (dist < (float)start_dist || dist >= (float)end_dist) {
                            continue;
                        }
                        aaa = i;
                        bbb = j;
                        if (get_heightmap_texture(bbb, aaa) != k) {
                            continue;
                        }
                        p1.x = j + now_tex_pres;
                        p1.y = get_heightmap_height((float)(bbb + now_tex_pres), (float)(aaa + now_tex_pres));
                        p1.z = i + now_tex_pres;

                        p2.x = j;
                        p2.y = get_heightmap_height((float)(bbb), (float)(aaa + now_tex_pres));
                        p2.z = i + now_tex_pres;

                        p3.x = j;
                        p3.y = get_heightmap_height((float)(bbb), (float)(aaa));
                        p3.z = i;

                        p4.x = j + now_tex_pres;
                        p4.y = get_heightmap_height((float)(bbb + now_tex_pres), (float)(aaa));
                        p4.z = i;

                        float v1x = p4.x - p1.x; float v1y = p4.y - p1.y; float v1z = p4.z - p1.z;
                        float v2x = p2.x - p1.x; float v2y = p2.y - p1.y; float v2z = p2.z - p1.z;
                        float nx = (v1y * v2z) - (v1z * v2y);
                        float ny = (v1z * v2x) - (v1x * v2z);
                        float nz = (v1x * v2y) - (v1y * v2x);
                        float delka = sqrtf(nx * nx + ny * ny + nz * nz);
                        if (delka > 0.0f) {
                            nx /= delka; ny /= delka; nz /= delka;
                        }
                        m.normals.push_back({nx,ny,nz});
                        m.uvs.push_back({0.0f,1.0f});
                        m.uvs.push_back({1.0f,1.0f});
                        m.uvs.push_back({1.0f,0.0f});
                        m.uvs.push_back({0.0f,0.0f});
                        m.vertices.push_back({p1.x, p1.y, p1.z});
                        m.vertices.push_back({p2.x, p2.y, p2.z});
                        m.vertices.push_back({p3.x, p3.y, p3.z});
                        m.vertices.push_back({p4.x, p4.y, p4.z});
                        ObjFace f;
                        f.vn[0]=m.normals.size()-1;
                        f.vn[1]=m.normals.size()-1;
                        f.vn[2]=m.normals.size()-1;
                        f.vt[0]=m.uvs.size()-1;
                        f.vt[1]=m.uvs.size()-2;
                        f.vt[2]=m.uvs.size()-3;
                        f.v[0]=m.vertices.size()-1;
                        f.v[1]=m.vertices.size()-2;
                        f.v[2]=m.vertices.size()-3;
                        m.faces.push_back(f);
                        ObjFace f2;
                        f2.vn[0]=m.normals.size()-1;
                        f2.vn[1]=m.normals.size()-1;
                        f2.vn[2]=m.normals.size()-1;
                        f2.vt[0]=m.uvs.size()-1;
                        f2.vt[1]=m.uvs.size()-3;
                        f2.vt[2]=m.uvs.size()-4;
                        f2.v[0]=m.vertices.size()-1;
                        f2.v[1]=m.vertices.size()-3;
                        f2.v[2]=m.vertices.size()-4;
                        m.faces.push_back(f2);
                    }
                }
            }
            static_objects.push_back(m);
        }
    }
    add_center_lines_to_road(0.15f);
    return;
}
void rend_map_other(){
    float x_shift=0.0f;
    float z_shift=0.0f;


    printf("creating the other object mesh stuff \n");
        staticObjectBuffers.clear();

    // 1. Sesbíráme objekty podle textury
    std::map<int, std::vector<std::pair<mesh*, Vec3>>> objectsByTexture;

    int center_cx = (int)((x_pos+x_shift) / CHUNK_SIZE);
    int center_cz = (int)((z_pos+z_shift) / CHUNK_SIZE);
    int chunk_range = (render_distance / CHUNK_SIZE)+2;

    for (int cz = center_cz - chunk_range; cz <= center_cz + chunk_range; cz++) {
        for (int cx = center_cx - chunk_range; cx <= center_cx + chunk_range; cx++) {
            if (cx < 0 || cz < 0 || cx >= CHUNKS_SIZE || cz >= CHUNKS_SIZE) continue;
            chunk* target = chunks[cz][cx];
            if (!target) continue;
            for (int i = 0; i < target->objects.size(); i++) {


// if (target->objects.size() != target->object_poses.size()) {
//     printf("DESYNC CHUNK OBJECTS AND POSES %zu %zu\n",
//         target->objects.size(),
//         target->object_poses.size());
//     SDL_Delay(100);
//     return;
// }
// int idx = target->objects[i];

// if (idx < 0 || idx >= (int)objects.size()) {
//     printf("BAD OBJECT INDEX %d\n", idx);
//     SDL_Delay(100);
//     return;
// }
// if (target->magic != 0xDEADBEEF) {
//     printf("CORRUPTED CHUNK, MAGIC ISNT OKAY\n");
//     return;
// }





                float final_x = target->object_poses[i].x - (float)x_shift;
                float final_y = target->object_poses[i].y;
                float final_z = target->object_poses[i].z - (float)z_shift;
                mesh m;
                if (((target->object_poses[i].x - x_pos) * (target->object_poses[i].x - x_pos) + (target->object_poses[i].z - z_pos) * (target->object_poses[i].z - z_pos)) < (other_lod_level_size * other_lod_level_size)){
                    m = objects[target->objects[i]];
                } else {
                    m = lqobjects[target->objects[i]];
                }
                GLuint texID;
                if (m.texture_type == 1) {
                    texID = textures[m.texture];
                } else {
                    texID = obj_textures[m.texture];
                }
                m.texture=texID;
                m.x=target->object_poses[i].x-x_shift;
                m.y=target->object_poses[i].y;
                m.z=target->object_poses[i].z-z_shift;

                static_objects.push_back(m);
            }
        }
    }
    return;
}
void update_static_objects(){
    printf("updating static buffers\n");
    frames_from_reload=0;
    last_fps=fps;
    static_objects.clear();
    rend_map_terrain();
    rend_map_other();
    makeStaticBuffers(static_objects);
}
void add_object(mesh m){
    objects.push_back(m);
    lqobjects.push_back(m);
}
void add_object(mesh m, mesh m_lq){
    objects.push_back(m);
    lqobjects.push_back(m_lq);
}
void load_objects(){
    printf("loading objects\n");
    objects.push_back(load_obj("assets/objects/tree.obj", "assets/objects/textures/tree.png"));
    objects.push_back(load_obj("assets/objects/chaloupka.obj", "assets/objects/textures/chaloupka.png"));
    lqobjects.push_back(load_obj("assets/objects/tree_lowpoly.obj", "assets/objects/textures/tree_lowpoly.png"));
    lqobjects.push_back(load_obj("assets/objects/chaloupka.obj", "assets/objects/textures/chaloupka.png"));
    printf("loaded objects succesfully\n");
}
void add_object_to_world(float x, float y, float z, int object_id) {
    // 1. Převod souřadnic na indexy chunků
    int cx = (int)(x / CHUNK_SIZE);
    int cz = (int)(z / CHUNK_SIZE);

    // 2. Kontrola mezí pole (aby ti to nespadlo)
    if (cx < 0 || cx >= CHUNKS_SIZE || cz < 0 || cz >= CHUNKS_SIZE) {
        return;
    }

    // 3. Získání pointeru na chunk
    chunk* target = chunks[cz][cx];

    // 4. KONTROLA: Existuje ten chunk vůbec? (u pointerů nutné!)
    if (target == NULL) {
        // Tady bys ho mohl případně vytvořit, pokud bys chtěl
        return; 
    }

    // 5. Máme místo v poli [500]?
    if (target->objects.size() < 500) {
        int idx = target->objects.size();

        // Uložení ID a pozice (používáme -> protože target je pointer)
        target->objects.push_back(object_id);
        target->object_poses.push_back({x, y, z});
    }
}
void add_collision_box_to_world(collision_box box) {
    // 1. Vypočítáme střed pro určení chunku (podle start a end souřadnic)
    float mid_x = (box.sx + box.ex) / 2.0f;
    float mid_z = (box.sz + box.ez) / 2.0f;

    // 2. Indexy chunku
    int cx = (int)(mid_x / CHUNK_SIZE);
    int cz = (int)(mid_z / CHUNK_SIZE);

    // 3. Kontrola mezí
    if (cx < 0 || cz < 0 || cx >= CHUNKS_SIZE || cz >= CHUNKS_SIZE) {
        return;
    }

    // 4. Získání pointeru
    chunk* target = chunks[cz][cx];
    if (target == NULL) return;

    // 5. Uložení celého boxu
    if (target->collision_boxes_count < 500) {
        // V C můžeš struktury kopírovat takhle jednoduše přiřazením
        target->collision_boxes[target->collision_boxes_count] = box;
        target->collision_boxes_count++;
    } else {
        printf("VAROVANI: Chunk [%d, %d] je plny (500 boxu)!\n", cx, cz);
    }
}
void create_cube(float x, float y, float z, float scx, float scy, float scz, float rx, float ry, float rz, int tex_id){
    rx/=RAD_DEG;
    ry/=RAD_DEG;
    rz/=RAD_DEG;
    mesh m;
    m.texture=tex_id;
    m.texture_type=1;
    float hx = scx * 0.5f;
    float hy = scy * 0.5f;
    float hz = scz * 0.5f;


    m.vertices.clear();
    m.uvs.clear();
    m.normals.clear();
    m.faces.clear();

    // ===== NORMALS (6 stěn) =====
    ObjNormal norms[6] = {
        { 0, 0, 1}, { 0, 0,-1},
        { 0, 1, 0}, { 0,-1, 0},
        { 1, 0, 0}, {-1, 0, 0}
    };
    for(int i=0;i<6;i++){
        ObjNormal n = norms[i];
        n = rotateNormal(n, rx, ry, rz);
        m.normals.push_back(n);
    }
    // ===== VERTICES (24, 4 na stěnu) =====
    ObjVertex verts[] = {
        {-hx,-hy, hz}, { hx,-hy, hz}, { hx, hy, hz}, {-hx, hy, hz}, // front
        {-hx,-hy,-hz}, {-hx, hy,-hz}, { hx, hy,-hz}, { hx,-hy,-hz}, // back
        {-hx, hy,-hz}, {-hx, hy, hz}, { hx, hy, hz}, { hx, hy,-hz}, // top
        {-hx,-hy,-hz}, { hx,-hy,-hz}, { hx,-hy, hz}, {-hx,-hy, hz}, // bottom
        { hx,-hy,-hz}, { hx, hy,-hz}, { hx, hy, hz}, { hx,-hy, hz}, // right
        {-hx,-hy,-hz}, {-hx,-hy, hz}, {-hx, hy, hz}, {-hx, hy,-hz}  // left
    };

    // ===== ROTACE VRCHOLŮ =====
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);

    for(int i=0;i<24;i++){
        float x = verts[i].x;
        float y = verts[i].y;
        float z = verts[i].z;

        // rotace Y
        float x1 = x*cy + z*sy;
        float z1 = -x*sy + z*cy;
        x = x1; z = z1;
        // rotace X
        float y1 = y*cx - z*sx;
        float z2 = y*sx + z*cx;
        y = y1; z = z2;
        // rotace Z
        float x2 = x*cz - y*sz;
        float y2 = x*sz + y*cz;
        x = x2; y = y2;

        verts[i].x = x;
        verts[i].y = y;
        verts[i].z = z;
    }

    for(int i=0;i<24;i++) m.vertices.push_back(verts[i]);

    // ===== UV 0..1 pro každý face (4 vrcholy) =====
    ObjUV faceUV[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
    for(int f=0; f<6; f++){
        for(int i=0;i<4;i++) m.uvs.push_back(faceUV[i]);
    }

    // ===== FACES (2 trojúhelníky na stěnu) =====
    int faceVerts[6][4] = {
        {0,1,2,3}, {4,5,6,7}, {8,9,10,11},
        {12,13,14,15}, {16,17,18,19}, {20,21,22,23}
    };

    for(int f=0; f<6; f++){
        ObjFace t1, t2;

        // trojúhelník 1
        t1.v[0]=faceVerts[f][0]; t1.v[1]=faceVerts[f][1]; t1.v[2]=faceVerts[f][2];
        // trojúhelník 2
        t2.v[0]=faceVerts[f][0]; t2.v[1]=faceVerts[f][2]; t2.v[2]=faceVerts[f][3];

        // UV indexy
        t1.vt[0]=f*4+0; t1.vt[1]=f*4+1; t1.vt[2]=f*4+2;
        t2.vt[0]=f*4+0; t2.vt[1]=f*4+2; t2.vt[2]=f*4+3;

        // normály
        t1.vn[0]=t1.vn[1]=t1.vn[2]=f;
        t2.vn[0]=t2.vn[1]=t2.vn[2]=f;

        m.faces.push_back(t1);
        m.faces.push_back(t2);
    }







    // 2. VYTVOŘENÍ COLLISION BOXU
    collision_box box={};
    // Převedeme střed a scale na sx/ex formát
    box.sx = x - (scx * 0.5f);
    box.sy = y - (scy * 0.5f);
    box.sz = z - (scz * 0.5f);
    box.ex = x + (scx * 0.5f);
    box.ey = y + (scy * 0.5f);
    box.ez = z + (scz * 0.5f);
    
    // Rotace
    box.rx = rx;
    box.ry = ry;
    box.rz = rz;
    
    // friction
    box.friction=0.7f;
    add_collision_box_to_world(box);
    add_object(m);
    add_object_to_world(x,y,z,objects.size()-1);
    collision_boxes_count++;
    return;
}
void create_cube(float x, float y, float z, float scx, float scy, float scz, float rx, float ry, float rz, int tex_id, float friction){
    rx/=RAD_DEG;
    ry/=RAD_DEG;
    rz/=RAD_DEG;
    mesh m;
    m.texture=tex_id;
    m.texture_type=1;
    float hx = scx * 0.5f;
    float hy = scy * 0.5f;
    float hz = scz * 0.5f;



    m.vertices.clear();
    m.uvs.clear();
    m.normals.clear();
    m.faces.clear();

    // ===== NORMALS (6 stěn) =====
    ObjNormal norms[6] = {
        { 0, 0, 1}, { 0, 0,-1},
        { 0, 1, 0}, { 0,-1, 0},
        { 1, 0, 0}, {-1, 0, 0}
    };
    for(int i=0;i<6;i++){
        ObjNormal n = norms[i];
        n = rotateNormal(n, rx, ry, rz);
        m.normals.push_back(n);
    }
    // ===== VERTICES (24, 4 na stěnu) =====
    ObjVertex verts[] = {
        {-hx,-hy, hz}, { hx,-hy, hz}, { hx, hy, hz}, {-hx, hy, hz}, // front
        {-hx,-hy,-hz}, {-hx, hy,-hz}, { hx, hy,-hz}, { hx,-hy,-hz}, // back
        {-hx, hy,-hz}, {-hx, hy, hz}, { hx, hy, hz}, { hx, hy,-hz}, // top
        {-hx,-hy,-hz}, { hx,-hy,-hz}, { hx,-hy, hz}, {-hx,-hy, hz}, // bottom
        { hx,-hy,-hz}, { hx, hy,-hz}, { hx, hy, hz}, { hx,-hy, hz}, // right
        {-hx,-hy,-hz}, {-hx,-hy, hz}, {-hx, hy, hz}, {-hx, hy,-hz}  // left
    };

    // ===== ROTACE VRCHOLŮ =====
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);

    for(int i=0;i<24;i++){
        float x = verts[i].x;
        float y = verts[i].y;
        float z = verts[i].z;

        // rotace Y
        float x1 = x*cy + z*sy;
        float z1 = -x*sy + z*cy;
        x = x1; z = z1;
        // rotace X
        float y1 = y*cx - z*sx;
        float z2 = y*sx + z*cx;
        y = y1; z = z2;
        // rotace Z
        float x2 = x*cz - y*sz;
        float y2 = x*sz + y*cz;
        x = x2; y = y2;

        verts[i].x = x;
        verts[i].y = y;
        verts[i].z = z;
    }

    for(int i=0;i<24;i++) m.vertices.push_back(verts[i]);

    // ===== UV 0..1 pro každý face (4 vrcholy) =====
    ObjUV faceUV[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
    for(int f=0; f<6; f++){
        for(int i=0;i<4;i++) m.uvs.push_back(faceUV[i]);
    }

    // ===== FACES (2 trojúhelníky na stěnu) =====
    int faceVerts[6][4] = {
        {0,1,2,3}, {4,5,6,7}, {8,9,10,11},
        {12,13,14,15}, {16,17,18,19}, {20,21,22,23}
    };

    for(int f=0; f<6; f++){
        ObjFace t1, t2;

        // trojúhelník 1
        t1.v[0]=faceVerts[f][0]; t1.v[1]=faceVerts[f][1]; t1.v[2]=faceVerts[f][2];
        // trojúhelník 2
        t2.v[0]=faceVerts[f][0]; t2.v[1]=faceVerts[f][2]; t2.v[2]=faceVerts[f][3];

        // UV indexy
        t1.vt[0]=f*4+0; t1.vt[1]=f*4+1; t1.vt[2]=f*4+2;
        t2.vt[0]=f*4+0; t2.vt[1]=f*4+2; t2.vt[2]=f*4+3;

        // normály
        t1.vn[0]=t1.vn[1]=t1.vn[2]=f;
        t2.vn[0]=t2.vn[1]=t2.vn[2]=f;

        m.faces.push_back(t1);
        m.faces.push_back(t2);
    }








    // 2. VYTVOŘENÍ COLLISION BOXU
    collision_box box={};
    // Převedeme střed a scale na sx/ex formát
    box.sx = x - (scx * 0.5f);
    box.sy = y - (scy * 0.5f);
    box.sz = z - (scz * 0.5f);
    box.ex = x + (scx * 0.5f);
    box.ey = y + (scy * 0.5f);
    box.ez = z + (scz * 0.5f);
    
    // Rotace
    box.rx = rx;
    box.ry = ry;
    box.rz = rz;

    // friction
    box.friction=friction;

    add_collision_box_to_world(box);
   add_object(m);
    add_object_to_world(x,y,z,objects.size()-1);
    collision_boxes_count++;
    return;
}
void create_ramp(float x, float y, float z, int steps, float width, float thickness, float length, float angle) {
    float xp=0.0f;
    float yp=thickness/-2.0f;
    float ang=(angle/((float)steps))/2.0f;
    float ang2=0.0f;
    for (i=0;i<steps;i++){
        create_cube(x+xp,y+yp,z,length,thickness,width,0.0f,0.0f,ang,3);
        ang+=angle/(float)steps-1.0f;
        ang2+=angle/(float)steps-1.0f;
        xp+=cosf(ang2/RAD_DEG)*length;
        yp+=sinf(ang2/RAD_DEG)*length;
    }
}
void create_ramp_z(float x, float y, float z, int steps, float width, float thickness, float length, float angle, float Yrot, float friction, float startangle) {
    Yrot-=90.0f;
    float xp = 0.0f;
    float zp = 0.0f;
    float yp = thickness / -2.0f; // Počáteční offset výšky
    // Úhel pro rotaci kostky kolem Z (stoupání)
    float angZ = (angle / (float)steps) / 2.0f;
    // Úhel pro směr posunu (kumulativní)
    float pathAngleZ = startangle;

    for (int i = 0; i < steps; i++) {
        // Vypočítáme aktuální střed kostky ve světě
        // Musíme promítnout horizontální posun (xp) do os X a Z podle Yrot
        float worldX = x + xp * cosf(Yrot / RAD_DEG);
        float worldZ = z - xp * sinf(Yrot / RAD_DEG); // Mínus pro standardní rotaci
        float worldY = y + yp;

        // Vytvoříme kostku s rotací Yrot (kam rampa míří) a angZ (jak moc stoupá)
        if (Yrot==90.0f || Yrot==-270.0f){
            create_cube(worldX, worldY, worldZ, width, thickness, length, angZ, Yrot+90.0f, 0.0f, 3, friction);
        } else {
            create_cube(worldX, worldY, worldZ, width, thickness, length, -angZ, Yrot+90.0f, 0.0f, 3, friction);
        }


        // V create_ramp_z změň volání create_cube na:
        // Místo: create_cube(..., -angZ, Yrot+90.0f, 0.0f, ...)
        // create_cube(worldX, worldY, worldZ, width, thickness, length, 0.0f, Yrot + 90.0f, angZ, 3, friction);

        
        // Aktualizujeme úhly pro další krok
        float stepDiff = angle / (float)steps;
        angZ += stepDiff;
        pathAngleZ += stepDiff;

        // Výpočet horizontálního a vertikálního kroku
        float horizontalStep = cosf(pathAngleZ / RAD_DEG) * length;
        float verticalStep = sinf(pathAngleZ / RAD_DEG) * length;

        // xp zde funguje jako "vzdálenost od startu na mapě"
        xp += horizontalStep;
        yp += verticalStep;
    }
}
void create_ramp_x(float x, float y, float z, int steps, float width, float thickness, float length, float angle, float Yrot, float friction, float startangle) {
    float xp = 0.0f;
    float zp = 0.0f;
    float yp = thickness / -2.0f; // Počáteční offset výšky
    Yrot-=90.0f;
    // Úhel pro rotaci kostky kolem Z (stoupání)
    float angZ = (angle / (float)steps) / 2.0f;
    // Úhel pro směr posunu (kumulativní)
    float pathAngleZ = startangle;

    for (int i = 0; i < steps; i++) {
        // Vypočítáme aktuální střed kostky ve světě
        // Musíme promítnout horizontální posun (xp) do os X a Z podle Yrot
        float worldX = x + xp * cosf(Yrot / RAD_DEG);
        float worldZ = z - xp * sinf(Yrot / RAD_DEG); // Mínus pro standardní rotaci
        float worldY = y + yp;

        // Vytvoříme kostku s rotací Yrot (kam rampa míří) a angZ (jak moc stoupá)
        create_cube(worldX, worldY, worldZ, length, thickness, width, 0.0f, Yrot, angZ, 3, friction);

        // Aktualizujeme úhly pro další krok
        float stepDiff = angle / (float)steps;
        angZ += stepDiff;
        pathAngleZ += stepDiff;

        // Výpočet horizontálního a vertikálního kroku
        float horizontalStep = cosf(pathAngleZ / RAD_DEG) * length;
        float verticalStep = sinf(pathAngleZ / RAD_DEG) * length;

        // xp zde funguje jako "vzdálenost od startu na mapě"
        xp += horizontalStep;
        yp += verticalStep;
    }
}
void create_ramp(float x, float y, float z, int steps, float width, float thickness, float length, float angle, float Yrot, float friction){
    if (Yrot==0.0f || abs(Yrot)==180.0f){
        create_ramp_z(x,  y,  z,  steps,  width,  thickness,  length,  angle,  Yrot,  friction, 0.0f);
    } else {
        create_ramp_x(x,  y,  z,  steps,  width,  thickness,  length,  angle,  Yrot,  friction, 0.0f);
    }
}
void create_ramp(float x, float y, float z, int steps, float width, float thickness, float length, float angle, float Yrot, float friction, float startangle){
    if (Yrot==0.0f || abs(Yrot)==180.0f){
        create_ramp_z(x,  y,  z,  steps,  width,  thickness,  length,  angle,  Yrot,  friction, startangle);
    } else {
        create_ramp_x(x,  y,  z,  steps,  width,  thickness,  length,  angle,  Yrot,  friction, startangle);
    }
}
void create_loop(float x, float y, float z, int steps, float width, float thickness, float length, float friction) {
    float xp=0.0f;
    float yp=thickness/-2.0f;
    float zp=0.0f;
    float ang=(360.0f/((float)steps))/2.0f;
    float ang2=0.0f;
    for (i=0;i<steps;i++){
        create_cube(x+xp,y+yp,z+zp,length,thickness,width,0.0f,0.0f,ang,3,friction);
        ang+=360.0f/(float)steps;
        ang2+=360.0f/(float)steps;
        xp+=cosf(ang2/RAD_DEG)*length;
        yp+=sinf(ang2/RAD_DEG)*length;
        zp+=width/(float)steps;
    }
}
void create_loop_v2(float x, float y, float z, int steps, float width, float thickness, float radius) {
    float stepAngle = 360.0f / (float)steps;
    float segmentLength = 2.0f * radius * sinf((180.0f / (float)steps) / RAD_DEG);

    // Starting at 270 degrees puts the first segment at the very bottom
    for (int i = 0; i < steps; i++) {
        float currentAngleDeg = 270.0f + (i * stepAngle);
        float currentAngleRad = currentAngleDeg / RAD_DEG;

        // X and Z stay similar, but Y is shifted by +radius so the bottom is at 'y'
        float cx = x + cosf(currentAngleRad) * radius;
        float cy = y + radius + sinf(currentAngleRad) * radius; 
        
        // Z moves forward to create space so you don't hit the back of the loop
        float cz = z + (i * (width / (float)steps));

        // Rotation: For RZ to follow the circle perfectly
        float rz = currentAngleDeg + 90.0f;

        create_cube(cx, cy, cz, segmentLength, thickness, width, 0.0f, 0.0f, rz, 3);
    }
}
void gen_forest(float cx, float cz, float radius, int density = 50) {
    int num_attempts = (int)(3.14f * radius * radius * density / 100.0f); // přibližně počet stromů

    for(int t = 0; t < num_attempts; t++) {
        // náhodná pozice stromu uvnitř čtverce okolo kruhu
        float j = cx - radius + (float)randint(0, (int)(2*radius));
        float k = cz - radius + (float)randint(0, (int)(2*radius));

        // zkontrolovat, jestli je uvnitř kruhu
        float dx = j - cx;
        float dz = k - cz;
        if (dx*dx + dz*dz > radius*radius)
            continue; // přeskočit, není uvnitř kruhu

        float tree_h = mapgen_get_heightmap_height(j, k);
        if (tree_h>300.0f){continue;} // moc vysoko, žádný strom

        // vytvoření kolizního boxu
        collision_box box;
        box.sx = j - 0.4f;
        box.sy = tree_h;
        box.sz = k - 0.4f;
        box.ex = j + 0.4f;
        box.ey = tree_h + 2.0f;
        box.ez = k + 0.4f;
        add_collision_box_to_world(box);

        // přidání vizuálního objektu stromu
        add_object_to_world(j, tree_h, k, 0);
    }
}
void add_retarder(float x, float y, float z, float roty){
    #define RETARDER_FFAT 0.35f
    #define RETARDER_FAT 0.28f
    #define RETARDER_MFAT 0.20f
    #define RETARDER_SFAT 0.12f
    #define RETARDER_LENGTH 6.0f
    collision_box box={};
    box.friction=0.5f;
    box.ry=roty/RAD_DEG;
    box.sx=x-(RETARDER_LENGTH/2.0f);
    box.sy=y-0.1f;
    box.sz=z-(RETARDER_FFAT);
    box.ex=x+(RETARDER_LENGTH/2.0f);
    box.ey=y+0.03f;
    box.ez=z+(RETARDER_FFAT);
    add_collision_box_to_world(box);
    box.sx=x-(RETARDER_LENGTH/2.0f);
    box.sy=y;
    box.sz=z-(RETARDER_FAT);
    box.ex=x+(RETARDER_LENGTH/2.0f);
    box.ey=y+0.06f;
    box.ez=z+(RETARDER_FAT);
    add_collision_box_to_world(box);
    box.sx=x-(RETARDER_LENGTH/2.0f);
    box.sy=y;
    box.sz=z-(RETARDER_MFAT);
    box.ex=x+(RETARDER_LENGTH/2.0f);
    box.ey=y+0.09f;
    box.ez=z+(RETARDER_MFAT);
    add_collision_box_to_world(box);
    box.sx=x-(RETARDER_LENGTH/2.0f);
    box.sy=y;
    box.sz=z-(RETARDER_SFAT);
    box.ex=x+(RETARDER_LENGTH/2.0f);
    box.ey=y+0.12f;
    box.ez=z+(RETARDER_SFAT);
    add_collision_box_to_world(box);

    mesh m=load_obj("assets/objects/retarder.obj", "assets/objects/textures/retarder.png");
    rotate_mesh(&m, 0.0f, roty/RAD_DEG, 0.0f);
    add_object(m);
    add_object_to_world(x, y, z, objects.size() - 1);


    // create_cube(x+1,y+1,z+1,10.0f,10.0f,10.0f,0.0f,0.0f,0.0f,2,0.5); // was used for debug
}
void add_stopsign(float x, float y, float z, float roty){
    collision_box box;
    #define SIGN_FAT 0.6f
    roty+=180.0f;
    box.friction=0.9f;
    box.ry=roty/RAD_DEG;
    box.sx=x-(SIGN_FAT/2.0f);
    box.sy=y-0.15f;
    box.sz=z-(SIGN_FAT/2.0f);
    box.ex=x+(SIGN_FAT/2.0f);
    box.ey=y+2.20f;
    box.ez=z+(SIGN_FAT/2.0f);
    add_collision_box_to_world(box);
    
    mesh m=load_obj("assets/objects/stop.obj", "assets/objects/textures/stop.png");
    rotate_mesh(&m, 0.0f, roty/RAD_DEG, 0.0f);
   add_object(m);
    add_object_to_world(x, y, z, objects.size() - 1);
}
void add_retarders_to_road(float chance)
{
    float RAD2DEG = 180.0f / 3.1415926f;

    for (size_t i = 0; i < roadparts.size(); i++)
    {
        if ((float)rand() / (float)RAND_MAX > chance)
            continue;

        auto &p = roadparts[i];

        auto &a = roadpoints[p.p1];
        auto &b = roadpoints[p.p2];

        // ===== midpoint =====
        float mx = (a.y + b.y) * 0.5f;
        float my = (a.h + b.h) * 0.5f;
        float mz = (a.x + b.x) * 0.5f;

        // ===== direction =====
        float dx = b.y - a.y;
        float dz = b.x - a.x;

        float len = sqrtf(dx*dx + dz*dz);
        if (len < 1e-6f) continue;

        dx /= len;
        dz /= len;

        // ===== yaw (rotation around Y) =====
        float yaw = atan2f(dx, dz); // nebo atan2(z,x) podle os

        // ===== small random offset (optional goofy vibe) =====
        float offset = ((rand() % 100) / 100.0f - 0.5f) * 0.5f;

        mx += dx * offset;
        mz += dz * offset;

        // ===== spawn =====
        add_retarder(mx, my, mz, yaw * RAD2DEG);
        printf("Added retarder at (%.2f, %.2f, %.2f) with yaw %.2f\n", mx, my, mz, yaw * RAD2DEG);
    }
}
void gen_map_other()
{
    printf("generating other...\n");
    if (selected_map!=0){
        printf("generating trees...\n");
        // for (i=0;i<(int)((float)((MAP_SIZE)*(MAP_SIZE))/100000.0f);i++){
        //     j=randint(0, MAP_SIZE);
        //     k=randint(0, MAP_SIZE);
        //     tree_h=get_heightmap_height((float)j,(float)k);
        //     collision_box box;
        //     box.sx=(float)j-0.4f;
        //     box.sy=tree_h;
        //     box.sz=(float)k-0.4f;
        //     box.ex=(float)j+0.4f;
        //     box.ey=tree_h+2.0f;
        //     box.ez=(float)k+0.4f;
        //     add_collision_box_to_world(box);
        //     if (j>TREE_QUALITY_DISTANCE || k>TREE_QUALITY_DISTANCE){
        //         add_object_to_world((float)j, tree_h, (float)k, 1);
        //     } else {
        //         add_object_to_world((float)j, tree_h, (float)k, 1);
        //     }
        // }
        // for (i=0;i<(int)((float)((MAP_SIZE)*(MAP_SIZE))/1000000.0f);i++){
        for (int fr=0;fr<50;fr++){
            progress_bar((float)fr/20.0f, "Generating forests...");
            j=randint(0, MAP_SIZE);
            k=randint(0, MAP_SIZE); 
            int forest_size=100; 
            printf("made forest at %d, %d\n", j,k);
            for (l=0;l<2;l++){
                j=randint(j-(int)((float)forest_size*0.6f), j+(int)((float)forest_size*0.6f));
                k=randint(k-(int)((float)forest_size*0.6f), k+(int)((float)forest_size*0.6f));
                gen_forest(j,k,forest_size,5);

            }
        }
        printf("forests done\n");
        collision_boxes[0].sx=10.0f;
        collision_boxes[0].sy=0.0f;
        collision_boxes[0].sz=10.0f;
        collision_boxes[0].ex=11.0f;
        collision_boxes[0].ey=10.0f;
        collision_boxes[0].ez=11.0f;


    } else {
        printf("generating trees...\n");
        collision_boxes_count=0;
        for (i=0;i<80;i++){
            j=randint(50, 100);
            k=randint(50, 100);
            tree_h=mapgen_get_heightmap_height((float)j,(float)k);
            collision_box box={};
            box.sx=(float)j-0.4f;
            box.sy=tree_h;
            box.sz=(float)k-0.4f;
            box.ex=(float)j+0.4f;
            box.ey=tree_h+2.0f;
            box.ez=(float)k+0.4f;
            add_collision_box_to_world(box);
            collision_boxes_count++;
            if (j>TREE_QUALITY_DISTANCE || k>TREE_QUALITY_DISTANCE){
                add_object_to_world((float)j, tree_h, (float)k, 0);
            } else {
                add_object_to_world((float)j, tree_h, (float)k, 0);
            }
        }


        // the wall at the begining of the map
        create_cube(100.0f, 1.0f, 0.5f, 200.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 0.9f);

        // the goofyahh cube at the beggining of the map
        create_cube(5,1,5,2,2,2,0,1.0f,2.0f,1);

        create_cube(10,-0.4,50, 5,1,3, -5.0f,0.0f,0.0f,1);
        create_cube(10,0.11,53, 5,1,3, -15.0f,0.0f,0.0f,1);
        create_cube(10,1.15,55.85, 5,1,3, -25.0f,0.0f,0.0f,1);
        create_ramp(30.0f, 0.0f, 50.0f, 3, 10.0f, 1.0f, 4.0f, 30.0f);
        create_loop(50.0f, 0.0f, 10.0f, 30, 10.0f, 2.0f, 3.0f, 0.7f);
        create_cube(111.2f,0.25f,228.0f,  0.5f,2.0f,0.5f,  0.0f,0.0f,0.0f,  1);
        for (i=0;i<10;i++){
            float offy=33.3f;
            float offz=66.6f;
            create_cube(110.0f+(float)i*3.0f,50.5f+offy,120.0f-offz,  0.1f,1.0f,74.5f, 26.57f, 0.0f, 0.0f, 2, 0.5f);
            create_cube(110.0f+(float)i*3.0f,50.5f,120.0f,            0.1f,1.0f,74.5f, 26.57f, 0.0f, 0.0f, 2, 0.5f);
            create_cube(110.0f+(float)i*3.0f,50.5f-offy,120.0f+offz,  0.1f,1.0f,74.5f, 26.57f, 0.0f, 0.0f, 2, 0.5f);
        }


        create_ramp(130.0f,0.0f,224.0f, 10, 40.0f, 0.2f, 1.0f,26.5, 180.0f, 0.5f);
        create_ramp(115.0f,0.0f,229.0f, 28, 4.0f, 0.2f, 1.5f,80.0f, 0.0f, 0.05f);
        create_cube(117.0f,0.25f,228.0f,  0.5f,2.0f,0.5f,  0.0f,0.0f,0.0f,  1);
        create_cube(120.0f,1.0f,232.0f,  3.0f,0.1f,13.0f,  5.0f,0.0f,0.0f,  1, 0.5f);
        create_cube(123.5f,1.0f,227.0f,  3.0f,4.0f,0.5f,  0.0f,0.0f,0.0f,  1, 0.1f);
        create_ramp(128.0f, 0.0f, 230.0f, 3, 1.5f, 1.0f, 3.0f, 30.0f, 0.0f, 0.0f);
        // those ramps nex to each other getting bigger
        create_ramp(45.0f, 0.0f, 100.0f, 13, 5.0f, 1.0f, 2.5f, 144.0f, 0.0f, 0.0f);
        create_ramp(40.0f, 0.0f, 100.0f, 12, 5.0f, 1.0f, 2.4f, 127.0f, 0.0f, 0.0f);
        create_ramp(35.0f, 0.0f, 100.0f, 11, 5.0f, 1.0f, 2.3f, 110.0f, 0.0f, 0.0f);
        create_ramp(30.0f, 0.0f, 100.0f, 10, 5.0f, 1.0f, 2.2f, 93.0f, 0.0f, 0.0f);
        create_ramp(25.0f, 0.0f, 100.0f,  9, 5.0f, 1.0f, 2.1f, 76.0f, 0.0f, 0.0f);
        create_ramp(20.0f, 0.0f, 100.0f,  8, 5.0f, 1.0f, 2.0f, 59.0f, 0.0f, 0.0f);
        create_ramp(15.0f, 0.0f, 100.0f,  7, 5.0f, 1.0f, 1.9f, 42.0f, 0.0f, 0.0f);
        create_ramp(10.0f, 0.0f, 100.0f,  6, 5.0f, 1.0f, 1.8f, 25.0f, 0.0f, 0.0f);
        create_ramp( 5.0f, 0.0f, 100.0f,  5, 5.0f, 1.0f, 1.8f, 10.0f, 0.0f, 0.0f);

        // the U ramp thing
        create_ramp(101.0f,0.0f,100.0f, 15, 30.0f, 1.0f, 1.3f, 180.0f, 90.0f, 1.0f);

        //ice
        create_cube(75.0f,0.0f,120.0f, 50.0f, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 1,1.0f);

        // the right wall next to the slope
        create_cube(108.9f,1.0f,25.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(108.9f,1.0f,75.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(108.9f,1.0f,125.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(108.9f,1.0f,175.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(108.9f,1.0f,208.0f, 0.5f,2.0f,16.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        
        // the left wall next to the slope
        create_cube(150.9f,2.0f,25.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(150.9f,2.0f,75.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(150.9f,2.0f,125.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(150.9f,2.0f,175.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f);
        create_cube(150.9f,2.0f,208.0f, 0.5f,4.0f,16.0f, 0.0f,0.0f,0.0f, 3, 0.5f);

        // those goofyahh ramps with walls
        create_ramp(170.0f,0.0f,  30.0f,  40,  40.0f, 1.0f, 1.5f, 90.0f, 180.0f, 0.5f);
        create_cube(170.0f,80.0f, -7.6f,       40.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f);
        create_ramp(170.0f,120.0f,-7.6f,  40, 40.0f, 1.6f, 1.5f, 90.0f, 180.0f, 0.5f, 90.0f);
        create_cube(170.0f,100.0f,200.0f,     40.0f, 80.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f);

        create_ramp(200.0f,0.0f, 30.0f,  20,  20.0f, 1.0f, 1.5f, 90.0f, 180.0f, 0.5f);
        create_cube(200.0f,40.0f,11.6f,       20.0f, 50.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f);
        create_ramp(200.0f,60.0f,11.6f,  20,  20.0f, 1.6f, 1.5f, 90.0f, 180.0f, 0.5f, 90.0f);
        create_cube(200.0f,60.0f,100.0f,      20.0f, 40.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f);

        create_ramp(215.0f,0.0f, 30.0f,  10,  10.0f, 1.0f, 1.5f, 90.0f, 180.0f, 0.5f);
        create_cube(215.0f,20.0f,21.2f,       10.0f, 25.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f);
        create_ramp(215.0f,30.0f,21.2f,  10,  10.0f, 1.6f, 1.5f, 90.0f, 180.0f, 0.5f, 90.0f);
        create_cube(215.0f,30.0f,50.0f,       10.0f, 20.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f);

        // the wall behind the huge slope
        create_cube(130,5.0f, 0.5f,  80.0f, 11.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3, 0.5f);
        printf("collision boxes created: %d\n", collision_boxes_count);

        for (i=0;i<5;i++){
            add_retarder(75.0f, 0.0f, 220.0f+(i*10.0f), i*10.0f);
            add_stopsign(95.0f, 0.0f, 220.0f+(i*10.0f), i*10.0f);
        }
    }

    // for (int bullshit=0;bullshit<chaloupky.size();bullshit++){
    //     for (int shit=0;shit<20;shit++){
    //         for (int poo=0;poo<20;poo++){
    //             set_heightmap_texture(chaloupky[bullshit].y+shit, chaloupky[bullshit].x+poo, 2);
    //         }
    //     }
    // }
    j=randint(0, MAP_SIZE);
    k=randint(0, MAP_SIZE);
    chaloupky.push_back((Vec2){(float)j,(float)k});
    printf("udelal jsem chalouppky na pozici %d, %d", j,k);
    for (i=0;i<chaloupky.size();i++){
        j=chaloupky[i].y;
        k=chaloupky[i].x;
        float h=mapgen_get_heightmap_height((float)j,(float)k)+0.3f;

        chaloupka_x=(float)j;
        chaloupka_z=(float)k;
        chaloupka_y=h;
        add_object_to_world((float)j, h, (float)k, 1);
        collision_box box={};
        box.sx=(float)j-2.0f;
        box.sy=h-3.0f;
        box.sz=(float)k-1.6f;
        box.ex=(float)j+1.5f;
        box.ey=h+3.0f;
        box.ez=(float)k+1.6f;
        add_collision_box_to_world(box);
    }
    printf("udelano %d chaloupek\n", chaloupky.size());

    add_retarders_to_road(0.03f);

    return;
}
unsigned int rend_arena()
{
    unsigned int map_list = glGenLists(1);
    glNewList(map_list, GL_COMPILE);

	p1.x=0;
	p1.y=0;
	p1.z=-0.1;

	p2.x=0.1;
	p2.y=0;
	p2.z=-0.1;

	p3.x=0.1;
	p3.y=0.5;
	p3.z=-0.1;

	p4.x=0;
	p4.y=0.5;
	p4.z=-0.1;
    glNormal3f(1.0f,0.0f,0.0f);
	rend_face(8,p1,p2,p3,p4);

	for (j=0;j<500;j++){
		for (i=0;i<50;i++){
            glNormal3f(0.0f,1.0f,0.0f);

			p1.x=i;
			p1.y=0;
			p1.z=j;

			p2.x=i+1.0f;
			p2.y=0;
			p2.z=j;

			p3.x=i+1.0;
			p3.y=0;
			p3.z=j+1.0;

			p4.x=i;
			p4.y=0;
			p4.z=j+1.0;
			rend_face(8,p1,p2,p3,p4);
		}
	}
	p1.x=6;
	p1.y=0;
	p1.z=0;

	p2.x=12;
	p2.y=0;
	p2.z=0;

	p3.x=12;
	p3.y=1.5;
	p3.z=0;

	p4.x=6;
	p4.y=1.5;
	p4.z=0;
    glNormal3f(1.0f,0.0f,0.0f);

	rend_face(8,p1,p2,p3,p4);



    glEndList();
    
    return map_list;
}
void init_map_memory() {
	printf("allocating map memory... (%d x %d chunks, total %d, MAP_SIZE=%d, CHUNK_SIZE=%d)\n", CHUNKS_SIZE, CHUNKS_SIZE, CHUNKS_SIZE*CHUNKS_SIZE, MAP_SIZE, CHUNK_SIZE);
    CHUNKS_SIZE=MAP_SIZE/CHUNK_SIZE;
    for (int z = 0; z < CHUNKS_SIZE; z++) {
        chunks.emplace_back();
        for (int x = 0; x < CHUNKS_SIZE; x++) {
            chunks[z].push_back((chunk*)calloc(1, sizeof(chunk)));
            if (chunks[z][x] == NULL) {
                printf("FATAL ERROR: Out of memory for chunk [%d][%d]\n", x, z);
                SDL_Delay(100);
                exit(1);
            }
        }
    }
}
void clear_chunk(int cz, int cx){
    chunks[cz][cx]->collision_boxes_count=0;
    chunks[cz][cx]->objects.clear();
    chunks[cz][cx]->object_poses.clear();
    chunks[cz][cx]->permanent=false;
    chunks[cz][cx]->tex_pres=-1;
}
void clear_chunk_heightmap(int cz, int cx){
    chunks[cz][cx]->heightmap.clear();
    chunks[cz][cx]->heightmap_tex.clear();
    chunks[cz][cx]->loaded=false;
    chunks[cz][cx]->generated=false;
}
void clear_chunks_heightmap(){
    printf("clearing chunks heightmap... \n");
    for (i=0;i<CHUNKS_SIZE;i++){
        for (j=0;j<CHUNKS_SIZE;j++){
            if (!chunks[i][j]->permanent){
                clear_chunk_heightmap(i,j);
            }
        }   
    }
}
void clear_chunks(){
    printf("clearing chunks... \n");
    for (i=0;i<CHUNKS_SIZE;i++){
        for (j=0;j<CHUNKS_SIZE;j++){
            clear_chunk(i,j);
        }   
    }
    clear_chunks_heightmap();
    chaloupky.clear();
}
void lhh(std::string text){
    // text="[ic] "+text;
    // log(text);
}
bool init_chunk(int cx, int cy, int pres=1){
    chunks[cx][cy]->pres=1;
    pres=CHUNK_SIZE/pres;
    log("init chunk at "+std::to_string(cx)+", "+std::to_string(cy));
    if (chunks[cx][cy]->loaded){
        log("this one is already loaded");
    }
    if (chunks_loaded==max_chunks_loaded){
        printf("too much chunks");
        return false;
    }
    lhh("i1");
    chunks[cx][cy]->heightmap.resize(pres);
    chunks[cx][cy]->heightmap_tex.resize(pres);
    lhh("i2");

    for (auto& a:chunks[cx][cy]->heightmap){
        a.resize(pres);
    }

    for (auto& a:chunks[cx][cy]->heightmap_tex){
        a.resize(pres);
    }
    lhh("i3");

    for (int x=0;x<pres;x++){
        for (int y=0;y<pres;y++){
            chunks[cx][cy]->heightmap[x][y]=0.0f;
        }
    }
    for (int x=0;x<pres;x++){
        for (int y=0;y<pres;y++){
            chunks[cx][cy]->heightmap_tex[x][y]=0;
        }
    }
    lhh("i4");

    chunks[cx][cy]->loaded=true;
    lhh("i5");
    chunks_loaded++;

    return true;
}
std::vector<Vec3> generate_reference_points(int count) {
    std::vector<Vec3> points;
    int failed_attempts = 0;

    while (points.size() < count && failed_attempts < 2000) {
        // 1. Náhodný bod na mapě
        float x = (float)randint(0, MAP_SIZE);
        float z = (float)randint(0, MAP_SIZE);
        float y = mapgen_get_heightmap_height(z, x);

        // 2. Kritérium: Nechceme body ve vodě (předpokládám y > 0)
        if (y < 2.0f) { failed_attempts++; continue; }

        // 3. Kritérium: Nechceme body moc blízko u sebe (aby silnice měly smysl)
        bool too_close = false;
        for (const auto& p : points) {
            float dx = p.x - x;
            float dz = p.z - z;
            if (sqrt(dx*dx + dz*dz) < 200.0f) { // Minimálně 400 jednotek od sebe
                too_close = true;
                break;
            }
        }

        if (!too_close) {
            points.push_back({x, y, z});
        } else {
            failed_attempts++;
        }
    }
    printf("generated %d base roadpoints\n", points.size());
    for (int bullshit=0;bullshit<points.size();bullshit++){
        chaloupky.push_back((Vec2){points[bullshit].x+10.0f, points[bullshit].z+10.0f});
    }
    return points;
}
void plan_road_network(int num_points, std::vector<Vec3>& starts, std::vector<Vec3>& ends) {
    std::vector<Vec3> points = generate_reference_points(num_points);
    if (points.size() < 2) return;

    // A. Propojení do řady (hlavní tah mapou)
    for (size_t i = 0; i < points.size() - 1; i++) {
        starts.push_back(points[i]);
        ends.push_back(points[i+1]);
    }

    // B. Propojení "každý s nejbližším", co ještě není v řadě (křižovatky)
    for (size_t i = 0; i < points.size(); i++) {
        float min_dist = 1000000.0f;
        int best_neighbor = -1;

        for (size_t j = 0; j < points.size(); j++) {
            if (i == j) continue;
            // Nechceme propojovat sousedy, co už jsou v řadě A
            if (abs((int)i - (int)j) <= 1) continue;

            float d = get_dist(points[i], points[j]);
            if (d < min_dist) {
                min_dist = d;
                best_neighbor = j;
            }
        }

        // Pokud najdeme rozumného souseda blízko (např. do 800m), propojíme to
        if (best_neighbor != -1 && min_dist < 800.0f) {
            // Přidáme jen pokud už takové spojení neexistuje (symetrie)
            starts.push_back(points[i]);
            ends.push_back(points[best_neighbor]);
        }
    }
}
#define ROAD_MAX_HEIGHT 600
void plan_road_network_2(float spacing, std::vector<Vec3>& starts, std::vector<Vec3>& ends) {
    float jitter = spacing * 0.35f;

    std::vector<Vec3> points;

    unsigned int seed = 12345;
    auto rnd = [&]() -> float {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        return (float)(seed & 0xFFFF) / (float)0xFFFF;
    };

    for (float gx = spacing * 0.5f; gx < MAP_SIZE; gx += spacing) {
        for (float gz = spacing * 0.5f; gz < MAP_SIZE; gz += spacing) {
            float x = gx + (rnd() * 2.0f - 1.0f) * jitter;
            float z = gz + (rnd() * 2.0f - 1.0f) * jitter;

            x = clamp((int)x, 0, MAP_SIZE);
            z = clamp((int)z, 0, MAP_SIZE);

            float y = mapgen_get_heightmap_height(x, z);
            if (y <= ROAD_MAX_HEIGHT) {
                points.push_back({x, y, z});
            }
        }
    }

    if (points.size() < 2) return;

    int neighbors = 3;
    for (int i = 0; i < (int)points.size(); i++) {
        std::vector<std::pair<float, int>> dists;
        for (int j = 0; j < (int)points.size(); j++) {
            if (i == j) continue;
            float dx = points[i].x - points[j].x;
            float dz = points[i].z - points[j].z;
            dists.push_back({ dx*dx + dz*dz, j });
        }
        std::sort(dists.begin(), dists.end());

        for (int k = 0; k < std::min(neighbors, (int)dists.size()); k++) {
            starts.push_back(points[i]);
            ends.push_back(points[dists[k].second]);
        }
    }
}
void add_tree(int x, int y){
    float tree_h = mapgen_get_heightmap_height(y, x);

    // vytvoření kolizního boxu
    collision_box box={};
    box.sx = y - 0.4f;
    box.sy = tree_h;
    box.sz = x - 0.4f;
    box.ex = y + 0.4f;
    box.ey = tree_h + 2.0f;
    box.ez = x + 0.4f;
    add_collision_box_to_world(box);

    // přidání vizuálního objektu stromu
    add_object_to_world(y, tree_h, x, 0);
}
// void make_village(float x, float y, int houses){
//     chaloupky.push_back({y, x});
//     printf("added village at %f, %f\n", y, x);
//     // for (int i = -3; i < 3; i++) {
//     //     for (int j = -3; j < 3; j++) {
//     //         add_tree((int)(y + i*4), (int)(x + j*4));
//     //     }
//     // }
// }

// Pomocná funkce pro výpočet vzdálenosti (pokud ji už nemáš jinde)
float get_distance(float x1, float y1, float x2, float y2) {
    return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void make_village(float center_x, float center_y, int num_houses) {
    // 1. GENEROVÁNÍ CHALOUPCH (Organické rozmístění kolem centra)
    printf("--- Generating Village at %f, %f ---\n", center_x, center_y);
    
    for (int i = 0; i < num_houses; i++) {
        // Rozmístíme chaloupky náhodně v určitém radiusu od centra vesnice
        // Použijeme goniometrické funkce pro kruhové rozptýlení
        float angle = (float)rand() / RAND_MAX * 2.0f * M_PI;
        // Chaloupky budou v okruhu 5 až 25 metrů/pixelů od středu
        float radius = 5.0f + ((float)rand() / RAND_MAX * 20.0f); 
        
        float h_x = center_x + std::cos(angle) * radius;
        float h_y = center_y + std::sin(angle) * radius;
        
        // Pozor: v původním kódu jsi měl push_back({y, x}), 
        // tak to držím stejně, pokud má tvoje struktura pořadí {y, x}
        chaloupky.push_back({h_y, h_x});
        printf("  Added house #%d at Y: %f, X: %f\n", i + 1, h_y, h_x);
    }

    // 2. INTELIGENTNÍ STROMY (Přírodní prstenec okolo)
    // Vygenerujeme les kolem vesnice, ale stromy nesmí spawnovat přímo na domech
    int total_trees_to_try = num_houses * 8; // Množství stromů škálujeme podle velikosti vesnice
    float village_radius = 40.0f;           // Hranice, kam až vesnice sahá

    for (int i = 0; i < total_trees_to_try; i++) {
        // Vygenerujeme náhodný bod v širším okolí vesnice
        float angle = (float)rand() / RAND_MAX * 2.0f * M_PI;
        // Stromy rostou od vnitřku vesnice až kousek za ni (radius 15 až 60)
        float radius = 15.0f + ((float)rand() / RAND_MAX * 45.0f);
        
        int tree_x = (int)(center_x + std::cos(angle) * radius);
        int tree_y = (int)(center_y + std::sin(angle) * radius);

        // Inteligentní kontrola: Je strom příliš blízko nějaké chaloupky?
        bool too_close_to_house = false;
        
        // Projdeme nově vygenerované chaloupky pro tuto vesnici
        // (Pokud jsou v globálním vektoru, bereme poslední 'num_houses' prvků)
        int start_idx = chaloupky.size() - num_houses;
        for (int j = start_idx; j < chaloupky.size(); j++) {
            // bacha na tvoje prohozené {y, x} v struktuře
            float house_y = chaloupky[j].y; 
            float house_x = chaloupky[j].x; 

            if (get_distance((float)tree_x, (float)tree_y, house_x, house_y) < 8.0f) {
                too_close_to_house = true; // Strom je nalepený na domě, nechceme
                break;
            }
        }

        // Pokud je místo bezpečné, zasadíme strom
        if (!too_close_to_house) {
            add_tree(tree_y, tree_x);
        }
    }
}
void subdivide_roads(int start_part_idx, int end_part_idx) {
    if (start_part_idx >= end_part_idx) return;
    if (start_part_idx < 0 || end_part_idx > roadparts_len) return;

    // --- Stupeň (degree) každého bodu přes CELOU síť silnic ---
    // (křižovatka může být napojena i segmenty mimo tento úsek)
    std::vector<int> point_degree(roadpoints.size(), 0);
    for (int i = 0; i < roadparts_len; i++) {
        point_degree[roadparts[i].p1]++;
        point_degree[roadparts[i].p2]++;
    }

    // --- Body se stejnou pozicí (x,y) = fakticky taky křižovatka, ---
    // --- jen reprezentovaná dvěma různými indexy.                 ---
    std::unordered_map<long long, int> pos_count;
    pos_count.reserve(roadpoints.size() * 2);
    for (int i = 0; i < (int)roadpoints.size(); i++) {
        long long key = ((long long)roadpoints[i].x << 32) ^ (unsigned int)roadpoints[i].y;
        pos_count[key]++;
    }

    auto is_locked = [&](int idx) {
        if (point_degree[idx] > 2) return true; // křižovatka
        long long key = ((long long)roadpoints[idx].x << 32) ^ (unsigned int)roadpoints[idx].y;
        return pos_count[key] > 1; // duplicitní bod na stejné pozici
    };

    std::vector<roadpart> new_parts;
    new_parts.reserve((end_part_idx - start_part_idx) * 2);

    for (int pi = start_part_idx; pi < end_part_idx; pi++) {
        int p1 = roadparts[pi].p1;
        int p2 = roadparts[pi].p2;
        float width = roadparts[pi].width;

        // Segment se nedělí, pokud se dotýká křižovatky / duplicitního bodu.
        // Původní body se v takovém případě vůbec nehýbou ani neduplikují.
        if (is_locked(p1) || is_locked(p2)) {
            new_parts.push_back(roadparts[pi]); // beze změny, původní indexy
            continue;
        }

        Vec3 a = {(float)roadpoints[p1].x, (float)roadpoints[p1].h, (float)roadpoints[p1].y};
        Vec3 b = {(float)roadpoints[p2].x, (float)roadpoints[p2].h, (float)roadpoints[p2].y};

        // Sousedé pro Catmull-Rom (hledáme jen v rámci zpracovávaného úseku)
        Vec3 p0 = a, p3 = b;
        for (int pj = start_part_idx; pj < end_part_idx; pj++) {
            if (roadparts[pj].p2 == p1) {
                int np = roadparts[pj].p1;
                p0 = {(float)roadpoints[np].x, (float)roadpoints[np].h, (float)roadpoints[np].y};
                break;
            }
        }
        for (int pj = start_part_idx; pj < end_part_idx; pj++) {
            if (roadparts[pj].p1 == p2) {
                int np = roadparts[pj].p2;
                p3 = {(float)roadpoints[np].x, (float)roadpoints[np].h, (float)roadpoints[np].y};
                break;
            }
        }

        // Jediný nový bod uprostřed (t = 0.5), Catmull-Rom interpolace.
        // 'a' a 'b' se nikdy nemění - zůstávají na svých původních indexech.
        float tt = 0.5f, tt2 = tt*tt, tt3 = tt2*tt;
        float cx = 0.5f * ((2.0f*a.x) + (-p0.x + b.x)*tt + (2.0f*p0.x - 5.0f*a.x + 4.0f*b.x - p3.x)*tt2 + (-p0.x + 3.0f*a.x - 3.0f*b.x + p3.x)*tt3);
        float cz = 0.5f * ((2.0f*a.z) + (-p0.z + b.z)*tt + (2.0f*p0.z - 5.0f*a.z + 4.0f*b.z - p3.z)*tt2 + (-p0.z + 3.0f*a.z - 3.0f*b.z + p3.z)*tt3);
        float cy = 0.5f * ((2.0f*a.y) + (-p0.y + b.y)*tt + (2.0f*p0.y - 5.0f*a.y + 4.0f*b.y - p3.y)*tt2 + (-p0.y + 3.0f*a.y - 3.0f*b.y + p3.y)*tt3);

        cx = fmaxf(0.0f, fminf((float)MAP_SIZE, cx));
        cz = fmaxf(0.0f, fminf((float)MAP_SIZE, cz));

        roadpoint mid;
        mid.x = (int)cx;
        mid.y = (int)cz;
        mid.h = cy;
        int mid_idx = (int)roadpoints.size();
        roadpoints.push_back(mid); // pouze APPEND, nic starého se nepřepisuje

        roadpart r1, r2;
        r1.p1 = p1;      r1.p2 = mid_idx; r1.width = width;
        r2.p1 = mid_idx; r2.p2 = p2;      r2.width = width;
        new_parts.push_back(r1);
        new_parts.push_back(r2);
    }

    // Přepíšeme pouze segmenty v rozsahu [start,end) - VŠECHNY body (i ty
    // před/za rozsahem) zůstávají zcela netknuté, žádné indexy se neposouvají.
    roadparts.erase(roadparts.begin() + start_part_idx, roadparts.begin() + end_part_idx);
    roadparts.insert(roadparts.begin() + start_part_idx, new_parts.begin(), new_parts.end());

    roadpoints_len = (int)roadpoints.size();
    roadparts_len  = (int)roadparts.size();
}

// Omezí sklon (incline) silnice mezi po sobě jdoucími body v rozsahu
// [start_part_idx, end_part_idx) tak, aby nikde nepřekročil max_incline.
//
// max_incline: 0.0f = úplně rovně, 1.0f = 45°, obecně sklon = tan(uhel)
//              (výškový rozdíl / vodorovná vzdálenost)
//
// Křižovatky (bod sdílený >2 částmi silnice) a duplicitní body (stejná
// pozice, jiný index) se NIKDY nehýbou - fungují jako pevné kotvy, ke
// kterým se profil silnice "srovná". Volné body mezi kotvami se posouvají
// tak, aby žádný sousední pár nepřekročil povolený sklon.
// Omezí sklon (incline) mezi sousedními body v rozsahu [start_part_idx, end_part_idx).
//
// max_incline: 0.0f = rovně, 1.0f = 45° (sklon = výškový rozdíl / vodorovná vzdálenost)
//
// Nikdy nehýbe:
//   - prvním bodem rozsahu (roadparts[start_part_idx].p1)
//   - posledním bodem rozsahu (roadparts[end_part_idx-1].p2)
//   - žádnou křižovatkou (bod sdílený >2 částmi silnice)
//   - žádným duplicitním bodem (jiný index, stejná pozice)
// Všechny ostatní body mezi kotvami se opakovaně upravují, dokud
// celý úsek nesplňuje max_incline všude.
// Omezí NE absolutní sklon, ale ROZDÍL sklonu mezi dvěma po sobě jdoucími
// částmi silnice sdílejícími společný bod. Tím se odstraní ostré "zlomy"
// (náhlá změna sklonu - hrbol/prohlubeň), ale plynulé stoupání/klesání
// do kopce zůstane netknuté, protože tam je rozdíl sklonu mezi sousedními
// částmi malý.
//
// max_incline_delta: 0.0f = sklon musí být všude naprosto identický,
//                     1.0f = mezi sousedními částmi je povolená změna
//                            sklonu odpovídající 45°
//
// Nikdy nehýbe:
//   - prvním bodem rozsahu (roadparts[start_part_idx].p1)
//   - posledním bodem rozsahu (roadparts[end_part_idx-1].p2)
//   - křižovatkami (bod sdílený >2 částmi silnice)
//   - duplicitními body (jiný index, stejná pozice)
void limit_road_incline(int start_part_idx, int end_part_idx, float max_incline_delta,
                               int max_iterations = 200) {
    if (start_part_idx >= end_part_idx) return;
    if (start_part_idx < 0 || end_part_idx > roadparts_len) return;
    if (max_incline_delta < 0.0f) return;
    std::vector<int> point_degree(roadpoints.size(), 0);
    for (int i = 0; i < roadparts_len; i++) {
        point_degree[roadparts[i].p1]++;
        point_degree[roadparts[i].p2]++;
    }

    std::unordered_map<long long, int> pos_count;
    pos_count.reserve(roadpoints.size() * 2);
    for (int i = 0; i < (int)roadpoints.size(); i++) {
        long long key = ((long long)roadpoints[i].x << 32) ^ (unsigned int)roadpoints[i].y;
        pos_count[key]++;
    }

    int first_pt = roadparts[start_part_idx].p1;
    int last_pt  = roadparts[end_part_idx - 1].p2;

    auto is_locked = [&](int idx) {
        if (idx == first_pt || idx == last_pt) return true; // hranice úseku
        if (point_degree[idx] > 2) return true;              // křižovatka
        long long key = ((long long)roadpoints[idx].x << 32) ^ (unsigned int)roadpoints[idx].y;
        return pos_count[key] > 1;                            // duplicitní pozice
    };

    auto horiz_dist = [&](int a, int b) {
        float dx = (float)roadpoints[b].x - (float)roadpoints[a].x;
        float dz = (float)roadpoints[b].y - (float)roadpoints[a].y;
        return sqrtf(dx * dx + dz * dz);
    };

    // Najde v rozsahu [start,end) index části, jejíž p1 == idx (sousední
    // část navazující ZA bodem idx). Vrací -1, pokud žádná neexistuje
    // (idx je konec úseku).
    auto find_part_starting_at = [&](int idx) {
        for (int pj = start_part_idx; pj < end_part_idx; pj++) {
            if (roadparts[pj].p1 == idx) return pj;
        }
        return -1;
    };

    for (int iter = 0; iter < max_iterations; iter++) {
        bool changed = false;

        for (int pi = start_part_idx; pi < end_part_idx; pi++) {
            int mid = roadparts[pi].p2;
            if (is_locked(mid)) continue; // tenhle bod se nesmí hýbat

            int next_pi = find_part_starting_at(mid);
            if (next_pi == -1) continue; // konec řetězce v tomto rozsahu

            int prev_pt = roadparts[pi].p1;
            int next_pt = roadparts[next_pi].p2;

            float d_in  = horiz_dist(prev_pt, mid);
            float d_out = horiz_dist(mid, next_pt);
            if (d_in < 0.01f || d_out < 0.01f) continue;

            float slope_in  = (roadpoints[mid].h     - roadpoints[prev_pt].h) / d_in;
            float slope_out = (roadpoints[next_pt].h - roadpoints[mid].h)     / d_out;
            float delta = slope_out - slope_in;

            if (fabsf(delta) <= max_incline_delta + 0.0001f) continue; // v pořádku

            // Chceme nový sklon mid->next takový, aby rozdíl vůči sklonu
            // prev->mid byl přesně na povolené hranici (se stejným
            // znaménkem jako původní zlom, jen useknutý na limit).
            float sign = (delta > 0.0f) ? 1.0f : -1.0f;
            float target_delta = sign * max_incline_delta;

            // slope_out_new - slope_in = target_delta, slope_in závisí na
            // novém h[mid] taky, takže řešíme přímo pro h[mid]:
            // (next.h - h)/d_out - (h - prev.h)/d_in = target_delta
            float inv_sum = 1.0f / (1.0f / d_out + 1.0f / d_in);
            float new_h = (roadpoints[next_pt].h / d_out + roadpoints[prev_pt].h / d_in - target_delta) * inv_sum;

            roadpoints[mid].h = new_h;
            changed = true;
        }

        if (!changed) break; // celý úsek už má plynulé přechody sklonu
    }
}
#include <queue>
#include <set>
#include <cmath>
#include <vector>
#include <algorithm>

// Pomocná struktura pro A* uzel
struct RoadNode {
    Vec3 pos;
    float last_angle;
    float g_score;
    float f_score;
    int parent_idx;
    int self_idx; // <-- add this
    bool operator>(const RoadNode& o) const { return f_score > o.f_score; }
};

// Pomocná struktura pro označení navštívených pozic v diskrétním prostoru (gridu)
struct GridPos {
    int x, z;
    bool operator<(const GridPos& other) const {
        if (x != other.x) return x < other.x;
        return z < other.z;
    }
};

void gen_road_fallback(Vec3 road_start, Vec3 road_end, float width, float roadpart_size=100.0f) {
    // --- NASTAVENÍ LIMITŮ ---
    const float MAX_HEIGHT_DIFF = 10.0f; // Maximální stoupání na jeden krok (roadpart_size)
    const float MAX_TUNNEL_DEPTH = 7.0f; // Max metrů pod zemí
    const float MAX_BRIDGE_HEIGHT = 80.0f; // Max výška mostu nad terénem
    const int DIRECTIONS = 16;           // Počet směrů k prozkoumání v každém kroku
    
    // Fronta pro otevřené uzly (Open Set) a pole pro všechny vygenerované uzly
    std::priority_queue<RoadNode, std::vector<RoadNode>, std::greater<RoadNode>> open_set;
    std::vector<RoadNode> all_nodes;
    std::set<GridPos> closed_set;

    // Vložení startovního bodu
    // Vložení startovního bodu
    RoadNode start_node;
    start_node.pos = road_start;
    start_node.last_angle = get_angle(road_start.x, road_start.z, road_end.x, road_end.z);
    start_node.g_score = 0.0f;
    start_node.f_score = get_dist(road_start, road_end);


    start_node.parent_idx = -1;
    start_node.self_idx = 0;   // <-- ADD THIS LINE

    open_set.push(start_node);
    all_nodes.push_back(start_node);


    
    int target_node_idx = -1;
    int iterations = 0;

    // Hlavní A* smyčka
    while (!open_set.empty() && iterations < 30000) {
        iterations++;
        RoadNode current = open_set.top();
        open_set.pop();

        int current_idx = current.self_idx;


        // Kontrola, zda jsme blízko cíle
        if (get_dist(current.pos, road_end) < roadpart_size * 1.5f) {
            target_node_idx = current_idx;
            break;
        }

        // Označení pozice jako zpracované (Closed Set)
        GridPos c_pos = {(int)(current.pos.x / roadpart_size), (int)(current.pos.z / roadpart_size)};
        if (closed_set.count(c_pos)) continue;
        closed_set.insert(c_pos);

        // Zkoumáme sousedy (všech 16 směrů)
        for (int i = 0; i < DIRECTIONS; i++) {
            float angle = (PI * 2.0f / (float)DIRECTIONS) * (float)i;

            // Zabránění otočení o 180 stupňů (hladkost silnice)
            if (cosf(angle - current.last_angle) < -0.3f) continue;

            float xpos = current.pos.x + sinf(angle) * roadpart_size;
            float zpos = current.pos.z + cosf(angle) * roadpart_size;

            // Kontrola hranic mapy
            if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) continue;

            // Výška skutečného terénu v tomto bodě
            float terrain_y = mapgen_get_heightmap_height(zpos, xpos);
            float target_y = terrain_y;

            // --- LOGIKA PRO MOSTY A TUNELY ---
            // Výškový rozdíl mezi předchozí silnicí a čistým terénem
            float height_diff_to_terrain = target_y - current.pos.y;

            if (height_diff_to_terrain > MAX_HEIGHT_DIFF) {
                // Terén stoupá moc prudce -> Zkusíme zářez/tunel (silnice půjde níže než terén)
                float ideal_y = current.pos.y + MAX_HEIGHT_DIFF;
                if (terrain_y - ideal_y <= MAX_TUNNEL_DEPTH) {
                    target_y = ideal_y; // Jedeme maximálním stoupáním pod zemí
                } else {
                    continue; // Ani s tunelem to nevyjedeme, zkusíme jiný směr (serpentina!)
                }
            } 
            else if (height_diff_to_terrain < -MAX_HEIGHT_DIFF) {
                // Terén klesá moc prudce -> Uděláme most (silnice půjde výše než terén)
                float ideal_y = current.pos.y - MAX_HEIGHT_DIFF;
                if (ideal_y - terrain_y <= MAX_BRIDGE_HEIGHT) {
                    target_y = ideal_y; // Stavíme most vzduchem
                } else {
                    target_y = terrain_y + MAX_HEIGHT_DIFF; // Sjedeme dolů co nejvíc to jde
                }
            }
            else {
                // Terén je v pohodě, silnice přesně kopíruje povrch
                target_y = terrain_y;
            }

            // Finální kontrola sklonu vůči předchozímu bodu silnice (STRIKTNÍ LIMIT)
            if (fabs(current.pos.y - target_y) > MAX_HEIGHT_DIFF) continue;

            // Výpočet cen (G a F score)
            float step_dist = sqrtf(roadpart_size * roadpart_size + (current.pos.y - target_y) * (current.pos.y - target_y));
            float g_score = current.g_score + step_dist;
            
            // Heuristika: Euklidovská vzdálenost k cíli vzdušnou čarou
            Vec3 next_pos = {xpos, target_y, zpos};
            float h_score = get_dist(next_pos, road_end); 

            // Penalizace za stavbu mostů/tunelů, aby algoritmus preferoval povrch, pokud existuje
            if (fabs(target_y - terrain_y) > 1.0f) {
                g_score += roadpart_size * 0.5f; 
            }

            RoadNode neighbor;
            neighbor.pos = next_pos;
            neighbor.last_angle = angle;
            neighbor.g_score = g_score;
            neighbor.f_score = g_score + h_score;
            neighbor.parent_idx = current_idx;
            neighbor.self_idx = (int)all_nodes.size();   // <-- ADD THIS LINE

            open_set.push(neighbor);
            all_nodes.push_back(neighbor);
        }
    }

    // --- REKONSTRUKCE CESTY ---
    if (target_node_idx != -1) {
        std::vector<Vec3> final_path;
        int curr = target_node_idx;
        while (curr != -1) {
            final_path.push_back(all_nodes[curr].pos);
            curr = all_nodes[curr].parent_idx;
        }
        std::reverse(final_path.begin(), final_path.end());

        // Připojíme koncový bod POSTUPNĚ, ne jedním skokem - pokud se výška
        // road_end.y liší od posledního nalezeného bodu o víc, než dovoluje
        // MAX_HEIGHT_DIFF na jeden krok (např. terén se mezitím změnil kvůli
        // jiné silnici/mostu), vložíme dostatek mezibodů, aby žádný jednotlivý
        // segment nebyl příliš strmý. Zabraňuje to vzniku svislého "sloupu".
        Vec3 last_found = final_path.back();
        float height_gap = fabs(road_end.y - last_found.y);
        int extra_steps = (int)ceilf(height_gap / MAX_HEIGHT_DIFF);
        if (extra_steps < 1) extra_steps = 1;
        for (int s = 1; s <= extra_steps; s++) {
            float t = (float)s / (float)extra_steps;
            Vec3 p;
            p.x = last_found.x + (road_end.x - last_found.x) * t;
            p.z = last_found.z + (road_end.z - last_found.z) * t;
            p.y = last_found.y + (road_end.y - last_found.y) * t;
            final_path.push_back(p);
        }

        // Zápis do tvých globálních struktur `roadpoints` a `roadparts`
        int start_part_idx = (int)roadparts.size();

        for (size_t i = 0; i < final_path.size(); i++) {
            roadpoints.push_back({});
            roadpoints[roadpoints_len].x = final_path[i].x;
            roadpoints[roadpoints_len].y = final_path[i].z;
            roadpoints[roadpoints_len].h = final_path[i].y;
            roadpoints_len++;

            if (i > 0) {
                roadparts.push_back({});
                roadparts[roadparts_len].p1 = roadpoints_len - 2;
                roadparts[roadparts_len].p2 = roadpoints_len - 1;
                roadparts[roadparts_len].width = width;
                roadparts_len++;
            }
        }

        // Přidání hlavních bodů cesty
        Vec2 msp = {road_start.x, road_start.z};
        Vec2 mep = {road_end.x, road_end.z};
        major_road_points.push_back(msp);
        major_road_points.push_back(mep);

        // Závěrečné vyhlazení cesty (Subdivision)
        int end_part_idx = (int)roadparts.size();
        limit_road_incline(start_part_idx, end_part_idx, 0.1f); // max sklon 0.5 = 26.565°

        for (int step = 0; step < ROAD_SUBDIVISIONS; step++) {
            subdivide_roads(start_part_idx, end_part_idx);
            end_part_idx = start_part_idx + (end_part_idx - start_part_idx) * 2;
        }
    }
}


void gen_road(Vec3 road_start, Vec3 road_end, float width, float roadpart_size=100.0f) {
    float weights[] = {0.1f, 0.2f, 0.7f};
    float min_relative_angle_score = 0.0f;
    float max_height_diff = 10.0f;
    float min_ang_score=0.0f;
    bool uspech=false;

    int angle_attempts = 32;    // directions to try per step
    int beam_width = 10;         // how many paths to keep alive
    int lookahead = 8;          // how many steps to look ahead
    if (get_dist(road_start, road_end) < roadpart_size * 2.0f) return;

    // --- ZÁLOHA STAVU PRO PŘÍPADNÝ ROLLBACK ---
    int orig_roadpoints_len = roadpoints_len;
    int orig_roadparts_len = roadparts_len;
    int orig_major_road_points_size = (int)major_road_points.size();
    int start_part_idx = (int)roadparts.size();
    // ------------------------------------------

    Vec3 last_pos  = road_start;
    float last_angle = get_angle(road_start.x, road_start.z, road_end.x, road_end.z);
    roadpoints.push_back({});
    roadpoints[roadpoints_len].x = road_start.x;
    roadpoints[roadpoints_len].y = road_start.z;
    roadpoints[roadpoints_len].h = road_start.y;
    roadpoints_len++;

    int iterations = 0;
    while (iterations < 200000) {
        if (get_dist(last_pos, road_end) < roadpart_size){
            uspech=true;
            break;
        }

        float target_angle = get_angle(last_pos.x, last_pos.z, road_end.x, road_end.z);

        std::vector<Candidate> beam;
        for (j = 0; j < angle_attempts; j++) {
            float angle = (PI * 2.0f / (float)angle_attempts) * (float)j;

            float relative_angle_score = cosf(angle - last_angle);
            if (relative_angle_score < min_relative_angle_score) continue;

            float xpos = last_pos.x + sinf(angle) * roadpart_size;
            float zpos = last_pos.z + cosf(angle) * roadpart_size;
            if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) continue;
            float ypos = mapgen_get_heightmap_height(zpos, xpos);
            if (fabs(last_pos.y - ypos) > max_height_diff) continue;

            float absolute_angle_score = cosf(angle - target_angle);
            float height_score = 1.0f - fminf(1.0f, fabs(last_pos.y - ypos) / max_height_diff);
            float score = weights[0]*height_score + weights[1]*relative_angle_score + weights[2]*absolute_angle_score;
            if (absolute_angle_score<min_ang_score){
                score=absolute_angle_score;
            }

            Candidate c;
            c.pos = {xpos, ypos, zpos};
            c.last_angle = angle;
            c.total_score = score;
            c.path.push_back(c.pos);
            beam.push_back(c);
        }

        if (beam.empty()) break;

        for (int step = 1; step < lookahead; step++) {
            std::vector<Candidate> next_beam;

            for (auto& cand : beam) {
                float step_target_angle = get_angle(cand.pos.x, cand.pos.z, road_end.x, road_end.z);

                for (j = 0; j < angle_attempts; j++) {
                    float angle = (PI * 2.0f / (float)angle_attempts) * (float)j;

                    float relative_angle_score = cosf(angle - cand.last_angle);
                    if (relative_angle_score < min_relative_angle_score) continue;

                    float xpos = cand.pos.x + sinf(angle) * roadpart_size;
                    float zpos = cand.pos.z + cosf(angle) * roadpart_size;
                    if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) continue;
                    float ypos = mapgen_get_heightmap_height(zpos, xpos);
                    if (fabs(cand.pos.y - ypos) > max_height_diff) continue;

                    float absolute_angle_score = cosf(angle - step_target_angle);
                    float height_score = 1.0f - fminf(1.0f, fabs(cand.pos.y - ypos) / max_height_diff);
                    float score = weights[0]*height_score + weights[1]*relative_angle_score + weights[2]*absolute_angle_score;

                    Candidate nc;
                    nc.pos = {xpos, ypos, zpos};
                    nc.last_angle = angle;
                    nc.total_score = cand.total_score + score;
                    nc.path = cand.path;
                    nc.path.push_back(nc.pos);
                    next_beam.push_back(nc);
                }
            }

            if (next_beam.empty()) break;

            std::sort(next_beam.begin(), next_beam.end(), [](const Candidate& a, const Candidate& b){
                return a.total_score > b.total_score;
            });
            if ((int)next_beam.size() > beam_width)
                next_beam.resize(beam_width);
            beam = next_beam;
        }

        if (beam.empty()) break;

        Vec3 next = beam[0].path[0];

        float dx;
        float dz;
        bool looping = false;
        int looping_index = -1;
        for (int k = 0; k < roadpoints_len - 2; k++) {
            dx = roadpoints[k].x - next.x;
            dz = roadpoints[k].y - next.z;
            if (sqrtf(dx*dx + dz*dz) < roadpart_size * 2.0f) {
                looping = true;
                looping_index = k;
                break;
            }
        }
        if (looping) {
            if (looping_index>start_part_idx){
                break;
            }
            
            int start_point_idx = roadpoints_len - 1;
            int end_point_idx = looping_index;

            roadpoints.push_back({});
            int p_mid1 = roadpoints_len++;
            roadpoints[p_mid1].x = roadpoints[start_point_idx].x + (roadpoints[end_point_idx].x - roadpoints[start_point_idx].x) * (1.0f / 3.0f);
            roadpoints[p_mid1].y = roadpoints[start_point_idx].y + (roadpoints[end_point_idx].y - roadpoints[start_point_idx].y) * (1.0f / 3.0f);
            roadpoints[p_mid1].h = ((roadpoints[start_point_idx].h + (roadpoints[end_point_idx].h - roadpoints[start_point_idx].h) * (2.0f / 3.0f)) * 0.2f)+(mapgen_get_heightmap_height(roadpoints[p_mid1].y, roadpoints[p_mid1].x)*0.8f);

            roadpoints.push_back({});
            int p_mid2 = roadpoints_len++;
            roadpoints[p_mid2].x = roadpoints[start_point_idx].x + (roadpoints[end_point_idx].x - roadpoints[start_point_idx].x) * (2.0f / 3.0f);
            roadpoints[p_mid2].y = roadpoints[start_point_idx].y + (roadpoints[end_point_idx].y - roadpoints[start_point_idx].y) * (2.0f / 3.0f);
            roadpoints[p_mid2].h = ((roadpoints[start_point_idx].h + (roadpoints[end_point_idx].h - roadpoints[start_point_idx].h) * (2.0f / 3.0f)) * 0.3f)+(roadpoints[looping_index].h*0.7f);

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = start_point_idx;
            roadparts[roadparts_len].p2 = p_mid1;
            roadparts[roadparts_len].width = 15.0f;
            roadparts_len++;

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = p_mid1;
            roadparts[roadparts_len].p2 = p_mid2;
            roadparts[roadparts_len].width = 15.0f;
            roadparts_len++;

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = p_mid2;
            roadparts[roadparts_len].p2 = end_point_idx;
            roadparts[roadparts_len].width = 15.0f;
            roadparts_len++;

            float sign_angle = get_angle((float)roadpoints[looping_index].x,
                                        (float)roadpoints[looping_index].y,
                                        (float)roadpoints[start_point_idx].x,
                                        (float)roadpoints[start_point_idx].y) * RAD_DEG;
            
            add_stopsign((float)roadpoints[looping_index].y,
                        roadpoints[looping_index].h,
                        (float)roadpoints[looping_index].x,
                        sign_angle);
            uspech=true;

            break;
        }

        roadpoints.push_back({});
        roadparts.push_back({});
        roadpoints[roadpoints_len].x = next.x;
        roadpoints[roadpoints_len].y = next.z;
        roadpoints[roadpoints_len].h = next.y;
        roadparts[roadparts_len].p1    = roadpoints_len - 1;
        roadparts[roadparts_len].p2    = roadpoints_len;
        roadparts[roadparts_len].width = 15.0f;
        roadpoints_len++;
        roadparts_len++;
        iterations++;

        last_angle = get_angle(last_pos.x, last_pos.z, next.x, next.z);
        last_pos   = next;
    }

    // Pokud se cesta zdárně dokončila, provedeme vyhlazení (Subdivision)
    if (uspech) {
        Vec2 msp={road_start.x, road_start.z};
        Vec2 mep={road_end.x, road_end.z};
        major_road_points.push_back(msp);
        major_road_points.push_back(mep);

        int end_part_idx = (int)roadparts.size();
        limit_road_incline(start_part_idx, end_part_idx, 0.1f); // max sklon 0.5 = 26.565°

        for(int step = 0; step < ROAD_SUBDIVISIONS; step++) {
            subdivide_roads(start_part_idx+1, end_part_idx-1);
            end_part_idx = start_part_idx + (end_part_idx - start_part_idx) * 2;
        }
    } 
    else {
        // --- CELKOVÝ FAIL: KLÍČOVÝ ROLLBACK A ZAVOLÁNÍ FALLBACKU ---
        roadpoints_len = orig_roadpoints_len;
        roadparts_len = orig_roadparts_len;
        
        roadpoints.resize(orig_roadpoints_len);
        roadparts.resize(orig_roadparts_len);
        major_road_points.resize(orig_major_road_points_size);

        // Zde voláme fallback metodu. Pokud ti kompilátor stále píše "undeclared identifier",
        // ujisti se, že funkce `gen_road_fallback` je v souboru definovaná (nebo má prototyp) NAD funkcí `gen_road`.
        gen_road_fallback(road_start, road_end, width, roadpart_size);
    }
}
struct LocationScore {
    int x, y;
    float flatness; // Čím nižší hodnota, tím rovnější terén
};

void gen_villages(int num_villages, int houses_per_village) {
    // NASTAVENÍ Rozestupů a limitů
    float MIN_VILLAGE_DISTANCE = 80.0f; 
    float MIN_DIST_SQ = MIN_VILLAGE_DISTANCE * MIN_VILLAGE_DISTANCE;
    
    // Mírně zvýšíme toleranci, aby v horách vůbec nějaké místo prošlo,
    // ale pokud je to vyloženě kolmá stěna, tak tam stavět nebudeme.
    float MAX_ALLOWED_VARIANCE = 150.0f; 

    struct PlacedVillage { int x, y; };
    std::vector<PlacedVillage> placed_villages;

    // --- KLÍČOVÁ ZMĚNA: ROZLOŽENÍ NA MŘÍŽKU ---
    // Spočítáme, kolik buněk potřebujeme (např. pro 9 vesnic mřížka 3x3)
    int grid_size = (int)std::ceil(std::sqrt(num_villages));
    int cell_width = MAP_SIZE / grid_size;
    int cell_height = MAP_SIZE / grid_size;

    int RADIUS = 4;
    int check_step = 4; // Jemnější krok pro přesnější hledání v horách

    printf("[GEN VILLAGES] Generuji %d vesnic pomocí mřížky %dx%d...\n", num_villages, grid_size, grid_size);

    // Procházíme jednotlivé buňky mřížky po celé mapě
    for (int gY = 0; gY < grid_size; ++gY) {
        for (int gX = 0; gX < grid_size; ++gX) {
            
            // Pokud už máme požadovaný počet vesnic, můžeme skončit
            if ((int)placed_villages.size() >= num_villages) break;

            // Definice hranic aktuální buňky
            int start_x = std::max(RADIUS, gX * cell_width);
            int end_x   = std::min(MAP_SIZE - RADIUS, (gX + 1) * cell_width);
            int start_y = std::max(RADIUS, gY * cell_height);
            int end_y   = std::min(MAP_SIZE - RADIUS, (gY + 1) * cell_height);

            int best_x = -1;
            int best_y = -1;
            float best_flatness = 1e10f; // Hledáme MINIMUM variance

            // V rámci této buňky najdeme NEJROVNĚJŠÍ místo
            for (int y = start_y; y < end_y; y += check_step) {
                for (int x = start_x; x < end_x; x += check_step) {
                    
                    float avg_height = 0;
                    float variance = 0;
                    int count = 0;

                    for (int dy = -RADIUS; dy <= RADIUS; ++dy) {
                        for (int dx = -RADIUS; dx <= RADIUS; ++dx) {
                            avg_height += get_heightmap_pixel(x + dx, y + dy);
                            count++;
                        }
                    }
                    avg_height /= count;

                    for (int dy = -RADIUS; dy <= RADIUS; ++dy) {
                        for (int dx = -RADIUS; dx <= RADIUS; ++dx) {
                            float diff = get_heightmap_pixel(x + dx, y + dy) - avg_height;
                            variance += diff * diff;
                        }
                    }

                    // Pokud je místo rovnější než dosavadní nejlepší v této buňce
                    if (variance < best_flatness && variance < MAX_ALLOWED_VARIANCE) {
                        best_flatness = variance;
                        best_x = x;
                        best_y = y;
                    }
                }
            }

            // Pokud jsme v buňce našli aspoň trochu rozumné místo
            if (best_x != -1 && best_y != -1) {
                
                // Kontrola vzdálenosti vůči ostatním (pro jistotu na hranicích buněk)
                bool too_close = false;
                for (const auto& pv : placed_villages) {
                    float dx = best_x - pv.x;
                    float dy = best_y - pv.y;
                    if ((dx * dx + dy * dy) < MIN_DIST_SQ) {
                        too_close = true;
                        break;
                    }
                }
                if (too_close) continue;

                // --- STAVBA VESNICE V DANÉ BUŇCE ---
                int vx = best_x;
                int vy = best_y;

                make_village(vx, vy, houses_per_village);
                placed_villages.push_back({vx, vy});
                int current_village_idx = (int)placed_villages.size() - 1;

                // Spojení silnicí k nejbližšímu bodu
                int closest_index = -1;
                float min_dist = 1e10f;

                for (int j = 0; j < (int)roadpoints.size(); ++j) {
                    float dx = vx - roadpoints[j].x;
                    float dy = vy - roadpoints[j].y;
                    float dist_sq = dx*dx + dy*dy;

                    if (dist_sq < min_dist) {
                        min_dist = dist_sq;
                        closest_index = j;
                    }
                }

                if (closest_index != -1) {
                    printf("[GEN VILLAGES] Vesnice %d v buňce [%d,%d] připojena k roadpointu na (%d, %d)\n", 
                           current_village_idx, gX, gY, (int)roadpoints[closest_index].x, (int)roadpoints[closest_index].y);

                    Vec3 road_start = {(float)roadpoints[closest_index].x, (float)roadpoints[closest_index].h, (float)roadpoints[closest_index].y};
                    Vec3 road_end = {(float)vy, mapgen_get_heightmap_height(vx, vy), (float)vx};
                    
                    // road_start.y = mapgen_get_heightmap_height(road_start.z, road_start.x);
                    road_end.y   = mapgen_get_heightmap_height(road_end.z,   road_end.x);
                    
                    gen_road(road_end, road_start, 15.0f);

                    // Přidáme do sítě, aby se další buňka mohla napojit na tuto horskou silnici
                } else {
                    float hy = mapgen_get_heightmap_height(vx, vy);
                }
            }
        }
    }

    printf("[GEN VILLAGES] Hotovo. Rovnoměrně rozmístěno %zu vesnic.\n", placed_villages.size());
}
void gen_roads() {
    printf("generating roads\n");


    roadpoints_len = 0;
    roadparts_len = 0;

    std::vector<Vec3> road_starts;
    std::vector<Vec3> road_ends;
    // plan_road_network_2(3000.0f, road_starts, road_ends);
    Vec3 teststart={3000.0f, 400.0f, 3000.0f};
    Vec3 testend={7000.0f, 400.0f, 7000.0f};
    road_starts.push_back(teststart);
    road_ends.push_back(testend);
    testend={3000.0f, 400.0f, 3000.0f};
    teststart={5000.0f, 400.0f, 2000.0f};
    road_starts.push_back(teststart);
    road_ends.push_back(testend);

    for (i = 0; i < (int)road_starts.size(); i++) {
        progress_bar((float)i / (float)road_starts.size(), "Generating roads...");
        int start_part_idx = (int)roadparts.size();
        Vec3 road_start = road_starts[i];
        Vec3 road_end   = road_ends[i];
        road_start.y = mapgen_get_heightmap_height(road_start.z, road_start.x);
        road_end.y   = mapgen_get_heightmap_height(road_end.z,   road_end.x);
        gen_road(road_start, road_end, 15.0f);
        // int end_part_idx = (int)roadparts.size();

        // // Vyhladíme POUZE tuto nově vytvořenou cestu (celkem 10x jako v tvém kódu)
        // for(int step = 0; step < 2; step++) {
        //     subdivide_roads(start_part_idx, end_part_idx);
        //     // Po první subdivizi se počet úseků zdvojnásobí, musíme aktualizovat end_part_idx
        //     end_part_idx = start_part_idx + (end_part_idx - start_part_idx) * 2;
        // }
        
    }

    printf("generated %d roadpoints and %d roadparts\n", roadpoints_len, roadparts_len);
    gen_villages(15, 10);

}
void gen_chunk(int cz, int cx){
    gen_chunk_heightmap(cz,cx);
    chunk* c = chunks[cz][cx];
}
