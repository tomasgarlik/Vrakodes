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
#endif
float get_car_rotation_y(cardata& car) {
    // 1. Získáme pozice kol
    point &pLF = car.points[car.wheel_LF];
    point &pRF = car.points[car.wheel_RF];
    point &pLB = car.points[car.wheel_LB];
    point &pRB = car.points[car.wheel_RB];

    // 2. Spočítáme střed přední nápravy (Front Center)
    float frontX = (pLF.x + pRF.x) * 0.5f;
    float frontZ = (pLF.z + pRF.z) * 0.5f;

    // 3. Spočítáme střed zadní nápravy (Back Center)
    float backX = (pLB.x + pRB.x) * 0.5f;
    float backZ = (pLB.z + pRB.z) * 0.5f;

    // 4. Vektor směru od zadku k předku
    float dirX = frontX - backX;
    float dirZ = frontZ - backZ;

    // 5. Výpočet úhlu v radiánech
    // atan2f(x, z) vrátí úhel od osy Z. 
    // Pokud chceš úhel od osy X, prohoď parametry na atan2f(z, x).
    float angle = atan2f(dirX, dirZ);

    return angle; // Výsledek v radiánech (-PI až PI)
}
float get_raw_car_rotation_x(cardata& car) {
    point &pLF = car.points[car.wheel_LF];
    point &pRF = car.points[car.wheel_RF];
    point &pLB = car.points[car.wheel_LB];
    point &pRB = car.points[car.wheel_RB];
    point &pUp = car.points[car.up_point];

    float frontY = (pLF.y + pRF.y) * 0.5f;
    float frontZ = (pLF.z + pRF.z) * 0.5f;

    float backY  = (pLB.y + pRB.y) * 0.5f;
    float backZ  = (pLB.z + pRB.z) * 0.5f;

    float centerY = (frontY + backY) * 0.5f;
    float centerZ = (frontZ + backZ) * 0.5f;

    // Forward direction vector (in YZ plane)
    float dirY = frontY - backY;
    float dirZ = frontZ - backZ;

    // Vector from center to up point
    float upY = pUp.y - centerY;
    float upZ = pUp.z - centerZ;

    // 2D cross product: tells us which side the up point is on
    // cross = dirZ * upY - dirY * upZ
    float cross = dirZ * upY - dirY * upZ;

    float angle = atan2f(dirY, dirZ);

    // If cross is negative, the up point is on the wrong side — flip
    if (cross < 0.0f)
        angle += PI;

    return angle;
}
int load_car_tetrahedra(cardata& car, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open tetrahedron file: %s\n", filename);
        return 0;
    }

    // Read number of tetrahedra (4‑byte little‑endian)
    int32_t count;
    if (fread(&count, sizeof(count), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    // Free any previously loaded tetrahedra
    if (car.tetrahedra) {
        free(car.tetrahedra);
        car.tetrahedra = NULL;
    }

    // Allocate memory for the tetrahedra
    car.tetrahedra = (tetrahedron*)malloc(count * sizeof(tetrahedron));
    if (!car.tetrahedra) {
        fclose(f);
        return 0;
    }

    // Read each tetrahedron's four indices
    for (int i = 0; i < count; i++) {
        int32_t idx[4];
        if (fread(idx, sizeof(idx), 1, f) != 1) {
            // Error: free allocated memory and return
            free(car.tetrahedra);
            car.tetrahedra = NULL;
            car.tetrahedra_count = 0;
            fclose(f);
            return 0;
        }
        car.tetrahedra[i].idx[0] = idx[0];
        car.tetrahedra[i].idx[1] = idx[1];
        car.tetrahedra[i].idx[2] = idx[2];
        car.tetrahedra[i].idx[3] = idx[3];
    }

    car.tetrahedra_count = count;
    fclose(f);
    printf("Loaded %d tetrahedra from %s\n", count, filename);
    return 1;
}
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <stdio.h>
#include <string.h>
#include <locale.h> // Nutné pro tečku v číslech

void load_car_properties(cardata& car, FILE *f) {
    // KLÍČOVÉ: Přinutíme C, aby četlo tečku (.) jako desetinný oddělovač
    // Bez tohoto řádku sscanf na českém systému u "300.0" selže.
    setlocale(LC_NUMERIC, "C");

    char line[512];
    char *data;

    printf("--- START LOADING CAR PROPERTIES ---\n");

    while (fgets(line, sizeof(line), f)) {
        // Najdeme dvojtečku, která odděluje název od hodnoty
        data = strchr(line, ':');
        if (!data) continue; 

        // data+1 ukazuje na všechno, co je za dvojtečkou
        char *val = data + 1;

        if (strstr(line, "\"engine_power\"")) {
            if (sscanf(val, " %f", &car.engine_power) != 1) 
                printf("[ERROR] engine_power selhal na radku: %s", line);
        }
        else if (strstr(line, "\"steering_type\"")) {
            if (sscanf(val, " %d", &car.steering_type) != 1) 
                printf("[ERROR] steering_type selhal na radku: %s", line);
        }
        else if (strstr(line, "\"max_steer\"")) {
            if (sscanf(val, " %f", &car.max_steer) != 1) 
                printf("[ERROR] max_steer selhal na radku: %s", line);
        }
        else if (strstr(line, "\"steer_speed\"")) {
            if (sscanf(val, " %f", &car.steer_speed) != 1) 
                printf("[ERROR] steer_speed selhal na radku: %s", line);
        }
        else if (strstr(line, "\"deformation_system\"")) {
            if (sscanf(val, " %d", &car.deformation_system) != 1) 
                printf("[ERROR] deformation_system selhal na radku: %s", line);
        }
        else if (strstr(line, "\"sound_divider\"")) {
            if (sscanf(val, " %f", &car.sound_divider) != 1) 
                printf("[ERROR] sound_divider selhal na radku: %s", line);
        }
        else if (strstr(line, "\"spawn_type\"")) {
            if (sscanf(val, " %d", &car.spawn_type) != 1) 
                printf("[ERROR] spawn_type selhal na radku: %s", line);
        }
        else if (strstr(line, "\"view_3rd\"")) {
            if (sscanf(val, " [%f,%f]", &car.v3rd.x, &car.v3rd.y) != 2) 
                printf("[ERROR] view_3rd selhal na radku: %s", line);
        }
        else if (strstr(line, "\"view_cab\"")) {
            if (sscanf(val, " [%f,%f,%f]", &car.vcab.x, &car.vcab.y, &car.vcab.z) != 3) 
                printf("[ERROR] view_cab selhal na radku: %s", line);
        }
        else if (strstr(line, "\"force_clamp\"")) {
            if (sscanf(val, " %f", &car.force_clamp) != 1) 
                printf("[ERROR] force_clamp selhal na radku: %s", line);
        }
    }
    rewind(f);
    printf("--- LOADING FINISHED ---\n");
    
    // Pro kontrolu si vypíšeme, co jsme reálně načetli do paměti:
    printf("DEBUG: Power: %f, Steer: %f, Speed: %f\n", 
            car.engine_power, car.max_steer, car.steer_speed);
}
void load_car(cardata& car){
    printf("loading car: %s\n", car_name.c_str());
    char path[256];
    sprintf(path,"assets/cars/%s/vehicle.json", car_name.c_str());
    FILE* f = fopen(path, "r");
    if (!f) return;
    sprintf(path,"assets/cars/%s/vehicle.tet", car_name.c_str());
    load_car_tetrahedra(car, path);
    load_car_properties(car, f);
    int points_count = 0;
    int joints_count = 0;
    int faces_count = 0;
    int balls_count = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        // if (strstr(line, "\"engine_power\"")) {
        //     sscanf(line, " \"engine_power\": %f,", &engine_power);
        // }
        // if (strstr(line, "\"steering_type\"")) {
        //     sscanf(line, " \"steering_type\": %d,", &car.steering_type);
        // }
        // if (strstr(line, "\"max_steer\"")) {
        //     sscanf(line, " \"max_steer\": %f,", &car.max_steer);
        // }
        // if (strstr(line, "\"steer_speed\"")) {
        //     sscanf(line, " \"steer_speed\": %f,", &car.steer_speed);
        // }
        // if (strstr(line, "\"deformation_system\"")) {
        //     sscanf(line, " \"deformation_system\": %d,", &car.deformation_system);
        // }
        // if (strstr(line, "\"sound_divider\"")) {
        //     sscanf(line, " \"sound_divider\": %f,", &car.sound_divider);
        // }
        // if (strstr(line, "\"spawn_type\"")) {
        //     sscanf(line, " \"spawn_type\": %d,", &car.spawn_type);
        // }
        // if (strstr(line, "\"view_3rd\"")) {
        //     sscanf(line, " \"view_3rd\": [%f,%f],", &car.v3rd.x, &car.v3rd.y);
        // }
        // if (strstr(line, "\"view_cab\"")) {
        //     sscanf(line, " \"view_cab\": [%f,%f,%f],", &car.vcab.x, &car.vcab.y, &car.vcab.z);
        // }
        // if (strstr(line, "\"force_clamp\"")) {
        //     sscanf(line, " \"force_clamp\": %f", &car.force_clamp);
        // }
        
        // ---- POINT ----
        if (strstr(line, "\"x\"")) {
            point p;

            sscanf(line,
                " {\"x\":%f,\"y\":%f,\"z\":%f,\"mass\":%f,\"attribute\":%d,\"group\":%d,\"friction\":%f,\"collide\":%d}",
                &p.x, &p.y, &p.z, &p.mass, &p.attribute, &p.group, &p.friction, &p.collide
            );
            car.points[points_count++] = p;
        }

        // ---- JOINT ----
        if (strstr(line, "\"p1\"") && strstr(line, "\"p2\"") && strstr(line, "\"stiffness\"")) {
            joint j;

            sscanf(line,
                " {\"p1\":%d,\"p2\":%d,\"group\":%d,"
                "\"stiffness\":%f,\"damping\":%f,"
                "\"min_len\":%f,\"elastic_margin\":%f,\"attribute\":%d,\"snap\":%d}",
                &j.p1,
                &j.p2,
                &j.group,
                &j.stiffness,
                &j.damping,
                &j.min_len,
                &j.elastic_margin,
                &j.attribute,
                &j.snap
            );
            // printf("%d ", j.snap);
            j.snapped=false;
            car.joints[joints_count++] = j;
        }

        // ---- FACE ----
        if (strstr(line, "\"p1\"") && strstr(line, "\"p2\"") && strstr(line, "\"p3\"")) {
            // Tohle je face (má p1, p2, p3)
            face fce;

            sscanf(line,
                " {\"p1\":%d,\"p2\":%d,\"p3\":%d,\"exists\":true}",
                &fce.vertices[0],
                &fce.vertices[1],
                &fce.vertices[2]
            );
            fce.group=0;
            car.col_faces[faces_count++] = fce;
        }
        if (strstr(line, "\"radius\"")) {
            ball b;

            sscanf(line,
                " {\"p\":%d,\"radius\":%f,\"target\":%d,\"exists\":true}",
                &b.p,
                &b.radius,
                &b.target
            );

            car.balls[balls_count++] = b;
        }
    }
	car.points_count=points_count;
	car.joints_count=joints_count;
	car.balls_count=balls_count;
	car.col_faces_count=faces_count;
    // car.engine_power=engine_power;
    // car.steer_speed=steer_speed;
    // car.max_steer=max_steer;

    engine_power=car.engine_power;
	steering_type=car.steering_type;
	max_steer=car.max_steer;
	steer_speed=car.steer_speed;
	car.niceni=0.0;
    niceni=car.niceni;
    for (i=0;i<car.tetrahedra_count;i++){
        car.tetrahedra[i].group=car.points[car.tetrahedra[i].idx[0]].group;
    }
    fclose(f);
    printf("[LOAD CAR] engine_power=%f\n", engine_power);
    printf("[LOAD CAR] steering_type=%d\n", steering_type);
    printf("[LOAD CAR] max_steer=%f\n", max_steer);
    printf("[LOAD CAR] steer_speed=%f\n", steer_speed);
    printf("[LOAD CAR] loaded %d joints, %d points, %d col faces\n", car.joints_count, car.points_count, car.col_faces_count);
}
#include <map>
#include <utility>
#include <vector>

// načte jeden roh facu: v, v/t, v//n nebo v/t/n (normálu ignoruje)
static void parse_corner(const char* tok, int& v, int& t) {
    int n = 0;
    v = t = 0;
    if (sscanf(tok, "%d/%d/%d", &v, &t, &n) == 3) return;
    v = t = 0;
    if (sscanf(tok, "%d//%d", &v, &n) == 2) { t = 0; return; }
    v = t = 0;
    if (sscanf(tok, "%d/%d", &v, &t) == 2) return;
    v = t = 0;
    sscanf(tok, "%d", &v);
}

// OBJ index (1-based, může být záporný) -> 0-based, -1 pokud chybí
static int resolve_idx(int idx, int count) {
    if (idx == 0) return -1;
    if (idx < 0) idx = count + idx + 1;
    idx -= 1;
    return (idx >= 0 && idx < count) ? idx : -1;
}

void load_car_mesh(cardata& car) {
    printf("loading car mesh %s...\n", car_name.c_str());
    int v_counter = 0;
    int f_counter = 0;

    char files[CAR_MESH_GROUPS][256];
    for (int g = 0; g < CAR_MESH_GROUPS; g++)
        snprintf(files[g], sizeof(files[g]), "assets/cars/%s/mesh_g%d.obj", car_name.c_str(), g + 1);

    char path[256];
    if (selected_skin_variant == 0) {
        snprintf(path, sizeof(path), "assets/cars/%s/texture.png", car_name.c_str());
    } else {
        snprintf(path, sizeof(path), "assets/cars/%s/texture%d.png",
            car_name.c_str(), selected_skin_variant);
    }
    car.texture = load_texture_PNG(path);
    if (selected_skin_variant == 0) {
        snprintf(path, sizeof(path), "assets/cars/%s/roughness.png", car_name.c_str());
    } else {
        snprintf(path, sizeof(path), "assets/cars/%s/roughness%d.png",
            car_name.c_str(), selected_skin_variant);
    }
    car.roughness = load_texture_PNG(path);
    if (!car.roughness) { car.roughness = texture; }
    sprintf(path, "assets/cars/%s/specular.png", car_name.c_str());
    car.specular = load_texture_PNG(path);
    if (!car.specular) { car.specular = texture; }
    sprintf(path, "assets/cars/%s/metallic.png", car_name.c_str());
    car.metallic = load_texture_PNG(path);
    if (!car.metallic) { car.metallic = texture; }
    sprintf(path, "assets/cars/%s/normal.png", car_name.c_str());
    car.normal = load_texture_PNG(path);
    if (!car.normal) { car.normal = texture; }

    for (int brnchv = 0; brnchv < CAR_MESH_GROUPS; brnchv++) {
        if (brnchv == 2) { continue; }

        std::vector<ObjVertex> temp_vertices;
        std::vector<ObjUV> temp_uvs;
        std::map<std::pair<int, int>, int> corner_map;   // (v, vt) -> index v car.vertices
                                                         // mapa je per soubor, indexy v OBJ jsou per soubor

        FILE* f = fopen(files[brnchv], "r");
        if (!f) { printf("error loading car mesh for group %d\n", brnchv); continue; }

        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "v ", 2) == 0) {
                ObjVertex v;
                sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
                temp_vertices.push_back(v);
            }
            else if (strncmp(line, "vt ", 3) == 0) {
                ObjUV uv;
                sscanf(line, "vt %f %f", &uv.u, &uv.v);
                uv.v = 1.0f - uv.v;
                temp_uvs.push_back(uv);
            }
            else if (strncmp(line, "f ", 2) == 0) {
                std::vector<int> corner;                  // indexy do car.vertices
                const char* p = line + 2;
                char tok[128];
                int used = 0;
                bool ok = true;

                while (sscanf(p, "%127s%n", tok, &used) == 1) {
                    p += used;

                    int vi, ti;
                    parse_corner(tok, vi, ti);
                    vi = resolve_idx(vi, (int)temp_vertices.size());
                    ti = resolve_idx(ti, (int)temp_uvs.size());
                    if (vi < 0) { ok = false; break; }

                    std::pair<int, int> key(vi, ti);
                    std::map<std::pair<int, int>, int>::iterator it = corner_map.find(key);

                    if (it != corner_map.end()) {
                        corner.push_back(it->second);
                    } else {
                        if (v_counter >= CAR_MAX_VERTICES) { ok = false; break; }   // uprav na svou konstantu

                        car_vertex &cv = car.vertices[v_counter];
                        cv.x = temp_vertices[vi].x;
                        cv.y = temp_vertices[vi].y;
                        cv.z = temp_vertices[vi].z;
                        cv.group = brnchv;
                        cv.u = (ti >= 0) ? temp_uvs[ti].u : 0.0f;
                        cv.v = (ti >= 0) ? temp_uvs[ti].v : 0.0f;
                        cv.nx = 0.0f; cv.ny = 1.0f; cv.nz = 0.0f;

                        corner_map[key] = v_counter;
                        corner.push_back(v_counter);
                        v_counter++;
                    }
                }
                if (!ok) continue;

                // triangulace vějířem: (0,1,2), (0,2,3), ...
                for (int k = 1; k + 1 < (int)corner.size(); k++) {
                    if (f_counter >= CAR_MAX_POLY) break;                          // uprav na svou konstantu

                    face &cf = car.faces[f_counter];
                    cf.vertices[0] = corner[0];
                    cf.vertices[1] = corner[k];
                    cf.vertices[2] = corner[k + 1];

                    car_vertex &a = car.vertices[cf.vertices[0]];
                    car_vertex &b = car.vertices[cf.vertices[1]];
                    car_vertex &c = car.vertices[cf.vertices[2]];

                    float ax = b.x - a.x, ay = b.y - a.y, az = b.z - a.z;
                    float bx = c.x - a.x, by = c.y - a.y, bz = c.z - a.z;
                    float nx = ay * bz - az * by;
                    float ny = az * bx - ax * bz;
                    float nz = ax * by - ay * bx;
                    float length = sqrt(nx * nx + ny * ny + nz * nz);
                    if (length > 0.0001f) {
                        cf.nx = nx / length; cf.ny = ny / length; cf.nz = nz / length;
                    } else {
                        cf.nx = 0.0f; cf.ny = 1.0f; cf.nz = 0.0f;
                    }
                    f_counter++;
                }
            }
        }
        fclose(f);
    }

    car.faces_count = f_counter;
    car.vertices_count = v_counter;
    printf("[LOAD CAR MESH] loaded %d faces and %d vertices\n", f_counter, v_counter);
}
int merge_duplicate_vertices(cardata& car, float eps = 1e-5f) {
    typedef std::tuple<long long, long long, long long, int> Key;
    std::map<Key, int> seen;                       // pozice+skupina -> nový index
    std::vector<int> remap(car.vertices_count, -1);

    int newCount = 0;
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex &v = car.vertices[i];

        Key key(llroundf(v.x / eps),
                llroundf(v.y / eps),
                llroundf(v.z / eps),
                v.group);

        std::map<Key, int>::iterator it = seen.find(key);
        if (it != seen.end()) {
            remap[i] = it->second;                 // duplicita -> použij existující
        } else {
            remap[i] = newCount;
            seen[key] = newCount;
            // newCount <= i, takže přepsání na místě je bezpečné
            if (newCount != i) car.vertices[newCount] = car.vertices[i];
            newCount++;
            printf("[MERGE] Vertex %d kept as new index %d\n", i, newCount - 1);
        }
    }

    // přepiš indexy ve facech
    for (int i = 0; i < car.faces_count; i++) {
        face &f = car.faces[i];
        for (int k = 0; k < 3; k++) {
            int idx = f.vertices[k];
            if (idx >= 0 && idx < (int)remap.size()) f.vertices[k] = remap[idx];
        }
    }

    int removed = car.vertices_count - newCount;
    car.vertices_count = newCount;
    return removed;                                // kolik vertexů zmizelo
}
void merge_duplicate_vertices_all() {
    for (int i = 0; i < cars.size(); i++) merge_duplicate_vertices(cars[i]);
}
void smooth_normals_car(cardata& car) {
    // 1. vynuluj normály vertexů
    for (int i = 0; i < car.vertices_count; i++) {
        car.vertices[i].nx = 0.0f;
        car.vertices[i].ny = 0.0f;
        car.vertices[i].nz = 0.0f;
    }

    // 2. přičti normálu každého facu jeho třem vertexům
    for (int i = 0; i < car.faces_count; i++) {
        face &f = car.faces[i];
        int i0 = f.vertices[0], i1 = f.vertices[1], i2 = f.vertices[2];
        if (i0 < 0 || i0 >= car.vertices_count ||
            i1 < 0 || i1 >= car.vertices_count ||
            i2 < 0 || i2 >= car.vertices_count) continue;

        car_vertex &v0 = car.vertices[i0];
        car_vertex &v1 = car.vertices[i1];
        car_vertex &v2 = car.vertices[i2];

        float ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
        float bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;

        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        float len = sqrtf(nx * nx + ny * ny + nz * nz);
        if (len < 1e-8f) continue;          // degenerovaný face nic nepřičte
        nx /= len; ny /= len; nz /= len;

        v0.nx += nx; v0.ny += ny; v0.nz += nz;
        v1.nx += nx; v1.ny += ny; v1.nz += nz;
        v2.nx += nx; v2.ny += ny; v2.nz += nz;
    }

    // 3. normalizuj (součet normál -> průměrný směr)
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex &v = car.vertices[i];
        float l = sqrtf(v.nx * v.nx + v.ny * v.ny + v.nz * v.nz);
        if (l > 1e-8f) {
            v.nx /= l; v.ny /= l; v.nz /= l;
        } else {
            v.nx = 0.0f; v.ny = 1.0f; v.nz = 0.0f;   // vertex bez použitelných facu
        }
        // v.nx=0.0f; v.ny=1.0f; v.nz=0.0f; // debug
    }
}
void recalculate_normals() {
    // merge_duplicate_vertices_all();
    for (int mlem=0;mlem<cars.size();mlem++){
        cardata& car=cars[mlem];
        car.flat_shading=false;
        for (int i = 0; i < car.faces_count; i++) {
            face &f = car.faces[i];

            // Získáme aktuální pozice vrcholů trojúhelníku
            car_vertex &v0 = car.vertices[f.vertices[0]];
            car_vertex &v1 = car.vertices[f.vertices[1]];
            car_vertex &v2 = car.vertices[f.vertices[2]];

            // Vektor A = v1 - v0
            float ax = v1.x - v0.x;
            float ay = v1.y - v0.y;
            float az = v1.z - v0.z;

            // Vektor B = v2 - v0
            float bx = v2.x - v0.x;
            float by = v2.y - v0.y;
            float bz = v2.z - v0.z;

            // Vektorový součin (Cross Product) - výpočet kolmice k ploše
            float nx = ay * bz - az * by;
            float ny = az * bx - ax * bz;
            float nz = ax * by - ay * bx;

            // Normalizace (převedení na délku 1.0)
            float length = sqrt(nx * nx + ny * ny + nz * nz);
            
            if (length > 0.0001f) {
                f.nx = nx / length;
                f.ny = ny / length;
                f.nz = nz / length;
            } else {
                // Pokud je trojúhelník zdegenerovaný (má nulovou plochu)
                f.nx = 0.0f;
                f.ny = 1.0f;
                f.nz = 0.0f;
            }
        }










        if (car.flat_shading){continue;} // Pokud je flat shading, nemusíme počítat vertex normály
        smooth_normals_car(car);
    }
}
void reset_vehicle(cardata& car){
    printf("reseting vehicle\n");
    for (i=0;i<MAX_PARTS-1;i++){
        car.points[i].x=0.0f;
        car.points[i].y=0.0f;
        car.points[i].z=0.0f;
        car.points[i].vx=0.0f;
        car.points[i].vy=0.0f;
        car.points[i].vz=0.0f;
        car.points[i].oldx=0.0f;
        car.points[i].oldy=0.0f;
        car.points[i].oldz=0.0f;
        car.joints[i].p1=-1;
        car.joints[i].p2=-1;
        car.joints[i].stiffness=0.0f;
        car.faces[i].group=0;
        car.faces[i].vertices[0]=0;
        car.faces[i].vertices[1]=0;
        car.faces[i].vertices[2]=0;
    }
    printf("reset vehicle done\n");
}
void find_volume_count(cardata& car) {
    printf("find volume count\n");
    int max_group = -1;

    for (int j = 0; j < car.col_faces_count; j++) {
        if (car.col_faces[j].group > max_group) {
            max_group = car.col_faces[j].group;
        }
    }

    // Pokud jsme nenašli žádnou groupu, vrátíme 0, jinak max + 1
    car.volumes_count=max_group + 1;
    printf("    found %d volumes\n", car.volumes_count);
}
void init_vehicle(cardata& car, float spawnz, float spawnx, float spawny){


	for (int i = 0; i < car.joints_count; i++) {

		joint &j = car.joints[i];
		point &a = car.points[j.p1];
		point &b = car.points[j.p2];

		float dx = b.x - a.x;
		float dy = b.y - a.y;
		float dz = b.z - a.z;

		j.rest_len = sqrtf(dx*dx + dy*dy + dz*dz);
		j.default_rest_len = j.rest_len;
        j.default_stiffness=j.stiffness;
	}



	for (i=0;i<car.points_count;i++){
		if (car.points[i].attribute==1){
			car.wheel_LB=i;
		} else if (car.points[i].attribute==2){
			car.wheel_RB=i;
		} else if (car.points[i].attribute==3){
			car.wheel_LF=i;
		} else if (car.points[i].attribute==4){
			car.wheel_RF=i;
		} else if (car.points[i].attribute==7){
			car.up_point=i;
		}
		car.points[i].velocity=0;
		car.points[i].vx=0;
		car.points[i].vy=0;
		car.points[i].vz=0;
	}


	for (i=0;i<car.points_count;i++){
		// float y=car.points[i].z;
		// float x=car.points[i].y;
		// car.points[i].y=y;
		// car.points[i].z=x;

        if (car.spawn_type==0){
		    car.points[i].y+=get_heightmap_height(spawnx,spawnz)+1.0f;
        } else {
		    car.points[i].y+=spawny;
        }
		car.points[i].x+=spawnx;
		car.points[i].z+=spawnz;
		car.points[i].oldx=car.points[i].x;
		car.points[i].oldy=car.points[i].y;
		car.points[i].oldz=car.points[i].z;
	}
    find_volume_count(car);
}
void change_driving_car_vars(){
    engine_power=cars[driving_car].engine_power;
	steering_type=cars[driving_car].steering_type;
	max_steer=cars[driving_car].max_steer;
	steer_speed=cars[driving_car].steer_speed;
    niceni=cars[driving_car].niceni;
    return;
}
void calculate_car_deformation_1(cardata& car) {
    for (int j = 0; j < CAR_MESH_GROUPS; j++) {
        for (int i = 0; i < car.vertices_count; i++) {
            car_vertex &vv = car.vertices[i];
            if (vv.group != j) continue;

            Vec3 vPos = {vv.x, vv.y, vv.z};

            // 1. tři nejbližší fyzikální body (tvůj původní výběr)
            int bestIdx[3] = {-1, -1, -1};
            float bestDist[3] = {1e10, 1e10, 1e10};

            for (int p = 0; p < MAX_PARTS; p++) {
                if (car.points[p].group == j) {
                    Vec3 pPos = {car.points[p].x, car.points[p].y, car.points[p].z};
                    float d = lengthSq(sub(vPos, pPos));

                    if (d < bestDist[0]) {
                        bestDist[2] = bestDist[1]; bestIdx[2] = bestIdx[1];
                        bestDist[1] = bestDist[0]; bestIdx[1] = bestIdx[0];
                        bestDist[0] = d; bestIdx[0] = p;
                    } else if (d < bestDist[1]) {
                        bestDist[2] = bestDist[1]; bestIdx[2] = bestIdx[1];
                        bestDist[1] = d; bestIdx[1] = p;
                    } else if (d < bestDist[2]) {
                        bestDist[2] = d; bestIdx[2] = p;
                    }
                }
            }

            // fallback: méně než 3 body ve skupině -> vertex se nehýbe
            if (bestIdx[0] == -1 || bestIdx[1] == -1 || bestIdx[2] == -1) {
                vv.fyz_ref_points[0] = vv.fyz_ref_points[1] = vv.fyz_ref_points[2] = -1;
                vv.weights[0] = 1; vv.weights[1] = 0; vv.weights[2] = 0;
                vv.loc[0] = vv.loc[1] = vv.loc[2] = 0;
                vv.restScale = 0;
                continue;
            }

            int id[3] = {bestIdx[0], bestIdx[1], bestIdx[2]};
            Vec3 Q[3];
            for (int k = 0; k < 3; k++)
                Q[k] = {car.points[id[k]].x, car.points[id[k]].y, car.points[id[k]].z};

            // 2. seřaď tak, aby nejdelší hrana byla mezi body 0 a 1
            float lAB = lengthSq(sub(Q[1], Q[0]));
            float lAC = lengthSq(sub(Q[2], Q[0]));
            float lBC = lengthSq(sub(Q[2], Q[1]));
            if (lAC >= lAB && lAC >= lBC)      { std::swap(id[1], id[2]); std::swap(Q[1], Q[2]); }
            else if (lBC >= lAB && lBC >= lAC) { std::swap(id[0], id[2]); std::swap(Q[0], Q[2]); }

            Vec3 A = Q[0], B = Q[1], C = Q[2];
            Vec3 v0 = sub(B, A), v1 = sub(C, A), v2 = sub(vPos, A);

            // 3. barycentrické váhy
            float d00 = dot(v0, v0), d01 = dot(v0, v1), d11 = dot(v1, v1);
            float d20 = dot(v2, v0), d21 = dot(v2, v1);
            float denom = d00 * d11 - d01 * d01;

            float w[3];
            if (fabsf(denom) < 1e-12f) {
                // kolineární body: vertex se přilepí na nejbližší bod
                w[0] = 1; w[1] = 0; w[2] = 0;
            } else {
                w[1] = (d11 * d20 - d01 * d21) / denom;
                w[2] = (d00 * d21 - d01 * d20) / denom;
                w[0] = 1.0f - w[1] - w[2];
            }

            // 4. ořež na <0,1> a přenormuj
            for (int k = 0; k < 3; k++) w[k] = fmaxf(0.0f, w[k]);
            float s = w[0] + w[1] + w[2];
            if (s < 1e-8f) { w[0] = 1; w[1] = w[2] = 0; s = 1; }
            for (int k = 0; k < 3; k++) w[k] /= s;

            vv.fyz_ref_points[0] = id[0];
            vv.fyz_ref_points[1] = id[1];
            vv.fyz_ref_points[2] = id[2];
            vv.weights[0] = w[0]; vv.weights[1] = w[1]; vv.weights[2] = w[2];

            // 5. bod na trojúhelníku a zbytek v lokálním rámu
            Vec3 P = {A.x*w[0] + B.x*w[1] + C.x*w[2],
                      A.y*w[0] + B.y*w[1] + C.y*w[2],
                      A.z*w[0] + B.z*w[1] + C.z*w[2]};

            Vec3 d = sub(vPos, P);
            Vec3 cr = cross(v0, v1);
            float crLen = sqrtf(dot(cr, cr));

            if (crLen < 1e-8f || d00 < 1e-12f) {
                // degenerovaný rám: ulož světový offset (update to pozná stejnou podmínkou)
                vv.loc[0] = d.x; vv.loc[1] = d.y; vv.loc[2] = d.z;
            } else {
                Vec3 nrm = {cr.x / crLen, cr.y / crLen, cr.z / crLen};
                Vec3 e1  = normalize(v0);
                Vec3 e2  = cross(nrm, e1);
                vv.loc[0] = dot(d, e1);
                vv.loc[1] = dot(d, e2);
                vv.loc[2] = dot(d, nrm);
            }

            vv.restScale = (sqrtf(lAB) + sqrtf(lAC) + sqrtf(lBC)) / 3.0f;
        }
    }
}
void update_car_mesh_1(cardata& car) {
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex &vv = car.vertices[i];
        int r0 = vv.fyz_ref_points[0], r1 = vv.fyz_ref_points[1], r2 = vv.fyz_ref_points[2];
        if (r0 < 0 || r0 >= car.points_count ||
            r1 < 0 || r1 >= car.points_count ||
            r2 < 0 || r2 >= car.points_count) continue;

        Vec3 A = {car.points[r0].x, car.points[r0].y, car.points[r0].z};
        Vec3 B = {car.points[r1].x, car.points[r1].y, car.points[r1].z};
        Vec3 C = {car.points[r2].x, car.points[r2].y, car.points[r2].z};

        // 1. pozice na trojúhelníku (kladné váhy, nic se nezesiluje)
        Vec3 P = {A.x*vv.weights[0] + B.x*vv.weights[1] + C.x*vv.weights[2],
                  A.y*vv.weights[0] + B.y*vv.weights[1] + C.y*vv.weights[2],
                  A.z*vv.weights[0] + B.z*vv.weights[1] + C.z*vv.weights[2]};

        Vec3 cr = cross(sub(B, A), sub(C, A));
        float len2 = dot(cr, cr);
        Vec3 AB = sub(B, A);
        float lenAB2 = dot(AB, AB);

        if (len2 < 1e-12f || lenAB2 < 1e-12f) {
            // degenerovaný trojúhelník: jen posun
            vv.x = P.x + vv.loc[0];
            vv.y = P.y + vv.loc[1];
            vv.z = P.z + vv.loc[2];
            continue;
        }

        // 2. tuhý rám trojúhelníku
        float inv = 1.0f / sqrtf(len2);
        Vec3 nrm = {cr.x * inv, cr.y * inv, cr.z * inv};
        float invAB = 1.0f / sqrtf(lenAB2);
        Vec3 e1 = {AB.x * invAB, AB.y * invAB, AB.z * invAB};
        Vec3 e2 = cross(nrm, e1);

        // 3. jemné škálování zbytku podle celkové změny velikosti trojúhelníku (omezené)
        float sc = 1.0f;
        if (vv.restScale > 1e-8f) {
            float cur = (sqrtf(lenAB2) + sqrtf(lengthSq(sub(C, A))) + sqrtf(lengthSq(sub(C, B)))) / 3.0f;
            sc = fmaxf(0.5f, fminf(2.0f, cur / vv.restScale));
        }
        float l0 = vv.loc[0] * sc, l1 = vv.loc[1] * sc, l2 = vv.loc[2] * sc;

        vv.x = P.x + e1.x*l0 + e2.x*l1 + nrm.x*l2;
        vv.y = P.y + e1.y*l0 + e2.y*l1 + nrm.y*l2;
        vv.z = P.z + e1.z*l0 + e2.z*l1 + nrm.z*l2;
    }
}
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <iostream>

// Pomocná funkce pro ověření existence souboru
bool file_exists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0);
}

void discover_car_skin_variants(MenuNode& car) {
    car.skin_variants.clear();
    car.skin_icons.clear();
    car.skin_variants.push_back(0);

    DIR* directory = opendir(car.path.c_str());
    if (directory) {
        struct dirent* entry;
        while ((entry = readdir(directory)) != nullptr) {
            const char* filename = entry->d_name;
            if (strncmp(filename, "texture", 7) != 0) continue;

            char* suffix_end = nullptr;
            long suffix = strtol(filename + 7, &suffix_end, 10);
            if (suffix < 1 || suffix_end == filename + 7 ||
                strcmp(suffix_end, ".png") != 0) {
                continue;
            }

            if (file_exists(car.path + "/" + filename)) {
                car.skin_variants.push_back((int)suffix);
            }
        }
        closedir(directory);
    }

    std::sort(car.skin_variants.begin(), car.skin_variants.end());
    car.skin_variants.erase(
        std::unique(car.skin_variants.begin(), car.skin_variants.end()),
        car.skin_variants.end()
    );
    car.selected_skin = car.skin_variants.front();

    for (int suffix : car.skin_variants) {
        if (suffix == 0) {
            car.skin_icons.push_back(nullptr);
            continue;
        }

        std::string icon_path = car.path + "/icon" + std::to_string(suffix) + ".png";
        if (!file_exists(icon_path) && suffix == 1) {
            std::string second_skin_icon_path = car.path + "/icon2.png";
            if (file_exists(second_skin_icon_path)) {
                icon_path = second_skin_icon_path;
            }
        }
        car.skin_icons.push_back(file_exists(icon_path) ? IMG_Load(icon_path.c_str()) : nullptr);
    }
}

// Prototyp funkce pro rekurzi
void scan_dir_architecture(const std::string& path, int parent_category_idx);

// Společná logika pro zpracování nalezené složky
void process_directory(const std::string& base_path, const std::string& d_name, int parent_category_idx) {
    std::string full_path = base_path + "/" + d_name;
    std::string json_path = full_path + "/vehicle.json";

    printf("[discover vehicles] Kontrola složky: %s\n", full_path.c_str());

    if (file_exists(json_path)) {
        // --- JE TO AUTO ---
        printf("[discover vehicles]   -> Nalezeno AUTO (vehicle.json existuje)\n");
        
        MenuNode car;
        car.path = full_path;
        
        // --- ÚPRAVA: Skládání názvu auta s kategorií ---
        if (parent_category_idx != -1) {
            // Pokud má auto rodiče, spojíme: "název_rodiče/název_auta"
            car.name = menu_items[parent_category_idx].name + "/" + d_name;
        } else {
            // Pokud je auto přímo v rootu, zůstane jen čistý název složky
            car.name = d_name;
        }
        
        car.is_category = false;
        discover_car_skin_variants(car);

        // Načtení ikony auta
        std::string icon_path = full_path + "/icon.png";
        car.icon = IMG_Load(icon_path.c_str());
        // ... (zbytek kódu zůstává úplně stejný)
        
        
        if (!car.icon) {
            printf("[discover vehicles]   [CHYBA] Nepodařilo se načíst ikonu auta (%s): %s\n", icon_path.c_str(), IMG_GetError());
        } else {
            printf("[discover vehicles]   -> Ikona načtena úspěšně\n");
        }

        // Uložíme do hlavního seznamu a získáme index
        menu_items.push_back(car);
        int current_idx = (int)menu_items.size() - 1;

        // Zařazení do struktury menu podle toho, kde se nacházíme
        if (parent_category_idx != -1) {
            printf("[discover vehicles]   -> Přidávám auto do kategorie s indexem %d\n", parent_category_idx);
            menu_items[parent_category_idx].sub_items.push_back(current_idx);
        } else {
            printf("[discover vehicles]   -> Přidávám auto do hlavního menu (root)\n");
            current_root_items.push_back(current_idx);
        }
    } 
    else if (parent_category_idx == -1) {
        // --- JE TO KATEGORIE (složka v rootu bez jsonu) ---
        printf("[discover vehicles]   -> Nalezena KATEGORIE (vnořená složka)\n");
        
        MenuNode cat;
        cat.path = full_path;
        cat.name = d_name;
        cat.is_category = true;

        // Načtení ikony kategorie
        std::string icon_path = full_path + "/icon.png";
        cat.icon = IMG_Load(icon_path.c_str());
        if (!cat.icon) {
            printf("[discover vehicles]   [CHYBA] Nepodařilo se načíst ikonu kategorie (%s): %s\n", icon_path.c_str(), IMG_GetError());
        } else {
            printf("[discover vehicles]   -> Ikona kategorie načtena úspěšně\n");
        }

        // Uložíme do hlavního seznamu a získáme index
        menu_items.push_back(cat);
        int current_idx = (int)menu_items.size() - 1;

        // Kategorie patří do hlavního menu
        current_root_items.push_back(current_idx);

        // Rekurzivní prohledání vnitřku kategorie
        printf("[discover vehicles] -> Vstupuji rekurzivně do kategorie: %s\n", full_path.c_str());
        scan_dir_architecture(full_path, current_idx);
    } else {
        printf("[discover vehicles]   -> Ignoruji vnořenou složku (hlubší zanoření než 1 úroveň nepodporujeme)\n");
    }
}

void scan_dir_architecture(const std::string& path, int parent_category_idx) {
    struct dirent *entry;
    DIR *dp = opendir(path.c_str());
    if (dp == NULL) {
        printf("[discover vehicles] [CHYBA] Nepodařilo se otevřít složku %s\n", path.c_str());
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        bool is_directory = false;

        #ifdef _WIN32
        std::string full_path_win = path + "/" + entry->d_name;
        struct stat pathname_stat;
        if (stat(full_path_win.c_str(), &pathname_stat) == 0) {
            if (S_ISDIR(pathname_stat.st_mode)) {
                is_directory = true;
            }
        }
        #else
        if (entry->d_type == DT_DIR) {
            is_directory = true;
        }
        #endif

        if (is_directory) {
            process_directory(path, entry->d_name, parent_category_idx);
        }
    }
    closedir(dp);
}

// Hlavní spouštěcí funkce
void discoverVehicles() {
    printf("[discover vehicles] START skenování složky assets/cars...\n");
    
    menu_items.clear();
    current_root_items.clear();
    car_names.clear(); // Vyčistíme tvůj std::vector<std::string>
    current_category_index = -1;

    scan_dir_architecture("assets/cars", -1);

    printf("[discover vehicles] Skenování dokončeno.\n");
    printf("[discover vehicles] Celkem prvků v menu_items: %zu\n", menu_items.size());
    printf("[discover vehicles] Položek v hlavním menu: %zu\n", current_root_items.size());

    // --- BEZPEČNÉ NAPLNĚNÍ VECTORU CAR_NAMES ---
    printf("[discover vehicles] Zahajuji plnění std::vector<std::string> car_names...\n");
    for (size_t i = 0; i < menu_items.size(); i++) {
        // Použijeme push_back, což u vektoru zabrání segfaultu
        car_names.push_back(menu_items[i].name);
    }
    
    number_of_vehicles = (int)car_names.size(); 
    printf("[discover vehicles] Vektor car_names úspěšně naplněn (%d prvků).\n", number_of_vehicles);
}
void calculate_car_deformation_2(cardata& car) {
    for (int j=0; j<CAR_MESH_GROUPS; j++) {
        for (int i = 0; i < car.vertices_count; i++) {
            car_vertex &vv = car.vertices[i];
            if (vv.group == j) {
                Vec3 vPos = {vv.x, vv.y, vv.z};

                // Find 3 closest physics points of same group
                int bestIdx[3] = {-1, -1, -1};
                float bestDist[3] = {1e10, 1e10, 1e10};

                for (int p = 0; p < MAX_PARTS; p++) {
                    if (car.points[p].group == j) {
                        Vec3 pPos = {car.points[p].x, car.points[p].y, car.points[p].z};
                        float d = lengthSq(sub(vPos, pPos));

                        if (d < bestDist[0]) {
                            bestDist[2] = bestDist[1]; bestIdx[2] = bestIdx[1];
                            bestDist[1] = bestDist[0]; bestIdx[1] = bestIdx[0];
                            bestDist[0] = d; bestIdx[0] = p;
                        } else if (d < bestDist[1]) {
                            bestDist[2] = bestDist[1]; bestIdx[2] = bestIdx[1];
                            bestDist[1] = d; bestIdx[1] = p;
                        } else if (d < bestDist[2]) {
                            bestDist[2] = d; bestIdx[2] = p;
                        }
                    }
                }
        
                if (vv.fyz_ref_points[0]==-1 || vv.fyz_ref_points[1]==-1 ||vv.fyz_ref_points[2]==-1){
                    vv.fyz_ref_points[0] = 0;
                    vv.fyz_ref_points[1] = 0;
                    vv.fyz_ref_points[2] = 0;
                    vv.weights[0] = 1.0f;
                    vv.weights[1] = 0.0f;
                    vv.weights[2] = 0.0f;
                    vv.height_offset = 0.0f;
                    continue;
                }

                vv.fyz_ref_points[0] = bestIdx[0];
                vv.fyz_ref_points[1] = bestIdx[1];
                vv.fyz_ref_points[2] = bestIdx[2];

                // Calculate barycentric weights
                Vec3 A = {car.points[bestIdx[0]].x, car.points[bestIdx[0]].y, car.points[bestIdx[0]].z};
                Vec3 B = {car.points[bestIdx[1]].x, car.points[bestIdx[1]].y, car.points[bestIdx[1]].z};
                Vec3 C = {car.points[bestIdx[2]].x, car.points[bestIdx[2]].y, car.points[bestIdx[2]].z};

                Vec3 v0 = sub(B, A);
                Vec3 v1 = sub(C, A);
                Vec3 v2 = sub(vPos, A);
                
                float d00 = dot(v0, v0);
                float d01 = dot(v0, v1);
                float d11 = dot(v1, v1);
                float d20 = dot(v2, v0);
                float d21 = dot(v2, v1);
                float denom = d00 * d11 - d01 * d01;

                if (fabs(denom) > 1e-6) {
                    vv.weights[1] = (d11 * d20 - d01 * d21) / denom;
                    vv.weights[2] = (d00 * d21 - d01 * d20) / denom;
                    vv.weights[0] = 1.0f - vv.weights[1] - vv.weights[2];
                } else {
                    vv.weights[0] = 0.33f;
                    vv.weights[1] = 0.33f;
                    vv.weights[2] = 0.34f;
                }

                // Calculate relative offset from the triangle center in LOCAL CAR SPACE
                Vec3 triangleCenter = {
                    A.x * vv.weights[0] + B.x * vv.weights[1] + C.x * vv.weights[2],
                    A.y * vv.weights[0] + B.y * vv.weights[1] + C.y * vv.weights[2],
                    A.z * vv.weights[0] + B.z * vv.weights[1] + C.z * vv.weights[2]
                };
                
                vv.relative_offset = sub(vPos, triangleCenter);
            }
        }
    }
}
Vec3 rotate_vector_by_car(cardata& car, Vec3 v) {
    // 1. Směr DOPŘEDU (otočený o 180 - prohozeno back a front)
    Vec3 fwd = {
        ((car.points[car.wheel_LB].x + car.points[car.wheel_RB].x) * 0.5f) - ((car.points[car.wheel_LF].x + car.points[car.wheel_RF].x) * 0.5f),
        ((car.points[car.wheel_LB].y + car.points[car.wheel_RB].y) * 0.5f) - ((car.points[car.wheel_LF].y + car.points[car.wheel_RF].y) * 0.5f),
        ((car.points[car.wheel_LB].z + car.points[car.wheel_RB].z) * 0.5f) - ((car.points[car.wheel_LF].z + car.points[car.wheel_RF].z) * 0.5f)
    };
    fwd = normalize(fwd);

    // 2. Směr DOPRAVA (otočený o 180 - prohozeno RF a LF)
    Vec3 side = {
        car.points[car.wheel_LF].x - car.points[car.wheel_RF].x,
        car.points[car.wheel_LF].y - car.points[car.wheel_RF].y,
        car.points[car.wheel_LF].z - car.points[car.wheel_RF].z
    };
    side = normalize(side);

    // 3. Směr NAHORU (Zůstává stejný, protože (-fwd) x (-side) = fwd x side)
    Vec3 up = {
        fwd.y * side.z - fwd.z * side.y,
        fwd.z * side.x - fwd.x * side.z,
        fwd.x * side.y - fwd.y * side.x
    };
    up = normalize(up);

    // 4. Finální transformace
    Vec3 result;
    result.x = v.x * side.x + v.y * up.x + v.z * fwd.x;
    result.y = v.x * side.y + v.y * up.y + v.z * fwd.y;
    result.z = v.x * side.z + v.y * up.z + v.z * fwd.z;

    return result;
}
float get_car_rotation_x(cardata& car) {
    // 1. Směrový vektor DOPŘEDU
    float fwdX = ((car.points[car.wheel_LF].x + car.points[car.wheel_RF].x) * 0.5f) - 
                 ((car.points[car.wheel_LB].x + car.points[car.wheel_RB].x) * 0.5f);
    float fwdY = ((car.points[car.wheel_LF].y + car.points[car.wheel_RF].y) * 0.5f) - 
                 ((car.points[car.wheel_LB].y + car.points[car.wheel_RB].y) * 0.5f);
    float fwdZ = ((car.points[car.wheel_LF].z + car.points[car.wheel_RF].z) * 0.5f) - 
                 ((car.points[car.wheel_LB].z + car.points[car.wheel_RB].z) * 0.5f);

    // 2. Směrový vektor DOPRAVA
    float rightX = car.points[car.wheel_RF].x - car.points[car.wheel_LF].x;
    float rightY = car.points[car.wheel_RF].y - car.points[car.wheel_LF].y;
    float rightZ = car.points[car.wheel_RF].z - car.points[car.wheel_LF].z;

    // 3. Vektor NAHORU (pomocný bod střechy pomocí křížového součinu)
    float upY = (fwdZ * rightX) - (fwdX * rightZ);

    // 4. Výpočet úhlu
    float distXZ = sqrtf(fwdX * fwdX + fwdZ * fwdZ);
    float angleX = atan2f(fwdY, distXZ);

    // FIX: Pokud upY míří dolů, auto je na střeše
    if (upY < 0) {
        angleX = (fwdY > 0) ? (PI - angleX) : (-PI - angleX);
    }

    return angleX;
}
float get_car_rotation_z(cardata& car) {
    // 1. Vektory (znovu, pro jistotu kompletní funkce)
    float fwdX = ((car.points[car.wheel_LF].x + car.points[car.wheel_RF].x) * 0.5f) - 
                 ((car.points[car.wheel_LB].x + car.points[car.wheel_RB].x) * 0.5f);
    float fwdZ = ((car.points[car.wheel_LF].z + car.points[car.wheel_RF].z) * 0.5f) - 
                 ((car.points[car.wheel_LB].z + car.points[car.wheel_RB].z) * 0.5f);

    float rightX = car.points[car.wheel_RF].x - car.points[car.wheel_LF].x;
    float rightY = car.points[car.wheel_RF].y - car.points[car.wheel_LF].y;
    float rightZ = car.points[car.wheel_RF].z - car.points[car.wheel_LF].z;

    // 2. Vektor NAHORU
    float upY = (fwdZ * rightX) - (fwdX * rightZ);

    // 3. Výpočet úhlu náklonu
    float distSideXZ = sqrtf(rightX * rightX + rightZ * rightZ);
    float angleZ = atan2f(rightY, distSideXZ);

    // FIX: Pokud jsme na střeše, Roll musí jít k PI (180 stupňů)
    if (upY < 0) {
        angleZ = (rightY > 0) ? (PI - angleZ) : (-PI - angleZ);
    }

    return angleZ;
}
void update_car_mesh_2(cardata& car) {
    car.rot_y=get_car_rotation_y(car);
    car.rot_x=get_car_rotation_x(car);
    car.rot_z=get_car_rotation_z(car);
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex &vv = car.vertices[i];
        if (vv.fyz_ref_points[0]==-1 || vv.fyz_ref_points[1]==-1 ||vv.fyz_ref_points[2]==-1){continue;}
        point &pA = car.points[vv.fyz_ref_points[0]];
        point &pB = car.points[vv.fyz_ref_points[1]];
        point &pC = car.points[vv.fyz_ref_points[2]];
        
        Vec3 A = {pA.x, pA.y, pA.z};
        Vec3 B = {pB.x, pB.y, pB.z};
        Vec3 C = {pC.x, pC.y, pC.z};

        // Calculate triangle center using barycentric weights
        Vec3 triangleCenter = {
            A.x * vv.weights[0] + B.x * vv.weights[1] + C.x * vv.weights[2],
            A.y * vv.weights[0] + B.y * vv.weights[1] + C.y * vv.weights[2],
            A.z * vv.weights[0] + B.z * vv.weights[1] + C.z * vv.weights[2]
        };

        // Rotate the relative offset by car's current rotation
        Vec3 rotatedOffset = rotate_vector_by_car(car, vv.relative_offset);

        // Final vertex position = triangle center + rotated offset
        vv.x = triangleCenter.x + rotatedOffset.x;
        vv.y = triangleCenter.y + rotatedOffset.y;
        vv.z = triangleCenter.z + rotatedOffset.z;
    }
}
void calculate_car_deformation_0(cardata& car){
    for (i=0;i<car.vertices_count;i++){
        car_vertex& v=car.vertices[i];
        Vec3 v_pos={v.x, v.y, v.z};
        float best_dist=1e15f;
        int best_ind=0;
        for (j=0;j<car.points_count;j++){
            point& p=car.points[j];
            Vec3 p_pos={p.x, p.y, p.z};
            float dist=get_dist(v_pos,p_pos);
            if (dist<best_dist){
                best_dist=dist;
                best_ind=j;
            }
        }
        v.fyz_ref_points[0]=best_ind;
    }
}
void update_car_mesh_0(cardata& car){
    for (i=0;i<car.vertices_count;i++){
        car_vertex& v=car.vertices[i];
        point& p=car.points[v.fyz_ref_points[0]];
        v.x=p.x;
        v.y=p.y;
        v.z=p.z;
    }
}

#include "system3.hpp"
#include "system4.hpp"
static bool strain_display_force = false;

void toggle_strain_display_mode(){
    strain_display_force=!strain_display_force;
    printf("Technical car view: %s\n", strain_display_force ? "joint force" : "strain");
}

void calculate_car_deformation(){
    printf("calculating car deformation\n");
    for (int mlem=0;mlem<cars.size();mlem++){
        cardata& car=cars[mlem];
        if (car.deformation_system==0){
            calculate_car_deformation_0(car);
        } else if (car.deformation_system==1){
            calculate_car_deformation_1(car);
        } else if (car.deformation_system==2){
            calculate_car_deformation_2(car);
        } else if (car.deformation_system==3){
            calculate_car_deformation_3(car);
        } else if (car.deformation_system==4){
            calculate_car_deformation_4(car);
        }
    }
}
void update_car_mesh(){
    for (int mlem=0;mlem<cars.size();mlem++){
        cardata& car=cars[mlem];
        if (car.deformation_system==0){
            update_car_mesh_0(car);
        } else if (car.deformation_system==1){
            update_car_mesh_1(car);
        } else if (car.deformation_system==2){
            update_car_mesh_2(car);
        } else if (car.deformation_system==3){
            update_car_mesh_3(car);
        } else if (car.deformation_system==4){
            update_car_mesh_4(car);
        }
        if (technical_render){
            car.point_strain.resize(car.points_count, 0.0f);
            car.point_strain_joint_count.resize(car.points_count, 0);
            std::fill(car.point_strain.begin(), car.point_strain.end(), 0.0f);
            std::fill(car.point_strain_joint_count.begin(), car.point_strain_joint_count.end(), 0);

            std::vector<float> joint_forces;
            std::vector<unsigned char> valid_force_joints;
            if (strain_display_force){
                joint_forces.resize(car.joints_count, 0.0f);
                valid_force_joints.resize(car.joints_count, 0);
            }

            for (int joint_index=0; joint_index<car.joints_count; joint_index++){
                const joint& j=car.joints[joint_index];
                if (j.p1<0 || j.p1>=car.points_count || j.p2<0 || j.p2>=car.points_count){
                    continue;
                }

                const point& a=car.points[j.p1];
                const point& b=car.points[j.p2];
                float dx=b.x-a.x;
                float dy=b.y-a.y;
                float dz=b.z-a.z;
                float distance=sqrtf(dx*dx+dy*dy+dz*dz);
                float value=0.0f;
                if (strain_display_force){
                    if (j.snapped || distance<1e-6f ||
                        (j.attribute==ATTRIBUTE_ICJ && !(distance<j.rest_len-j.rest_len*j.elastic_margin))){
                        continue;
                    }

                    float displacement=distance-j.rest_len;
                    float spring_force=j.default_stiffness*displacement*20000.0f;

                    float relative_velocity=((b.vx-a.vx)*dx+(b.vy-a.vy)*dy+(b.vz-a.vz)*dz)/distance;
                    float damping_force=j.damping*300.0f*relative_velocity;
                    float force_limit=fabsf(car.force_clamp);
                    float force=fmaxf(-force_limit,fminf(spring_force+damping_force,force_limit));
                    joint_forces[joint_index]=fabsf(force);
                    valid_force_joints[joint_index]=1;
                } else {
                    if (j.snapped){
                        value=1.0f;
                    } else {
                        float default_length=fmaxf(j.default_rest_len, 0.001f);
                        float relative_stretch=fabsf(distance-j.default_rest_len)/default_length;
                        value=fminf(relative_stretch*10.0f, 1.0f);
                    }
                    car.point_strain[j.p1]+=value;
                    car.point_strain[j.p2]+=value;
                    car.point_strain_joint_count[j.p1]++;
                    car.point_strain_joint_count[j.p2]++;
                }
            }

            if (strain_display_force){
                for (int joint_index=0; joint_index<car.joints_count; joint_index++){
                    if (!valid_force_joints[joint_index]){
                        continue;
                    }
                    const joint& j=car.joints[joint_index];
                    float force=joint_forces[joint_index];
                    car.point_strain[j.p1]+=force;
                    car.point_strain[j.p2]+=force;
                    car.point_strain_joint_count[j.p1]++;
                    car.point_strain_joint_count[j.p2]++;
                }
            }

            float max_point_force=0.0f;
            for (int point_index=0; point_index<car.points_count; point_index++){
                unsigned int joint_count=car.point_strain_joint_count[point_index];
                if (joint_count>0){
                    car.point_strain[point_index]/=(float)joint_count;
                }
                if (strain_display_force){
                    max_point_force=fmaxf(max_point_force, car.point_strain[point_index]);
                }
            }
            if (strain_display_force && max_point_force>0.0f){
                for (float& force : car.point_strain){
                    force/=max_point_force;
                }
            }

            car.vertex_strain.resize(car.vertices_count, 0.0f);
            for (int vertex_index=0; vertex_index<car.vertices_count; vertex_index++){
                const car_vertex& vertex=car.vertices[vertex_index];
                car.vertex_strain[vertex_index]=0.0f;
                float weighted_strain=0.0f;
                float total_weight=0.0f;
                auto add_influence=[&](int slot, float weight){
                    int point_index=vertex.fyz_ref_points[slot];
                    if (point_index<0 || point_index>=car.points_count){
                        return;
                    }
                    weight=fabsf(weight);
                    weighted_strain+=car.point_strain[point_index]*weight;
                    total_weight+=weight;
                };

                if (car.deformation_system==0){
                    add_influence(0, 1.0f);
                } else if (car.deformation_system==4){
                    add_influence(0, vertex.blend_factor);
                    add_influence(1, 1.0f-vertex.blend_factor);
                } else {
                    add_influence(0, vertex.weights[0]);
                    add_influence(1, vertex.weights[1]);
                    add_influence(2, vertex.weights[2]);
                    if (car.deformation_system==3 && vertex.fyz_ref_points[3]>=0){
                        add_influence(3, vertex.weights[3]);
                    }
                }

                if (total_weight>0.0f){
                    car.vertex_strain[vertex_index]=weighted_strain/total_weight;
                } else {
                    int influence_count=car.deformation_system==0 ? 1 :
                        (car.deformation_system==3 && vertex.fyz_ref_points[3]>=0 ? 4 :
                        (car.deformation_system==4 ? 2 : 3));
                    for (int slot=0; slot<influence_count; slot++){
                        int point_index=vertex.fyz_ref_points[slot];
                        if (point_index>=0 && point_index<car.points_count){
                            car.vertex_strain[vertex_index]+=car.point_strain[point_index];
                            total_weight+=1.0f;
                        }
                    }
                    if (total_weight>0.0f){
                        car.vertex_strain[vertex_index]/=total_weight;
                    }
                }
            }
        }
    }
}
