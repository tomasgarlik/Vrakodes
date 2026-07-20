#ifndef INCLUDES
#define INCLUDES
#include "types.hpp"
#include "global_vars.hpp"
#include "constants.h"
#endif
//sphere data: x,y,z,radius
//cylinder data: x1,y1,z1,x2,y2,z2,radius
//line data: x1,y1,z1,x2,y2,z2, thickness
//cube data: x,y,z,sx,sy,sz
//mesh data (triangles): x11,y11,z11,x12,y12,z12,x13,y13,z13,     x21,y21,z21,x22,y22,z22,x23,y23,z23, and so on for each triangle


void debug_set_color(float r, float g, float b) {
    debug_data.push_back({DEBUG_COLOR, {r, g, b}});
}

void debug_draw_sphere(Vec3 pos, float radius) {
    debug_data.push_back({DEBUG_SPHERE, {pos.x, pos.y, pos.z, radius}});
}

void debug_draw_line(Vec3 p1, Vec3 p2, float thickness) {
    debug_data.push_back({DEBUG_LINE, {p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, thickness}});
}

void debug_draw_cylinder(Vec3 p1, Vec3 p2, float radius) {
    debug_data.push_back({DEBUG_CYLINDER, {p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, radius}});
}

void debug_draw_cube(Vec3 pos, Vec3 size) {
    debug_data.push_back({DEBUG_CUBE, {pos.x, pos.y, pos.z, size.x, size.y, size.z}});
}

void debug_draw_triangle(Vec3 v1, Vec3 v2, Vec3 v3) {
    debug_data.push_back({DEBUG_MESH, {v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z}});
}

void debug_draw_road_mesh() {
    if (road_debug_triangles.empty() && road_debug_segments.empty()) return;

    const float triangle_draw_dist = 100.0f;
    const float roadpart_draw_dist = 200.0f;

    debug_set_color(0.0f, 1.0f, 0.0f);
    for (const auto& tri : road_debug_triangles) {
        Vec3 center = (tri.a + tri.b + tri.c) * (1.0f / 3.0f);
        float dx = center.x - x_pos;
        float dy = center.y - y_pos;
        float dz = center.z - z_pos;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (dist > triangle_draw_dist) continue;

        debug_draw_triangle(tri.a, tri.b, tri.c);
        debug_draw_line(tri.a, tri.b, 0.3f);
        debug_draw_line(tri.b, tri.c, 0.3f);
        debug_draw_line(tri.c, tri.a, 0.3f);
    }

    debug_set_color(1.0f, 0.0f, 0.0f);
    for (const auto& seg : road_debug_segments) {
        float dx = seg.a.x - x_pos;
        float dy = seg.a.y - y_pos;
        float dz = seg.a.z - z_pos;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (dist > roadpart_draw_dist) continue;
        debug_draw_line(seg.a, seg.b, 0.5f);
    }
}

void draw_car_debug() {
    if (debug == 0) return;
    if (debug == 6){
        debug_draw_road_mesh();
    }
    for (auto& chal:chaloupky){
        debug_data.push_back({DEBUG_COLOR, {0.8f,0.8f,0.2f}});
        // debug_data.push_back({DEBUG_SPHERE, {chal.y, 50.0f, chal.x, 50.0f}});
        debug_draw_cylinder({chal.y, 0.0f, chal.x}, {chal.y, 2000.0f, chal.x}, 20.0f);
    }
    for (auto& chal:major_road_points){
        debug_data.push_back({DEBUG_COLOR, {0.2f,0.2f,1.0f}});
        // debug_data.push_back({DEBUG_SPHERE, {chal.y, 50.0f, chal.x, 50.0f}});
        debug_draw_cylinder({chal.y, 0.0f, chal.x}, {chal.y, 3000.0f, chal.x}, 15.0f);
    }
    if (debug==5){
        debug_set_color(1.0f, 0.0f, 0.0f);
        int chunks_drawn=10;
        for (i=((int)(x_pos/50.0f)*50)-50*chunks_drawn;i<((int)(x_pos/50.0f)*50)+50*chunks_drawn;i+=50){
            for (j=((int)(x_pos/50.0f)*50)-50*chunks_drawn;j<((int)(z_pos/50.0f)*50)+50*chunks_drawn;j+=50){
                Vec3 start={(float)i,0.0f,(float)j};
                Vec3 end={(float)i,1000.0f,(float)j};
                debug_draw_cylinder(start, end, 0.5f);
            }
        }
    }
    if (debug!=3){
        // Nastavíme barvu pro kolizní boxy (např. červená, ať je to dobře vidět)
        debug_set_color(0.0f, 1.0f, 0.0f);

        int chunks_drawn = 2; // Rozsah chunků kolem hráče
        // Velikost jednoho chunku předpokládám 50.0f podle tvého kódu
        float chunk_size = 50.0f; 

        int start_x_meter = ((int)(x_pos / chunk_size) * chunk_size) - chunk_size * chunks_drawn;
        int end_x_meter   = ((int)(x_pos / chunk_size) * chunk_size) + chunk_size * chunks_drawn;
        int start_z_meter = ((int)(z_pos / chunk_size) * chunk_size) - chunk_size * chunks_drawn;
        int end_z_meter   = ((int)(z_pos / chunk_size) * chunk_size) + chunk_size * chunks_drawn;

        for (int i = start_x_meter; i < end_x_meter; i += (int)chunk_size) {
            for (int j = start_z_meter; j < end_z_meter; j += (int)chunk_size) {
                
                // Převod světových souřadnic (i, j) na indexy do pole chunks[cz][cx]
                // POZOR: Uprav si dělení podle toho, jak reálně převádíš metry na indexy chunků!
                int cx = i / (int)chunk_size;
                int cz = j / (int)chunk_size;

                if (cx < 0 || cz < 0 || cx >= CHUNKS_SIZE || cz >= CHUNKS_SIZE) continue;
                chunk* target = chunks[cz][cx];
                if (target == NULL) continue;

                for (int b_idx = 0; b_idx < target->collision_boxes_count; b_idx++) {
                    collision_box& b = target->collision_boxes[b_idx];
                    
                    float mid_x = (b.sx + b.ex) * 0.5f;
                    float mid_y = (b.sy + b.ey) * 0.5f;
                    float mid_z = (b.sz + b.ez) * 0.5f;

                    float hx = (b.ex - b.sx) * 0.5f;
                    float hy = (b.ey - b.sy) * 0.5f;
                    float hz = (b.ez - b.sz) * 0.5f;

                    float sx = sinf(b.rx), cx_f = cosf(b.rx);
                    float sy = sinf(b.ry), cy = cosf(b.ry);
                    float sz = sinf(b.rz), cz_f = cosf(b.rz);

                    float vx[8] = {-hx,  hx,  hx, -hx, -hx,  hx,  hx, -hx};
                    float vy[8] = {-hy, -hy,  hy,  hy, -hy, -hy,  hy,  hy};
                    float vz[8] = { hz,  hz,  hz,  hz, -hz, -hz, -hz, -hz};

                    Vec3 v[8];
                    for(int v_idx=0; v_idx<8; v_idx++) {
                        float x = vx[v_idx];
                        float y = vy[v_idx];
                        float z = vz[v_idx];

                        // 1. Rotace Y
                        float x1 = x*cy + z*sy;
                        float z1 = -x*sy + z*cy;
                        x = x1; z = z1;

                        // 2. Rotace X
                        float y1 = y*cx_f - z*sx;
                        float z2 = y*sx + z*cx_f;
                        y = y1; z = z2;

                        // 3. Rotace Z
                        float x2 = x*cz_f - y*sz;
                        float y2 = x*sz + y*cz_f;
                        x = x2; y = y2;

                        v[v_idx].x = x + mid_x;
                        v[v_idx].y = y + mid_y;
                        v[v_idx].z = z + mid_z;
                    }

                    // Kreslení hran pomocí tvé moderní funkce debug_draw_line / debug_draw_cylinder
                    // Pokud nemáš debug_draw_line, můžeš použít tenký debug_draw_cylinder(start, end, 0.05f)
                    auto draw_edge = [](Vec3 s, Vec3 e) {
                        debug_draw_cylinder(s, e, 0.05f); 
                    };

                    // Spodní podstava
                    draw_edge(v[0], v[1]); draw_edge(v[1], v[5]);
                    draw_edge(v[5], v[4]); draw_edge(v[4], v[0]);
                    
                    // Horní podstava
                    draw_edge(v[3], v[2]); draw_edge(v[2], v[6]);
                    draw_edge(v[6], v[7]); draw_edge(v[7], v[3]);
                    
                    // Svislé stojny
                    draw_edge(v[0], v[3]); draw_edge(v[1], v[2]);
                    draw_edge(v[5], v[6]); draw_edge(v[4], v[7]);
                }
            }
        }
    }
    
    for (int meh = 0; meh < (int)cars.size(); meh++) {

        // --- joints ---
        for (int ii = 0; ii < cars[meh].joints_count; ii++) {
            float r,g,b;
            if      (cars[meh].joints[ii].group==0){r=0.0f;g=0.0f;b=0.4f;}
            else if (cars[meh].joints[ii].group==1){r=0.8f;g=0.7f;b=0.0f;}
            else if (cars[meh].joints[ii].group==2){r=0.3f;g=0.0f;b=0.3f;}
            else if (cars[meh].joints[ii].group==3){r=0.0f;g=1.0f;b=1.0f;}
            else if (cars[meh].joints[ii].group==4){r=0.0f;g=1.0f;b=0.0f;}
            else if (cars[meh].joints[ii].group==5){r=1.0f;g=0.0f;b=0.0f;}
            else                                   {r=1.0f;g=1.0f;b=1.0f;}
            if (cars[meh].joints[ii].clamped){r=1.0f;g=1.0f;b=1.0f;}

            debug_data.push_back({DEBUG_COLOR, {r,g,b}});
            debug_data.push_back({DEBUG_LINE, {
                cars[meh].points[cars[meh].joints[ii].p1].x,
                cars[meh].points[cars[meh].joints[ii].p1].y,
                cars[meh].points[cars[meh].joints[ii].p1].z,
                cars[meh].points[cars[meh].joints[ii].p2].x,
                cars[meh].points[cars[meh].joints[ii].p2].y,
                cars[meh].points[cars[meh].joints[ii].p2].z,
                0.01f
            }});
        }

        if (debug != 3) {
            debug_set_color(1.0f, 0.0f, 0.0f);
            // 1. Spočítáme surový vektor pohybu (rozdíl pozic)
float dx = cars[meh].pos_x - cars[meh].oldx;
float dy = cars[meh].pos_y - cars[meh].oldy;
float dz = cars[meh].pos_z - cars[meh].oldz;

// 2. Spočítáme délku tohoto vektoru (vzdálenost, kterou auto urazilo)
float distance = sqrtf(dx*dx + dy*dy + dz*dz);

// Výchozí koncové body (pokud auto úplně stojí, čára ukáže aspoň kousek dopředu)
float target_x = cars[meh].pos_x;
float target_y = cars[meh].pos_y;
float target_z = cars[meh].pos_z;

// Fixní délka debug čáry ve světě (např. 4 metry / jednotky)
float line_length = 20.0f; 

// 3. Pokud se auto hýbe, normalizujeme vektor a protáhneme ho o line_length
if (distance > 0.0001f) {
    target_x += (dx / distance) * line_length;
    target_y += (dy / distance) * line_length;
    target_z += (dz / distance) * line_length;
} else {
    // Nouzovka: Pokud auto absolutně stojí, můžeme čáru vykreslit 
    // ve směru, kam auto kouká (pokud máš uložený forward vektor),
    // nebo ji nechat nulovou. Nechme ji nulovou, ať je jasné, že stojí.
}

// 4. Poslat do debug dat
debug_draw_line({cars[meh].pos_x, cars[meh].pos_y, cars[meh].pos_z},
                {target_x, target_y, target_z},
                0.05f);
            // --- collision faces ---
            for (int ii = 0; ii < cars[meh].col_faces_count; ii++) {
                face& f = cars[meh].col_faces[ii];
                bool collided = std::find(cars_collided_faces[meh].begin(),
                                          cars_collided_faces[meh].end(), ii)
                                != cars_collided_faces[meh].end();
                if (collided) debug_data.push_back({DEBUG_COLOR, {0.8f,0.2f,0.2f}});
                else          debug_data.push_back({DEBUG_COLOR, {0.2f,0.8f,0.2f}});

                point& v0 = cars[meh].points[f.vertices[0]];
                point& v1 = cars[meh].points[f.vertices[1]];
                point& v2 = cars[meh].points[f.vertices[2]];
                debug_data.push_back({DEBUG_MESH, {
                    v0.x,v0.y,v0.z,
                    v1.x,v1.y,v1.z,
                    v2.x,v2.y,v2.z
                }});
            }

            // --- volume spheres ---
            debug_data.push_back({DEBUG_COLOR, {0.1f,0.2f,1.0f}});
            for (int prd = 0; prd < cars[meh].volumes_count; prd++) {
                debug_data.push_back({DEBUG_SPHERE, {
                    cars[meh].volume_poses[prd].x,
                    cars[meh].volume_poses[prd].y,
                    cars[meh].volume_poses[prd].z,
                    0.07f
                }});
            }
        }

        // --- tetrahedra (debug==4) ---
        if (debug == 4) {
            debug_data.push_back({DEBUG_COLOR, {0.8f,0.3f,0.3f}});
            for (int t = 0; t < cars[meh].tetrahedra_count; t++) {
                tetrahedron& tet = cars[meh].tetrahedra[t];
                Vec3 A = {cars[meh].points[tet.idx[0]].x, cars[meh].points[tet.idx[0]].y, cars[meh].points[tet.idx[0]].z};
                Vec3 B = {cars[meh].points[tet.idx[1]].x, cars[meh].points[tet.idx[1]].y, cars[meh].points[tet.idx[1]].z};
                Vec3 C = {cars[meh].points[tet.idx[2]].x, cars[meh].points[tet.idx[2]].y, cars[meh].points[tet.idx[2]].z};
                Vec3 D = {cars[meh].points[tet.idx[3]].x, cars[meh].points[tet.idx[3]].y, cars[meh].points[tet.idx[3]].z};
                // 4 faces of the tetrahedron
                debug_data.push_back({DEBUG_MESH, {B.x,B.y,B.z, C.x,C.y,C.z, D.x,D.y,D.z}});
                debug_data.push_back({DEBUG_MESH, {A.x,A.y,A.z, C.x,C.y,C.z, D.x,D.y,D.z}});
                debug_data.push_back({DEBUG_MESH, {A.x,A.y,A.z, B.x,B.y,B.z, D.x,D.y,D.z}});
                debug_data.push_back({DEBUG_MESH, {A.x,A.y,A.z, B.x,B.y,B.z, C.x,C.y,C.z}});
            }
        }

    }
}
