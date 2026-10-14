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

    // --- helpery pro tiling mapy ---
    // zabalí souřadnici do rozsahu [0, size) i pro záporná čísla
    auto wrap_index = [](int v, int size) -> int {
        int r = v % size;
        if (r < 0) r += size;
        return r;
    };
    // dělení zaokrouhlené dolů (na rozdíl od / v C++, které zaokrouhluje k nule)
    auto floor_div = [](int a, int b) -> int {
        int q = a / b;
        int r = a % b;
        if (r != 0 && ((r < 0) != (b < 0))) q--;
        return q;
    };
    // zabalení souřadnice přímo do rozměru heightmapy
    auto wrap_map = [&](int v) -> int { return wrap_index(v, MAP_SIZE); };

    int levels=render_distance/terrain_lod_level_size;
    if (levels<1){levels=1;}
    int start_dist=0;
    int end_dist;
    int now_tex_pres=tex_pres;
    int base_tex_pres = tex_pres; // referenční rozlišení pro škálování UV
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

        // Ořezávání na hranice mapy je pryč - mapa se místo toho tiluje (viz wrap_map níže),
        // takže i/j klidně přetečou do mínusu nebo za MAP_SIZE.

        std::vector<intVec2> special_chunks;
        if (level == 0) {
            int chunk_start_z = floor_div(start_i, CHUNK_SIZE);
            int chunk_end_z   = floor_div(end_i - 1, CHUNK_SIZE);
            int chunk_start_x = floor_div(start_j, CHUNK_SIZE);
            int chunk_end_x   = floor_div(end_j - 1, CHUNK_SIZE);
            for (int cz = chunk_start_z; cz <= chunk_end_z; cz++) {
                int wcz = wrap_index(cz, CHUNKS_SIZE);
                for (int cx = chunk_start_x; cx <= chunk_end_x; cx++) {
                    int wcx = wrap_index(cx, CHUNKS_SIZE);
                    int chunk_tex_pres = chunks[wcz][wcx]->tex_pres;
                    if (chunk_tex_pres > 0 && chunk_tex_pres != tex_pres) {
                        special_chunks.push_back({cx, cz});
                    }
                }
            }
        }

        for (k=0;k<10;k++){
            mesh m;
            m.castShadow = false;
            m.texture=textures[k];
            m.x=0.0f;
            m.y=0.0f;
            m.z=0.0f;

            if (level == 0) {
                int chunk_start_z = floor_div(start_i, CHUNK_SIZE);
                int chunk_end_z   = floor_div(end_i - 1, CHUNK_SIZE);
                int chunk_start_x = floor_div(start_j, CHUNK_SIZE);
                int chunk_end_x   = floor_div(end_j - 1, CHUNK_SIZE);
                for (int cz = chunk_start_z; cz <= chunk_end_z; cz++) {
                    int wcz = wrap_index(cz, CHUNKS_SIZE);
                    for (int cx = chunk_start_x; cx <= chunk_end_x; cx++) {
                        int wcx = wrap_index(cx, CHUNKS_SIZE);
                        int chunk_tex_pres = chunks[wcz][wcx]->tex_pres;
                        if (chunk_tex_pres <= 0) chunk_tex_pres = tex_pres;

                        // pokud má chunk jiné rozlišení než okolí, přesahujeme o 1 čtverec
                        // jeho mřížky na každou stranu, aby se zacelily díry na hranici LODů
                        int extend = (chunk_tex_pres != tex_pres) ? chunk_tex_pres : 0;

                        int chunk_z0 = cz * CHUNK_SIZE - extend;
                        int chunk_z1 = cz * CHUNK_SIZE + CHUNK_SIZE + extend;
                        int chunk_x0 = cx * CHUNK_SIZE - extend;
                        int chunk_x1 = cx * CHUNK_SIZE + CHUNK_SIZE + extend;

                        int render_start_i = std::max(start_i, chunk_z0);
                        int render_end_i   = std::min(end_i, chunk_z1);
                        int render_start_j = std::max(start_j, chunk_x0);
                        int render_end_j   = std::min(end_j, chunk_x1);

                        for (i = render_start_i; i < render_end_i; i += chunk_tex_pres) {
                            for (j = render_start_j; j < render_end_j; j += chunk_tex_pres) {
                                float diff_i = (float)i - z_pos;
                                float diff_j = (float)j - x_pos;
                                float dist = sqrtf(diff_i * diff_i + diff_j * diff_j);
                                if (dist < (float)start_dist || dist >= (float)end_dist) {
                                    continue;
                                }

                                // zabalené souřadnice pro čtení z heightmapy (tiling)
                                aaa = wrap_map(i);
                                bbb = wrap_map(j);
                                if (get_heightmap_texture(bbb, aaa) != k) {
                                    continue;
                                }

                                int aaa2 = wrap_map(i + chunk_tex_pres);
                                int bbb2 = wrap_map(j + chunk_tex_pres);

                                // p.x/p.z zůstávají v NEzabalených, skutečných světových souřadnicích,
                                // takže se terén ve světě opakuje, ale plynule (žádný švy/skoky)
                                p1.x = j + chunk_tex_pres;
                                p1.y = get_heightmap_height((float)bbb2, (float)aaa2);
                                p1.z = i + chunk_tex_pres;

                                p2.x = j;
                                p2.y = get_heightmap_height((float)bbb, (float)aaa2);
                                p2.z = i + chunk_tex_pres;

                                p3.x = j;
                                p3.y = get_heightmap_height((float)bbb, (float)aaa);
                                p3.z = i;

                                p4.x = j + chunk_tex_pres;
                                p4.y = get_heightmap_height((float)bbb2, (float)aaa);
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
                                // m.uvs.push_back({0.0f,      (float)chunk_tex_pres});
                                // m.uvs.push_back({(float)chunk_tex_pres,  (float)chunk_tex_pres});
                                // m.uvs.push_back({(float)chunk_tex_pres,  0.0f});
                                // m.uvs.push_back({0.0f,      0.0f});
                                m.uvs.push_back({0.0f, 1.0f});
                                m.uvs.push_back({1.0f, 1.0f});
                                m.uvs.push_back({1.0f, 0.0f});
                                m.uvs.push_back({0.0f, 0.0f});
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

                        aaa = wrap_map(i);
                        bbb = wrap_map(j);
                        if (get_heightmap_texture(bbb, aaa) != k) {
                            continue;
                        }

                        int aaa2 = wrap_map(i + now_tex_pres);
                        int bbb2 = wrap_map(j + now_tex_pres);

                        p1.x = j + now_tex_pres;
                        p1.y = get_heightmap_height((float)bbb2, (float)aaa2);
                        p1.z = i + now_tex_pres;

                        p2.x = j;
                        p2.y = get_heightmap_height((float)bbb, (float)aaa2);
                        p2.z = i + now_tex_pres;

                        p3.x = j;
                        p3.y = get_heightmap_height((float)bbb, (float)aaa);
                        p3.z = i;

                        p4.x = j + now_tex_pres;
                        p4.y = get_heightmap_height((float)bbb2, (float)aaa);
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
                        // m.uvs.push_back({0.0f,      (float)now_tex_pres});
                        // m.uvs.push_back({(float)now_tex_pres,  (float)now_tex_pres});
                        // m.uvs.push_back({(float)now_tex_pres,  0.0f});
                        // m.uvs.push_back({0.0f,      0.0f});
                        m.uvs.push_back({0.0f, 1.0f});
                        m.uvs.push_back({1.0f, 1.0f});
                        m.uvs.push_back({1.0f, 0.0f});
                        m.uvs.push_back({0.0f, 0.0f});
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
    add_center_lines_to_road(0.1f);
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
                // preserve the original object's shadow casting flag
                // and allow explicit castShadow=false for cubes/objects
                // instead of forcing all objects to cast shadows.
                // m.castShadow = true;
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
void create_cube(float x, float y, float z, float scx, float scy, float scz, float rx, float ry, float rz, int tex_id, bool castShadow=true){
    rx/=RAD_DEG;
    ry/=RAD_DEG;
    rz/=RAD_DEG;
    mesh m;
    m.texture=tex_id;
    m.texture_type=1;
    m.castShadow = castShadow;
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
void create_cube(float x, float y, float z, float scx, float scy, float scz, float rx, float ry, float rz, int tex_id, float friction, bool castShadow=true){
    rx/=RAD_DEG;
    ry/=RAD_DEG;
    rz/=RAD_DEG;
    mesh m;
    m.texture=tex_id;
    m.texture_type=1;
    m.castShadow = castShadow;
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
        //green walls on the slope
        for (i=0;i<10;i++){
            float offy=33.3f;
            float offz=66.6f;
            create_cube(110.0f+(float)i*3.0f,50.5f+offy,120.0f-offz,  0.1f,1.0f,74.5f, 26.57f, 0.0f, 0.0f, 2, 0.9f, false);
            create_cube(110.0f+(float)i*3.0f,50.5f,120.0f,            0.1f,1.0f,74.5f, 26.57f, 0.0f, 0.0f, 2, 0.9f, false);
            create_cube(110.0f+(float)i*3.0f,50.5f-offy,120.0f+offz,  0.1f,1.0f,74.5f, 26.57f, 0.0f, 0.0f, 2, 0.9f, false);
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
        create_cube(75.0f,0.0f,120.0f, 50.0f, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 1,1.0f, false);

        // the right wall next to the slope
        create_cube(108.9f,1.0f,25.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(108.9f,1.0f,75.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(108.9f,1.0f,125.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(108.9f,1.0f,175.0f, 0.5f,2.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(108.9f,1.0f,208.0f, 0.5f,2.0f,16.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        
        // the left wall next to the slope
        create_cube(150.9f,2.0f,25.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(150.9f,2.0f,75.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(150.9f,2.0f,125.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(150.9f,2.0f,175.0f, 0.5f,4.0f,50.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);
        create_cube(150.9f,2.0f,208.0f, 0.5f,4.0f,16.0f, 0.0f,0.0f,0.0f, 3, 0.5f, false);

        // those goofyahh ramps with walls
        create_ramp(170.0f,0.0f,  30.0f,  40,  40.0f, 1.0f, 1.5f, 90.0f, 180.0f, 0.5f);
        create_cube(170.0f,80.0f, -7.6f,       40.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f, false);
        create_ramp(170.0f,120.0f,-7.6f,  40, 40.0f, 1.6f, 1.5f, 90.0f, 180.0f, 0.5f, 90.0f);
        create_cube(170.0f,100.0f,200.0f,     40.0f, 80.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f, false);

        create_ramp(200.0f,0.0f, 30.0f,  20,  20.0f, 1.0f, 1.5f, 90.0f, 180.0f, 0.5f);
        create_cube(200.0f,40.0f,11.6f,       20.0f, 50.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f, false);
        create_ramp(200.0f,60.0f,11.6f,  20,  20.0f, 1.6f, 1.5f, 90.0f, 180.0f, 0.5f, 90.0f);
        create_cube(200.0f,60.0f,100.0f,      20.0f, 40.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f, false);

        create_ramp(215.0f,0.0f, 30.0f,  10,  10.0f, 1.0f, 1.5f, 90.0f, 180.0f, 0.5f);
        create_cube(215.0f,20.0f,21.2f,       10.0f, 25.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f, false);
        create_ramp(215.0f,30.0f,21.2f,  10,  10.0f, 1.6f, 1.5f, 90.0f, 180.0f, 0.5f, 90.0f);
        create_cube(215.0f,30.0f,50.0f,       10.0f, 20.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3,0.5f, false);

        // the wall behind the huge slope
        create_cube(130,5.0f, 0.5f,  80.0f, 11.0f, 1.0f, 0.0f, 0.0f, 0.0f, 3, 0.5f, false);
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
void clear_chunk(int cz, int cx, bool permanent_ones=false){
    chunks[cz][cx]->collision_boxes_count=0;
    chunks[cz][cx]->objects.clear();
    chunks[cz][cx]->object_poses.clear();
    if (permanent_ones){
        chunks[cz][cx]->permanent=false;
    }
    chunks[cz][cx]->tex_pres=-1;
}
void clear_chunk_heightmap(int cz, int cx){
    // chunks[cz][cx]->heightmap.clear();
    // chunks[cz][cx]->heightmap_tex.clear();
    chunks[cz][cx]->loaded=false;
    chunks[cz][cx]->generated=false;
}
void clear_chunks_heightmap(){
    printf("clearing chunks heightmap... \n");
    int step=CHUNKS_SIZE/20;
    int idx=0;
    for (i=0;i<CHUNKS_SIZE;i++){
        for (j=0;j<CHUNKS_SIZE;j++){
            if (!chunks[i][j]->permanent){
                clear_chunk_heightmap(i,j);
            }
        }
        idx++;
        if (idx>step){
            idx=0;
            progress_bar((float)i/(float)CHUNKS_SIZE, "clearing map...");
        }
    }
}
void clear_chunks(bool completely=true){
    printf("clearing chunks... \n");
    for (i=0;i<CHUNKS_SIZE;i++){
        for (j=0;j<CHUNKS_SIZE;j++){
            clear_chunk(i,j, completely);
        }   
    }
    clear_chunks_heightmap();
    chaloupky.clear();
    chunks_loaded=0;
}
void unload_chunk(int cz,int cx){
    clear_chunk(cz,cx,false);
    clear_chunk_heightmap(cx,cz);
}
void lhh(std::string text){
    // text="[ic] "+text;
    // log(text);
}
inline int get_wrapped_chunk_idx(float pos, float chunk_size, int map_chunks_size) {
    int idx = static_cast<int>(std::floor(pos / chunk_size)) % map_chunks_size;
    if (idx < 0) idx += map_chunks_size; // Ošetření záporných souřadnic
    return idx;
}

// Pomocná funkce: Nejkratší vzdálenost mezi dvěma indexy na zacyklené mřížce
inline int get_wrapped_dist(int a, int b, int map_chunks_size) {
    int d = std::abs(a - b) % map_chunks_size;
    return (d > map_chunks_size / 2) ? (map_chunks_size - d) : d;
}
struct UnloadCandidate {
    int cz;
    int cx;
    int dist_player;
    bool near_car;
};

void auto_unload_chunks() {
    // 1. Ošetření marginu
    float margin = chunks_unload_margin;
    if (margin > 1.0f) margin /= 100.0f;

    int target_max_loaded = static_cast<int>(max_chunks_loaded * (1.0f - margin));
    if (target_max_loaded < 0) target_max_loaded = 0;

    // Pokud jsme pod limitem, nic nemazat
    if (chunks_loaded <= target_max_loaded) {
        return;
    }

    int needed_to_unload = chunks_loaded - target_max_loaded;

    // Souřadnice hráče v mřížce
    int player_cx = get_wrapped_chunk_idx(x_pos, CHUNK_SIZE, CHUNKS_SIZE);
    int player_cz = get_wrapped_chunk_idx(z_pos, CHUNK_SIZE, CHUNKS_SIZE);

    std::vector<UnloadCandidate> candidates;
    candidates.reserve(chunks_loaded); // Rezervace paměti pro rychlost

    // 2. POSBÍRÁME VŠECHNY NAČTENÉ A NE-PERMANENTNÍ CHUNKY
    for (int cz = 0; cz < CHUNKS_SIZE; ++cz) {
        for (int cx = 0; cx < CHUNKS_SIZE; ++cx) {
            
            if (!chunks[cz][cx]->loaded || chunks[cz][cx]->permanent) {
                continue;
            }

            // Vzdálenost od hráče (Čebyševova vzdálenost = čtverec kolem hráče)
            int dx = get_wrapped_dist(cx, player_cx, CHUNKS_SIZE);
            int dz = get_wrapped_dist(cz, player_cz, CHUNKS_SIZE);
            int dist_player = std::max(dx, dz);

            // Kontrola aut
            bool near_car = false;
            for (size_t i = 0; i < cars.size(); ++i) {
                int car_cx = get_wrapped_chunk_idx(cars[i].pos_x, CHUNK_SIZE, CHUNKS_SIZE);
                int car_cz = get_wrapped_chunk_idx(cars[i].pos_z, CHUNK_SIZE, CHUNKS_SIZE);

                if (get_wrapped_dist(cx, car_cx, CHUNKS_SIZE) <= 1 &&
                    get_wrapped_dist(cz, car_cz, CHUNKS_SIZE) <= 1) {
                    near_car = true;
                    break;
                }
            }

            candidates.push_back({cz, cx, dist_player, near_car});
        }
    }

    // Pokud fyzicky nemáme žádné chunky na smazání, končíme
    if (candidates.empty()) {
        printf("[UNLOAD WARNING] Zadne chunky k dispozici pro smazani!\n");
        return;
    }

    // 3. SEŘADÍME KANDIDÁTY OD NEJMÉNĚ DŮLEŽITÉHO (Ty půjdou na řadu první)
    std::sort(candidates.begin(), candidates.end(), [](const UnloadCandidate& a, const UnloadCandidate& b) {
        // Priorita 1: Chunky MIMO auta mají přednost ke smazání
        if (a.near_car != b.near_car) {
            return !a.near_car; // false (NENÍ u auta) jde dopředu
        }
        // Priorita 2: Větší vzdálenost od hráče má přednost ke smazání
        return a.dist_player > b.dist_player;
    });

    // 4. GARANTOVANÉ PROVEDENÍ MAZÁNÍ
    int unloaded_count = 0;
    for (const auto& cand : candidates) {
        if (unloaded_count >= needed_to_unload) {
            break;
        }

        unload_chunk(cand.cz, cand.cx);
        unloaded_count++;
    }
    chunks_loaded-=unloaded_count;
    printf("[UNLOAD SUCCESS] Potreba: %d | Nalezeno kandidatu: %zu | Skutecne smazano: %d\n", 
            needed_to_unload, candidates.size(), unloaded_count);
}
bool init_chunk(int cx, int cy, int pres=1){
    chunks[cx][cy]->pres=1;
    pres=CHUNK_SIZE/pres;
    log("init chunk at "+std::to_string(cx)+", "+std::to_string(cy));
    if (chunks[cx][cy]->loaded){
        log("this one is already loaded");
    }
    if (chunks_loaded==max_chunks_loaded){
        // auto_unload_chunks();
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

    unsigned int seed = MAP_SEED;
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

struct LocationScore {
    int x, y;
    float flatness; // Čím nižší hodnota, tím rovnější terén
};
#include <vector>
#include <algorithm>
#include <cmath>

struct DSU {
    std::vector<int> parent;
    DSU(int n) {
        parent.resize(n);
        for (int i = 0; i < n; i++) parent[i] = i;
    }
    int find(int i) {
        if (parent[i] == i) return i;
        return parent[i] = find(parent[i]);
    }
    bool unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) {
            parent[root_i] = root_j;
            return true;
        }
        return false;
    }
};

struct RoadEdge {
    int u, v;
    float dist;
};

void gen_roads() {
    printf("[ROADS] Generuji chytrou síť se zkratkami...\n");

    roadpoints_len = 0;
    roadparts_len = 0;
    roadpoints.clear();
    roadparts.clear();

    std::vector<Vec3> nodes;

    // --- 1. DEFINICE PEVNÝCH HLAVNÍCH BODŮ A POVINNÝCH SPOJŮ ---
    Vec3 p1 = {3000.0f, 0.0f, 3000.0f};
    Vec3 p2 = {7000.0f, 0.0f, 7000.0f};
    Vec3 p3 = {5000.0f, 0.0f, 2000.0f};

    p1.y = mapgen_get_heightmap_height(p1.z, p1.x);
    p2.y = mapgen_get_heightmap_height(p2.z, p2.x);
    p3.y = mapgen_get_heightmap_height(p3.z, p3.x);

    nodes.push_back(p1); // Index 0
    nodes.push_back(p2); // Index 1
    // nodes.push_back(p3); // Index 2 //used to be used for intersection debug, is useless now

    // Definice povinné silnice, která MUSÍ zůstat (např. z p1 do p2)
    std::vector<std::pair<int, int>> compulsory_edges = {
        {0, 1}, // Pevná silnice z 3000,3000 do 7000,7000
        {2, 0}  // Pevná silnice z 5000,2000 do 3000,3000
    };

    // --- 2. VÝBĚR A POSTAVENÍ VESNIC ---
    int target_villages = 10;
    float min_dist_sq = 1000.0f * 1000.0f;

    for (const auto& cand : flattest_places) {
        if ((int)nodes.size() - 3 >= target_villages) break;

        bool too_close = false;
        for (const auto& node : nodes) {
            float dx = cand.x - node.x;
            float dz = cand.y - node.z;
            if ((dx * dx + dz * dz) < min_dist_sq) {
                too_close = true;
                break;
            }
        }
        if (too_close) continue;

        make_village(cand.x, cand.y, 10);
        nodes.push_back({(float)cand.y, cand.height, (float)cand.x});
    }

    int N = (int)nodes.size();
    if (N < 2) return;

    DSU dsu(N);
    std::vector<RoadEdge> final_network;

    // --- 3. APLIKACE POVINNÝCH SILNIC DO SÍTĚ ---
    for (const auto& comp : compulsory_edges) {
        if (comp.first < N && comp.second < N) {
            float dx = nodes[comp.first].x - nodes[comp.second].x;
            float dz = nodes[comp.first].z - nodes[comp.second].z;
            float d = sqrtf(dx * dx + dz * dz);

            dsu.unite(comp.first, comp.second);
            final_network.push_back({comp.first, comp.second, d});
        }
    }

    // --- 4. VYTVOŘENÍ A SEŘAZENÍ VŠECH MOŽNÝCH HRAN ---
    std::vector<RoadEdge> edges;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            float dx = nodes[i].x - nodes[j].x;
            float dz = nodes[i].z - nodes[j].z;
            edges.push_back({i, j, sqrtf(dx * dx + dz * dz)});
        }
    }

    std::sort(edges.begin(), edges.end(), [](const RoadEdge& a, const RoadEdge& b) {
        return a.dist < b.dist;
    });

    // --- 5. KRUSKALŮV MST PRO SPOJENÍ ZBYTKU SÍTĚ ---
    for (const auto& edge : edges) {
        if (dsu.unite(edge.u, edge.v)) {
            final_network.push_back(edge);
        }
    }

    // --- 6. ELIMINACE OB JÍŽDĚK T VARU "U" (DETOUR FACTOR SHORTCUTS) ---
    // Matice vzdáleností po silniční síti
    std::vector<std::vector<float>> graph_dist(N, std::vector<float>(N, 1e9f));

    for (int i = 0; i < N; i++) graph_dist[i][i] = 0.0f;
    for (const auto& e : final_network) {
        graph_dist[e.u][e.v] = e.dist;
        graph_dist[e.v][e.u] = e.dist;
    }

    // Pomocná lambda pro dopočítání nejkratších cest (Floyd-Warshall)
    auto update_shortest_paths = [&]() {
        for (int k = 0; k < N; k++) {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    if (graph_dist[i][k] + graph_dist[k][j] < graph_dist[i][j]) {
                        graph_dist[i][j] = graph_dist[i][k] + graph_dist[k][j];
                    }
                }
            }
        }
    };

    update_shortest_paths();

    // Hledání vrcholů "U", kde se vyplatí postavit zkratku
    for (const auto& edge : edges) {
        // Ignorujeme extrémně dlouhé přemosťovací cesty přes půl mapy (> 2500m)
        if (edge.dist > 2500.0f) continue;

        // Pokud je cesta po síti 1.8x delší než přímá vzdušná čára, přidáme zkratku!
        if (graph_dist[edge.u][edge.v] > edge.dist * 1.8f) {
            final_network.push_back(edge);
            graph_dist[edge.u][edge.v] = edge.dist;
            graph_dist[edge.v][edge.u] = edge.dist;
            update_shortest_paths(); // Aktualizujeme cestovní vzdálenosti
        }
    }

    // --- 7. GENEROVÁNÍ FYZICKÝCH SILNIC ---
    printf("[ROADS] Generuji %zu optimálních úseků včetně zkratek...\n", final_network.size());
    for (size_t i = 0; i < final_network.size(); i++) {
        progress_bar((float)i / (float)final_network.size(), "Generating road network...");
        gen_road(nodes[final_network[i].u], nodes[final_network[i].v], 8.0f);
    }

    printf("[ROADS] Hotovo. Vygenerováno %d bodů a %d úseků.\n", roadpoints_len, roadparts_len);
}
void gen_chunk(int cz, int cx){
    gen_chunk_heightmap(cz,cx);
    chunk* c = chunks[cz][cx];
}
