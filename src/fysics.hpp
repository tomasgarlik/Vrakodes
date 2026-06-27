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
#include <OpenGL/gl.h>
#include "constants.h"
#include "general_functions.hpp"
#include "types.hpp"
#include "global_vars.hpp"
#include "softsoft.hpp"
#endif
void apply_force_dir(point &from, point &to, float force, float dt)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float dz = to.z - from.z;

    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    if (dist < 1e-6f) return;

    float nx = dx / dist;
    float ny = dy / dist;
    float nz = dz / dist;

    from.vx += (nx * force / from.mass) * dt;
    from.vy += (ny * force / from.mass) * dt;
    from.vz += (nz * force / from.mass) * dt;
}
float get_joint_damage_score(float L_default, float L_rest) {
    // 1. Zjistíme absolutní rozdíl (o kolik metrů se to natáhlo/zkrátilo)
    float diff = fabsf(L_rest - L_default);

    // 2. Definujeme, co je "kritická mez" (cim menci, tim vetsi cisla score)
    float limit = L_default * 0.001f; 

    // 3. Score: Kolikrát se do toho rozdílu vejde ten limit
    // Pokud je výsledek 5.0, znamená to, že se to ohnulo 5x víc, než by mělo.
    return diff / limit;
}
bool isPointInTriangle(float px, float pz, float x1, float z1, float x2, float z2, float x3, float z3) {
    float det = (z2 - z3) * (x1 - x3) + (x3 - x2) * (z1 - z3);
    float l1 = ((z2 - z3) * (px - x3) + (x3 - x2) * (pz - z3)) / det;
    float l2 = ((z3 - z1) * (px - x3) + (x1 - x3) * (pz - z3)) / det;
    float l3 = 1.0f - l1 - l2;
    return l1 >= -0.001f && l2 >= -0.001f && l3 >= -0.001f; // S malou tolerancí
}





// Vnější normála trojúhelníku (A,B,C) vůči bodu D (vrchol tetrahedronu)
Vec3 face_normal(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D) {
    Vec3 AB = subtract(B, A);
    Vec3 AC = subtract(C, A);
    Vec3 n = cross(AB, AC);
    float len = sqrtf(lengthSq(n));
    if (len < 1e-6) return {0.0f, 1.0f, 0.0f};
    n = multiply(n, 1.0f / len);
    // Otestujeme, zda D leží na straně normály
    if (dot(subtract(D, A), n) < 0.0f) {
        n = multiply(n, -1.0f);
    }
    return n;
}

// Jednoduché barycentrické souřadnice (těžiště)
void barycentric(const Vec3& P, const Vec3& A, const Vec3& B, const Vec3& C,
                 float& u, float& v, float& w) {
    Vec3 v0 = subtract(B, A);
    Vec3 v1 = subtract(C, A);
    Vec3 v2 = subtract(P, A);
    
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    
    float denom = d00 * d11 - d01 * d01;
    if (fabs(denom) < 1e-6) {
        u = v = w = 1.0f / 3.0f;
        return;
    }
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}


bool raycast(point p, point p1, point p2, point p3, Vec3 dir){
    return ray_triangle_intersection(p,p1,p2,p3,dir).hit;
}
void calculate_ballz_collisions(point& p, int this_car_ind){
    // ballz
    for (int indian = 0; indian < cars.size(); indian++) {
        if (indian == this_car_ind) continue;
        cardata& car = cars[indian];
        for (int bi = 0; bi < car.balls_count; bi++) {
            ball& b = car.balls[bi];
            point& bp = car.points[b.p];

            float dx = p.x - bp.x;
            float dy = p.y - bp.y;
            float dz = p.z - bp.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

            if (dist < b.radius && dist > 1e-6f) {
                // Push point out of ball
                float penetration = b.radius - dist;
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;

                // Separate positions by mass ratio
                float total_mass = p.mass + bp.mass;
                float ratio_p  = bp.mass / total_mass;
                float ratio_bp = p.mass  / total_mass;

                p.x  += nx * penetration * ratio_p;
                p.y  += ny * penetration * ratio_p;
                p.z  += nz * penetration * ratio_p;
                bp.x -= nx * penetration * ratio_bp;
                bp.y -= ny * penetration * ratio_bp;
                bp.z -= nz * penetration * ratio_bp;

                // Relative velocity along normal
                float rvx = p.vx - bp.vx;
                float rvy = p.vy - bp.vy;
                float rvz = p.vz - bp.vz;
                float relVelN = rvx*nx + rvy*ny + rvz*nz;

                // Only resolve if approaching
                if (relVelN < 0.0f) {
                    float restitution = 0.8f;
                    float impulse = -(1.0f + restitution) * relVelN / total_mass;

                    p.vx  += impulse * bp.mass * nx;
                    p.vy  += impulse * bp.mass * ny;
                    p.vz  += impulse * bp.mass * nz;
                    bp.vx -= impulse * p.mass  * nx;
                    bp.vy -= impulse * p.mass  * ny;
                    bp.vz -= impulse * p.mass  * nz;
                }
            }
        }
    }
}
void calculate_terrain_collisions(point& p, float dt){
    if (p.y <= get_heightmap_height(p.x,p.z)) {
        p.y = get_heightmap_height(p.x,p.z);

        // odraz
        if (p.vy < 0) p.vy *= -0.5f;

        // ==== TŘENÍ O ZEM ====
        float friction=0.0f;
        if (p.group==1){
            friction = 3000.0f;   //  hlavní slider (zkoušej 2–20)
        } else  if (p.group==0){
            friction = 1000.0f;   //  hlavní slider (zkoušej 2–20)
        }
        float vxz_len = sqrtf(p.vx*p.vx + p.vz*p.vz);
        if (vxz_len > 0.0001f) {
            float drop = friction * dt;

            // suché tření – nikdy nejde do opačnýho směru
            float new_len = vxz_len - drop;
            if (new_len < 0) new_len = 0;

            float k = new_len / vxz_len;
            p.vx *= k;
            p.vz *= k;
        }
    }
    if (p.x >= (float)MAP_SIZE) {
        p.x = (float)MAP_SIZE;
        p.vx *= -0.5f;
    }
    if (p.z >= (float)MAP_SIZE) {
        p.z = (float)MAP_SIZE;
        p.vz *= -0.5f;
    }
}
void calculate_OBB_collisions(point& p, float dt, float x_shift, float z_shift){
    checked_collision_boxes=0;



    float world_x = p.x + (float)x_shift;
    float world_z = p.z + (float)z_shift;

    int center_chx = (int)(world_x / CHUNK_SIZE);
    int center_chz = (int)(world_z / CHUNK_SIZE);

    for (int chz = center_chz - 1; chz <= center_chz + 1; chz++) {
        for (int chx = center_chx - 1; chx <= center_chx + 1; chx++) {
            if (chx < 0 || chz < 0 || chx >= CHUNKS_SIZE || chz >= CHUNKS_SIZE) continue;
            chunk* target = chunks[chz][chx];
            if (!target) continue;

            for (int asfd = 0; asfd < target->collision_boxes_count; asfd++) {
                collision_box& b = target->collision_boxes[asfd];
                // 1. RELATIVE POSITION
                float dx = world_x - b.cx;
                float dy = p.y - b.cy;
                float dz = world_z - b.cz;

                // 2. EARLY EXIT (Sféra)
                if ((dx*dx + dy*dy + dz*dz) > b.max_r * b.max_r) continue;
                checked_collision_boxes++;

                // 3. TRANSFORM TO LOCAL SPACE (Matrix multiplication)
                float tx = dx * b.m0 + dy * b.m1 + dz * b.m2;
                float ty = dx * b.m3 + dy * b.m4 + dz * b.m5;
                float tz = dx * b.m6 + dy * b.m7 + dz * b.m8;

                // 4. AABB TEST v Local Space
                if (fabsf(tx) < b.hx && fabsf(ty) < b.hy && fabsf(tz) < b.hz) {
                    float bounce = -0.5f;
                    // Transformace rychlosti do Local Space
                    float lvx = p.vx * b.m0 + p.vy * b.m1 + p.vz * b.m2;
                    float lvy = p.vx * b.m3 + p.vy * b.m4 + p.vz * b.m5;
                    float lvz = p.vx * b.m6 + p.vy * b.m7 + p.vz * b.m8;



                    float adx = b.hx - fabsf(tx);
                    float ady = b.hy - fabsf(ty);
                    float adz = b.hz - fabsf(tz);

                    float friction = (p.group == 1) ? 0.2f : 0.5f;
                    friction = clamp(friction + b.friction, 0.0f, 1.0f);
                    float bias = 0.001f;

                    if (ady < adx + bias && ady < adz + bias) {
                        ty = (ty > 0) ? b.hy : -b.hy;
                        lvy *= bounce;   // Odraz ve svislém směru boxu

                        // stuff for the col skip
                        float skip_factor = (float)collision_skip_rate; 
                        float actual_friction = powf(friction, skip_factor);
                        lvx *= actual_friction;
                        lvz *= actual_friction;
                        // lvx *= friction; // Tření v horizontálních směrech boxu
                        // lvz *= friction;
                    } 
                    else if (adx < adz) {
                        tx = (tx > 0) ? b.hx : -b.hx;
                        lvx *= bounce;
                        lvy *= friction;
                        lvz *= friction;
                    } 
                    else {
                        tz = (tz > 0) ? b.hz : -b.hz;
                        lvz *= bounce;
                        lvx *= friction;
                        lvy *= friction;
                    }

                    // 5. TRANSFORM BACK TO WORLD SPACE (Matrix multiplication)
                    p.vx = lvx * b.im0 + lvy * b.im1 + lvz * b.im2;
                    p.vy = lvx * b.im3 + lvy * b.im4 + lvz * b.im5;
                    p.vz = lvx * b.im6 + lvy * b.im7 + lvz * b.im8;

                    float ntx = tx * b.im0 + ty * b.im1 + tz * b.im2;
                    float nty = tx * b.im3 + ty * b.im4 + tz * b.im5;
                    float ntz = tx * b.im6 + ty * b.im7 + tz * b.im8;

                    p.x = ntx + b.cx - x_shift;
                    p.y = nty + b.cy;
                    p.z = ntz + b.cz - z_shift;
                    // float bounce = -0.5f;
                    // // Transformace rychlosti do Local Space
                    // float lvx = p.vx * b.m0 + p.vy * b.m1 + p.vz * b.m2;
                    // float lvy = p.vx * b.m3 + p.vy * b.m4 + p.vz * b.m5;
                    // float lvz = p.vx * b.m6 + p.vy * b.m7 + p.vz * b.m8;

                    // float adx = b.hx - fabsf(tx);
                    // float ady = b.hy - fabsf(ty);
                    // float adz = b.hz - fabsf(tz);

                    // // Základní tření podle skupiny částice
                    // float base_friction = (p.group == 1) ? 0.05f : 0.5f; // group 1 defaultně málo tření
                    
                    // // Výsledné tření: 0 = led, 1 = totální stop
                    // float f_total = clamp(base_friction + b.friction, 0.0f, 1.0f);
                    
                    // // PŘEVRÁCENÍ LOGIKY PRO NÁSOBENÍ RYCHLOSTI:
                    // // Když f_total == 0 (led) -> multiplier = 1 (plná rychlost)
                    // // Když f_total == 1 (lepidlo) -> multiplier = 0 (zastavení)
                    // float friction_multiplier = 1.0f - f_total;

                    // float bias = 0.001f;

                    // if (ady < adx + bias && ady < adz + bias) {
                    //     ty = (ty > 0) ? b.hy : -b.hy;
                    //     lvy *= bounce;   // Odraz ve svislém směru boxu

                    //     // Aplikace skip rate na brzdný efekt
                    //     float skip_factor = (float)collision_skip_rate; 
                    //     float actual_multiplier = powf(friction_multiplier, skip_factor);
                        
                    //     lvx *= actual_multiplier;
                    //     lvz *= actual_multiplier;
                    // } 
                    // else if (adx < adz) {
                    //     tx = (tx > 0) ? b.hx : -b.hx;
                    //     lvx *= bounce;
                        
                    //     float skip_factor = (float)collision_skip_rate; 
                    //     float actual_multiplier = powf(friction_multiplier, skip_factor);
                        
                    //     lvy *= actual_multiplier;
                    //     lvz *= actual_multiplier;
                    // } 
                    // else {
                    //     tz = (tz > 0) ? b.hz : -b.hz;
                    //     lvz *= bounce;
                        
                    //     float skip_factor = (float)collision_skip_rate; 
                    //     float actual_multiplier = powf(friction_multiplier, skip_factor);
                        
                    //     lvx *= actual_multiplier;
                    //     lvy *= actual_multiplier;
                    // }

                    // // 5. TRANSFORM BACK TO WORLD SPACE (Matrix multiplication)
                    // p.vx = lvx * b.im0 + lvy * b.im1 + lvz * b.im2;
                    // p.vy = lvx * b.im3 + lvy * b.im4 + lvz * b.im5;
                    // p.vz = lvx * b.im6 + lvy * b.im7 + lvz * b.im8;

                    // float ntx = tx * b.im0 + ty * b.im1 + tz * b.im2;
                    // float nty = tx * b.im3 + ty * b.im4 + tz * b.im5;
                    // float ntz = tx * b.im6 + ty * b.im7 + tz * b.im8;

                    // p.x = ntx + b.cx - x_shift;
                    // p.y = nty + b.cy;
                    // p.z = ntz + b.cz - z_shift;
                    return; // Hit found
                }
            }
        }
    }
    return;
}
void calculate_volume_poses(cardata& car){
    for (int i = 0; i < car.volumes_count; i++) {
        // log("another volume");
        Vec3 sum = {0.0f, 0.0f, 0.0f};
        int total_points = 0;

        for (int j = 0; j < car.col_faces_count; j++) {
            face& f = car.col_faces[j];
            if (f.group != i) continue;

            // Pro každý face vezmeme jeho 3 vrcholy
            for (int k = 0; k < 3; k++) {
                point& p = car.points[f.vertices[k]];
                sum.x += p.x;
                sum.y += p.y;
                sum.z += p.z;
                total_points++;
            }
        }

        // Pokud jsme našli nějaké body, vypočítáme průměr (těžiště objemu)
        if (total_points > 0) {
            car.volume_poses[i].x = sum.x / (float)total_points;
            car.volume_poses[i].y = sum.y / (float)total_points;
            car.volume_poses[i].z = sum.z / (float)total_points;
        }
    }
}
void calculate_bounding_sphere(cardata& car) {
    float max_dist = 0;
    for (int i = 0; i < car.points_count; i++) {
        if (!car.points[i].exists) continue;
        float dx = car.points[i].x - car.pos_x;
        float dy = car.points[i].y - car.pos_y;
        float dz = car.points[i].z - car.pos_z;
        float dist = dx*dx + dy*dy + dz*dz;
        if (dist > max_dist) max_dist = dist;
    }
    car.bound = sqrtf(max_dist);
}
inline void process_shift(cardata& car){
    // Určíme, co je teď hlavní "střed" světa (buď auto, nebo kamera)

    float shift_val_x = 0.0f;
    float shift_val_z = 0.0f;

    // Kontrola X hranice (kladná i záporná!)
    if (car.pos_x > shift_interval) shift_val_x = -shift_interval;
    else if (car.pos_x < 0.0f) shift_val_x = shift_interval;

    // Kontrola Z hranice
    if (car.pos_z > shift_interval) shift_val_z = -shift_interval;
    else if (car.pos_z < 0.0f) shift_val_z = shift_interval;

    // 3. POKUD DOŠLO K PŘEKROČENÍ, POSUNEME VŠECHNO NAJEDNOU
    if (shift_val_x != 0.0f || shift_val_z != 0.0f) {
    
        // Posuneme globální posun mapy (pro rendering)
        car.x_shift -= shift_val_x; // Pozor na znaménka podle tvého rend_map_terrain
        car.z_shift -= shift_val_z;

        // Posuneme kameru
        x_pos += shift_val_x;
        z_pos += shift_val_z;

        // Posuneme auto (střed i všechny body)
        cars[driving_car].pos_x += shift_val_x;
        cars[driving_car].pos_z += shift_val_z;
        for (int i = 0; i < cars[driving_car].points_count; i++) {
            cars[driving_car].points[i].x += shift_val_x;
            cars[driving_car].points[i].oldx += shift_val_x;
            cars[driving_car].points[i].z += shift_val_z;
            cars[driving_car].points[i].oldz += shift_val_z;
        }
        
    }
}
void step_simulation(float dt) {
    simulation_steps=(int)((dt/(1.0f/SIMULATION_FREQ))*0.95f);
    if (simulation_steps<1){
        simulation_steps=1;
    } else {
        dt/=(float)simulation_steps;
    }
    dt=clamp(dt,0.00001, 1.0f/(float)(SIMULATION_FREQ)*1.1f);
    step_dt=dt;
    cars_collided_faces.clear();
    for (int hovno=0;hovno<cars.size();hovno++){
        cars_collided_faces.push_back({});
    }
    for (l=0;l<simulation_steps;l++){
        col_skip_frame++;
        if (col_skip_frame>collision_skip_rate){
            col_skip_frame=0;
        }
        for (int hovno=0;hovno<cars.size();hovno++){
            cardata& carr=cars[hovno];
            // log("calculate volume poses");
            calculate_volume_poses(carr);
            calculate_bounding_sphere(carr);
            // log("joints");
            carr.breaking_score=0;
            for (int i = 0; i < carr.joints_count; i++) {
                joint &j = carr.joints[i];
                if (!j.exists || j.snapped) continue;
                j.clamped=false;
                // if (j.attribute==ATTRIBUTE_ICJ){continue;}
                if (j.attribute==5){
                    j.rest_len=j.default_rest_len-(j.default_rest_len*carr.volant_pos*carr.max_steer);
                }
                if (j.attribute==6){
                    j.rest_len=j.default_rest_len+(j.default_rest_len*carr.volant_pos*carr.max_steer);
                }
                point &a = carr.points[j.p1];
                point &b = carr.points[j.p2];

                float dx = b.x - a.x;
                float dy = b.y - a.y;
                float dz = b.z - a.z;
                // this should apparently be more stable, as some random guy on youtube said, but it doesnt realy make a difference here
                // float dx = (b.x+(b.vx*dt*POINT_PREDICTION)) - (a.x+(a.vx*dt*POINT_PREDICTION));
                // float dy = (b.y+(b.vy*dt*POINT_PREDICTION)) - (a.y+(a.vy*dt*POINT_PREDICTION));
                // float dz = (b.z+(b.vz*dt*POINT_PREDICTION)) - (a.z+(a.vz*dt*POINT_PREDICTION));
                float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                if (j.attribute!=ATTRIBUTE_ICJ){
                    if (dist<(j.rest_len-j.rest_len*j.elastic_margin) && j.elastic_margin!=1.0){
                        if (j.snap){
                            j.snapped=true;
                            continue;
                        } else {
                            j.rest_len=dist+dist*j.elastic_margin;
                        }
                    } else if (dist>(j.rest_len+j.rest_len*j.elastic_margin) && j.elastic_margin!=1.0){
                        if (j.snap){
                            j.snapped=true;
                            continue;
                        } else {
                            j.rest_len=dist-j.rest_len*j.elastic_margin;
                        }
                    }
                } else if (j.attribute==ATTRIBUTE_ICJ && (!(dist<(j.rest_len-j.rest_len*j.elastic_margin)))){
                    continue; // if the internal collision joint isnt sompressed enough, we will skip
                }
                if (dist < 1e-6f) continue;
                // if (dist<j.rest_len*j.min_len){
                //     j.stiffness*=5.0f;
                // }
                // if (dist>j.rest_len*j.min_len){
                //     j.stiffness=j.default_stiffness;
                // }
                // Definuj si, jak moc chceš, aby to tuhlo (progresivní faktor)
                // float compression = (j.rest_len * j.min_len) - dist;

                // if (compression > 0.0f) {
                //     // Čím víc je to zmáčknuté pod limit, tím víc roste tuhost.
                //     // Tady je lineární verze:
                //     float multiplier = 1.0f + (compression / (j.rest_len * j.min_len)) * 100.0f; 
                    
                //     // Zastropujeme to, aby nám to nevystřelilo auto do vesmíru (např. max 10x stiffness)
                //     if (multiplier > 100.0f) multiplier = 100.0f;
                    
                //     j.stiffness = j.default_stiffness * multiplier;
                // } else {
                //     j.stiffness = j.default_stiffness;
                // }
                // Pokud je min_len rovno 0, celou tuhost necháme na defaultu a kód přeskočíme
                if (j.min_len > 0.0f) {
                    
                    // Vypočítáme absolutní minimální a maximální povolenou délku v jednotkách vzdálenosti
                    float limit_min = j.rest_len * j.min_len;
                    
                    // Pokud nemáš v proměnných j.max_len, můžeš použít např. j.max_len = 2.0f (dvojnásobek délky)
                    float limit_max = j.rest_len * j.min_len; 

                    float multiplier = 1.0f;

                    // 1. KONTROLA STLAČENÍ (Compression)
                    if (dist < limit_min) {
                        float compression = limit_min - dist;
                        multiplier = 1.0f + (compression / limit_min) * 100.0f;
                    }
                    // 2. KONTROLA TAHU / ROZTAŽENÍ (Extension)
                    else if (dist > limit_max) {
                        float extension = dist - limit_max;
                        multiplier = 1.0f + (extension / limit_max) * 100.0f;
                    }

                    // Zastropování násobitele (stejně jako v tvém původním kódu)
                    if (multiplier > 100.0f) multiplier = 100.0f;

                    // Aplikace výsledné tuhosti
                    j.stiffness = j.default_stiffness * multiplier;

                } else {
                    // Pokud je min_len == 0, podvozek si drží svou základní tuhost
                    j.stiffness = j.default_stiffness;
                }


                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;
                
float x = dist - j.rest_len;

float fs_base = j.default_stiffness * x * 20000.0f;

float fs_hydro = 0.0f;

if (j.attribute == ATTRIBUTE_CRJ) {

    j.hydro_filtered = j.hydro_filtered * 0.85f + x * 0.15f;

    float fx = j.hydro_filtered;

    float dead = j.rest_len * 0.02f;
    if (fabsf(fx) < dead) fx = 0.0f;
    else fx -= (fx > 0 ? dead : -dead);

    float factor = 1.0f + fabsf(fx) * 30.0f;
    if (factor > 10.0f) factor = 10.0f;

    fs_hydro = j.default_stiffness * fx * factor * 500.0f;
}

float fs = fs_base + fs_hydro;

                float dvx = b.vx - a.vx;
                float dvy = b.vy - a.vy;
                float dvz = b.vz - a.vz;
                float fd = j.damping*300.0f * (dvx*nx + dvy*ny + dvz*nz);

                float force = clamp(fs + fd, -carr.force_clamp,carr.force_clamp);
                if (fs + fd>force){j.clamped=true;}
                float fx = force * nx;
                float fy = force * ny;
                float fz = force * nz;

                a.vx += fx / a.mass * dt;
                a.vy += fy / a.mass * dt;
                a.vz += fz / a.mass * dt;
                
                b.vx -= fx / b.mass * dt;
                b.vy -= fy / b.mass * dt;
                b.vz -= fz / b.mass * dt;





                dx = b.x - a.x;
                dy = b.y - a.y;
                dz = b.z - a.z;

                if (j.attribute!=ATTRIBUTE_ICJ){
                    if (dist<(j.rest_len-j.rest_len*j.elastic_margin) && j.elastic_margin!=1.0){
                        j.rest_len=dist+dist*j.elastic_margin;
                    } else if (dist>(j.rest_len+j.rest_len*j.elastic_margin) && j.elastic_margin!=1.0){
                        j.rest_len=dist-j.rest_len*j.elastic_margin;
                    }
                    carr.breaking_score+=get_joint_damage_score(j.default_rest_len, j.rest_len);
                }
            }
            carr.breaking_score/=(float)carr.joints_count;
            

            for (int i=0;i<carr.col_faces_count;i++){
                face& f=carr.col_faces[i];
                compute_normal(carr.points[f.vertices[0]], carr.points[f.vertices[1]], carr.points[f.vertices[2]], f.nx, f.ny, f.nz);
            }
            // 2) Update pozice
            carr.pos_x=0.0f;
            carr.pos_y=0.0f;
            carr.pos_z=0.0f;
            if (col_skip_frame==collision_skip_rate){
                shall_calculate_collisions=true;
            } else {
                shall_calculate_collisions=false;
            }
            for (int i = 0; i < carr.points_count; i++) {
                point &p = carr.points[i];
                p.vy -= 9.81f * dt;

                // update pozice
                p.x += p.vx * dt;
                p.y += p.vy * dt;
                p.z += p.vz * dt;
                if (p.collide){
                    calculate_terrain_collisions(p, dt);
                    if (shall_calculate_collisions){
                        calculate_OBB_collisions(p, dt, carr.x_shift, carr.z_shift);
                        calculate_softsoft_collisions(p, hovno, dt);
                        calculate_ballz_collisions(p, hovno);
                    }
                }

                carr.pos_x+=p.x;
                carr.pos_y+=p.y;
                carr.pos_z+=p.z;
                p.oldx=p.x;
                p.oldy=p.y;
                p.oldz=p.z;
            }
            //ballz collisions with ground
            // for (int bx=0;bx<carr.balls_count;bx++){
            //     ball& b=carr.balls[bx];
            //     point& p=carr.points[b.p];
            //     if (p.y-b.radius<get_heightmap_height(p.x,p.z)){
            //         p.y=get_heightmap_height(p.x,p.z)+b.radius;
            //         if (p.vy < 0.0f){p.vy *= -0.1f;}
            //     }
            // }
            for (int bx = 0; bx < carr.balls_count; bx++) {
                ball& b = carr.balls[bx];
                point& p = carr.points[b.p];
                
                float current_h = get_heightmap_height(p.x, p.z);
                
                // Check for ground collision
                if (p.y - b.radius < current_h) {
                    // 1. Position correction
                    p.y = current_h + b.radius;
                    
                    // 2. Sample ground gradient
                    float eps = 0.1f; 
                    float h_x = get_heightmap_height(p.x + eps, p.z) - get_heightmap_height(p.x - eps, p.z);
                    float h_z = get_heightmap_height(p.x, p.z + eps) - get_heightmap_height(p.x, p.z - eps);
                    
                    // 3. Gravity Vector (Downhill force)
                    // Force = mass * acceleration. 
                    // We use a gravity constant (e.g., 9.8) scaled for your world.
                    float gravity_constant = 9.8f; 
                    float accel_x = -h_x * gravity_constant;
                    float accel_z = -h_z * gravity_constant;

                    // Apply acceleration to velocity
                    p.vx += accel_x;
                    p.vz += accel_z;

                    // 4. Vertical Bounce
                    // Heavy objects (higher mass) should bounce less
                    if (p.vy < 0.0f) {
                        p.vy *= -0.01;
                    }

                    // 5. Friction / Resistance
                    // Friction is typically Force_friction = coefficient * mass * gravity
                    // Here we simulate it by slowing velocity based on mass
                    float friction_coeff = 0.02f; 
                    float drag = 1.0f - (friction_coeff / p.mass);
                    p.vx *= drag;
                    p.vz *= drag;
                }
            }
            carr.pos_x/=carr.points_count;
            carr.pos_y/=carr.points_count;
            carr.pos_z/=carr.points_count;



            //engine and differential steering
            if (carr.steering_type==1){
                apply_force_dir(
                    carr.points[carr.wheel_LB],
                    carr.points[carr.wheel_LF],
                    carr.engine_force+(carr.engine_force*volant_pos),
                    dt
                );

                apply_force_dir(
                    carr.points[carr.wheel_RB],
                    carr.points[carr.wheel_RF],
                    carr.engine_force-(carr.engine_force*volant_pos),
                    dt
                );
            } else {
                apply_force_dir(
                    carr.points[carr.wheel_LB],
                    carr.points[carr.wheel_LF],
                    carr.engine_force,
                    dt
                );

                apply_force_dir(
                    carr.points[carr.wheel_RB],
                    carr.points[carr.wheel_RF],
                    carr.engine_force,
                    dt
                );
            }
        }
    }
    for (int hovno=0;hovno<cars.size();hovno++){
        cardata& carr=cars[hovno];
        float dx = carr.pos_x - carr.oldx;
        float dy = carr.pos_y - carr.oldy;
        float dz = carr.pos_z - carr.oldz;

        // 2. Pythagorova věta pro skutečnou vzdálenost (Euclidean distance)
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);

        // 3. Výpočet rychlosti: v = s / dt
        // Násobíme 3.6 pro převod z m/s na km/h
        if (dt > 0.0f) {
            carr.velocity = (distance / (dt*simulation_steps)) * 3.6f;
        }
        carr.oldx=carr.pos_x;
        carr.oldy=carr.pos_y;
        carr.oldz=carr.pos_z;
    }
    if (cars.size()==2){
    printf("%f, %f\n", cars[0].engine_force, cars[1].engine_force);
    }
}
void update_collision_box(collision_box& b) {
    // 1. Výpočet středu (Center)
    // Fabf je tvůj kamarád. Rozměry boxu musí být vždy absolutní.
    b.hx = fabsf(b.ex - b.sx) * 0.5f;
    b.hy = fabsf(b.ey - b.sy) * 0.5f;
    b.hz = fabsf(b.ez - b.sz) * 0.5f;

    // 2. Výpočet polovičních rozměrů (Half-extents)
    b.hx = fabsf(b.ex - b.sx) * 0.5f;
    b.hy = fabsf(b.ey - b.sy) * 0.5f;
    b.hz = fabsf(b.ez - b.sz) * 0.5f;

    // 3. Předvýpočet rotací (v radiánech)
    // Používáme záporné úhly pro kolizní test (inverzní rotace)
    b.cosX = cosf(-b.rx); b.sinX = sinf(-b.rx);
    b.cosY = cosf(-b.ry); b.sinY = sinf(-b.ry);
    b.cosZ = cosf(-b.rz); b.sinZ = sinf(-b.rz);






    b.cx = (b.sx + b.ex) * 0.5f;
    b.cy = (b.sy + b.ey) * 0.5f;
    b.cz = (b.sz + b.ez) * 0.5f;
    b.hx = fabsf(b.ex - b.sx) * 0.5f;
    b.hy = fabsf(b.ey - b.sy) * 0.5f;
    b.hz = fabsf(b.ez - b.sz) * 0.5f;

    // Early exit radius (vzdálenost k nejvzdálenějšímu rohu)
    b.max_r = sqrtf(b.hx*b.hx + b.hy*b.hy + b.hz*b.hz) + 0.01f;

    float cx = cosf(-b.rx), sx = sinf(-b.rx);
    float cy = cosf(-b.ry), sy = sinf(-b.ry);
    float cz = cosf(-b.rz), sz = sinf(-b.rz);

    // Předvýpočet matice (Pořadí Y -> X -> Z jako v tvém kódu)
    // Tohle je kombinovaná matice, která tě dostane do Local Space jedním násobením
    b.m0 = cy*cz + sy*sx*sz;   b.m1 = -cy*sz + sy*sx*cz;  b.m2 = sy*cx;
    b.m3 = cx*sz;             b.m4 = cx*cz;              b.m5 = -sx;
    b.m6 = -sy*cz + cy*sx*sz; b.m7 = sy*sz + cy*sx*cz;   b.m8 = cy*cx;

    // Inverzní matice (pro cestu zpět do World Space)
    // U rotačních matic je inverze jen transpozice (prohození řádků a sloupců)
    b.im0 = b.m0; b.im1 = b.m3; b.im2 = b.m6;
    b.im3 = b.m1; b.im4 = b.m4; b.im5 = b.m7;
    b.im6 = b.m2; b.im7 = b.m5; b.im8 = b.m8;



}
void init_collision_boxes() {
    printf("init collision boxes\n");
    for (int cz = 0; cz < CHUNKS_SIZE; cz++) {
        for (int cx = 0; cx < CHUNKS_SIZE; cx++) {
            chunk* c = chunks[cz][cx];
            if (c == NULL) continue;

            for (int i = 0; i < c->collision_boxes_count; i++) {
                update_collision_box(c->collision_boxes[i]);
            }
        }
    }
}