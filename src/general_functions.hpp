//these aren't all the includes, its here just to shut up VSCode
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
#endif

int randint(int min, int max)
{
    return min + rand() % (max - min + 1);
}
Vec3 sub(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 cross(Vec3 a, Vec3 b) { 
    return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; 
}
inline float dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float dot(Vec2 a, Vec2 b) { return a.x*b.x + a.y*b.y; }
inline float lengthSq(Vec3 a) { return dot(a, a); }
inline float get_dist(Vec3 v1, Vec3 v2) {
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    // Pythagorova věta v 3D
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
inline float get_dist(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    // Pythagorova věta: c = sqrt(a^2 + b^2)
    // Používáme násobení (dx*dx) místo pow(), protože je to mnohem rychlejší
    return sqrtf(dx * dx + dy * dy);
}
inline float get_dist(Vec2 start, Vec2 end) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    // Pythagorova věta: c = sqrt(a^2 + b^2)
    // Používáme násobení (dx*dx) místo pow(), protože je to mnohem rychlejší
    return sqrtf(dx * dx + dy * dy);
}
inline Vec3 normalize(Vec3 a) {
    float l = sqrt(lengthSq(a));
    if (l == 0) return {0,0,1};
    return {a.x/l, a.y/l, a.z/l};
}
void compute_normal(
    const point& p1, const point& p2, const point& p3,
    float& nx, float& ny, float& nz
) {
    // Vektory hran trojúhelníku
    Vec3 v1 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    Vec3 v2 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    
    // Vektorový součin = normála (kolmý vektor)
    Vec3 n = cross(v1, v2);
    
    // Normalizace (aby měl délku 1)
    float len = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
    
    if (len > 0.000001f) {
        nx = n.x / len;
        ny = n.y / len;
        nz = n.z / len;
    } else {
        // Degenerovaný trojúhelník (všechny body v jedné přímce)
        nx = 0; ny = 0; nz = 1;  // výchozí normála
    }
}
float max(float number){
    if (number>0.0f){
        return number;
    }
    return -number;
}
int max(int number){
    if (number>0){
        return number;
    }
    return -number;
}
bool point_in_triangle(const Vec3& P, const Vec3& T1, const Vec3& T2, const Vec3& T3) {
    Vec3 v0 = T3 - T1;
    Vec3 v1 = T2 - T1;
    Vec3 v2 = P - T1;
    
    float dot00 = dot(v0, v0);
    float dot01 = dot(v0, v1);
    float dot02 = dot(v0, v2);
    float dot11 = dot(v1, v1);
    float dot12 = dot(v1, v2);
    
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
}
int load_texture(char *filename)
{
    SDL_Surface *tex = SDL_LoadBMP(filename);
	if (!tex) {
		printf("Error loading texture %s: %s\n", filename, SDL_GetError());
		return 0;
	}
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (antiastropic_filtering){
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        // 2. Nastavíme maximální možnou kvalitu pro aktuálně vázanou (bindnutou) texturu
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

        // 3. Pro nejlepší výsledek to musíš kombinovat s Mipmapami!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->w, tex->h, 0, GL_BGR, GL_UNSIGNED_BYTE, tex->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(tex);

    return texture;
}
GLuint load_texture_PNG(char *filename)
{
    // 1. Use IMG_Load instead of SDL_LoadBMP
    SDL_Surface *temp = IMG_Load(filename);
    if (!temp) {
        printf("Error loading texture %s: %s\n", filename, IMG_GetError());
        return 0;
    }

    // 2. Convert to a guaranteed RGBA format
    // This ensures that even if the PNG is 8-bit or RGB, OpenGL gets 32-bit RGBA
    SDL_Surface *tex = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(temp); 

    if (!tex) return 0;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 3. Upload as GL_RGBA
    // Note: SDL_PIXELFORMAT_RGBA32 usually maps to GL_RGBA on most systems
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // X osa se opakuje (dokola)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    SDL_FreeSurface(tex);

    return texture;
}
void loadVehicleIcons() {
    char path_buffer[256]; // Pomocný buffer pro sestavení cesty

    for (int i = 0; i < number_of_vehicles; i++) {
        // Dynamicky sestavíme cestu: assets/cars/ + jméno + /icon.png
        snprintf(path_buffer, sizeof(path_buffer), "assets/cars/%s/icon.png", car_names[i].c_str());

        vehicle_icons[i] = IMG_Load(path_buffer);

        if (!vehicle_icons[i]) {
            printf("[LOAD VEHICLE ICONS] Chyba načítání (%s): %s\n", path_buffer, IMG_GetError());
        }
    }
    printf("Loaded %d vehicle icons\n", number_of_vehicles);
}
void load_icons() {
    const char* paths[] = {
        "assets/background.png",
        "assets/background_blured.png",
        "assets/jezdit.png",
        "assets/right_arrow.png",
        "assets/left_arrow.png",
        "assets/nastaveni.png",
        "assets/button_background.png"
    };

    for (int i = 0; i < MAX_ICONS; i++) {
        icons[i] = IMG_Load(paths[i]);
        if (!icons[i]) {
            printf("[LOAD VEHICLE ICONS] Chyba načítání: %s\n", IMG_GetError());
        }
    }
    printf("loaded icons\n");
}
void load_map_icons() {
    const char* paths[] = {
        "assets/testmap_icon.png",
        "assets/redmap_icon.png",
        "assets/mountainmap_icon.png",
    };

    for (int i = 0; i < MAX_MAPS; i++) {
        map_icons[i] = IMG_Load(paths[i]);
        if (!map_icons[i]) {
            printf("[LOAD MAP ICONS] Chyba načítání: %s\n", IMG_GetError());
        }
    }
    printf("loaded map icons\n");
}
float clamp(float value, float min, float max){
    if (value>=max){
        return max;
    } else if (value<=min){
        return min;
    }
    return value;
}
int clamp(int value, int min, int max){
    if (value>max){
        return max;
    } else if (value<min){
        return min;
    }
    return value;
}
void lh(std::string text){
    // text="[h] "+text;
    // log(text);
}
float get_heightmap_pixel(int x, int z, float x_shift=0.0f, float z_shift=0.0f) {
    lh("p1");
    // 1. Ošetření posunu (shiftu)
    x += (int)x_shift;
    z += (int)z_shift;
    lh("p2");

    // 2. Korektní Wrap-around (místo fmodf)
    // Takhle se to dělá u intů pro mapy, co se opakují
    x %= MAP_SIZE; if (x < 0) x += MAP_SIZE;
    z %= MAP_SIZE; if (z < 0) z += MAP_SIZE;
    lh("p3");

    // 3. Totální jistota indexů (Clamp)
    int cx = x / CHUNK_SIZE;
    int cz = z / CHUNK_SIZE;
    lh("p4");

    if (!chunks[cz][cx]->loaded){
        lh("p5");

        if (!init_chunk(cz,cx,1)){
            lh("p6");
            return -1.0f;
        };
    }
    lh("p8");
    if (!chunks[cz][cx]->generated){
        lh("p8.5");
        gen_chunk(cz,cx);
    }
    lh("p8.almost9");
    
    // Tohle je ten kritický fix pro tvůj Segfault:
    if (cx >= CHUNKS_SIZE) cx = CHUNKS_SIZE - 1;
    if (cz >= CHUNKS_SIZE) cz = CHUNKS_SIZE - 1;
    lh("p9");

    int lx = x % CHUNK_SIZE;
    int lz = z % CHUNK_SIZE;
    lh("p10");
    
    return chunks[cz][cx]->heightmap[lz][lx];
}
// #define MAPGEN
#ifndef MAPGEN
float get_heightmap_height(float x, float z, float x_shift=0.0f, float z_shift=0.0f) {
    int gx0 = (int)x;
    int gz0 = (int)z;

    // fx a fz MUSÍ být 0.0 až 1.0
    float fx = x - (float)gx0;
    float fz = z - (float)gz0;

    // Načteme 4 sousední body i PŘES HRANICE chunků
    float h1 = get_heightmap_pixel(gx0,     gz0, x_shift, z_shift);
    float h2 = get_heightmap_pixel(gx0 + 1, gz0, x_shift, z_shift);
    float h3 = get_heightmap_pixel(gx0,     gz0 + 1, x_shift, z_shift);
    float h4 = get_heightmap_pixel(gx0 + 1, gz0 + 1, x_shift, z_shift);

    float top = h1 * (1.0f - fx) + h2 * fx;
    float bottom = h3 * (1.0f - fx) + h4 * fx;
    return top * (1.0f - fz) + bottom * fz;
}
#endif
float mapgen_get_heightmap_height(float x, float z, float x_shift=0.0f, float z_shift=0.0f) {
    int spacing = mapgen_spacing;
    if (spacing <= 0) spacing = 1;

    float real_x = x + x_shift;
    float real_z = z + z_shift;

    // --- 1. Pojistka proti přelezení celé mapy ---
    if (real_x < 0.0f) real_x = 0.0f;
    if (real_z < 0.0f) real_z = 0.0f;
    if (real_x > MAP_SIZE - 1) real_x = MAP_SIZE - 1;
    if (real_z > MAP_SIZE - 1) real_z = MAP_SIZE - 1;

    // --- 2. Grid snap ---
    int gx0 = ((int)real_x / spacing) * spacing;
    int gz0 = ((int)real_z / spacing) * spacing;

    int gx1 = gx0 + spacing;
    int gz1 = gz0 + spacing;

    // --- 3. Výpočet vah ---
    float fx = (real_x - (float)gx0) / (float)spacing;
    float fz = (real_z - (float)gz0) / (float)spacing;

    // --- 4. Načtení výšek (S ochranou proti rekurzi sousedních chunků) ---
    float h1 = get_heightmap_pixel(gx0, gz0);
    
    // Pokud je fx == 0, h2 se v bilineární interpolaci vynásobí nulou.
    // Tím, že v tom případě použijeme h1 místo gx1, zabráníme get_heightmap_pixel,
    // aby sahal do sousedního (třeba ještě nevygenerovaného) chunku!
    float h2 = (fx > 0.0001f) ? get_heightmap_pixel(std::min(gx1, (int)MAP_SIZE - 1), gz0) : h1;
    
    // To samé pro osu Z
    float h3 = (fz > 0.0001f) ? get_heightmap_pixel(gx0, std::min(gz1, (int)MAP_SIZE - 1)) : h1;
    
    // Pro h4 musí platit, že se hýbeme v obou osách
    float h4 = (fx > 0.0001f && fz > 0.0001f) ? get_heightmap_pixel(std::min(gx1, (int)MAP_SIZE - 1), std::min(gz1, (int)MAP_SIZE - 1)) : h1;

    // --- 5. Bilineární interpolace (Naprosto stejná, jak jsi ji měl) ---
    float top = h1 * (1.0f - fx) + h2 * fx;
    float bottom = h3 * (1.0f - fx) + h4 * fx;

    // return top * (1.0f - fz) + bottom * fz;
    if (selected_map==MAP_MOUNTAINMAP){
        return gen_mountains_pixel(x,z);
    } else {
        return get_heightmap_height(x,z);
    }
}
#ifdef MAPGEN
float get_heightmap_height(float x, float z, float x_shift=0.0f, float z_shift=0.0f) {
    return mapgen_get_heightmap_height(x,z);
}
#endif
void set_heightmap_height(int x, int z, float val) {
    x = fmodf(x, MAP_SIZE); 
    if (x < 0) x += MAP_SIZE;
    z = fmodf(z, MAP_SIZE); 
    if (z < 0) z += MAP_SIZE;
    if (x >= 0 && x < MAP_SIZE && z >= 0 && z < MAP_SIZE) {
        if (!chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->loaded){init_chunk(z / CHUNK_SIZE,x / CHUNK_SIZE,1);}
        chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->heightmap[z % CHUNK_SIZE][x % CHUNK_SIZE] = val;
    }
}
/* x and z are world space, not chunk coordinates*/
void set_chunk_permanency(int x, int z, bool permanent){
    x = fmodf(x, MAP_SIZE); 
    if (x < 0) x += MAP_SIZE;
    z = fmodf(z, MAP_SIZE); 
    if (z < 0) z += MAP_SIZE;
    if (x >= 0 && x < MAP_SIZE && z >= 0 && z < MAP_SIZE) {
        if (!chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->loaded){init_chunk(z / CHUNK_SIZE,x / CHUNK_SIZE,1);}
        chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->permanent = permanent;
    }
}
void add_heightmap_height(int x, int z, float val) {
    lh("add heightmap height");
    x = fmodf(x, MAP_SIZE); 
    if (x < 0) x += MAP_SIZE;
    z = fmodf(z, MAP_SIZE); 
    if (z < 0) z += MAP_SIZE;
    if (x >= 0 && x < MAP_SIZE && z >= 0 && z < MAP_SIZE) {

        int cx = x / CHUNK_SIZE;
        int cz = z / CHUNK_SIZE;
        
        if (!chunks[cz][cx]->loaded){
            lh("5");

            if (!init_chunk(cz, cx, 1)){
                lh("6");
            };
            lh("7");
        }

        // Tady jsi měl v kódu "=" místo "+=", opravil jsem to:
        chunks[cz][cx]->heightmap[z % CHUNK_SIZE][x % CHUNK_SIZE] += val;
    }
    lh("add heightmap height done");

}
void set_heightmap_texture(int x, int z, int val) {
    x = fmodf(x, MAP_SIZE); 
    if (x < 0) x += MAP_SIZE;
    z = fmodf(z, MAP_SIZE); 
    if (z < 0) z += MAP_SIZE;
    if (x >= 0 && x < MAP_SIZE && z >= 0 && z < MAP_SIZE) {
        if (!chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->loaded){init_chunk(z / CHUNK_SIZE,x / CHUNK_SIZE,1);}
        chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->heightmap_tex[z % CHUNK_SIZE][x % CHUNK_SIZE] = val;
    }
}
uint8_t get_heightmap_texture(int x, int z, float x_shift=0.0f, float z_shift=0.0f) {


    x+=(int)x_shift;
    z+=(int)z_shift;

    x = fmodf(x, MAP_SIZE); 


    if (x < 0) x += MAP_SIZE;

    z = fmodf(z, MAP_SIZE); 

    if (z < 0) z += MAP_SIZE;
    if (x >= 0 && x < MAP_SIZE && z >= 0 && z < MAP_SIZE) {
        if (!chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->loaded){
            if (!init_chunk(z / CHUNK_SIZE,x / CHUNK_SIZE,1)){
                return 0;
            };
        }
        if (!chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->generated){
            gen_chunk(z / CHUNK_SIZE,x / CHUNK_SIZE);
        }
        return chunks[z / CHUNK_SIZE][x / CHUNK_SIZE]->heightmap_tex[z % CHUNK_SIZE][x % CHUNK_SIZE];
    }


    return 0;
}
void wrap_text(char* text, int max_line_width) {
    int len = strlen(text);
    int last_space = -1;
    int current_line_len = 0;

    for (int i = 0; i < len; i++) {
        current_line_len++;

        // Pokud narazíme na existující nový řádek, resetujeme počítadlo
        if (text[i] == '\n') {
            current_line_len = 0;
            last_space = -1;
            continue;
        }

        // Zapamatujeme si pozici poslední mezery
        if (text[i] == ' ') {
            last_space = i;
        }

        // Pokud délka řádku přesáhne limit
        if (current_line_len > max_line_width) {
            if (last_space != -1) {
                // Nahradíme poslední mezeru znakem nového řádku
                text[last_space] = '\n';
                // Nový řádek teď začíná za tou mezerou
                current_line_len = i - last_space;
                last_space = -1;
            } else {
                // Pokud v celém řádku není mezera, musíme to splitnout natvrdo (uprostřed slova)
                // Ale to se u normálního textu většinou nestává.
            }
        }
    }
}
int gpx(int val){
    // return (int)(((float)val/600.0f)*(float)HEIGHT*retina_scale);
    return val;
}
ObjNormal rotateNormal(ObjNormal n, float rx, float ry, float rz) {
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);

    float x = n.x, y = n.y, z = n.z;

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

    return {x, y, z};
}
Vec3 subtract(Vec3 a, Vec3 b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec3 subtract(point a, point b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec3 add(Vec3 a, Vec3 b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vec3 multiply(Vec3 a, float scalar) {
    return { a.x * scalar, a.y * scalar, a.z * scalar };
}
float get_angle(float x1, float y1, float x2, float y2) {
    float deltaX = x2 - x1;
    float deltaY = y2 - y1;

    // Prohozením X a Y v atan2 dosáhneme toho, že 0 je na ose Y
    // a úhel se měří směrem k ose X (po směru hodinových ručiček)
    float angle = std::atan2(deltaX, deltaY);

    // Převedení z rozsahu (-PI, PI] na [0, 2*PI)
    if (angle < 0) {
        angle += 2.0f * PI;
    }

    return angle;
}
void add_unique(std::vector<Vec3>& vec, Vec3 newVal) {
    // Prohledáme vektor od začátku do konce
    auto it = std::find_if(vec.begin(), vec.end(), [&](const Vec3& v) {
        return v.x == newVal.x && v.y == newVal.y && v.z == newVal.z;
    });

    // Pokud jsme nic nenašli (it == end), přidáme to
    if (it == vec.end()) {
        vec.push_back(newVal);
    }
}
inline void log(std::string text){
    #ifdef LOGGING
    printf("[LOG] %s\n", text.c_str());
    SDL_Delay(1);
    #endif
}
inline bool in_chunk(float x, float y, int cz, int cx){
    return (x>=(float)(cz*CHUNK_SIZE) && x<(float)(cz*CHUNK_SIZE+CHUNK_SIZE) && y>=(float)(cx*CHUNK_SIZE) && y<(float)(cx*CHUNK_SIZE+CHUNK_SIZE))?true:false;
}
Vec3 get_outward_normal(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D) {
    Vec3 AB = subtract(B, A);
    Vec3 AC = subtract(C, A);
    Vec3 n = cross(AB, AC);
    float len = sqrtf(dot(n, n));
    if (len < 1e-6) return {0.0f, 1.0f, 0.0f}; // degenerovaný trojúhelník
    n = multiply(n, 1.0f / len);
    // Pokud D leží na straně normály, normála směřuje ven, jinak ji otočíme
    if (dot(subtract(D, A), n) < 0.0f) {
        n = multiply(n, -1.0f);
    }
    return n;
}

// Výsledek raycastu
struct RaycastResult {
    bool hit;        // true pokud paprsek protnul trojúhelník
    float t;         // vzdálenost od počátku k průsečíku (v jednotkách směrového vektoru)
    float u, v;      // barycentrické souřadnice v trojúhelníku (u + v <= 1, w = 1 - u - v)
    Vec3 point;      // souřadnice průsečíku
};

// Raycast proti trojúhelníku (Møller–Trumbore algoritmus)
// origin = počátek paprsku (třeba bod auta)
// dir = směr paprsku (nemusí být jednotkový, ale musí být nenulový)
// v0, v1, v2 = vrcholy trojúhelníku (face)
RaycastResult ray_triangle_intersection(
    point& origin,
    point& v0,
    point& v1,
    point& v2,
    Vec3& dir,
    float epsilon = 0.0001f
) {
    RaycastResult result = {false, 0.0f, 0.0f, 0.0f, {0,0,0}};
    
    Vec3 edge1 = subtract(v1, v0);
    Vec3 edge2 = subtract(v2, v0);
    Vec3 h = cross(dir, edge2);
    float a = dot(edge1, h);
    
    // Pokud je a velmi blízko 0, paprsek je rovnoběžný s trojúhelníkem (nebo míří mimo)
    if (fabs(a) < epsilon) return result;
    
    float f = 1.0f / a;
    Vec3 s = subtract(origin, v0);
    float u = f * dot(s, h);
    
    // Kontrola, jestli je u v mezích (0-1)
    if (u < 0.0f || u > 1.0f) return result;
    
    Vec3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    
    // Kontrola, jestli je v v mezích (0-1) a u+v <= 1
    if (v < 0.0f || u + v > 1.0f) return result;
    
    float t = f * dot(edge2, q);
    
    // t musí být kladné (průsečík před námi)
    if (t < epsilon) return result;
    
    // Úspěch – všechno sedí
    result.hit = true;
    result.t = t;
    result.u = u;
    result.v = v;
    result.point.x = origin.x + dir.x * t;
    result.point.y = origin.y + dir.y * t;
    result.point.z = origin.z + dir.z * t;
    
    return result;
}
float col(int x){return (float)x/255.0;}
float sin_deg(float ang){
	return sin(ang/RAD_DEG);
}
float cos_deg(float ang){
	return cos(ang/RAD_DEG);
}

// ========== SETTINGS FUNCTIONS ==========

void save_settings(void) {
    FILE* file = fopen("settings.cfg", "w");
    if (!file) {
        printf("[SAVE SETTINGS] Error opening file for writing\n");
        return;
    }

    fprintf(file, "vsync=%d\n", vsync);
    fprintf(file, "shadows=%d\n", shadows);
    fprintf(file, "antiastropic_filtering=%d\n", antiastropic_filtering);
    fprintf(file, "fullscreen=%d\n", fullscreen);
    fprintf(file, "highdpi=%d\n", highdpi);
    fprintf(file, "shader_level=%d\n", shader_level);
    fprintf(file, "renderer=%d\n", renderer);
    fprintf(file, "hud_render_interval=%d\n", default_hud_render_interval);
    fprintf(file, "tex_pres=%d\n", tex_pres);
    fprintf(file, "STARTPAUSELENGTH=%d\n", STARTPAUSELENGTH);
    fprintf(file, "TREE_QUALITY_DISTANCE=%d\n", TREE_QUALITY_DISTANCE);
    fprintf(file, "MAP_SIZE=%d\n", MAP_SIZE);
    fprintf(file, "HUD_WIDTH=%d\n", HUD_WIDTH);
    fprintf(file, "HUD_HEIGHT=%d\n", HUD_HEIGHT);
    fprintf(file, "WIDTH=%d\n", WIDTH);
    fprintf(file, "HEIGHT=%d\n", HEIGHT);
    fprintf(file, "SHADOW_RESOLUTION=%d\n", SHADOW_RESOLUTION);
    fprintf(file, "RAD_DEG=%f\n", RAD_DEG);
    fprintf(file, "PI=%f\n", PI);
    fprintf(file, "SIMULATION_FREQ=%f\n", SIMULATION_FREQ);
    fprintf(file, "shift_interval=%f\n", shift_interval);
    fprintf(file, "retina_scale=%f\n", retina_scale);
    fprintf(file, "min_fps=%f\n", min_fps);
    fprintf(file, "render_distance=%f\n", render_distance);
    fprintf(file, "slomo_slowness=%f\n", slomo_slowness);
    fprintf(file, "light_ambient=%f\n", light_ambient);
    fprintf(file, "zoom_fov=%f\n", zoom_fov);
    fprintf(file, "fov=%f\n", FOV);
    fprintf(file, "shadow_range=%f\n", shadow_range);
    fprintf(file, "shadow_jump=%f\n", shadow_jump);
    fprintf(file, "exposure_multiplyer=%f\n", exposure_multiplyer);

    fclose(file);
    printf("[SAVE SETTINGS] Settings saved to settings.cfg\n");
}

void load_settings(std::string filename="settings.cfg") {
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        printf("[LOAD SETTINGS] File not found, using default values\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[128], value[128];
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            if (strcmp(key, "vsync") == 0) vsync = atoi(value);
            else if (strcmp(key, "shadows") == 0) shadows = atoi(value);
            else if (strcmp(key, "antiastropic_filtering") == 0) antiastropic_filtering = atoi(value);
            else if (strcmp(key, "fullscreen") == 0) fullscreen = atoi(value);
            else if (strcmp(key, "highdpi") == 0) highdpi = atoi(value);
            else if (strcmp(key, "shader_level") == 0) shader_level = atoi(value);
            else if (strcmp(key, "renderer") == 0) renderer = atoi(value);
            else if (strcmp(key, "hud_render_interval") == 0) default_hud_render_interval = atoi(value);
            else if (strcmp(key, "tex_pres") == 0) tex_pres = atoi(value);
            else if (strcmp(key, "STARTPAUSELENGTH") == 0) STARTPAUSELENGTH = atoi(value);
            else if (strcmp(key, "TREE_QUALITY_DISTANCE") == 0) TREE_QUALITY_DISTANCE = atoi(value);
            else if (strcmp(key, "MAP_SIZE") == 0) MAP_SIZE = atoi(value);
            else if (strcmp(key, "HUD_WIDTH") == 0) HUD_WIDTH = atoi(value);
            else if (strcmp(key, "HUD_HEIGHT") == 0) HUD_HEIGHT = atoi(value);
            else if (strcmp(key, "WIDTH") == 0) WIDTH = atoi(value);
            else if (strcmp(key, "HEIGHT") == 0) HEIGHT = atoi(value);
            else if (strcmp(key, "RAD_DEG") == 0) RAD_DEG = atof(value);
            else if (strcmp(key, "PI") == 0) PI = atof(value);
            else if (strcmp(key, "SIMULATION_FREQ") == 0) SIMULATION_FREQ = atof(value);
            else if (strcmp(key, "shift_interval") == 0) shift_interval = atof(value);
            else if (strcmp(key, "retina_scale") == 0) retina_scale = atof(value);
            else if (strcmp(key, "min_fps") == 0) min_fps = atof(value);
            else if (strcmp(key, "render_distance") == 0) render_distance = atof(value);
            else if (strcmp(key, "slomo_slowness") == 0) slomo_slowness = atof(value);
            else if (strcmp(key, "light_ambient") == 0) light_ambient = atof(value);
            else if (strcmp(key, "zoom_fov") == 0) zoom_fov = atof(value);
            else if (strcmp(key, "fov") == 0) FOV = atof(value);
            else if (strcmp(key, "shadow_range") == 0) shadow_range = atof(value);
            else if (strcmp(key, "shadow_jump") == 0) shadow_jump = atof(value);
            else if (strcmp(key, "SHADOW_RESOLUTION") == 0) SHADOW_RESOLUTION = atof(value);
            else if (strcmp(key, "exposure_multiplyer") == 0) exposure_multiplyer = atof(value);
        }
    }
t_vsync = vsync, t_shadows = shadows, t_antiastropic_filtering = antiastropic_filtering, t_fullscreen = fullscreen, t_highdpi = highdpi;
t_shader_level = shader_level, t_renderer = renderer, t_tex_pres = tex_pres, t_hud_render_interval = hud_render_interval, t_STARTPAUSELENGTH = STARTPAUSELENGTH, t_TREE_QUALITY_DISTANCE = TREE_QUALITY_DISTANCE, t_WIDTH = WIDTH, t_HEIGHT = HEIGHT, t_MAP_SIZE = MAP_SIZE, t_HUD_WIDTH = HUD_WIDTH, t_HUD_HEIGHT = HUD_HEIGHT, t_collision_skip_rate = collision_skip_rate, t_SHADOW_RESOLUTION = SHADOW_RESOLUTION;
t_RAD_DEG = RAD_DEG, t_PI = PI, t_SIMULATION_FREQ = SIMULATION_FREQ, t_shift_interval = shift_interval, t_retina_scale = retina_scale, t_min_fps = min_fps, t_render_distance = render_distance, t_slomo_slowness = slomo_slowness, t_light_ambient = light_ambient, t_zoom_fov = zoom_fov, t_shadow_range = shadow_range, t_shadow_jump = shadow_jump, t_exposure_multiplyer = exposure_multiplyer;

    fclose(file);
    printf("[LOAD SETTINGS] Settings loaded from settings.cfg\n");
}

void reset_settings(void) {
    load_settings("settings_default.cfg");
    printf("[RESET SETTINGS] All settings reset to defaults\n");
}
void menu_go_back(){
    menu_type=last_menu_type;
}
void change_menu(int type){
    if (menu_type==MENU_TYPE_INTRO){
        last_menu_type=MENU_TYPE_INTRO;
    } else if (menu_type==MENU_TYPE_NONE){
        last_menu_type=MENU_TYPE_NONE;
    } else if (menu_type==MENU_TYPE_NONE){
        last_menu_type=MENU_TYPE_NONE;
    }
    menu_type=type;
}