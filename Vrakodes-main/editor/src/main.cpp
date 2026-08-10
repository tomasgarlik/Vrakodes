#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#define KMOD_COMMAND KMOD_GUI
#define STANDARTPICEHEIGHT 22
#define STANDARTBORDER 1
#define RETINA_DISPLAY
#define SPLASHSCREENTIME 10
#define CURSORPERIOD 500
#define HOVER_DARKEN 0.6
#define TOPPICEHEIGHT 70
#define MAX_PARTS 20000
#define OBJ_POINT_MASS     1.0f
#define OBJ_JOINT_GROUP     0
#define OBJ_JOINT_STIFF    50.0f
#define OBJ_JOINT_DAMP     0.8f
#define OBJ_JOINT_MINLEN   0.8f
#define OBJ_JOINT_ELASTIC  0.05f
#define HOVER_TOLERANCE 10
#define ATTRIBUTE_LB 1
#define ATTRIBUTE_RB 2
#define ATTRIBUTE_LF 3
#define ATTRIBUTE_RF 4
#define ATTRIBUTE_SS 5
#define ATTRIBUTE_LS 6
#define ATTRIBUTE_UP 7
#define ATTRIBUTE_ICJ 8
#define MODE_SELECT 0
#define MODE_UNION 1
#define MODE_ICJ 2
#include "types.hpp"
#include "topbar_init.hpp"
#include "global_vars.hpp"
#include "general_functions.hpp"
#include "SDL_Addons.hpp"
#include "tinyfiledialogs.hpp"
#include "gui.hpp"
void openfunc()
{
    const char* path = tinyfd_openFileDialog(
        "Load physics mesh",
        "",
        0,
        NULL,
        NULL,
        0
    );

    if (!path) return;

    FILE* f = fopen(path, "r");
    if (!f) return;

    points_count = 0;
    joints_count = 0;
    faces_count = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "\"engine_power\"")) {
            sscanf(line, " \"engine_power\": %f,", &engine_power);
        }
        if (strstr(line, "\"steering_type\"")) {
            sscanf(line, " \"steering_type\": %d,", &steering_type);
        }
        if (strstr(line, "\"max_steer\"")) {
            sscanf(line, " \"max_steer\": %f,", &max_steer);
        }
        if (strstr(line, "\"steer_speed\"")) {
            sscanf(line, " \"steer_speed\": %f,", &steer_speed);
        }
        if (strstr(line, "\"deformation_system\"")) {
            sscanf(line, " \"deformation_system\": %d,", &deformation_system);
        }
        if (strstr(line, "\"sound_divider\"")) {
            sscanf(line, " \"sound_divider\": %f,", &sound_divider);
        }
        if (strstr(line, "\"spawn_type\"")) {
            sscanf(line, " \"spawn_type\": %d,", &spawn_type);
        }
        if (strstr(line, "\"view_3rd\"")) {
            sscanf(line, " \"view_3rd\": [%f,%f],", &v3rd_x, &v3rd_y);
        }
        if (strstr(line, "\"view_cab\"")) {
            sscanf(line, " \"view_cab\": [%f,%f,%f],", &vcrd_x, &vcrd_y, &vcrd_z);
        }
        if (strstr(line, "\"force_clamp\"")) {
            sscanf(line, " \"force_clamp\": %f", &force_clamp);
        }
        // ---- POINT ----
        if (strstr(line, "\"x\"")) {
            point p;
            p.exists = true;
            p.selected = false;
            p.hidden = false;

            sscanf(line,
                " {\"x\":%f,\"y\":%f,\"z\":%f,\"mass\":%f,\"attribute\":%d,\"group\":%d,\"friction\":%f,\"collide\":%d}",
                &p.x, &p.y, &p.z, &p.mass, &p.attribute, &p.group, &p.friction, &p.collide
            );

            points[points_count++] = p;
        }

        // ---- JOINT ----
        if (strstr(line, "\"p1\"") && strstr(line, "\"p2\"") && strstr(line, "\"stiffness\"")) {
            joint j;
            j.exists = true;
            j.selected = false;
            j.hidden = false;

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
            // printf("%d ", j.attribute);

            joints[joints_count++] = j;
        }

        // ---- FACE ----
        if (strstr(line, "\"p1\"") && strstr(line, "\"p2\"") && strstr(line, "\"p3\"")) {
            // Tohle je face (má p1, p2, p3)
            face fce;
            fce.exists = true;

            sscanf(line,
                " {\"p1\":%d,\"p2\":%d,\"p3\":%d,\"exists\":true}",
                &fce.p1,
                &fce.p2,
                &fce.p3
            );

            faces[faces_count++] = fce;
        }
        // ---- BALL ----
        if (strstr(line, "\"radius\"")) {
            ball b;
            b.exists = true;

            sscanf(line,
                " {\"p\":%d,\"radius\":%f,\"target\":%d,\"exists\":true}",
                &b.p,
                &b.radius,
                &b.target
            );

            balls[balls_count++] = b;
        }
    }
    fclose(f);
}
void newfunc(){
    return;
}
void saveasfunc(){
    const char* path = tinyfd_saveFileDialog(
        "Save physics mesh",
        "vehicle.json",
        0,
        NULL,
        NULL
    );

    if (!path) return;

    FILE* f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "{\n");
    fprintf(f, "  \"properties\": {\n");
    fprintf(f, "    \"engine_power\": %f,\n", engine_power);
    fprintf(f, "    \"steering_type\": %d,\n", steering_type);
    fprintf(f, "    \"max_steer\": %f,\n", max_steer);
    fprintf(f, "    \"steer_speed\": %f,\n", steer_speed);
    fprintf(f, "    \"deformation_system\": %d,\n", deformation_system);
    fprintf(f, "    \"sound_divider\": %f,\n", sound_divider);
    fprintf(f, "    \"spawn_type\": %d,\n", spawn_type);
    fprintf(f, "    \"view_3rd\": [%f,%f],\n", v3rd_x, v3rd_y);
    fprintf(f, "    \"view_cab\": [%f,%f,%f],\n", vcrd_x, vcrd_y, vcrd_z);
    fprintf(f, "    \"force_clamp\": %f\n", force_clamp);
    fprintf(f, "  },\n");
    // ---- POINTS ----
    fprintf(f, "  \"points\": [\n");
    for (int i = 0; i < points_count; i++) {
        if (!points[i].exists) continue;

        fprintf(f,
            "    {\"x\":%f,\"y\":%f,\"z\":%f,\"mass\":%f,\"attribute\":%d,\"group\":%d,\"friction\":%f,\"collide\":%d}",
            points[i].x,
            points[i].y,
            points[i].z,
            points[i].mass,
            points[i].attribute,
            points[i].group,
            points[i].friction,
            (int)(points[i].collide)
        );

        fprintf(f, (i < points_count - 1) ? ",\n" : "\n");
    }
    fprintf(f, "  ],\n");

    // ---- JOINTS ----
    fprintf(f, "  \"joints\": [\n");
    for (int i = 0; i < joints_count; i++) {
        if (!joints[i].exists) continue;

        fprintf(f,
            "    {\"p1\":%d,\"p2\":%d,\"group\":%d,"
            "\"stiffness\":%f,\"damping\":%f,"
            "\"min_len\":%f,\"elastic_margin\":%f,\"attribute\":%d,\"snap\":%d}",
            joints[i].p1,
            joints[i].p2,
            joints[i].group,
            joints[i].stiffness,
            joints[i].damping,
            joints[i].min_len,
            joints[i].elastic_margin,
            joints[i].attribute,
            (int)(joints[i].snap)
        );

        fprintf(f, (i < joints_count - 1) ? ",\n" : "\n");
    }
    fprintf(f, "  ],\n");
    
    // Faces
    fprintf(f, "  \"faces\": [\n");
    for (int i = 0; i < faces_count; i++) {
        if (!faces[i].exists) continue;

        fprintf(f,
            "    {\"p1\":%d,\"p2\":%d,\"p3\":%d,\"exists\":true}",
            faces[i].p1,
            faces[i].p2,
            faces[i].p3
        );

        fprintf(f, (i < faces_count - 1) ? ",\n" : "\n");
    }
    fprintf(f, "  ],\n");
        
    // Balls
    fprintf(f, "  \"balls\": [\n");
    for (int i = 0; i < balls_count; i++) {
        if (!balls[i].exists) continue;

        fprintf(f,
            "    {\"p\":%d,\"radius\":%f,\"target\":%d,\"exists\":true}",
            balls[i].p,
            balls[i].radius,
            balls[i].target
        );

        fprintf(f, (i < balls_count - 1) ? ",\n" : "\n");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    fclose(f);
}
bool joint_exists(int a, int b) {
    for (int i = 0; i < joints_count; i++) {
        if (!joints[i].exists) continue;

        if ((joints[i].p1 == a && joints[i].p2 == b) ||
            (joints[i].p1 == b && joints[i].p2 == a)) {
            return true;
        }
    }
    return false;
}
bool joint_exists(int idx) {
    int a=joints[idx].p1;
    int b=joints[idx].p2;
    for (int i = 0; i < joints_count; i++) {
        if (i == idx) continue; // skip self
        if (!joints[i].exists) continue;

        if ((joints[i].p1 == a && joints[i].p2 == b) ||
            (joints[i].p1 == b && joints[i].p2 == a)) {
            return true;
        }
    }
    return false;
}
void load_obj() {
    int begin_points=points_count;
    const char* filters[] = { "*.obj" };
    const char* path = tinyfd_openFileDialog(
        "Load OBJ",
        "",
        1,
        filters,
        "Wavefront OBJ",
        0
    );

    if (!path) return;

    FILE* f = fopen(path, "r");
    if (!f) {
        printf("OBJ: failed to open %s\n", path);
        return;
    }



    char line[512];

    while (fgets(line, sizeof(line), f)) {

        // ---------- VERTEX ----------
        if (line[0] == 'v' && line[1] == ' ') {
            point* p = &points[points_count++];

            sscanf(line, "v %f %f %f", &p->x, &p->y, &p->z);

            p->mass     = OBJ_POINT_MASS;
            p->attribute    = 0;
            p->selected = false;
            p->exists   = true;
        }

        // ---------- FACE → EDGES ----------
        else if (line[0] == 'l' && line[1] == ' ') {

            int idx[16];
            int cnt = 0;

            char* tok = strtok(line + 2, " ");
            while (tok && cnt < 16) {
                idx[cnt++] = atoi(tok) - 1; // OBJ je 1-based
                tok = strtok(NULL, " ");
            }

            for (int i = 0; i < cnt; i++) {
                int a = idx[i];
                int b = idx[(i + 1) % cnt];

                if (a < 0 || b < 0) continue;
                if (a == b) continue;

                // ❗ DUPLICITY PRYČ
                // some extra crap useful when adding another OBJ to an existing project
                if (joint_exists(a+begin_points, b+begin_points)) continue;

                // if (joint_exists(a, b)) continue;

                joint* j = &joints[joints_count++];

                j->p1 = a+begin_points;
                j->p2 = b+begin_points;



                j->group           = OBJ_JOINT_GROUP;
                j->stiffness      = OBJ_JOINT_STIFF;
                j->damping        = OBJ_JOINT_DAMP;
                j->min_len        = OBJ_JOINT_MINLEN;
                j->elastic_margin = OBJ_JOINT_ELASTIC;
                j->selected       = false;
                j->exists         = true;
            }
        }
    }

    fclose(f);

    printf("OBJ loaded: %d points, %d joints (deduplicated)\n",
           points_count, joints_count);
}
void quitfunc(){
    SDL_Quit();
    running=false;
}
bool find_path(int start, int end, std::vector<int>& out_path) {
    float dist[MAX_PARTS];
    int parent[MAX_PARTS];
    bool visited[MAX_PARTS];
    for (int i = 0; i < MAX_PARTS; i++) {
        dist[i] = 1e30f;
        parent[i] = -1;
        visited[i] = false;
    }
    dist[start] = 0;

    for (int iter = 0; iter < points_count; iter++) {
        // find unvisited node with smallest dist
        int current = -1;
        float best = 1e30f;
        for (int i = 0; i < points_count; i++) {
            if (!visited[i] && dist[i] < best) {
                best = dist[i];
                current = i;
            }
        }
        if (current == -1 || current == end) break;
        visited[current] = true;

        // relax neighbors
        for (int i = 0; i < joints_count; i++) {
            joint& j = joints[i];
            if (!j.exists) continue;

            int neighbor = -1;
            if (j.p1 == current) neighbor = j.p2;
            else if (j.p2 == current) neighbor = j.p1;
            if (neighbor == -1 || visited[neighbor]) continue;

            // edge weight = 3D distance between the two points
            point& pa = points[current];
            point& pb = points[neighbor];
            float dx = pb.x - pa.x;
            float dy = pb.y - pa.y;
            float dz = pb.z - pa.z;
            float d = sqrtf(dx*dx + dy*dy + dz*dz);

            if (dist[current] + d < dist[neighbor]) {
                dist[neighbor] = dist[current] + d;
                parent[neighbor] = current;
            }
        }
    }

    if (parent[end] == -1 && end != start) return false;

    out_path.clear();
    int node = end;
    while (node != -1) {
        out_path.push_back(node);
        node = parent[node];
    }
    std::reverse(out_path.begin(), out_path.end());
    return true;
}
void fill_strip_between_joints(int p1, int p2, int p3, int p4) {
    // joint& ja = joints[joint_a_idx];
    // joint& jb = joints[joint_b_idx];
    joint ja = {p1,p2,.exists=true};
    joint jb = {p3,p4,.exists=true};

    if (!ja.exists || !jb.exists) return;

    // find path from ja.p1 to jb.p1
    std::vector<int> path1, path2;

    if (!find_path(ja.p1, jb.p1, path1)) {
        printf("fill_strip: no path found between p1s\n");
        return;
    }

    // find path from ja.p2 to jb.p2
    if (!find_path(ja.p2, jb.p2, path2)) {
        printf("fill_strip: no path found between p2s\n");
        return;
    }

    // paths must be same length for a clean strip
    if (path1.size() != path2.size()) {
        printf("fill_strip: path lengths differ (%d vs %d), strip will be uneven\n",
               (int)path1.size(), (int)path2.size());
        // use shorter length to avoid out of bounds
    }

    int steps = (int)std::min(path1.size(), path2.size()) - 1;
    if (steps <= 0) return;

    for (int i = 0; i < steps; i++) {
        int a0 = path1[i];
        int a1 = path1[i+1];
        int b0 = path2[i];
        int b1 = path2[i+1];

        if (faces_count + 2 >= MAX_PARTS) {
            printf("fill_strip: col_faces full!\n");
            return;
        }

        // quad as two triangles
        // tri 1
        face& f1 = faces[faces_count++];
        f1.p1 = a0;
        f1.p2 = a1;
        f1.p3 = b0;
        f1.exists = true;

        // tri 2
        face& f2 = faces[faces_count++];
        f2.p1 = a1;
        f2.p2 = b1;
        f2.p3 = b0;
        f2.exists = true;
    }

    printf("fill_strip: created %d quads (%d triangles)\n", steps, steps*2);
}
void deduplicate_func(){
    for (i=0;i<joints_count;i++){
        if (joint_exists(i)){
            joints[i].exists=false;
        }
    }
}
bool renderPoint(int x, int y, float z, int r, int g, int b) {
    bool ret=false;
    if (x >= 0 && x < viewport_x && y >= 0 && y < viewport_y && z > 0.0f) {
        int index = y * viewport_x + x;
        if (((((int)x) / pxlenght(HOVER_TOLERANCE)) == ((mousex - HOVER_TOLERANCE/2 - viewport_xpos) / pxlenght(HOVER_TOLERANCE)) && 
            (((int)y) / pxlenght(HOVER_TOLERANCE)) == ((mousey - HOVER_TOLERANCE/2 - viewport_ypos) / pxlenght(HOVER_TOLERANCE))) || 
            (boxselecting && 
            (int)x > std::min(boxselect_startx, mousex) - viewport_xpos && 
            (int)x < std::max(boxselect_startx, mousex) - viewport_xpos && 
            (int)y > std::min(boxselect_starty, mousey) - viewport_ypos && 
            (int)y < std::max(boxselect_starty, mousey) - viewport_ypos)){
            ret = true;
            if (clicked){
                clickedOnSomething=true;
            }
        }

        if (z_buffer[index] > z) {
            z_buffer[index] = z;
            viewport_pixels[index] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }

    }
    return (!some_dropmenu_opened && ret);
}
bool renderPointXray(int x, int y, int r, int g, int b) {
    if (x >= 0 && x < viewport_x && y >= 0 && y < viewport_y) {
        int index = y * viewport_x + x;

        // 1. Vytáhneme stávající barvu
        uint32_t oldColor = viewport_pixels[index];
        
        // 2. Rozložíme na složky
        int oldR = (oldColor >> 16) & 0xFF;
        int oldG = (oldColor >> 8) & 0xFF;
        int oldB = oldColor & 0xFF;

        // 3. Sečteme barvy a použijeme "clamping" (zarážku na 255)
        // Používáme fintu: (a + b > 255) ? 255 : (a + b)
        int newR = (oldR + r > 255) ? 255 : (oldR + r);
        int newG = (oldG + g > 255) ? 255 : (oldG + g);
        int newB = (oldB + b > 255) ? 255 : (oldB + b);

        // 4. Zapíšeme zpět (Alpha necháváme 0xFF)
        viewport_pixels[index] = (0xFF << 24) | (newR << 16) | (newG << 8) | newB;
    }
    return false;
}
bool renderPoint(float x, float y, float z, bool selected, int group) {
    int size=pxlenght(7);
    int ix = (int)x;
    int iy = (int)y;

    if (z <= 0.0f) return false;

    bool hovered = false;

    int half = size / 2;

    // hover check (AABB)
    if ((mousex >= ix - half && mousex <= ix + half && mousey - viewport_ypos >= iy - half && mousey - viewport_ypos <= iy + half) || 
        (boxselecting && 
        ix > std::min(boxselect_startx, mousex) - viewport_xpos && 
        ix < std::max(boxselect_startx, mousex) - viewport_xpos && 
        iy > std::min(boxselect_starty, mousey) - viewport_ypos && 
        iy < std::max(boxselect_starty, mousey) - viewport_ypos)) {
        hovered = true;
        if (clicked){
            clickedOnSomething=true;
        }
    }
    int r;
    int g;
    int b;
    for (int dx = -half; dx <= half; dx++) {
        for (int dy = -half; dy <= half; dy++) {

            int px = ix + dx;
            int py = iy + dy;

            if (hovered) {
                r = 150;
                g = 255;
                b = 150;
            } else if (selected){
                r = 255;
                g = 255;
                b = 255;
            } else {
                if (group==0){
                    r = 0;
                    g = 150;
                    b = 255;
                } else if (group==1){
                    r = 255;
                    g = 200;
                    b = 000;
                } else if (group==2){
                    r = 255;
                    g = 0;
                    b = 255;
                } else if (group==3){
                    r = 0;
                    g = 255;
                    b = 255;
                } else if (group==4){
                    r = 0;
                    g = 255;
                    b = 0;
                } else if (group==5){
                    r = 255;
                    g = 0;
                    b = 0;
                } else if (group==6){
                    r = 0;
                    g = 0;
                    b = 100;
                } else if (group==7){
                    r = 100;
                    g = 0;
                    b = 0;
                } else if (group==8){
                    r = 0;
                    g = 100;
                    b = 0;
                } else if (group==9){
                    r = 100;
                    g = 0;
                    b = 100;
                } else {
                    r = 100;
                    g = 100;
                    b = 0;
                }
            }
            renderPoint(px,py,z, r,g,b);
        }
    }

    return (!some_dropmenu_opened && hovered);
}
bool renderLine(float x1,float y1,float z1,float x2,float y2,float z2,int r,int g,int b){
    if (((int)z1<0.0f && (int)z2<0.0f) || ((int)x1<0.0f && (int)x2<0.0f) || ((int)y1<0.0f && (int)y2<0.0f) || ((int)x1>viewport_x && (int)x2>viewport_x) || ((int)y1>viewport_y && (int)y2>viewport_y)){return false;}
    float xf=x1;
    float yf=y1;
    float zf=z1;
    int steps;
    bool ret=false;
    if (max(x2-x1)>max(y2-y1)){
        steps=(int)(max(x2-x1)*1.1);
    } else {
        steps=(int)(max(y2-y1)*1.1);
    }
    for (int step=0;step<steps;step++){
        if (renderPoint(xf,yf,zf, r, g, b)){ret=true;}
        xf+=(x2-x1)/(float)steps;
        yf+=(y2-y1)/(float)steps;
        zf+=(z2-z1)/(float)steps;
    }
    return ret;
}
bool renderLineXray(float x1,float y1,float z1,float x2,float y2,float z2,int r,int g,int b){
    if (((int)z1<0.0f && (int)z2<0.0f) || ((int)x1<0.0f && (int)x2<0.0f) || ((int)y1<0.0f && (int)y2<0.0f) || ((int)x1>viewport_x && (int)x2>viewport_x) || ((int)y1>viewport_y && (int)y2>viewport_y)){return false;}
    float xf=x1;
    float yf=y1;
    float zf=z1;
    int steps;
    bool ret=false;
    if (max(x2-x1)>max(y2-y1)){
        steps=(int)(max(x2-x1)*1.1);
    } else {
        steps=(int)(max(y2-y1)*1.1);
    }
    for (int step=0;step<steps;step++){
        if (renderPointXray((int)xf,(int)yf, r, g, b)){ret=true;}
        xf+=(x2-x1)/(float)steps;
        yf+=(y2-y1)/(float)steps;
        zf+=(z2-z1)/(float)steps;
    }
    return ret;
}
void renderFace(point p1, point p2, point p3, int r, int g, int b) {
    // --- KONSTANTY ---
    const float PIXEL_GAP = 10.0f;              // Mezera v pixelech (změň si podle chuti)

    // 2. VÝPOČET VNITŘKU S PEVNOU PIXELOVOU MEZEROU
    float cx = (p1.x + p2.x + p3.x) / 3.0f;
    float cy = (p1.y + p2.y + p3.y) / 3.0f;
    float cz = (p1.z + p2.z + p3.z) / 3.0f;

    point v[3] = {p1, p2, p3};

    for(int i = 0; i < 3; i++) {
        // Vektor od bodu ke středu (v pixelech)
        float dx = cx - v[i].x;
        float dy = cy - v[i].y;
        float dz = cz - v[i].z; // Z necháme, aby seděl Z-buffer

        float dist = sqrt(dx*dx + dy*dy); // Vzdálenost v 2D rovině (pixely)

        if (dist > PIXEL_GAP) {
            // Posuneme bod o přesný počet pixelů směrem ke středu
            float scale = PIXEL_GAP / dist;
            v[i].x += dx * scale;
            v[i].y += dy * scale;
            // Z neměníme, aby vnitřek neuskakoval dopředu/dozadu
        } else {
            // Pokud je trojúhelník menší než mezera, smrskneme ho do středu
            v[i].x = cx;
            v[i].y = cy;
        }
    }

    // 3. SEŘAZENÍ (v1 nejvýš, v3 nejníž)
    point v1 = v[0], v2 = v[1], v3 = v[2];
    if (v1.y > v2.y) std::swap(v1, v2);
    if (v1.y > v3.y) std::swap(v1, v3);
    if (v2.y > v3.y) std::swap(v2, v3);

    // 4. SCANLINE VÝPLŇ (Vnitřní menší plocha)
    float total_h = v3.y - v1.y;
    if (total_h <= 0) return;

    for (int i = 0; i < (int)total_h; i++) {
        bool second_half = i > (v2.y - v1.y) || v2.y == v1.y;
        float segment_h = second_half ? (v3.y - v2.y) : (v2.y - v1.y);
        
        float alpha = (float)i / total_h;
        float beta  = (float)(i - (second_half ? (v2.y - v1.y) : 0)) / segment_h;

        float ax = v1.x + (v3.x - v1.x) * alpha;
        float ay = v1.y + (v3.y - v1.y) * alpha;
        float az = v1.z + (v3.z - v1.z) * alpha;

        float bx, by, bz;
        if (!second_half) {
            bx = v1.x + (v2.x - v1.x) * beta;
            by = v1.y + (v2.y - v1.y) * beta;
            bz = v1.z + (v2.z - v1.z) * beta;
        } else {
            bx = v2.x + (v3.x - v2.x) * beta;
            by = v2.y + (v3.y - v2.y) * beta;
            bz = v2.z + (v3.z - v2.z) * beta;
        }
        if (xray.checked){
            renderLineXray(ax, ay, az, bx, by, bz, r, g, b);
        } else {
            renderLine(ax, ay, az, bx, by, bz, r, g, b);
        }
    }
}
void init_viewport_texture(SDL_Renderer* renderer) {
    viewport_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        viewport_x,
        viewport_y
    );
}
void resize_viewport(SDL_Renderer* renderer, int new_w, int new_h)
{
    viewport_x = new_w;
    viewport_y = new_h;

    // zabij starou texture
    if (viewport_texture) {
        SDL_DestroyTexture(viewport_texture);
        viewport_texture = NULL;
    }

    // vytvoř novou texture s novým rozměrem
    viewport_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        viewport_x,
        viewport_y
    );

    if (!viewport_texture) {
        printf("texture resize failed: %s\n", SDL_GetError());
    }
    viewport_pixels = new uint32_t[viewport_x * viewport_y];
    z_buffer = new float[viewport_x * viewport_y];
}
void project_point(float x, float y, float z, float* out_x, float* out_y){
    if (z <= 0.001f) z = 0.001f; // aby to neexplodovalo

    float viewport_fov_rad = viewport_fov * (3.141592f / 180.0f);
    float f = 1.0f / tanf(viewport_fov_rad * 0.5f);

    *out_x = (x * f / z) * viewport_aspect_ratio;
    *out_y = (y * f / z);
}
void project_point_ortho(float x, float y, float z, float* out_x, float* out_y){
    float fov_rad = viewport_fov * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad * 0.5f);

    float scale = f / z_pos;

    *out_x = (x * scale) * viewport_aspect_ratio;
    *out_y = (y * scale);
}
void display_viewport(SDL_Renderer* renderer) {
    if (viewport_pixels == nullptr){
        printf("viewport pixels are nullptr\n");
        return;
    } // Jistota je jistota
    renderPoint(100, 100, 10.0f, 255,255,0);
    void* pixels_void;
    int pitch;

    if (SDL_LockTexture(viewport_texture, NULL, &pixels_void, &pitch) != 0) {
        return; // Nepovedlo se zamknout
    }
    SDL_SetTextureBlendMode(viewport_texture, SDL_BLENDMODE_NONE);
    // Pokud se pitch rovná přesně šířce (4 bajty na pixel), můžeme použít bleskové memcpy
    if (pitch == viewport_x * sizeof(uint32_t)) {
        memcpy(pixels_void, viewport_pixels, viewport_x * viewport_y * sizeof(uint32_t));
    } else {
        // Pokud je pitch jiný (zarovnání paměti), musíme to kopírovat po řádcích
        uint8_t* dst = (uint8_t*)pixels_void;
        uint8_t* src = (uint8_t*)viewport_pixels;
        for (int y = 0; y < viewport_y; y++) {
            memcpy(dst + y * pitch, src + y * viewport_x * sizeof(uint32_t), viewport_x * sizeof(uint32_t));
        }
    }

    SDL_UnlockTexture(viewport_texture);

    SDL_Rect dstrect = { viewport_xpos, viewport_ypos, viewport_x, viewport_y };
    SDL_RenderCopy(renderer, viewport_texture, NULL, &dstrect);
}
void render(SDL_Renderer* renderer){
    memset(viewport_pixels, 0, viewport_x * viewport_y * sizeof(uint32_t));
    std::fill(z_buffer, z_buffer + (viewport_x * viewport_y), 100000.0f);
    if (typedChar.c_str()[0]=='s'){
        for (i=0;i<MAX_PARTS;i++){
            render_points[i].x=points[i].x;
            render_points[i].y=points[i].y;
            render_points[i].z=points[i].z;
            render_points[i].exists=points[i].exists;
            points[i].hidden=false;
            joints[i].hidden=false;
        }
    } else {
        for (i=0;i<MAX_PARTS;i++){
            render_points[i].x=points[i].x;
            render_points[i].y=points[i].y;
            render_points[i].z=points[i].z;
            render_points[i].exists=points[i].exists;
        }
    }


    if (rotatingviewport){
        x_rot+=(float)(mousex-viewport_drag_start_mousex)/viewport_scale*3.0f;
        y_rot-=(float)(mousey-viewport_drag_start_mousey)/viewport_scale*3.0f;
        viewport_drag_start_mousex=mousex;
        viewport_drag_start_mousey=mousey;
    }
    for (i = 0; i < MAX_PARTS; i++) {
        if (!render_points[i].exists) continue;

        float x = render_points[i].x;
        float y = render_points[i].y;
        float z = render_points[i].z;

        // --- ROT Y ---
        float cy = cosf(-x_rot);
        float sy = sinf(-x_rot);
        float x2 =  x * cy + z * sy;
        float z2 = -x * sy + z * cy;

        x = x2;
        z = z2;

        // --- ROT X ---
        float cx = cosf(y_rot);
        float sx = sinf(y_rot);
        float y1 = y * cx - z * sx;
        float z1 = y * sx + z * cx;

        y = y1;
        z = z1;


        // --- ROT Z ---
        float cz = cosf(z_rot);
        float sz = sinf(z_rot);
        float x3 = x * cz - y * sz;
        float y3 = x * sz + y * cz;

        render_points[i].x = x3;
        render_points[i].y = y3;
        render_points[i].z = z;
    }





    if (movingviewport){
        x_pos+=(float)(mousex-viewport_drag_start_mousex)/viewport_scale*2.5f;
        y_pos-=(float)(mousey-viewport_drag_start_mousey)/viewport_scale*2.5f;
        viewport_drag_start_mousex=mousex;
        viewport_drag_start_mousey=mousey;
    }
    part_index=0;
    for (i=0;i<MAX_PARTS;i++){
        if (render_points[i].exists){
            total_mass+=points[i].mass;
            render_points[i].x+=x_pos;
            render_points[i].y+=y_pos;
            render_points[i].z+=z_pos;
            if (projection.selected==0){
                project_point(
                    render_points[i].x,
                    render_points[i].y,
                    render_points[i].z,
                    &render_points[i].x,
                    &render_points[i].y
                );
            } else {
                project_point_ortho(
                    render_points[i].x,
                    render_points[i].y,
                    render_points[i].z,
                    &render_points[i].x,
                    &render_points[i].y
                );
            }
            render_points[i].x*=viewport_scale;
            render_points[i].y*=-viewport_scale;
            render_points[i].z*=viewport_scale;
            render_points[i].x+=(float)(viewport_x/2);
            render_points[i].y+=(float)(viewport_y/2);
            if (points[i].selected && typedChar.c_str()[0]=='h'){
                points[i].hidden=true;
                points[i].selected=false;
            }
            if (show_points.checked && !points[i].hidden){
                if (
                    (points[i].group==0 && show_g1.checked==true) || 
                    (points[i].group==1 && show_g2.checked==true) ||
                    (points[i].group==2 && show_g3.checked==true) ||
                    (points[i].group==3 && show_g4.checked==true) ||
                    (points[i].group==4 && show_g5.checked==true) ||
                    (points[i].group==5 && show_g6.checked==true) ||
                    (points[i].group==6 && show_g7.checked==true) ||
                    (points[i].group==7 && show_g8.checked==true) ||
                    (points[i].group==8 && show_g9.checked==true) ||
                    (points[i].group==9 && show_g10.checked==true)){
                    if (renderPoint(render_points[i].x,render_points[i].y,render_points[i].z, points[i].selected, points[i].group) && clicked){
                        points[i].selected=!points[i].selected;
                        if (boxselect){
                            points[i].selected=true;
                        }
                    }
                    if (points[i].selected){
                        number_of_selected++;
                        if (selected_index==-1){
                            selected_index=part_index;
                        } else {
                            selected_index=-2;
                        }
                        if (mode.selected==MODE_UNION){
                            if (group.dashed){
                                group.selected=points[i].group;
                            }
                            if (attribute.dashed){
                                attribute.selected=points[i].attribute;
                            }                        
                            if (mass.dashed){
                                mass.text=std::to_string((int)(points[i].mass*100.0f));
                            }
                            if (friction.dashed){
                                friction.text=std::to_string((int)(points[i].friction*100.0f));
                            }
                            if (!collide.just_changed){
                                collide.checked=points[i].collide;
                            }
                            group.dashed=false;
                            attribute.dashed=false;
                            mass.dashed=false;
                            friction.dashed=false;
                            points[i].group=group.selected;
                            points[i].attribute=attribute.selected;
                            points[i].mass=(float)mass.number/100.0f;
                            points[i].friction=(float)friction.number/100.0f;
                            points[i].collide=collide.checked;
                        }
                        
                        something_selected=true;
                        if (p1ind==-1){
                            p1ind=i;
                        } else if (p2ind==-1){
                            p2ind=i;
                        } else if (p3ind==-1){
                            p3ind=i;
                        } else if (p4ind==-1){
                            p4ind=i;
                        }
                    }
                }
            } else if (points[i].hidden){
                number_of_hidden++;
            }
            part_index++;
        }
    }
    part_index=0;
    for (i=0;i<MAX_PARTS;i++){
        if (joints[i].exists){
            if (mode.selected==MODE_ICJ){
                joints[i].selected=false;
            }
            if (show_joints.checked && !joints[i].hidden){
                j=0;
                if (joints[i].attribute==ATTRIBUTE_ICJ && !show_icj.checked){
                    continue;
                }
                if (joints[i].attribute==ATTRIBUTE_ICJ && show_icj.checked){
                    jr=200;
                    jg=200;
                    jb=200;
                    j=1;
                } else if (joints[i].group==0 && show_g1.checked){
                    jr=0;
                    jg=150;
                    jb=255;
                    j=1;
                } else if (joints[i].group==1 && show_g2.checked){
                    jr=255;
                    jg=255;
                    jb=0;
                    j=1;
                } else if (joints[i].group==2 && show_g3.checked){
                    jr=255;
                    jg=0;
                    jb=255;
                    j=1;
                } else if (joints[i].group==3 && show_g4.checked){
                    jr=0;
                    jg=255;
                    jb=255;
                    j=1;
                } else if (joints[i].group==4 && show_g5.checked){
                    jr=0;
                    jg=255;
                    jb=0;
                    j=1;
                } else if (joints[i].group==5 && show_g6.checked){
                    jr=255;
                    jg=0;
                    jb=0;
                    j=1;
                } else if (joints[i].group==6 && show_g7.checked){
                    jr=0;
                    jg=0;
                    jb=100;
                    j=1;
                } else if (joints[i].group==7 && show_g8.checked){
                    jr=100;
                    jg=0;
                    jb=0;
                    j=1;
                } else if (joints[i].group==8 && show_g9.checked){
                    jr=0;
                    jg=100;
                    jb=0;
                    j=1;
                } else if (joints[i].group==9 && show_g10.checked){
                    jr=100;
                    jg=0;
                    jb=100;
                    j=1;
                }
                if (j==1){
                    if (i==0){
                    }
                    if (renderLine(render_points[joints[i].p1].x, render_points[joints[i].p1].y, render_points[joints[i].p1].z, render_points[joints[i].p2].x, render_points[joints[i].p2].y, render_points[joints[i].p2].z, jr,jg,jb)){
                        if (mode.selected!=MODE_ICJ){
                            renderLine(render_points[joints[i].p1].x, render_points[joints[i].p1].y, 0.1f, render_points[joints[i].p2].x, render_points[joints[i].p2].y, 0.1f, 150,255,150);
                            if (clicked){
                                joints[i].selected=!joints[i].selected;
                            }
                            if (boxselect){
                                joints[i].selected=true;
                            }
                        }
                    } else if (joints[i].selected){
                        renderLine(render_points[joints[i].p1].x, render_points[joints[i].p1].y, 0.1f, render_points[joints[i].p2].x, render_points[joints[i].p2].y, 0.1f, 255,255,255);
                        number_of_selected++;
                        if (selected_index==-1){
                            selected_index=part_index;
                        } else {
                            selected_index=-2;
                        }
                        if (j1ind==-1){
                            j1ind=i;
                        } else if (j2ind==-1){
                            j2ind=i;
                        } 
                        if (mode.selected==MODE_UNION){
                            if (group.dashed){
                                group.selected=joints[i].group;
                            }
                            if (attribute.dashed){
                                attribute.selected=joints[i].attribute;
                            }
                            if (stiffness.dashed){
                                stiffness.text=std::to_string((int)(joints[i].stiffness*100.0f));
                            }
                            if (damping.dashed){
                                damping.text=std::to_string((int)(joints[i].damping*100.0f));
                            }
                            if (min_len.dashed){
                                min_len.text=std::to_string((int)(joints[i].min_len*100.0f));
                            }
                            if (elastic_margin.dashed){
                                elastic_margin.text=std::to_string((int)(joints[i].elastic_margin*100.0f));
                            }
                            if (!snap.just_changed){
                                snap.checked=joints[i].snap;
                            }
                            group.dashed=false;
                            attribute.dashed=false;
                            stiffness.dashed=false;
                            damping.dashed=false;
                            min_len.dashed=false;
                            elastic_margin.dashed=false;
                            joints[i].group=group.selected;
                            joints[i].attribute=attribute.selected;
                            joints[i].stiffness=(float)stiffness.number/100.0f;
                            joints[i].damping=(float)damping.number/100.0f;
                            joints[i].min_len=(float)min_len.number/100.0f;
                            joints[i].elastic_margin=(float)elastic_margin.number/100.0f;
                            joints[i].snap=snap.checked;
                        }
                        something_selected=true;
                    }
                }
            } else if (joints[i].hidden){
                number_of_hidden++;
            }
            if (joints[i].selected && typedChar.c_str()[0] == 'h'){
                joints[i].hidden=true;
                joints[i].selected=false;
            }
            part_index++;
        }
    }
    for (i=0;i<MAX_PARTS;i++){
        if (faces[i].exists && show_faces.checked){
            renderFace(render_points[faces[i].p1], render_points[faces[i].p2], render_points[faces[i].p3], 20,100,20);
        }
    }
    if (typedChar.c_str()[0]=='j' && p1ind!=-1 && p2ind!=-1){
        joints[joints_count].p1=p1ind;
        joints[joints_count].p2=p2ind;
        joints[joints_count].stiffness=stiffness.number;
        joints[joints_count].damping=damping.number;
        joints[joints_count].elastic_margin=elastic_margin.number;
        joints[joints_count].exists=true;
        joints[joints_count].group=group.selected;
        joints[joints_count].snap=snap.checked;
        if (mode.selected==MODE_ICJ){
            joints[joints_count].attribute=ATTRIBUTE_ICJ;
        } else {
            joints[joints_count].attribute=attribute.selected;
        }
        joints_count++;
        printf("added a joint\n");
    }
    if (typedChar.c_str()[0]=='f' && p1ind!=-1 && p2ind!=-1 && p3ind!=-1 && p4ind!=-1){
        fill_strip_between_joints(p1ind,p2ind,p3ind,p4ind);
    } else if (typedChar.c_str()[0]=='f' && p1ind!=-1 && p2ind!=-1 && p3ind!=-1){
        faces[faces_count].p1=p1ind;
        faces[faces_count].p2=p2ind;
        faces[faces_count].p3=p3ind;
        faces[faces_count].exists=true;
        faces_count++;
        for (i=0;i<MAX_PARTS;i++){
            points[i].selected=false;
            joints[i].selected=false;
            faces[i].selected=false;
        }
        printf("added a face\n");
    } else if (typedChar.c_str()[0]=='f' && j1ind!=-1 && j2ind!=-1){
        fill_strip_between_joints(joints[j1ind].p1,joints[j1ind].p2,joints[j2ind].p1,joints[j2ind].p2);
    }






// --- VYKRESLENÍ TĚŽIŠTĚ (CG) ---
    // Předpokládám, že proměnná 'cg' (např. typu Point) existuje a má složky .x, .y, .z
    float cg_x = cg.x;
    float cg_y = cg.y;
    float cg_z = cg.z;

    // 1. Rotace kolem Y
    float cy_cg = cosf(-x_rot);
    float sy_cg = sinf(-x_rot);
    float x2_cg = cg_x * cy_cg + cg_z * sy_cg;
    float z2_cg = -cg_x * sy_cg + cg_z * cy_cg;
    cg_x = x2_cg; cg_z = z2_cg;

    // 2. Rotace kolem X
    float cx_cg = cosf(y_rot);
    float sx_cg = sinf(y_rot);
    float y1_cg = cg_y * cx_cg - cg_z * sx_cg;
    float z1_cg = cg_y * sx_cg + cg_z * cx_cg;
    cg_y = y1_cg; cg_z = z1_cg;

    // 3. Rotace kolem Z
    float cz_cg = cosf(z_rot);
    float sz_cg = sinf(z_rot);
    float x3_cg = cg_x * cz_cg - cg_y * sz_cg;
    float y3_cg = cg_x * sz_cg + cg_y * cz_cg;
    cg_x = x3_cg; cg_y = y3_cg;

    // 4. Posun viewportu
    cg_x += x_pos;
    cg_y += y_pos;
    cg_z += z_pos;

    // 5. 3D -> 2D Projekce
    if (projection.selected == 0) {
        project_point(cg_x, cg_y, cg_z, &cg_x, &cg_y);
    } else {
        project_point_ortho(cg_x, cg_y, cg_z, &cg_x, &cg_y);
    }

    // 6. Škálování na velikost okna a vystředění
    cg_x *= viewport_scale;
    cg_y *= -viewport_scale;
    cg_z *= viewport_scale;
    cg_x += (float)(viewport_x / 2);
    cg_y += (float)(viewport_y / 2);

    // 7. Samotné vykreslení symbolu těžiště
    // Využijeme tvoji funkci renderLine, která kreslí do z-bufferu / pixelů
    float size = 10.0f; // Velikost symbolu v pixelech
    
    // Klasický zaměřovač / kříž (čára skrz střed)
    renderLine(cg_x - size, cg_y, cg_z, cg_x + size, cg_y, cg_z, 255, 255, 0); // Horizontální žlutá čára
    renderLine(cg_x, cg_y - size, cg_z, cg_x, cg_y + size, cg_z, 255, 255, 0); // Vertikální žlutá čára
    
    // Volitelně: Malé kolečko nebo čtvereček pro zvýraznění středu
    renderLine(cg_x - 3, cg_y - 3, cg_z, cg_x + 3, cg_y - 3, cg_z, 255, 255, 0);
    renderLine(cg_x + 3, cg_y - 3, cg_z, cg_x + 3, cg_y + 3, cg_z, 255, 255, 0);
    renderLine(cg_x + 3, cg_y + 3, cg_z, cg_x - 3, cg_y + 3, cg_z, 255, 255, 0);
    renderLine(cg_x - 3, cg_y + 3, cg_z, cg_x - 3, cg_y - 3, cg_z, 255, 255, 0);







    display_viewport(renderer);
}
void fps_update(void)
{
    Uint32 now = SDL_GetTicks();
    fps_frames++;

    if (now - fps_last_time >= 500) { // update 2× za sekundu
        fps = fps_frames * 1000.0f / (now - fps_last_time);
        fps_frames = 0;
        fps_last_time = now;
    }
}
int main() {
    #include "init.cpp"
    #include "topbar.hpp"
    for (i=0;i<MAX_PARTS;i++){
        points[i].exists=false;
        joints[i].exists=false;
        faces[i].exists=false;
        balls[i].exists=false;
    }
    points[0]={-1, -1, -1, .group=2};
    points[1]={1, -1, -1, .group=1};
    points[2]={1, -1, 1, .group=3};
    points[3]={-1, -1, 1};
    points[4]={-1, 1, -1};
    points[5]={1, 1, -1};
    points[6]={1, 1, 1};
    points[7]={-1, 1, 1};
    joints[0]={0,1, .group=2};
    joints[1]={1,2, .group=1};
    joints[2]={2,3};
    joints[3]={3,0};
    joints[4]={4,5};
    joints[5]={5,6};
    joints[6]={6,7};
    joints[7]={7,4};
    joints[8]={0,4};
    joints[9]={1,5};
    joints[10]={2,6};
    joints[11]={3,7};
    faces[0]={0,1,2, .exists=true};
    faces[1]={2,3,0, .exists=true};
    faces_count=2;
    for (i=0;i<8;i++){
        points[i].exists=true;
    }
    for (i=0;i<12;i++){
        joints[i].exists=true;
    }
    z_pos=4.0f;
    viewport_x=640;
    viewport_y=480;
    viewport_scale=200.0f;
    viewport_xpos=0;
    viewport_ypos=pxlenght(TOPPICEHEIGHT);
    viewport_aspect_ratio=1.0f;
    viewport_fov=80.0f;
    movingviewport=false;
    rotatingviewport=false;
    movestep=0.05f;
    rotstep=1.0f;
    z_rot=0;
    wait_for_boxselect_end=0;
    viewport_pixels = new uint32_t[viewport_x * viewport_y];
    z_buffer = new float[viewport_x * viewport_y];
    init_viewport_texture(renderer);
    while (running){
        if (clickup){clickup=false;}
        SDL_GetRendererOutputSize(renderer, &width, &height);
        clic=false;
        clicked=false;
        typedChar="";
        specialkey=0;
        modkey=KMOD_NONE;
        clickedOnSomething=false;
        boxselect=false;
        something_selected=false;
        number_of_selected=0;
        number_of_hidden=0;
        total_mass=0.0f;
        p1ind=-1;
        p2ind=-1;
        p3ind=-1;
        p4ind=-1;
        j1ind=-1;
        j2ind=-1;
        while (SDL_PollEvent(&event)) {
            switch (event.type){
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                    break;
                case SDL_MOUSEMOTION:
                    if (retina){
                        mousex = event.motion.x*2;
                        mousey = event.motion.y*2;
                    } else {
                        mousex = event.motion.x;
                       mousey = event.motion.y;
                    }
                    gonna_delelect=false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (mousex>viewport_xpos && mousey>viewport_ypos && mousex<viewport_xpos+viewport_x && mousey<viewport_ypos+viewport_y){
                            rotatingviewport=!shift_down;
                            if (boxselecting && !shift_down){clickedOnSomething=true;boxselect=true;}
                            if (shift_down){
                                boxselecting=true;
                                boxselect_startx=mousex;
                                boxselect_starty=mousey;
                            } else {
                                wait_for_boxselect_end=1;
                            }
                            viewport_drag_start_mousex=mousex;
                            viewport_drag_start_mousey=mousey;
                        }
                        clicked=true;
                        clic=true;
                    } else if (event.button.button == SDL_BUTTON_RIGHT){
                        if (mousex>viewport_xpos && mousey>viewport_ypos && mousex<viewport_xpos+viewport_x && mousey<viewport_ypos+viewport_y){
                            movingviewport=!shift_down;
                            viewport_drag_start_mousex=mousex;
                            viewport_drag_start_mousey=mousey;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    clickup=true;
                    clicked=false;
                    movingviewport=false;
                    rotatingviewport=false;
                    break;
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) {
                        z_pos += ((float)event.wheel.y/10.0f);
                    } else if (event.wheel.y < 0) {
                        z_pos += ((float)event.wheel.y/10.0f);
                    }
                case SDL_TEXTINPUT:
                    typedChar=event.text.text;
                    break;
                case SDL_KEYDOWN:
                    modkey = SDL_GetModState();
                    shift_down = modkey & KMOD_SHIFT;
                    if (modkey & KMOD_COMMAND) {
                        switch (event.key.keysym.sym){
                            case SDLK_EQUALS:
                                making_new_part=false;
                                scale+=0.2;
                                break;
                            case SDLK_MINUS:
                                making_new_part=false;
                                scale-=0.2;
                                if (scale<=0.0){scale=0.2;}
                                break;
                            }
                    } else {
                        switch (event.key.keysym.sym){
                            case SDLK_LEFT:
                                specialkey='l';
                                break;
                            case SDLK_RIGHT:
                                specialkey='r';
                                break;
                            case SDLK_BACKSPACE:
                                specialkey='b';
                                break;
                    }
                    break;
                case SDL_KEYUP:
                    shift_down=false;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        int new_w = pxlenght(event.window.data1);
                        int new_h = pxlenght(event.window.data2-TOPPICEHEIGHT);
                        viewport_scale=new_w/2.0;
                        resize_viewport(renderer, new_w, new_h);
                    }
                    break;
                }
            }
        }
        selected_index=-1;
        // viewport_x, viewport_y=SDL_GetWindowSizeInPixels(window);
        SDL_RenderClear(renderer);
        SDL_Rect rect = { 0, 0, width, height };
        SDL_SetRenderDrawColor(renderer, 15, 20, 35, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 50,50,60,255);
        // top bar background rect
        SDL_Rect rect1 = {0,0,width,pxlenght(TOPPICEHEIGHT)};
        SDL_RenderFillRect(renderer, &rect1);
        // dividers
        SDL_SetRenderDrawColor(renderer, 20,20,30,255);
        SDL_Rect rect2 = {pxlenght(207),0,pxlenght(6),pxlenght(TOPPICEHEIGHT)};
        SDL_RenderFillRect(renderer, &rect2);
        SDL_Rect rect3 = {pxlenght(498),0,pxlenght(6),pxlenght(TOPPICEHEIGHT)};
        SDL_RenderFillRect(renderer, &rect3);
        SDL_Rect rect4 = {pxlenght(697),0,pxlenght(6),pxlenght(TOPPICEHEIGHT)};
        SDL_RenderFillRect(renderer, &rect4);
        displayTex(renderer, createTextTexture(renderer, "                                     Joint                                                                                  Point                                                 Group              "), pxlenght(120), pxlenght(2), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Stiff. :"), pxlenght(230), pxlenght(STANDARTPICEHEIGHT), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Minlen:"), pxlenght(220), pxlenght(STANDARTPICEHEIGHT*2), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Damp:"), pxlenght(340), pxlenght(STANDARTPICEHEIGHT), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Elastic margin:"), pxlenght(340), pxlenght(STANDARTPICEHEIGHT*2), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Fric.:"), pxlenght(512), pxlenght(STANDARTPICEHEIGHT*2), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Mass:"), pxlenght(512), pxlenght(STANDARTPICEHEIGHT), pxlenght(18));
        displayTex(renderer, createTextTexture(renderer, "Attribute:"), pxlenght(711), pxlenght(STANDARTPICEHEIGHT*2), pxlenght(18));


        // find the cg
float aktualni_vaha = 0.0f;
        cg.x = 0.0f;
        cg.y = 0.0f;
        cg.z = 0.0f;

        for (auto& p : points) {
            if (p.exists) {
                cg.x += p.x * p.mass;
                cg.y += p.y * p.mass;
                cg.z += p.z * p.mass;
                aktualni_vaha += p.mass; // Sčítáme hmotnost jen existujících bodů
            }
        }

        // Podmínka, aby to nedělilo nulou, kdyby auto nemělo žádné body
        if (aktualni_vaha > 0.0f) { 
            cg.x /= aktualni_vaha;
            cg.y /= aktualni_vaha;
            cg.z /= aktualni_vaha;
        }

        render(renderer);
        if (boxselecting){
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,50, 200, 50, 50);
            SDL_Rect rect2 = {boxselect_startx,boxselect_starty,mousex-boxselect_startx, mousey-boxselect_starty};
            SDL_RenderFillRect(renderer, &rect2);
        }
        if (wait_for_boxselect_end>0 && wait_for_boxselect_end<2){
            wait_for_boxselect_end++;
        } else if (wait_for_boxselect_end>=2){
            wait_for_boxselect_end=0;
            boxselecting=false;
        }
        fps_update();
        snprintf(fpstext, sizeof(fpstext), "FPS: %d", (int)fps);
        displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(10), pxlenght(14));
        if (selected_index>=0){
            snprintf(fpstext, sizeof(fpstext), "Selected index: %d", selected_index);
            displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(30), pxlenght(14));
        }
        if (number_of_selected>0){
            snprintf(fpstext, sizeof(fpstext), "Number of selected parts: %d", number_of_selected);
            displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(50), pxlenght(14));
        }
        if (number_of_hidden>0){
            snprintf(fpstext, sizeof(fpstext), "Number of hidden parts: %d", number_of_hidden);
            displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(70), pxlenght(14));
        }
        snprintf(fpstext, sizeof(fpstext), "Total mass: %f", total_mass);
        displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(90), pxlenght(14));
        some_dropmenu_opened=false;
        updateNumberInputBox(renderer,&stiffness);
        updateNumberInputBox(renderer,&min_len);
        updateNumberInputBox(renderer,&mass);
        updateNumberInputBox(renderer,&damping);
        updateNumberInputBox(renderer,&elastic_margin);
        updateNumberInputBox(renderer,&friction);
        updateDropMenu(renderer, &projection);
        updateDropMenu(renderer, &mode);
        updateDropMenu(renderer, &group);
        updateDropMenu(renderer, &attribute);
        updateDropMenu(renderer, &filemenu);
        updateCheckBox(renderer, &show_points);
        updateCheckBox(renderer, &show_joints);
        updateCheckBox(renderer, &show_faces);
        updateCheckBox(renderer, &show_g1);
        updateCheckBox(renderer, &show_g2);
        updateCheckBox(renderer, &show_g3);
        updateCheckBox(renderer, &show_g4);
        updateCheckBox(renderer, &show_g5);
        updateCheckBox(renderer, &show_g6);
        updateCheckBox(renderer, &show_g7);
        updateCheckBox(renderer, &show_g8);
        updateCheckBox(renderer, &show_g9);
        updateCheckBox(renderer, &show_g10);
        updateCheckBox(renderer, &collide);
        updateCheckBox(renderer, &self_collide);
        updateCheckBox(renderer, &snap);
        updateCheckBox(renderer, &xray);
        updateCheckBox(renderer, &show_icj);
        if (!clickedOnSomething && clicked && mousex>viewport_xpos && mousey>viewport_ypos && mousex<viewport_x+viewport_xpos && mousey<viewport_y+viewport_ypos && !boxselecting){
            gonna_delelect=true;
        }
        if (clickup && gonna_delelect && !boxselecting){
            for (i=0;i<MAX_PARTS;i++){
                points[i].selected=false;
                joints[i].selected=false;
                faces[i].selected=false;
            }
        }
    
        // if (typedChar[0]=='x'){
        //     for (i=0;i<MAX_PARTS;i++){
        //         if (!points[i].exists || (points[i].exists && !points[i].selected)){continue;}
        //         points[i].exists=false;
        //         for (j=i;j<MAX_PARTS-1;j++){
        //             points[j]=points[j+1];
        //         }
        //         points[MAX_PARTS-1].exists=false;
        //         points_count--;
        //         for (j=0;j<MAX_PARTS;j++){
        //             if (joints[j].exists && (joints[j].p1==i || joints[j].p2==i)){
        //                 joints[j].exists=false;
        //             } else if (joints[j].exists && joints[j].p1>i){
        //                 joints[j].p1--;
        //             } else if (joints[j].exists && joints[j].p2>i){
        //                 joints[j].p2--;
        //             }
        //         }
        //         for (j=0;j<MAX_PARTS;j++){
        //             if (faces[j].exists && (faces[j].p1==i || faces[j].p2==i || faces[j].p3==i)){
        //                 faces[j].exists=false;
        //             } else if (faces[j].exists && faces[j].p1>i){
        //                 faces[j].p1--;
        //             } else if (faces[j].exists && faces[j].p2>i){
        //                 faces[j].p2--;
        //             } else if (faces[j].exists && faces[j].p3>i){
        //                 faces[j].p3--;
        //             }
        //         }

        //     }
        // }
        if (typedChar[0] == 'x') {
            // Procházíme ODZADU, abychom neovlivnili indexy prvků, které nás ještě čekají
            for (int i = MAX_PARTS - 1; i >= 0; i--) {
                if (joints[i].selected){
                    for (int j = i; j < MAX_PARTS - 1; j++) {
                        joints[j] = joints[j + 1];
                    }
                }
                if (!points[i].exists || !points[i].selected) { continue; }

                // 1. Smazání a posun bodů v poli
                points[i].exists = false;
                for (int j = i; j < MAX_PARTS - 1; j++) {
                    points[j] = points[j + 1];
                }
                points[MAX_PARTS - 1].exists = false;
                points_count--;

                // 2. Aktualizace joints (spoje) podle aktuálního smazaného indexu i
                for (int j = 0; j < MAX_PARTS; j++) {
                    if (!joints[j].exists) continue;

                    if (joints[j].p1 == i || joints[j].p2 == i) {
                        joints[j].exists = false; // Spoj zaniká, protože odkazoval na smazaný bod
                    } else {
                        if (joints[j].p1 > i) joints[j].p1--;
                        if (joints[j].p2 > i) joints[j].p2--;
                    }
                }

                // 3. Aktualizace faces (stěny) podle aktuálního smazaného indexu i
                for (int j = 0; j < MAX_PARTS; j++) {
                    if (!faces[j].exists) continue;

                    if (faces[j].p1 == i || faces[j].p2 == i || faces[j].p3 == i) {
                        faces[j].exists = false; // Stěna zaniká
                    } else {
                        if (faces[j].p1 > i) faces[j].p1--;
                        if (faces[j].p2 > i) faces[j].p2--;
                        if (faces[j].p3 > i) faces[j].p3--;
                    }
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}