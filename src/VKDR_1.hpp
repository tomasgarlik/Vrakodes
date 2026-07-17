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
#include "gui.hpp"
#endif
// this file contains my goofy ahh opengl renderer
GLuint backup_texture;
void VKDR1_setup(int width, int height)
{
	printf("[VKDR] setting up renderer...\n");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
    // --- LIGHTING SETUP ---
    glEnable(GL_LIGHTING);   // Turn on the lighting engine
    glEnable(GL_LIGHT0);     // Enable the first light source
    glEnable(GL_NORMALIZE);  // Automatically scale normals to unit length
    
    // This allows glColor3f to still control the color of your objects
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Define light properties
    GLfloat light_ambient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);


	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, metal_specular);

	// Nastav ostrost odlesku (0-128). 
	// 50.0f je slušný lesk, 128.0f je jako zrcadlo, 1.0f je matný plast.
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 70.0f);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    // ----------------------
    glShadeModel(GL_SMOOTH);

    glLineWidth(6.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    GLfloat ratio = (GLfloat)width / (GLfloat)height;
    float top = NEAR * tan(FOV * 0.5 * PI / 180.0);
    float bottom = -top;
    float right = top * ratio;
    float left = -right;
    
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

    glFrustum(left, right, bottom, top, NEAR, FAR_DISTANCE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Záporná hodnota = ostřejší, kladná hodnota = víc rozmazané (klidnější)
    // Zkus 0.5f až 1.0f pro ten "hladký" pocit v dálce
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0.5f);

    backup_texture=load_texture_PNG("assets/objects/textures/texture.png");
}
void __vkdr1_rend_skybox(float camX, float camY, float camZ) {
    glDisable(GL_LIGHTING); // Obloha nesvítí
    glDisable(GL_DEPTH_TEST); // Aby byla vždy v pozadí (nebo použij glDepthMask)
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[4]);
    
    glPushMatrix();
    glTranslatef(camX, camY, camZ); // Skybox se hýbe s kamerou
    
    // Barva bílá, aby textura nebyla zabarvená
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        // --- 1. ČTVRTINA (Přední) ---
        glTexCoord2f(0.001f, 0.9f); glVertex3f(-1, -1.5, -1);
        glTexCoord2f(0.25f, 0.9f); glVertex3f( 1, -1.5, -1);
        glTexCoord2f(0.25f, 0.1f); glVertex3f( 1,  1.5, -1);
        glTexCoord2f(0.001f, 0.1f); glVertex3f(-1,  1.5, -1);

        // --- 2. ČTVRTINA (Pravá) ---
        glTexCoord2f(0.25f, 0.9f); glVertex3f( 1, -1.5, -1);
        glTexCoord2f(0.50f, 0.9f); glVertex3f( 1, -1.5,  1);
        glTexCoord2f(0.50f, 0.1f); glVertex3f( 1,  1.5,  1);
        glTexCoord2f(0.25f, 0.1f); glVertex3f( 1,  1.5, -1);

        // --- 3. ČTVRTINA (Zadní) ---
        glTexCoord2f(0.50f, 0.9f); glVertex3f( 1, -1.5,  1);
        glTexCoord2f(0.75f, 0.9f); glVertex3f(-1, -1.5,  1);
        glTexCoord2f(0.75f, 0.1f); glVertex3f(-1,  1.5,  1);
        glTexCoord2f(0.50f, 0.1f); glVertex3f( 1,  1.5,  1);

        // --- 4. ČTVRTINA (Levá) ---
        glTexCoord2f(0.75f, 0.9f); glVertex3f(-1, -1.5,  1);
        glTexCoord2f(0.999f, 0.9f); glVertex3f(-1, -1.5, -1);
        glTexCoord2f(0.999f, 0.1f); glVertex3f(-1,  1.5, -1);
        glTexCoord2f(0.75f, 0.1f); glVertex3f(-1,  1.5,  1);

        // --- STROP (Vezme horní pruh textury 0.8 - 1.0) ---
        // Všechny horní body se sbíhají do středu nahoře
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1,  1.5, -1);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1,  1.5, -1);
        glTexCoord2f(1.0f, 0.1f); glVertex3f( 1,  1.5,  1);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(-1,  1.5,  1);
        
        // --- PODLAHA (Vezme spodní pruh textury 0.0 - 0.2) ---
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, -1.5, -1);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1, -1.5, -1);
        glTexCoord2f(1.0f, 0.9f); glVertex3f( 1, -1.5,  1);
        glTexCoord2f(0.0f, 0.9f); glVertex3f(-1, -1.5,  1);
    glEnd();

    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
void __vkdr1_rend_cube(Vec3 pos, float size) {
    float h = size * 0.5f; // "Half-size" - vzdálenost od středu ke stěnám
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    // --- Přední stěna (Z+) ---
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(pos.x - h, pos.y - h, pos.z + h);
    glVertex3f(pos.x + h, pos.y - h, pos.z + h);
    glVertex3f(pos.x + h, pos.y + h, pos.z + h);
    glVertex3f(pos.x - h, pos.y + h, pos.z + h);

    // --- Zadní stěna (Z-) ---
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(pos.x - h, pos.y - h, pos.z - h);
    glVertex3f(pos.x - h, pos.y + h, pos.z - h);
    glVertex3f(pos.x + h, pos.y + h, pos.z - h);
    glVertex3f(pos.x + h, pos.y - h, pos.z - h);

    // --- Horní stěna (Y+) ---
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(pos.x - h, pos.y + h, pos.z - h);
    glVertex3f(pos.x - h, pos.y + h, pos.z + h);
    glVertex3f(pos.x + h, pos.y + h, pos.z + h);
    glVertex3f(pos.x + h, pos.y + h, pos.z - h);

    // --- Spodní stěna (Y-) ---
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(pos.x - h, pos.y - h, pos.z - h);
    glVertex3f(pos.x + h, pos.y - h, pos.z - h);
    glVertex3f(pos.x + h, pos.y - h, pos.z + h);
    glVertex3f(pos.x - h, pos.y - h, pos.z + h);

    // --- Pravá stěna (X+) ---
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(pos.x + h, pos.y - h, pos.z - h);
    glVertex3f(pos.x + h, pos.y + h, pos.z - h);
    glVertex3f(pos.x + h, pos.y + h, pos.z + h);
    glVertex3f(pos.x + h, pos.y - h, pos.z + h);

    // --- Levá stěna (X-) ---
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(pos.x - h, pos.y - h, pos.z - h);
    glVertex3f(pos.x - h, pos.y - h, pos.z + h);
    glVertex3f(pos.x - h, pos.y + h, pos.z + h);
    glVertex3f(pos.x - h, pos.y + h, pos.z - h);

    glEnd();
}
void __vkdr1_render_hud(){

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Řekni OpenGL, že řádky v SDL surface jsou dlouhé podle "pitch"
    // To vyřeší ten "bordel" (rozsypaný obraz) elegantně bez kopírování
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hud_surf->pitch / 4); 

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                hud_surf->w, hud_surf->h, 0,
                GL_BGRA, GL_UNSIGNED_BYTE, hud_surf->pixels); // Posíláme přímo!

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); // Vrať to zpátky na nulu pro ostatní textury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);

		p1.x=-((float)(WIDTH)/(float)(HEIGHT));
		p1.y=-1.0;
		p1.z=-1.185;

		p2.x=((float)(WIDTH)/(float)(HEIGHT));
		p2.y=-1.0;
		p2.z=-1.185;

		p3.x=((float)(WIDTH)/(float)(HEIGHT));
		p3.y=1.0;
		p3.z=-1.185;

		p4.x=-((float)(WIDTH)/(float)(HEIGHT));
		p4.y=1.0;
		p4.z=-1.185;
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(p1.x, p1.y, p1.z);
			glTexCoord2i(1, 1); glVertex3f(p2.x, p2.y, p2.z);
			glTexCoord2i(1, 0); glVertex3f(p3.x, p3.y, p3.z);
			glTexCoord2i(0, 0); glVertex3f(p4.x, p4.y, p4.z);

		glEnd();





	// if (textSurface) {
    //     SDL_FreeSurface(textSurface);
    //     textSurface = NULL; // Pro jistotu
    // }

    // 2. Smaž OpenGL texturu HUDu (jinak ti shoří grafika)
    glDeleteTextures(1, &texture);
}
void __vkdr1_rend_cars(){
    for (int meh=0;meh<cars.size();meh++){
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, cars[meh].texture);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < cars[meh].faces_count; i++) {
			face &f = cars[meh].faces[i];

			// Nastavíme normálu JEDNOU pro celou plochu
			glNormal3f(f.nx, f.ny, f.nz);

			for (int j = 0; j < 3; j++) {
				car_vertex &v = cars[meh].vertices[f.vertices[j]];

				// UV a pozice se stále mění pro každý vrchol
				glTexCoord2f(v.u, v.v);
				glVertex3f(v.x, v.y, v.z);
			}
		}
		glEnd();
    }
}
void __vkdr1_draw_fat_line(float sx, float sy, float sz,float ex, float ey, float ez,float radius)
{
    Vec3 S{sx,sy,sz};
    Vec3 E{ex,ey,ez};

    Vec3 D{ex-sx, ey-sy, ez-sz};
    D = normalize(D);

    // najdeme libovolný vektor nekolmý s D
    Vec3 up = fabs(D.y) < 0.99f ? Vec3{0,1,0} : Vec3{1,0,0};

    // vytvoříme ortogonální bázi kolem směru čáry
    Vec3 R = normalize(cross(D, up));   // right
    Vec3 U = cross(R, D);               // druhý kolmý

    // 3 body kolem kružnice (120° rozestupy)
    Vec3 offsets[3];
    for(int i=0;i<3;i++){
        float ang = i * 2.0943951f; // 2π/3
        float c = cosf(ang);
        float s = sinf(ang);
        offsets[i] = {
            R.x*c*radius + U.x*s*radius,
            R.y*c*radius + U.y*s*radius,
            R.z*c*radius + U.z*s*radius
        };
    }

    // kreslení 3 obdélníků (stěny trubky)
    glBegin(GL_QUADS);
    for(int i=0;i<3;i++){
        int j = (i+1)%3;

        glVertex3f(S.x + offsets[i].x, S.y + offsets[i].y, S.z + offsets[i].z);
        glVertex3f(S.x + offsets[j].x, S.y + offsets[j].y, S.z + offsets[j].z);
        glVertex3f(E.x + offsets[j].x, E.y + offsets[j].y, E.z + offsets[j].z);
        glVertex3f(E.x + offsets[i].x, E.y + offsets[i].y, E.z + offsets[i].z);
    }
    glEnd();
}
void __vkdr1_drawstaticObjectBuffers() {
    for (auto &buf : staticObjectBuffers) {
        glBindTexture(GL_TEXTURE_2D, buf.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, buf.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf.ebo);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 8*sizeof(float), (void*)0);
        glNormalPointer(GL_FLOAT, 8*sizeof(float), (void*)(3*sizeof(float)));
        glTexCoordPointer(2, GL_FLOAT, 8*sizeof(float), (void*)(6*sizeof(float)));

        glDrawElements(GL_TRIANGLES, buf.indexCount, GL_UNSIGNED_INT, nullptr);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}
void VKDR1_MakeStaticBuffers(std::vector<mesh> objects){
    printf("[VKDR] making static buffers\n");
    staticObjectBuffers.clear();

    std::map<int, std::vector<std::pair<mesh*, Vec3>>> objectsByTexture;
    int polyCount=0;
    for (int i = 0; i < objects.size(); i++) {
        mesh* m=&objects[i];

        polyCount+=m->faces.size();
        objectsByTexture[m->texture].push_back({m, Vec3{m->x, m->y, m->z}});
    }
    printf("    [VKDR] Succesfully putted exactly %d objects into some std::map big %d. The poly count is %d.\n    [VKDR] making more stuff for no reason...\n", objects.size(), objectsByTexture.size(), polyCount);


    // 2. Pro každou texturu vytvoříme buffer
    for (auto &entry : objectsByTexture) {
        printf("    [VKDR] making another buffer\n");
        int texID = entry.first;
        auto &objList = entry.second;

        std::vector<float> verts;
        std::vector<float> norms;
        std::vector<float> uvs;
        std::vector<unsigned int> indices;
        unsigned int vertOffset = 0;

        for (auto &p : objList) {
            mesh* m = p.first;
            Vec3 pos = p.second;

            for (auto &fce : m->faces) {
                if (m->faces.empty()){
                    printf("    [VKDR] some mesh called \"m\" has no faces\n[VKDR]       Failed succesfully\n");
                }
                for (int i=0;i<3;i++) {
                    ObjVertex &v = m->vertices[fce.v[i]];
                    verts.push_back(v.x + pos.x);
                    verts.push_back(v.y + pos.y);
                    verts.push_back(v.z + pos.z);

                    if (fce.vn[i] >= 0 && fce.vn[i] < (int)m->normals.size()) {
                        ObjNormal &n = m->normals[fce.vn[i]];
                        norms.push_back(n.x); norms.push_back(n.y); norms.push_back(n.z);
                    } else {
                        norms.push_back(0); norms.push_back(1); norms.push_back(0); // fallback
                    }

                    if (fce.vt[i] >= 0 && fce.vt[i] < (int)m->uvs.size()) {
                        ObjUV &uv = m->uvs[fce.vt[i]];
                        uvs.push_back(uv.u); uvs.push_back(uv.v);
                    } else {
                        uvs.push_back(0); uvs.push_back(0);
                    }

                    indices.push_back(vertOffset++);
                }
            }
        }
        printf("        [VKDR] Success, made some %d vertecies, making even more buffers...\n", verts.size());
        if (verts.size()==0){
            printf("          [VKDR] Failed succesfully, vertecies are 0.\n");
            continue;
        }
	log("1");
        MeshBuffer buf;
        buf.textureID = texID;
        buf.indexCount = (GLsizei)indices.size();
        log("2");

        glGenBuffers(1, &buf.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, buf.vbo);
        log("3");

        // uložíme vertexy + normály + UV dohromady (interleaved)
        std::vector<float> vertexData;
        for (size_t vi = 0; vi < verts.size()/3; vi++) {
            vertexData.push_back(verts[vi*3+0]);
            vertexData.push_back(verts[vi*3+1]);
            vertexData.push_back(verts[vi*3+2]);

            vertexData.push_back(norms[vi*3+0]);
            vertexData.push_back(norms[vi*3+1]);
            vertexData.push_back(norms[vi*3+2]);

            vertexData.push_back(uvs[vi*2+0]);
            vertexData.push_back(uvs[vi*2+1]);
        }
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &buf.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        if (!buf.vbo || !buf.ebo){
            printf("[VKDR] Create static buffers failed succesfully.\n");
        }
        staticObjectBuffers.push_back(buf);
    }
    printf("[VKDR] Succesfully created static object buffers\n");
}
void __vkdr1_debug_draw_tetra(cardata& car) {
    // Povolíme osvětlení (pokud už není zapnuté)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // nebo jaké máš světlo
    
    // Nastavíme materiál (barvu, lesk atd.)
    GLfloat mat_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat mat_diffuse[] = {0.8f, 0.3f, 0.3f, 1.0f}; // červená
    GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat mat_shininess[] = {50.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    for (int t = 0; t < car.tetrahedra_count; t++) {
        tetrahedron& tet = car.tetrahedra[t];
        // Body tetrahedronu (převod na Vec3)
        Vec3 A = {car.points[tet.idx[0]].x, car.points[tet.idx[0]].y, car.points[tet.idx[0]].z};
        Vec3 B = {car.points[tet.idx[1]].x, car.points[tet.idx[1]].y, car.points[tet.idx[1]].z};
        Vec3 C = {car.points[tet.idx[2]].x, car.points[tet.idx[2]].y, car.points[tet.idx[2]].z};
        Vec3 D = {car.points[tet.idx[3]].x, car.points[tet.idx[3]].y, car.points[tet.idx[3]].z};

        // Kreslíme čtyři trojúhelníkové stěny
        // Stěna 1: B C D (protější A)
        {
            Vec3 n = get_outward_normal(B, C, D, A);
            glBegin(GL_TRIANGLES);
            glNormal3f(n.x, n.y, n.z);
            glVertex3f(B.x, B.y, B.z);
            glVertex3f(C.x, C.y, C.z);
            glVertex3f(D.x, D.y, D.z);
            glEnd();
        }
        // Stěna 2: A C D (protější B)
        {
            Vec3 n = get_outward_normal(A, C, D, B);
            glBegin(GL_TRIANGLES);
            glNormal3f(n.x, n.y, n.z);
            glVertex3f(A.x, A.y, A.z);
            glVertex3f(C.x, C.y, C.z);
            glVertex3f(D.x, D.y, D.z);
            glEnd();
        }
        // Stěna 3: A B D (protější C)
        {
            Vec3 n = get_outward_normal(A, B, D, C);
            glBegin(GL_TRIANGLES);
            glNormal3f(n.x, n.y, n.z);
            glVertex3f(A.x, A.y, A.z);
            glVertex3f(B.x, B.y, B.z);
            glVertex3f(D.x, D.y, D.z);
            glEnd();
        }
        // Stěna 4: A B C (protější D)
        {
            Vec3 n = get_outward_normal(A, B, C, D);
            glBegin(GL_TRIANGLES);
            glNormal3f(n.x, n.y, n.z);
            glVertex3f(A.x, A.y, A.z);
            glVertex3f(B.x, B.y, B.z);
            glVertex3f(C.x, C.y, C.z);
            glEnd();
        }
    }
}
void __vkdr1_debug_draw_sphere(float radius, int segments, int rings, Vec3 pos) {
    const float PI = 3.1415926535f;
    
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z); // Posune kouli na její souřadnice

    for (int i = 0; i < rings; ++i) {
        float lat0 = PI * (-0.5f + (float)i / rings);
        float z0  = sinf(lat0);
        float r0  = cosf(lat0);

        float lat1 = PI * (-0.5f + (float)(i + 1) / rings);
        float z1  = sinf(lat1);
        float r1  = cosf(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; ++j) {
            float lng = 2.0f * PI * (float)j / segments;
            float x = cosf(lng);
            float y = sinf(lng);

            // Normála pro smooth shading (v podstatě směr od středu k vrcholu)
            glNormal3f(x * r0, y * r0, z0); 
            glVertex3f(x * r0 * radius, y * r0 * radius, z0 * radius);

            glNormal3f(x * r1, y * r1, z1);
            glVertex3f(x * r1 * radius, y * r1 * radius, z1 * radius);
        }
        glEnd();
    }
    
    glPopMatrix();
}
void VKDR1_render()
{
    int now_time=SDL_GetTicks64();
    int old_time=now_time;
    log("   renderer clearing");
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    // Camera setup
    log("   rotating");
    glRotatef(-y_rot, 1.0, 0.0, 0.0);
    glRotatef(x_rot + 90, 0.0, 1.0, 0.0);
    glTranslatef(-x_pos, -y_pos, -z_pos);
    log("   skybox");
    __vkdr1_rend_skybox(x_pos,y_pos,z_pos);
    // 1. Position the Sun
    log("   light");
    GLfloat light_pos[4]; 
    if(selected_map==0){
        light_pos[0] = 100000.0f;
        light_pos[1] = 1000000.0f;
        light_pos[2] = 100000.0f;
        light_pos[3] = 1.0f;
    } else {
        light_pos[0] = 1000000.0f;
        light_pos[1] = 1000000.0f;
        light_pos[2] = 1000000.0f;
        light_pos[3] = 1.0f;
    }
    Vec3 light_pos_vec={light_pos[0],light_pos[1],light_pos[2]};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, other_specular);
    log("   draw static object buffers");
    now_time=SDL_GetTicks64();
    temp_render_process_times[0]=now_time-old_time;
    old_time=now_time;
    __vkdr1_drawstaticObjectBuffers();
    now_time=SDL_GetTicks64();
    temp_render_process_times[1]=now_time-old_time;
    old_time=now_time;




    // shadows



    now_time=SDL_GetTicks64();
    temp_render_process_times[2]=now_time-old_time;
    old_time=now_time;
    log("   car material");
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, metal_specular);

    // --- ZBYTEK RENDERU ---
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, other_specular);
    glEnable(GL_LIGHTING);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, metal_specular);
    if (debug == 0 || debug==1) {
        glColor4f(1.0f,1.0f,1.0f,1.0f);
        log("   update car vbo");
        log("   rend car");
        __vkdr1_rend_cars();
    }

    now_time=SDL_GetTicks64();
    temp_render_process_times[3]=now_time-old_time;
    old_time=now_time;
    glDisable(GL_LIGHTING);


	glColor3f(1.0,1.0,1.0);

	glCallList(white_list);




	glColor3f(0.0f,0.0f,0.0f);
    if (debug!=0){
        log("   debug");
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        
        // Nastav si vlastní čistý stav
        glDisable(GL_TEXTURE_2D);           // Vypni textury
        glEnable(GL_LIGHTING);               // Zapni světla (pokud je cube potřebuje)
        glEnable(GL_COLOR_MATERIAL);         // Aby barva fungovala
        glColor3f(0.0f, 0.0f, 0.0f);        // Tvoje barva
        glEnable(GL_LIGHTING);
        if (debug==3){
            for (int i=0;i<voxels.size();i++){
                __vkdr1_rend_cube(voxels[i],voxel_size);
            }
            voxels.clear();
        }
        glDisable(GL_LIGHTING);

        //joints debug

        for (int melm=0;melm<cars.size();melm++){
            for (i=0;i<cars[melm].joints_count;i++){
                char text[256];
                sprintf(text,"i=%d", i);
                log(text);
                if (cars[melm].joints[i].group==0){
                    glColor3f(0.0f,0.0f,0.4f);
                } else if (cars[melm].joints[i].group==1){
                    glColor3f(0.8f,0.7f,0.0f);
                } else if (cars[melm].joints[i].group==2){
                    glColor3f(0.3f,0.0f,0.3f);
                } else if (cars[melm].joints[i].group==3){
                    glColor3f(0.0f,1.0f,1.0f);
                } else if (cars[melm].joints[i].group==4){
                    glColor3f(0.0f,1.0f,0.0f);
                } else if (cars[melm].joints[i].group==5){
                    glColor3f(1.0f,0.0f,0.0f);
                }
                if (cars[melm].joints[i].clamped){
                    glColor3f(1.0f,1.0f,1.0f);
                }
                __vkdr1_draw_fat_line(cars[melm].points[cars[melm].joints[i].p1].x, cars[melm].points[cars[melm].joints[i].p1].y, cars[melm].points[cars[melm].joints[i].p1].z,
                                cars[melm].points[cars[melm].joints[i].p2].x, cars[melm].points[cars[melm].joints[i].p2].y, cars[melm].points[cars[melm].joints[i].p2].z, 0.01f);

            }

        }
        log("coliision faces");
        // now wthe collision faces
        if (debug!=3){
            glBegin(GL_TRIANGLES);
            glColor3f(0.2f, 0.8f, 0.2f);
            for (int meh=0;meh<cars.size();meh++){
                for (int i = 0; i < cars[meh].col_faces_count; i++) {
                    char text[256];
                    sprintf(text,"i=%d", i);
                    log(text);
                    face &f = cars[meh].col_faces[i];
                    if (std::find(cars_collided_faces[meh].begin(), cars_collided_faces[meh].end(), i) != cars_collided_faces[meh].end()){
                        glColor3f(0.8f, 0.2f, 0.2f);
                    } else {
                        glColor3f(0.2f, 0.8f, 0.2f);
                    }
                    // Nastavíme normálu jednou pro celou plochu
                    glNormal3f(0.0f, 1.0f, 0.0f);

                    for (int j = 0; j < 3; j++) {
                        point &v = cars[meh].points[f.vertices[j]];

                        glVertex3f(v.x, v.y, v.z);
                    }
                }
            }
            glEnd();
            // and we will also draw the volume poses
            glEnable(GL_LIGHTING);
            glColor3f(0.1f,0.2f,1.0f);
            for (int meh=0;meh<cars.size();meh++){
                for (int prd=0;prd<cars[meh].volumes_count;prd++){
                    __vkdr1_debug_draw_sphere(0.07f, 16,16,cars[meh].volume_poses[prd]);
                }
            }
            glDisable(GL_LIGHTING);
        }
        if (debug==4){
            for (int meh=0;meh<cars.size();meh++){
                __vkdr1_debug_draw_tetra(cars[meh]);
            }
        }
		// 1. Zjistíme, kde je kamera v "reálném" světě pro výběr chunků

        // float world_pos_x = x_pos + (float)x_shift;
        // float world_pos_z = z_pos + (float)z_shift;

        float world_pos_x = x_pos;
        float world_pos_z = z_pos;

        int center_cx = (int)(world_pos_x / CHUNK_SIZE);
        int center_cz = (int)(world_pos_z / CHUNK_SIZE);

        // Nastavíme barvu pro všechny boxy najednou (výkonnější než uvnitř cyklu)
        glColor3f(0.0f, 1.0f, 0.0f);

        // 2. Procházíme chunky v okolí kamery (např. v rozsahu 2 chunků)
        int range = 2; 
        for (int cz = center_cz - range; cz <= center_cz + range; cz++) {
            for (int cx = center_cx - range; cx <= center_cx + range; cx++) {
                
                if (cx < 0 || cz < 0 || cx >= CHUNKS_SIZE || cz >= CHUNKS_SIZE) continue;
                chunk* target = chunks[cz][cx];
                if (target == NULL) continue;
                for (int i = 0; i < target->collision_boxes_count; i++) {
                    collision_box& b = target->collision_boxes[i];
                    
                    // Střed ve světě relativně k shiftu
                    // float mid_x = (b.sx + b.ex) * 0.5f - (float)x_shift;
                    // float mid_y = (b.sy + b.ey) * 0.5f;
                    // float mid_z = (b.sz + b.ez) * 0.5f - (float)z_shift;

                    float mid_x = (b.sx + b.ex) * 0.5f;
                    float mid_y = (b.sy + b.ey) * 0.5f;
                    float mid_z = (b.sz + b.ez) * 0.5f;

                    // Rozměry
                    float hx = (b.ex - b.sx) * 0.5f;
                    float hy = (b.ey - b.sy) * 0.5f;
                    float hz = (b.ez - b.sz) * 0.5f;

                    // Předvýpočet sin/cos (stejně jako v tvém renderu)
                    float sx = sinf(b.rx), cx = cosf(b.rx);
                    float sy = sinf(b.ry), cy = cosf(b.ry);
                    float sz = sinf(b.rz), cz = cosf(b.rz);

                    // Definice 8 vrcholů boxu v local space
                    float vx[8] = {-hx,  hx,  hx, -hx, -hx,  hx,  hx, -hx};
                    float vy[8] = {-hy, -hy,  hy,  hy, -hy, -hy,  hy,  hy};
                    float vz[8] = { hz,  hz,  hz,  hz, -hz, -hz, -hz, -hz};

                    // Rotace každého vrcholu (přesně podle tvého renderovacího kódu)
                    for(int v=0; v<8; v++) {
                        float x = vx[v];
                        float y = vy[v];
                        float z = vz[v];

                        // 1. Rotace Y
                        float x1 = x*cy + z*sy;
                        float z1 = -x*sy + z*cy;
                        x = x1; z = z1;

                        // 2. Rotace X
                        float y1 = y*cx - z*sx;
                        float z2 = y*sx + z*cx;
                        y = y1; z = z2;

                        // 3. Rotace Z
                        float x2 = x*cz - y*sz;
                        float y2 = x*sz + y*cz;
                        x = x2; y = y2;

                        vx[v] = x + mid_x;
                        vy[v] = y + mid_y;
                        vz[v] = z + mid_z;
                    }

                    // Kreslení čar mezi rotovanými vrcholy
                    glBegin(GL_LINES);
                        // Spodní čtverec (indexy 0,1,5,4) - podle tvého verts pole
                        glVertex3f(vx[0], vy[0], vz[0]); glVertex3f(vx[1], vy[1], vz[1]);
                        glVertex3f(vx[1], vy[1], vz[1]); glVertex3f(vx[5], vy[5], vz[5]);
                        glVertex3f(vx[5], vy[5], vz[5]); glVertex3f(vx[4], vy[4], vz[4]);
                        glVertex3f(vx[4], vy[4], vz[4]); glVertex3f(vx[0], vy[0], vz[0]);
                        // Horní čtverec (indexy 3,2,6,7)
                        glVertex3f(vx[3], vy[3], vz[3]); glVertex3f(vx[2], vy[2], vz[2]);
                        glVertex3f(vx[2], vy[2], vz[2]); glVertex3f(vx[6], vy[6], vz[6]);
                        glVertex3f(vx[6], vy[6], vz[6]); glVertex3f(vx[7], vy[7], vz[7]);
                        glVertex3f(vx[7], vy[7], vz[7]); glVertex3f(vx[3], vy[3], vz[3]);
                        // Svislé hrany
                        glVertex3f(vx[0], vy[0], vz[0]); glVertex3f(vx[3], vy[3], vz[3]);
                        glVertex3f(vx[1], vy[1], vz[1]); glVertex3f(vx[2], vy[2], vz[2]);
                        glVertex3f(vx[5], vy[5], vz[5]); glVertex3f(vx[6], vy[6], vz[6]);
                        glVertex3f(vx[4], vy[4], vz[4]); glVertex3f(vx[7], vy[7], vz[7]);
                    glEnd();
                }
            }
        }
        glPopAttrib();

	}

    log("   time measurement");
    now_time=SDL_GetTicks64();
    temp_render_process_times[4]=now_time-old_time;
    old_time=now_time;
        glColor3f(1.0f,1.0f,1.0f);




        glDisable(GL_DEPTH_TEST);
        glTranslatef(x_pos, y_pos, z_pos);

        glRotatef(-x_rot-90, 0.0, 1.0, 0.0);
        glRotatef(y_rot, 1.0, 0.0, 0.0);
        // if (now_time-last_hud_render_time>=hud_render_interval){
        //     render_hud();
        //     last_hud_render_time=now_time;
        // } else {
        //     render_hud();
        // }
        __vkdr1_render_hud();
        glColor3f(1.0,1.0,1.0);
        glEnable(GL_LIGHTING);
    now_time=SDL_GetTicks64();
    temp_render_process_times[5]=now_time-old_time;
    old_time=now_time;
    for (i=0;i<RENDER_PROCESS_COUNT-1;i++){
        render_process_times[i]=temp_render_process_times[i];
    }
	return;

}
void VKDR1_get_gl_info() {
    printf("[VKDR] getting OpenGL info\n");
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    strncpy(info_vendor, vendor, sizeof(info_vendor) - 1);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    strncpy(info_renderer, renderer, sizeof(info_vendor) - 1);
    const char* version = (const char*)glGetString(GL_VERSION);
    strncpy(info_glversion, version, sizeof(info_vendor) - 1);
    strncpy(info_vkdr, "Vrakodes renderer 1.0", sizeof(info_vkdr) - 1);

}
