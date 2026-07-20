#ifndef INCLUDES
#define INCLUDES
#include <SDL_opengl.h>
#include <SDL_video.h>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <OpenGL/gl.h>
#include "constants.h"
#include "general_functions.hpp"
#include "types.hpp"
#include "global_vars.hpp"
#endif
// RoadVertex is defined in road_mesh.hpp (mesh generation)
bool get_barycentric(float x, float z, RoadVertex v1, RoadVertex v2, RoadVertex v3, float &out_h, float &out_w) {
    float det = (v2.z - v3.z) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.z - v3.z);
    if (fabsf(det) < 0.00001f) return false;

    float l1 = ((v2.z - v3.z) * (x - v3.x) + (v3.x - v2.x) * (z - v3.z)) / det;
    float l2 = ((v3.z - v1.z) * (x - v3.x) + (v1.x - v3.x) * (z - v3.z)) / det;
    float l3 = 1.0f - l1 - l2;

    // EPSILON PŘESAH: Povolíme pixelům, které jsou kousíček "venku", aby se ještě vykreslily
    float epsilon = -0.02f; 
    if (l1 >= epsilon && l1 <= 1.0f - epsilon && 
        l2 >= epsilon && l2 <= 1.0f - epsilon && 
        l3 >= epsilon && l3 <= 1.0f - epsilon) {
        
        // Bezpečné oříznutí souřadnic pro interpolaci, aby nevznikaly anomálie
        float cl1 = std::max(0.0f, std::min(1.0f, l1));
        float cl2 = std::max(0.0f, std::min(1.0f, l2));
        float cl3 = 1.0f - cl1 - cl2;

        out_h = cl1 * v1.h + cl2 * v2.h + cl3 * v3.h;
        out_w = cl1 * v1.weight + cl2 * v2.weight + cl3 * v3.weight;
        return true;
    }
    return false;
}
void rasterize_triangle(RoadVertex v1, RoadVertex v2, RoadVertex v3) {
    int minX = std::max(0, (int)std::floor(std::min({v1.x, v2.x, v3.x})));
    int maxX = std::min(MAP_SIZE - 1, (int)std::ceil(std::max({v1.x, v2.x, v3.x})));
    int minZ = std::max(0, (int)std::floor(std::min({v1.z, v2.z, v3.z})));
    int maxZ = std::min(MAP_SIZE - 1, (int)std::ceil(std::max({v1.z, v2.z, v3.z})));

    for (int z = minZ; z <= maxZ; z++) {
        for (int x = minX; x <= maxX; x++) {
            float h_interp, w_interp;
            if (get_barycentric((float)x, (float)z, v1, v2, v3, h_interp, w_interp)) {
                float blend_factor = w_interp;
                blend_factor = blend_factor * blend_factor * (3.0f - 2.0f * blend_factor);

                float orig_h = get_heightmap_height(x, z);
                float final_h = (h_interp * blend_factor) + (orig_h * (1.0f - blend_factor));

                if (blend_factor > 0.001f) {
                    set_heightmap_height(x, z, final_h);
                    set_chunk_permanency(x, z, true);
                    set_chunk_tex_pres(x,z,2);
                    if (w_interp > 0.85f) { 
                        set_heightmap_texture(x, z, 1); 
                    } else {
                        set_heightmap_texture(x, z, 6); 
                    }
                }
            }
        }
    }
}
void make_bridge(Vec3 start, Vec3 end, float width) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float dz = end.z - start.z;
    float length = sqrtf(dx * dx + dz * dz);
    if (length < 0.1f) return;

    // Rozumné rozsekání (např. 15m), žádný Xeon netřeba
    float max_seg_len = 15.0f;
    int num_segments = (int)ceilf(length / max_seg_len);
    if (num_segments < 1) num_segments = 1;

    for (int s = 0; s < num_segments; s++) {
        float t1 = (float)s / (float)num_segments;
        float t2 = (float)(s + 1) / (float)num_segments;

        Vec3 seg_start = { start.x + dx * t1, start.y + dy * t1, start.z + dz * t1 };
        Vec3 seg_end   = { start.x + dx * t2, start.y + dy * t2, start.z + dz * t2 };

        float sdx = seg_end.x - seg_start.x;
        float sdy = seg_end.y - seg_start.y;
        float sdz = seg_end.z - seg_start.z;
        float seg_len_3d = sqrtf(sdx * sdx + sdy * sdy + sdz * sdz);
        float seg_len_xz = sqrtf(sdx * sdx + sdz * sdz);

        Vec3 center = {
            seg_start.x + sdx * 0.5f,
            seg_start.y + sdy * 0.5f,
            seg_start.z + sdz * 0.5f
        };

        // --- MATEMATICKÁ EXTRAKCE ÚHLŮ PRO POŘADÍ Y -> X -> Z ---
        
        // 1. Cílová osa Z (Směr mostu dopředu)
        float m6 = sdx / seg_len_3d;
        float m7 = sdy / seg_len_3d;
        float m8 = sdz / seg_len_3d;

        // 2. Cílová osa X (Doprava - striktně horizontální, m1 = 0 zaručí nulový Roll)
        float m0 = sdz / seg_len_xz;
        float m1 = 0.0f;
        float m2 = -sdx / seg_len_xz;

        // 3. Cílová osa Y (Nahoru - křížový součin Z a X)
        float m3 = m7 * m2 - m8 * m1;
        float m4 = m8 * m0 - m6 * m2;
        float m5 = m6 * m1 - m7 * m0;

        // 4. Výpočet exaktních úhlů z tvé matice (update_collision_box)
        float rx_rad = asinf(m5);
        float cos_rx = cosf(rx_rad);

        float ry_rad = 0.0f;
        float rz_rad = 0.0f;

        if (fabsf(cos_rx) > 0.001f) {
            ry_rad = -atan2f(m2 / cos_rx, m8 / cos_rx);
            rz_rad = -atan2f(m3 / cos_rx, m4 / cos_rx);
        } else {
            // Gimbal lock pojistka
            ry_rad = -atan2f(-m6, m0);
            rz_rad = 0.0f;
        }

        // Převod na stupně pro create_cube
        float rx = rx_rad * (180.0f / M_PI);
        float ry = ry_rad * (180.0f / M_PI);
        float rz = rz_rad * (180.0f / M_PI);

        float bridge_thickness = 1.5f;
        float bridge_fric=0.1f;
        // 1. BETONOVÝ ZÁKLAD (Textura 3)
        create_cube(center.x, center.y - bridge_thickness * 0.5f, center.z, 
                    width, bridge_thickness, seg_len_3d + 0.1f, 
                    rx, ry, rz, 3, bridge_fric);

        // 2. ČERNÝ ASFALT NAVRCHU (Textura 1)
        create_cube(center.x, center.y + 0.02f, center.z, 
                    width - 0.1f, 0.05f, seg_len_3d + 0.1f, 
                    rx, ry, rz, 1, bridge_fric);

        // 3. SVODIDLA NA BOKÁCH (Textura 3)
        float barrier_h = 1.2f;
        float barrier_w = 0.4f;
        float offset_dist = (width * 0.5f) - (barrier_w * 0.5f);

        // Odsazení boků čistě podle spočítané osy X (m0, m2)
        float lx = center.x + m0 * offset_dist;
        float lz = center.z + m2 * offset_dist;
        create_cube(lx, center.y + barrier_h * 0.5f, lz,
                    barrier_w, barrier_h, seg_len_3d + 0.1f,
                    rx, ry, rz, 3, bridge_fric);

        float rx_pos = center.x - m0 * offset_dist;
        float rz_pos = center.z - m2 * offset_dist;
        create_cube(rx_pos, center.y + barrier_h * 0.5f, rz_pos,
                    barrier_w, barrier_h, seg_len_3d + 0.1f,
                    rx, ry, rz, 3, bridge_fric);

        // 4. PILÍŘE (Stojí kolmo k zemi, rotují jen podle směru jízdy ry)
        if (s % 2 == 0 || s == num_segments / 2) {
            float terrain_under = get_heightmap_height((int)center.x, (int)center.z);
            float pillar_top = center.y - bridge_thickness;
            float pillar_height = pillar_top - terrain_under;

            if (pillar_height > 3.0f) {
                float pillar_w = 1.2f;
                float pillar_d = width - 4.0f; 
                float pillar_y_pos = terrain_under + pillar_height * 0.5f;

                create_cube(center.x, pillar_y_pos, center.z,
                            pillar_d, pillar_height, pillar_w,
                            0.0f, ry, 0.0f, 3, bridge_fric);
            }
        }
    }
}
// Vyčistí (sníží) terén pod CELOU délkou a šířkou mostu, aby skalní/terénní
// "zuby" nemohly prorůstat skrz palubu. Na obou koncích mostu se výška
// plynule (smoothstep) napojí zpátky na původní terén, takže nevznikne
// ostrý schod na vstupu/výstupu z mostu.
//
// p1, p2          - body mostu (x,z = vodorovné souřadnice, y = výška paluby)
// bridge_width    - šířka mostu (stejně jako road_w)
// clearance       - kolik metrů terén musí být POD palubou mostu (volný prostor)
// approach_len    - na kolik metrů od KAŽDÉHO konce mostu se má terén plynule
//                    "dohojit" zpátky na původní výšku
void flatten_terrain_under_bridge(Vec3 p1, Vec3 p2, float bridge_width,
                                   float clearance = 6.0f, float approach_len = 12.0f) { // Mírně zvětšen approach_len
    float dx = p2.x - p1.x;
    float dz = p2.z - p1.z;
    float len = sqrtf(dx * dx + dz * dz);
    if (len < 0.1f) return;

    float ux = dx / len;
    float uz = dz / len;
    float nx = -uz; 
    float nz = ux;

    float half_w = bridge_width * 0.5f + 1.5f; // Mírně rozšířený okraj pro hladší přechod do stran

    int steps_along = (int)ceilf(len) + 1;
    int steps_across = (int)ceilf(half_w * 2.0f) + 1;

    for (int s = 0; s <= steps_along; ++s) {
        float t = (float)s / (float)steps_along; 
        float center_x = p1.x + dx * t;
        float center_z = p1.z + dz * t;
        float bridge_h = p1.y + (p2.y - p1.y) * t;
        
        // Vzdálenost od bližšího konce mostu
        float dist_from_start = t * len;
        float dist_from_end   = (1.0f - t) * len;
        float dist_from_nearest_end = std::min(dist_from_start, dist_from_end);

        // Dynamická clearance: na koncích mostu je clearance 0 (silnice se dotýká země), 
        // uprostřed mostu dosahuje plné clearance.
        float current_clearance = clearance;
        if (dist_from_nearest_end < approach_len) {
            float c_t = dist_from_nearest_end / approach_len;
            c_t = c_t * c_t * (3.0f - 2.0f * c_t); // smoothstep
            current_clearance = clearance * c_t;
        }
        
        float target_h = bridge_h - current_clearance;

        for (int a = 0; a <= steps_across; ++a) {
            float side_t = ((float)a / (float)steps_across) * 2.0f - 1.0f; 
            float px = center_x + nx * side_t * half_w;
            float pz = center_z + nz * side_t * half_w;

            float original_h = get_heightmap_height((int)px, (int)pz);
            float final_h;

            if (dist_from_nearest_end < approach_len) {
                // Přechodová zóna nájezdu:
                // Chceme, aby se terén plynule potkal s výškou mostu (bridge_h), nikoliv s target_h pod mostem.
                float blend_t = dist_from_nearest_end / approach_len;
                blend_t = blend_t * blend_t * (3.0f - 2.0f * blend_t); // smoothstep

                // Blendujeme mezi výškou pod mostem (target_h) a reálným nájezdem (bridge_h)
                float profile_h = target_h * (1.0f - blend_t) + bridge_h * blend_t;
                
                // Výsledná výška terénu se přizpůsobí náběhu, ale nenecháme ji vystřelit nad bridge_h
                final_h = original_h * blend_t + profile_h * (1.0f - blend_t);
                if (final_h > bridge_h - 0.1f) final_h = bridge_h - 0.1f; 
            } else {
                // Střed mostu - klasické podseknutí terénu
                final_h = std::min(original_h, target_h);
            }

            set_heightmap_height((int)px, (int)pz, final_h);
        }
    }
}
void gen_heightmap_roads() {
    printf("\ngenerating heightmap roads (SMOOTH APPROACH FIX)... %d\n", roadparts_len);
    float last_updated_progress = 0.0f;

    // --- PAS 1: DETEKCE MOSTŮ A VYHLAZENÍ NÁJEZDŮ (PRE-PROCESS) ---
    std::vector<bool> is_bridge(roadparts_len, false);
    std::vector<int> point_degree(roadpoints_len, 0);
    for (int i = 0; i < roadparts_len; i++) {
        point_degree[roadparts[i].p1]++;
        point_degree[roadparts[i].p2]++;
    }

    bool pre_building_bridge = false;

    for (int i = 0; i < roadparts_len; i++) {
        roadpoint& p1 = roadpoints[roadparts[i].p1];
        roadpoint& p2 = roadpoints[roadparts[i].p2];
        
        float dx = p2.y - p1.y;
        float dz = p2.x - p1.x;
        float len = sqrtf(dx * dx + dz * dz);
        if (len < 0.1f) continue;

        float mid_x = (p1.y + p2.y) * 0.5f;
        float mid_z = (p1.x + p2.x) * 0.5f;
        float mid_h_road = (p1.h + p2.h) * 0.5f;
        float mid_h_terrain = get_heightmap_height((int)mid_x, (int)mid_z);

        float ux = dx / len; 
        float uz = dz / len;
        float nx = -uz; 
        float nz = ux;

        float sample_dist = 4.0f; 
        float side_l_x = mid_x + nx * sample_dist;
        float side_l_z = mid_z + nz * sample_dist;
        float side_r_x = mid_x - nx * sample_dist;
        float side_r_z = mid_z - nz * sample_dist;

        float h_terrain_left  = get_heightmap_height((int)side_l_x, (int)side_l_z);
        float h_terrain_right = get_heightmap_height((int)side_r_x, (int)side_r_z);

        float cross_height_diff = fabs(h_terrain_left - h_terrain_right);
        float cross_slope = cross_height_diff / (sample_dist * 2.0f);

        // ZMĚNA: Snížení limitů, aby most začínal dříve ("most dál")
        float required_bridge_height = 4.0f; // Původně 10.0f
        if (cross_slope >= 1.0f) required_bridge_height = 8.0f; // Původně 20.0f

        bool endpoint_is_junction = (point_degree[roadparts[i].p1] > 2 || point_degree[roadparts[i].p2] > 2);
        bool segment_is_high = (mid_h_road - mid_h_terrain > required_bridge_height);
        if (endpoint_is_junction) {
            segment_is_high = false;
            pre_building_bridge = false;
        }

        // Hystereze a look-ahead logic
        if (pre_building_bridge) {
            bool terrain_reached = true;
            for (int k = i; k < std::min(roadparts_len, i + 3); k++) {
                roadpoint& np1 = roadpoints[roadparts[k].p1];
                roadpoint& np2 = roadpoints[roadparts[k].p2];
                if ((np1.h + np2.h)*0.5f - get_heightmap_height((int)(np1.y+np2.y)*0.5f, (int)(np1.x+np2.x)*0.5f) > 5.0f) { // Sníženo na 5.0f
                    terrain_reached = false;
                    break;
                }
            }
            if (!terrain_reached) segment_is_high = true;
        } else {
            if (segment_is_high) {
                int high_count = 0;
                for (int k = i; k < std::min(roadparts_len, i + 3); k++) {
                    roadpoint& np1 = roadpoints[roadparts[k].p1];
                    roadpoint& np2 = roadpoints[roadparts[k].p2];
                    if ((np1.h + np2.h)*0.5f - get_heightmap_height((int)(np1.y+np2.y)*0.5f, (int)(np1.x+np2.x)*0.5f) > 3.0f) { // Sníženo na 3.0f
                        high_count++;
                    }
                }
                if (high_count < 2) segment_is_high = false;
            }
        }

        if (segment_is_high) {
            pre_building_bridge = true;
            is_bridge[i] = true;
        } else {
            pre_building_bridge = false;
        }
    }

    // Lambda funkce pro matematické vyhlazení výškového profilu nájezdů silnice
    auto smooth_approach_heights = [&](int start_i, int end_i, bool reached_end) {
        roadpoint& b_start = roadpoints[roadparts[start_i].p1];
        roadpoint& b_end   = reached_end ? roadpoints[roadparts[end_i].p2] : roadpoints[roadparts[end_i].p1];

        float bdx = b_end.y - b_start.y;
        float bdz = b_end.x - b_start.x;
        float bridge_len = sqrtf(bdx * bdx + bdz * bdz);
        if (bridge_len < 0.1f) return;

        float bridge_slope = (b_end.h - b_start.h) / bridge_len; 

        // ZMĚNA: Přidána zóna absolutní roviny (FLAT) a prodlouženo celkové vyhlazení (SMOOTH)
        const float FLAT_DIST = 15.0f;   // 15 metrů dokonale rovného pokračování mostu před vjezdem
        const float SMOOTH_DIST = 60.0f; // Následných 45 metrů pozvolného náběhu do terénu (celkem 60m)

        // 1. Vyhlazení PŘED mostem (směrem dozadu do vnitrozemí)
        float dist_back = 0.0f;
        for (int idx = start_i - 1; idx >= 0; idx--) {
            if (is_bridge[idx]) break; 

            roadpoint& cp1 = roadpoints[roadparts[idx].p1];
            roadpoint& cp2 = roadpoints[roadparts[idx].p2];
            float seg_len = sqrtf((cp2.y-cp1.y)*(cp2.y-cp1.y) + (cp2.x-cp1.x)*(cp2.x-cp1.x));

            float d_cp2 = dist_back;
            float d_cp1 = dist_back + seg_len;

            // Úprava pro koncový bod segmentu (cp2)
            if (d_cp2 < FLAT_DIST) {
                cp2.h = b_start.h - d_cp2 * bridge_slope; // Přesná linie mostu (žádný zlom)
            } else if (d_cp2 < SMOOTH_DIST) {
                float t = (d_cp2 - FLAT_DIST) / (SMOOTH_DIST - FLAT_DIST);
                float blend = t * t * (3.0f - 2.0f * t); 
                float target_h = b_start.h - d_cp2 * bridge_slope;
                cp2.h = target_h * (1.0f - blend) + cp2.h * blend;
            }

            // Úprava pro počáteční bod segmentu (cp1)
            if (d_cp1 < FLAT_DIST) {
                cp1.h = b_start.h - d_cp1 * bridge_slope;
            } else if (d_cp1 < SMOOTH_DIST) {
                float t = (d_cp1 - FLAT_DIST) / (SMOOTH_DIST - FLAT_DIST);
                float blend = t * t * (3.0f - 2.0f * t);
                float target_h = b_start.h - d_cp1 * bridge_slope;
                cp1.h = target_h * (1.0f - blend) + cp1.h * blend;
            }
            
            dist_back += seg_len;
            if (dist_back >= SMOOTH_DIST) break;
        }

        // 2. Vyhlazení ZA mostem (směrem dopředu do vnitrozemí)
        float dist_fwd = 0.0f;
        int start_fwd_idx = reached_end ? end_i + 1 : end_i;
        for (int idx = start_fwd_idx; idx < roadparts_len; idx++) {
            if (is_bridge[idx]) break; 

            roadpoint& cp1 = roadpoints[roadparts[idx].p1];
            roadpoint& cp2 = roadpoints[roadparts[idx].p2];
            float seg_len = sqrtf((cp2.y-cp1.y)*(cp2.y-cp1.y) + (cp2.x-cp1.x)*(cp2.x-cp1.x));

            float d_cp1 = dist_fwd;
            float d_cp2 = dist_fwd + seg_len;

            // Úprava pro počáteční bod segmentu za mostem (cp1)
            if (d_cp1 < FLAT_DIST) {
                cp1.h = b_end.h + d_cp1 * bridge_slope;
            } else if (d_cp1 < SMOOTH_DIST) {
                float t = (d_cp1 - FLAT_DIST) / (SMOOTH_DIST - FLAT_DIST);
                float blend = t * t * (3.0f - 2.0f * t);
                float target_h = b_end.h + d_cp1 * bridge_slope;
                cp1.h = target_h * (1.0f - blend) + cp1.h * blend;
            }

            // Úprava pro koncový bod segmentu za mostem (cp2)
            if (d_cp2 < FLAT_DIST) {
                cp2.h = b_end.h + d_cp2 * bridge_slope;
            } else if (d_cp2 < SMOOTH_DIST) {
                float t = (d_cp2 - FLAT_DIST) / (SMOOTH_DIST - FLAT_DIST);
                float blend = t * t * (3.0f - 2.0f * t);
                float target_h = b_end.h + d_cp2 * bridge_slope;
                cp2.h = target_h * (1.0f - blend) + cp2.h * blend;
            }
            
            dist_fwd += seg_len;
            if (dist_fwd >= SMOOTH_DIST) break;
        }
    };

    // Aplikujeme vyhlazení na všechny nalezené úseky mostů
    bool in_b = false;
    int b_start_idx = -1;
    for (int i = 0; i < roadparts_len; i++) {
        if (is_bridge[i]) {
            if (!in_b) { in_b = true; b_start_idx = i; }
        } else {
            if (in_b) {
                smooth_approach_heights(b_start_idx, i, false);
                in_b = false;
            }
        }
    }
    if (in_b) {
        smooth_approach_heights(b_start_idx, roadparts_len - 1, true);
    }


    // --- PAS 2: FINÁLNÍ RASTRIZACE (HLAVNÍ SMYČKA) ---
    road_debug_triangles.clear();
    road_debug_segments.clear();

    // build road mesh from processed roadpoints
    std::vector<RoadTriangle> road_triangles = generate_road_mesh(is_bridge);
    for (const auto& t : road_triangles) {
        road_debug_triangles.push_back({
            {t.a.x, t.a.h, t.a.z},
            {t.b.x, t.b.h, t.b.z},
            {t.c.x, t.c.h, t.c.z}
        });
    }

    for (int i = 0; i < roadparts_len; i++) {
        roadpoint& p1 = roadpoints[roadparts[i].p1];
        roadpoint& p2 = roadpoints[roadparts[i].p2];
        road_debug_segments.push_back({
            {(float)p1.y, p1.h, (float)p1.x},
            {(float)p2.y, p2.h, (float)p2.x}
        });
    }

    bool building_bridge = false;
    int bridge_start_idx = -1;
    int bridge_end_idx = -1;

    for (int i = 0; i < roadparts_len; i++) {
        float progress = (float)i / (float)(roadparts_len - 1);
        if (progress - last_updated_progress >= 0.1f || i == 0) {
            display_background();
            render_progress_bar(progress, "Rasterizing gapless road geometry");
            run_essencials();
            last_updated_progress = progress;
        }

        roadpoint& p1 = roadpoints[roadparts[i].p1];
        roadpoint& p2 = roadpoints[roadparts[i].p2];
        
        float x1 = p1.y; float z1 = p1.x; 
        float x2 = p2.y; float z2 = p2.x;
        float h1 = p1.h; float h2 = p2.h;
        float road_w = roadparts[i].width;
        float blend_w = 8.0f; 
        
        float dx = x2 - x1;
        float dz = z2 - z1;
        float len = sqrtf(dx * dx + dz * dz);
        if (len < 0.1f) continue;

        float ux = dx / len; 
        float uz = dz / len;
        float nx = -uz; 
        float nz = ux;

        bool segment_is_high = is_bridge[i];

        if (segment_is_high) {
            if (!building_bridge) {
                building_bridge = true;
                bridge_start_idx = i;
                bridge_end_idx = i;
            } else {
                bridge_end_idx = i;
            }
            continue; 
        } else {
            if (building_bridge) {
                int start_i = bridge_start_idx;
                int end_i = bridge_end_idx;

                roadpoint& b_start = roadpoints[roadparts[start_i].p1];
                roadpoint& b_end   = roadpoints[roadparts[end_i].p2];

                Vec3 b_start_pt = { (float)b_start.y, b_start.h, (float)b_start.x };
                Vec3 b_end_pt   = { (float)b_end.y, b_end.h, (float)b_end.x };

                float bdx = b_end_pt.x - b_start_pt.x;
                float bdz = b_end_pt.z - b_start_pt.z;
                float bridge_len = sqrtf(bdx * bdx + bdz * bdz);
                if (bridge_len > 0.01f) {
                    float bridge_overlap = 1.0f;
                    float ext_x = bdx / bridge_len * bridge_overlap;
                    float ext_z = bdz / bridge_len * bridge_overlap;
                    b_start_pt.x -= ext_x; b_start_pt.z -= ext_z;
                    b_end_pt.x   += ext_x; b_end_pt.z   += ext_z;
                }

                flatten_terrain_under_bridge(b_start_pt, b_end_pt, road_w);
                make_bridge(b_start_pt, b_end_pt, road_w);
                
                building_bridge = false;
                bridge_start_idx = -1;
                bridge_end_idx = -1;
            }
        }
        
        // --- STANDARDNÍ RASTRIZACE SILNICE NA ZEMI ---
        float length_overlap = 0.5f;

        float ox1 = x1 - ux * length_overlap; float oz1 = z1 - uz * length_overlap;
        float ox2 = x2 + ux * length_overlap; float oz2 = z2 + uz * length_overlap;
        
        // mesh triangles for this segment are generated in road_triangles

        int next_idx = -1;
        if (i + 1 < roadparts_len && roadparts[i + 1].p1 == roadparts[i].p2) {
            next_idx = i + 1;
        } else {
            for (int k = 0; k < roadparts_len; k++) {
                if (roadparts[k].p1 == roadparts[i].p2) { next_idx = k; break; }
            }
        }

        if (next_idx != -1) {
            roadpoint& p3 = roadpoints[roadparts[next_idx].p2];
            float x3 = p3.y; float z3 = p3.x;
            float dx2 = x3 - x2; float dz2 = z3 - z2;
            float len2 = sqrtf(dx2 * dx2 + dz2 * dz2);
            
            if (len2 > 0.1f) {
                float ux2 = dx2 / len2; float uz2 = dz2 / len2;
                float nx2 = -uz2; float nz2 = ux2;

                // cross-turn triangles included in road_triangles
            }
        }
    }

    if (building_bridge) {
        int start_i = bridge_start_idx;
        int end_i = bridge_end_idx;
        roadpoint& b_start = roadpoints[roadparts[start_i].p1];
        roadpoint& b_end   = roadpoints[roadparts[end_i].p2];

        Vec3 b_start_pt = { (float)b_start.y, b_start.h, (float)b_start.x };
        Vec3 b_end_pt   = { (float)b_end.y, b_end.h, (float)b_end.x };

        float bdx = b_end_pt.x - b_start_pt.x;
        float bdz = b_end_pt.z - b_start_pt.z;
        float bridge_len = sqrtf(bdx * bdx + bdz * bdz);
        if (bridge_len > 0.01f) {
            float bridge_overlap = 1.0f; 
            float ext_x = bdx / bridge_len * bridge_overlap;
            float ext_z = bdz / bridge_len * bridge_overlap;
            b_start_pt.x -= ext_x; b_start_pt.z -= ext_z;
            b_end_pt.x   += ext_x; b_end_pt.z   += ext_z;
        }

        flatten_terrain_under_bridge(b_start_pt, b_end_pt, roadparts[end_i].width);
        make_bridge(b_start_pt, b_end_pt, roadparts[end_i].width);
        
        building_bridge = false;
    }
    // Rasterize the generated mesh into the heightmap
    for (auto &t : road_triangles) {
        rasterize_triangle(t.a, t.b, t.c);
    }

    printf("Perfect continuous roads and transitions generated!\n");
}