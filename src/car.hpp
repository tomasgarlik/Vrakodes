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
            p.exists = true;

            sscanf(line,
                " {\"x\":%f,\"y\":%f,\"z\":%f,\"mass\":%f,\"attribute\":%d,\"group\":%d,\"friction\":%f,\"collide\":%d}",
                &p.x, &p.y, &p.z, &p.mass, &p.attribute, &p.group, &p.friction, &p.collide
            );
            car.points[points_count++] = p;
        }

        // ---- JOINT ----
        if (strstr(line, "\"p1\"") && strstr(line, "\"p2\"") && strstr(line, "\"stiffness\"")) {
            joint j;
            j.exists = true;

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
void load_car_mesh(cardata& car) {
    printf("loading car mesh %s...\n", car_name.c_str());
    int v_counter = 0;
    int f_counter = 0;

    char files[CAR_MESH_GROUPS][256]; // 5 řetězcu, každý max 256 znaků

    // Naplnění cest
    snprintf(files[0], sizeof(files[0]), "assets/cars/%s/mesh_g1.obj", car_name.c_str());
    snprintf(files[1], sizeof(files[1]), "assets/cars/%s/mesh_g2.obj", car_name.c_str());
    snprintf(files[2], sizeof(files[2]), "assets/cars/%s/mesh_g3.obj", car_name.c_str());
    snprintf(files[3], sizeof(files[3]), "assets/cars/%s/mesh_g4.obj", car_name.c_str());
    snprintf(files[4], sizeof(files[4]), "assets/cars/%s/mesh_g5.obj", car_name.c_str());
    snprintf(files[5], sizeof(files[5]), "assets/cars/%s/mesh_g6.obj", car_name.c_str());
    snprintf(files[6], sizeof(files[6]), "assets/cars/%s/mesh_g7.obj", car_name.c_str());
    snprintf(files[7], sizeof(files[7]), "assets/cars/%s/mesh_g8.obj", car_name.c_str());
    snprintf(files[8], sizeof(files[8]), "assets/cars/%s/mesh_g9.obj", car_name.c_str());
    snprintf(files[9], sizeof(files[9]), "assets/cars/%s/mesh_g10.obj", car_name.c_str());

    char path[256];
    sprintf(path,"assets/cars/%s/texture.png", car_name.c_str());
    car.texture=load_texture_PNG(path);
    sprintf(path,"assets/cars/%s/roughness.png", car_name.c_str());
    car.roughness=load_texture_PNG(path);
    if (!car.roughness){car.roughness=texture;}
    sprintf(path,"assets/cars/%s/specular.png", car_name.c_str());
    car.specular=load_texture_PNG(path);
    if (!car.specular){car.specular=texture;}
    sprintf(path,"assets/cars/%s/metallic.png", car_name.c_str());
    car.metallic=load_texture_PNG(path);
    if (!car.metallic){car.metallic=texture;}
    sprintf(path,"assets/cars/%s/normal.png", car_name.c_str());
    car.normal=load_texture_PNG(path);
    if (!car.normal){car.normal=texture;}
    for (int brnchv = 0; brnchv < CAR_MESH_GROUPS; brnchv++) {
        if (brnchv==2){continue;}
        std::vector<ObjVertex> temp_vertices;
        std::vector<ObjUV> temp_uvs;
        
        FILE* f = fopen(files[brnchv], "r");
        if (!f) { printf("error loading car mesh for group %d\n", brnchv);continue; }

        char line[512];
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
                int v_idx[3], vt_idx[3], vn_idx[3];
                // Zkusíme načíst formáty s UV i bez
                int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &v_idx[0], &vt_idx[0], &vn_idx[0], &v_idx[1], &vt_idx[1], &vn_idx[1], &v_idx[2], &vt_idx[2], &vn_idx[2]);
                
                bool has_uv = (matches >= 6);
                if (matches < 3) { // f v1 v2 v3
                    matches = sscanf(line, "f %d %d %d", &v_idx[0], &v_idx[1], &v_idx[2]);
                    has_uv = false;
                }

                if (matches >= 3) {
                    face &current_face = car.faces[f_counter];

                    // 1. Uložíme vrcholy
                    for (int i = 0; i < 3; i++) {
                        int vi = v_idx[i] - 1;
                        car.vertices[v_counter].x = temp_vertices[vi].x;
                        car.vertices[v_counter].y = temp_vertices[vi].y;
                        car.vertices[v_counter].z = temp_vertices[vi].z;
                        car.vertices[v_counter].group = brnchv;

                        if (has_uv) {
                            int ti = vt_idx[i] - 1;
                            car.vertices[v_counter].u = temp_uvs[ti].u;
                            car.vertices[v_counter].v = temp_uvs[ti].v;
                        }
                        current_face.vertices[i] = v_counter;
                        v_counter++;
                    }

                    // 2. VÝPOČET NORMÁLY PLOCHY (Flat Shading)
                    // Vektor A = V1 - V0, Vektor B = V2 - V0
                    float ax = car.vertices[current_face.vertices[1]].x - car.vertices[current_face.vertices[0]].x;
                    float ay = car.vertices[current_face.vertices[1]].y - car.vertices[current_face.vertices[0]].y;
                    float az = car.vertices[current_face.vertices[1]].z - car.vertices[current_face.vertices[0]].z;

                    float bx = car.vertices[current_face.vertices[2]].x - car.vertices[current_face.vertices[0]].x;
                    float by = car.vertices[current_face.vertices[2]].y - car.vertices[current_face.vertices[0]].y;
                    float bz = car.vertices[current_face.vertices[2]].z - car.vertices[current_face.vertices[0]].z;

                    // Vektorový součin (Cross Product) pro kolmici
                    float nx = ay * bz - az * by;
                    float ny = az * bx - ax * bz;
                    float nz = ax * by - ay * bx;

                    // Normalizace (aby měl vektor délku 1)
                    float length = sqrt(nx*nx + ny*ny + nz*nz);
                    if (length > 0) {
                        current_face.nx = nx / length;
                        current_face.ny = ny / length;
                        current_face.nz = nz / length;
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
void recalculate_normals() {
    for (int mlem=0;mlem<cars.size();mlem++){
        cardata& car=cars[mlem];
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
    }
}
void reset_vehicle(cardata& car){
    printf("reseting vehicle\n");
    for (i=0;i<MAX_PARTS;i++){
        car.points[i].x=0.0f;
        car.points[i].y=0.0f;
        car.points[i].z=0.0f;
        car.points[i].vx=0.0f;
        car.points[i].vy=0.0f;
        car.points[i].vz=0.0f;
        car.points[i].oldx=0.0f;
        car.points[i].oldy=0.0f;
        car.points[i].oldz=0.0f;
        car.points[i].exists=false;
        car.joints[i].p1=-1;
        car.joints[i].p2=-1;
        car.joints[i].exists=-1;
        car.joints[i].stiffness=0.0f;
        car.faces[i].group=0;
        car.faces[i].vertices[0]=0;
        car.faces[i].vertices[1]=0;
        car.faces[i].vertices[2]=0;
    }
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
    for (j=0;j<CAR_MESH_GROUPS;j++){
        for (int i = 0; i < car.vertices_count; i++) {
            car_vertex &vv = car.vertices[i];
            if (vv.group==j){
                Vec3 vPos = {vv.x, vv.y, vv.z};

                // 1. Najdi 3 nejbližší fyzikální body (velmi hrubý search)
                // Pro 10k vertexů to může chvíli trvat, ale běží to jen jednou.
                int bestIdx[3] = {-1, -1, -1};
                float bestDist[3] = {1e10, 1e10, 1e10};

                for (int p = 0; p < MAX_PARTS; p++) {
                    if (car.points[p].exists && car.points[p].group==j){
                    
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
                if (bestIdx[0] == -1 || bestIdx[1] == -1 || bestIdx[2] == -1){
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

                // 2. Výpočet vah (Barycentrické souřadnice v 3D prostoru)
                Vec3 A = {car.points[bestIdx[0]].x, car.points[bestIdx[0]].y, car.points[bestIdx[0]].z};
                Vec3 B = {car.points[bestIdx[1]].x, car.points[bestIdx[1]].y, car.points[bestIdx[1]].z};
                Vec3 C = {car.points[bestIdx[2]].x, car.points[bestIdx[2]].y, car.points[bestIdx[2]].z};

                Vec3 v0 = sub(B, A), v1 = sub(C, A), v2 = sub(vPos, A);
                float d00 = dot(v0, v0);
                float d01 = dot(v0, v1);
                float d11 = dot(v1, v1);
                float d20 = dot(v2, v0);
                float d21 = dot(v2, v1);
                float denom = d00 * d11 - d01 * d01;

                vv.weights[1] = (d11 * d20 - d01 * d21) / denom;
                vv.weights[2] = (d00 * d21 - d01 * d20) / denom;
                vv.weights[0] = 1.0f - vv.weights[1] - vv.weights[2];

                // 3. Výpočet height_offset (vzdálenost od roviny ABC)
                Vec3 normal = normalize(cross(v0, v1));
                vv.height_offset = dot(sub(vPos, A), normal);
            }
        }
    }
}
void update_car_mesh_1(cardata& car) {
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex &vv = car.vertices[i];
        if (vv.fyz_ref_points[0] < 0 || vv.fyz_ref_points[0] >= car.points_count ||
            vv.fyz_ref_points[1] < 0 || vv.fyz_ref_points[1] >= car.points_count ||
            vv.fyz_ref_points[2] < 0 || vv.fyz_ref_points[2] >= car.points_count) {
            continue;
        }
        // Získej aktuální pozice fyzikálních bodů
        point &pA = car.points[vv.fyz_ref_points[0]];
        point &pB = car.points[vv.fyz_ref_points[1]];
        point &pC = car.points[vv.fyz_ref_points[2]];
        
        Vec3 A = {pA.x, pA.y, pA.z};
        Vec3 B = {pB.x, pB.y, pB.z};
        Vec3 C = {pC.x, pC.y, pC.z};

        // 1. Základní pozice na ploše trojúhelníku pomocí vah
        float newX = A.x * vv.weights[0] + B.x * vv.weights[1] + C.x * vv.weights[2];
        float newY = A.y * vv.weights[0] + B.y * vv.weights[1] + C.y * vv.weights[2];
        float newZ = A.z * vv.weights[0] + B.z * vv.weights[1] + C.z * vv.weights[2];

        // 2. Přidání height_offsetu ve směru aktuální normály
        Vec3 normal = normalize(cross(sub(B, A), sub(C, A)));
        
        vv.x = newX + normal.x * vv.height_offset;
        vv.y = newY + normal.y * vv.height_offset;
        vv.z = newZ + normal.z * vv.height_offset;
        
        // Bonus: Update vizuální normály vertexu (aby fungovalo stínování při deformaci)
        vv.nx = normal.x; vv.ny = normal.y; vv.nz = normal.z;
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
                    if (car.points[p].exists && car.points[p].group == j) {
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
    }
}
