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


//serves unknown purpose
struct SDL_PixelFormat;


void interpolate_chunk_heightmap(int cx, int cy, int spacing) {
    log("interpolating heightmap (single-threaded)");
    int chunk_start_x = cx * CHUNK_SIZE;
    int chunk_start_y = cy * CHUNK_SIZE;
    int chunk_end_x   = chunk_start_x + CHUNK_SIZE;
    int chunk_end_y   = chunk_start_y + CHUNK_SIZE;

    int border_x = chunk_end_x - 1;
    int border_y = chunk_end_y - 1;

    for (int i = chunk_start_x; i < chunk_end_x; i++) {
        for (int j = chunk_start_y; j < chunk_end_y; j++) {
            
            // Levý horní mřížkový bod (gx, gy)
            int gx = chunk_start_x + ((i - chunk_start_x) / spacing) * spacing;
            int gy = chunk_start_y + ((j - chunk_start_y) / spacing) * spacing;

            // Pravý dolní mřížkový bod (gx2, gy2) se ořízne o konec CHUNKU, ne o konec mapy!
            int gx2 = gx + spacing;
            int gy2 = gy + spacing;
            if (gx2 > border_x) gx2 = border_x;
            if (gy2 > border_y) gy2 = border_y;

            // Spočítáme reálnou vzdálenost mezi mřížkovými body (u okraje bude menší než spacing)
            int curr_spacing_x = gx2 - gx;
            int curr_spacing_y = gy2 - gy;

            // Poměr pozice (tx, ty) – ošetřeno proti dělení nulou, pokud i == gx
            float tx = (curr_spacing_x > 0) ? (float)(i - gx) / (float)curr_spacing_x : 0.0f;
            float ty = (curr_spacing_y > 0) ? (float)(j - gy) / (float)curr_spacing_y : 0.0f;

            // Všechny tyto body jsou garantovaně vygenerované v našem chunku
            float h00 = get_heightmap_pixel(gx,  gy);
            float h10 = get_heightmap_pixel(gx2, gy);
            float h01 = get_heightmap_pixel(gx,  gy2);
            float h11 = get_heightmap_pixel(gx2, gy2);

            // Bilineární interpolace výšky
            float h = h00 * (1.0f - tx) * (1.0f - ty)
                    + h10 * tx          * (1.0f - ty)
                    + h01 * (1.0f - tx) * ty
                    + h11 * tx          * ty;

            set_heightmap_height(i, j, h);
            set_heightmap_texture(i, j, get_heightmap_texture(gx, gy));
        }
    }
    log("interpolate heightmap done");
}
#define T_SIZE 500
float hill_stamp[T_SIZE][T_SIZE];
void create_hill_stamp() {
    // Vynulujeme razítko
    for(int i=0; i<T_SIZE; i++) 
        for(int j=0; j<T_SIZE; j++) hill_stamp[i][j] = 0.0f;

    float hill_y = 0.0f;
    int center = T_SIZE / 2;

    // Tvůj originální "perfect" algoritmus (vycentrovaný do razítka)
    for (int i=0; i<314; i++) {
        for (int j=0; j<(628/HILL_NEPRESTNOST + 2); j++) {
            int mx = (int)((sin((float)j/(100.0/HILL_NEPRESTNOST)))*(314-i)*HILL_SIZE + center);
            int my = (int)((cos((float)j/(100.0/HILL_NEPRESTNOST)))*(314-i)*HILL_SIZE + center);

            if (mx >= 0 && my >= 0 && mx < T_SIZE && my < T_SIZE) {
                hill_stamp[mx][my] = hill_y;
            }
        }
        hill_y += (sin((float)i/100.0)/(HILL_SIZE*10.0));
    }
}
void fast_hill(int x, int y) {
    int center = T_SIZE / 2;
    
    for (int i = 0; i < T_SIZE; i++) {
        for (int j = 0; j < T_SIZE; j++) {
            // Pokud na razítku v tomto bodě něco je
            if (hill_stamp[i][j] > 0.0f) {
                // Přepočítáme souřadnice z razítka na hlavní mapu
                int map_i = i - center + y;
                int map_j = j - center + x;

                // Kontrola hranic hlavní mapy
				add_heightmap_height(map_i,map_j,hill_stamp[i][j]);
            }
        }
    }
}
void create_checker_path(int startX, int startZ, float bumpHeight) {
    // Procházíme délku (20 metrů)
    for (int i = 0; i < 40; i++) {
        // Procházíme šířku (5 metrů)
        for (int j = 0; j < 5; j++) {
            int currentX = startZ + i;
            int currentZ = startX + j;

            // Logika šachovnice:
            // Pokud je součet (relativního) i a j sudý, zvedneme terén.
            if ((i + j) % 2 == 0) {
                add_heightmap_height(currentZ, currentX, bumpHeight);
                set_heightmap_texture(currentZ, currentX, 1); // nastavíme texturu silnice
            }
        }
    }
}
void smooth_heightmap() {
    // Potřebujeme pomocné pole, abychom nečetli už vyhlazené hodnoty během výpočtu
    // Pokud je tvoje mapa MAP_SIZE x MAP_SIZE:
    float* temp_map = new float[MAP_SIZE * MAP_SIZE];

    // 1. KROK: Spočítáme průměry a uložíme je do temp_map
    for (int z = 1; z < MAP_SIZE - 1; z++) {
        for (int x = 1; x < MAP_SIZE - 1; x++) {
            float sum = 0.0f;
            
            // Projdeme okolí 3x3 (střed + 8 sousedů)
            for (int sz = -1; sz <= 1; sz++) {
                for (int sx = -1; sx <= 1; sx++) {
                    sum += get_heightmap_pixel(x + sx, z + sz);
                }
            }
            
            // Průměr z 9 bodů
            temp_map[z * MAP_SIZE + x] = sum / 9.0f;
        }
    }

    // 2. KROK: Zapíšeme vyhlazené hodnoty zpět do tvojí mapy
    for (int z = 1; z < MAP_SIZE - 1; z++) {
        for (int x = 1; x < MAP_SIZE - 1; x++) {
            set_heightmap_height(x, z, temp_map[z * MAP_SIZE + x]);
        }
    }

    delete[] temp_map;
}
void gen_heightmap_redmap(){
    display_background();
    render_progress_bar(0.0f, "Generating map");
    run_essencials();	


    create_hill_stamp();
	for (i=0;i<MAP_SIZE;i++){
		for (j=0;j<MAP_SIZE;j++){
			set_heightmap_height(i,j,0.0f);
		}
	}
	float x;
	float y;
	int speeed=2;
	int brchv2;
	int brchv;

 

	for (int bla=0;bla<100;bla++){
		for (i=0;i<(int)((float)((MAP_SIZE)*(MAP_SIZE))/2000000.0f);i++){
			fast_hill(x,y);
			x=randint(0,MAP_SIZE);
			y=randint(0,MAP_SIZE);
		}
		printf("\r%d done", bla);
	}
    smooth_heightmap();
}
void gen_heightmap_testmap(){
    for (i=0;i<MAP_SIZE;i++){
		for (j=0;j<MAP_SIZE;j++){
			set_heightmap_height(i,j,0.0f);
		}
	}
    for (i=0;i<50;i++){
        for (j=0;j<50;j++){
            set_heightmap_texture(i+50,j+50,2);
        }
    }
    for (j=0;j<20;j++){
        for (i=0;i<10;i++){
            for (k=0;k<j;k++){
                set_heightmap_height(250+k+(j*j), 25+i,-1.0f);
            }
        }
    }
    for (j=0;j<20;j++){
        for (i=0;i<10;i++){
            for (k=0;k<2;k++){
                set_heightmap_height(50+j*4+k, 35+i,-1.0f);
            }
        }
    }
    for (j=0;j<20;j++){
        for (i=0;i<10;i++){
            for (k=0;k<2;k++){
                set_heightmap_height(230+i, 50+j*4+k,-1.0f);
            }
        }
    }
    for (i=0;i<200;i++){
        for (j=0;j<40;j++){
            set_heightmap_height(110+j,i+20,100.0f-(float)i/2.0f);
            set_heightmap_texture(109+j,i+20,3);
            set_heightmap_texture(110+j,i+20,3);
            set_heightmap_texture(111+j,i+20,3);
        }
    }

    for (i=0;i<40;i++){
        for (j=2;j<20;j++){
            set_heightmap_height(110+i,j,100.0f);
        }
    }
    for (i=0;i<5;i++){
        set_heightmap_height(100,i,10000.0f);
    }
    create_checker_path(60,150,0.05f);
    create_checker_path(70,150,0.1f);
    create_checker_path(80,150,0.2f);
    create_checker_path(90,150,0.4f);
    return;
}
float terrace(float h, float step, float start, float end)
{
    // start = kde schody začínají
    // end   = kde jsou na 100%

    float t = (h - start) / (end - start);
    t = clamp(t, 0.0f, 1.0f);

    // Smoothstep (měkký přechod)
    t = t * t * (3.0f - 2.0f * t);

    float terraced = floor(h / step) * step;

    // mix mezi normální výškou a "schodem"
    return h * (1.0f - t) + terraced * t;
}
float gen_mountains_pixel(int sample_i, int sample_j){
    int margin = 200;

            if (sample_j >= MAP_SIZE) sample_j = MAP_SIZE - 1;
            
            // --- Map end blending (stále kontroluje pozici vůči CELÉ MAPĚ) ---
            float weight = 1.0f;
            if (sample_i < margin) {
                weight *= (float)(sample_i + margin) / (float)(margin * 2);
            }
            if (sample_j < margin) {
                weight *= (float)(sample_j + margin) / (float)(margin * 2);
            }
            if (sample_i > MAP_SIZE - margin) {
                weight *= (float)(MAP_SIZE + margin - sample_i) / (float)(margin * 2);
            }
            if (sample_j > MAP_SIZE - margin) {
                weight *= (float)(MAP_SIZE + margin - sample_j) / (float)(margin * 2);
            }

            // Měřítko šumu (sample_i a sample_j jsou globální, takže hory budou dokonale navazovat)
            float x = (float)sample_i / 10.0f;
            float z = (float)sample_j / 10.0f;

            // Základní kopečky
            float b = lowNoise.GetNoise(x, z) + 1.0f;

            // Megalow
            float megalow = pow(lowNoise.GetNoise(x / 3.0f, z / 3.0f) + 1.0f, 1.5f);

            // Hlavní tvar hor
            float h = baseNoise.GetNoise(x, z); // -1..1
            h = pow((h + 1.0f) * 0.5f, 1.2f) * 1000.0f; // 0..1000
            h *= b;
            if (h > 600.0f) {
                h = h + (h * ((h - 600.0f) / 600.0f));
            }

            // Detaily
            float d = detailNoise.GetNoise(x * 2.0f, z * 2.0f); // -1..1
            h += d * h / 50.0f; // +-10m detaily

            // Velmi jemné detaily navrchu
            float vd = veryDetailNoise.GetNoise(x * 20.0f, z * 20.0f); // -1..1
            vd *= clamp(h - 400.0f, 0.0f, 1000000.0f) / 100.0f;
            h += vd;

            // Terasování
            if (h > 600.0f) {
                h = terrace(h, 20.0f + detailNoise.GetNoise(x, z) * 10.0f, 600.0f, 1200.0f);
            }

            // Zápis do heightmapy (aplikujeme váhu okrajů mapy)
            add_heightmap_height(sample_i, sample_j, (h * weight) * 0.7f);

            // --- Textura podle výšky a strmosti ---
            float th = h + ((baseNoise.GetNoise(x, z) + 1.0f) * 50.0f);
            int tex = 0; // default tráva
            if (th > 750.0f)       tex = 2; // kámen nahoře
            else if (th > 600.0f)  tex = 5; // hlína
            else if (th < 500.0f)  tex = 0; // nízko tráva
            else {
                tex = (d > 0.5f) ? 2 : 0;  
            }

            set_heightmap_texture(sample_i, sample_j, tex);
            return (h * weight) * 0.7f;

}
void gen_heightmap_mountains_chunk(int cx, int cy) {
    log("    generating mountain chunk at "+std::to_string(cx)+", "+std::to_string(cy));
    int spacing = chunks[cx][cy]->pres;
    if (spacing <= 0) spacing = 1;

    int chunk_start_x = cx * CHUNK_SIZE;
    int chunk_start_y = cy * CHUNK_SIZE;
    int chunk_end_x   = chunk_start_x + CHUNK_SIZE;
    int chunk_end_y   = chunk_start_y + CHUNK_SIZE;
    if (mapgen){
        for (int i = chunk_start_x; i < chunk_end_x; i += mapgen_spacing) {
            for (int j = chunk_start_y; j < chunk_end_y; j += mapgen_spacing) {
                gen_mountains_pixel(i, j);
            }
        }
        return;
    }
    // Indexy pro úplný konec (border) chunku
    int border_x = chunk_end_x - 1;
    int border_y = chunk_end_y - 1;

    // Inicializace šumů (tvoje původní)

    // 1. TVŮJ PŮVODNÍ CYKLUS (Zůstává na 100 % stejný)
    for (int i = chunk_start_x; i < chunk_end_x; i += spacing) {
        int sample_i = i;
        if (sample_i >= MAP_SIZE) sample_i = MAP_SIZE - 1;
        for (int j = chunk_start_y; j < chunk_end_y; j += spacing) {
            int sample_j = j;
            if (sample_j >= MAP_SIZE) sample_j = MAP_SIZE - 1;
            
            gen_mountains_pixel(sample_i, sample_j);
        }
    }

    // 2. DOPLNĚNÍ BORDERŮ (Pokud spacing netrefil přesný okraj chunku, dogenerujeme ho)
    
    // Pokud poslední krok v ose Y nebyl border, dogenerujeme pravou hranu (svislý pruh na konci X)
    if ((border_y - chunk_start_y) % spacing != 0) {
        for (int i = chunk_start_x; i < chunk_end_x; i += spacing) {
            int sample_i = i;
            if (sample_i >= MAP_SIZE) sample_i = MAP_SIZE - 1;
            gen_mountains_pixel(sample_i, border_y);
        }
    }

    // Pokud poslední krok v ose X nebyl border, dogenerujeme spodní hranu (vodorovný pruh na konci Y)
    if ((border_x - chunk_start_x) % spacing != 0) {
        for (int j = chunk_start_y; j < chunk_end_y; j += spacing) {
            int sample_j = j;
            if (sample_j >= MAP_SIZE) sample_j = MAP_SIZE - 1;
            gen_mountains_pixel(border_x, sample_j);
        }
    }

    // Úplný roh (pravý dolní), pokud nebyl pokryt ani jedním směrem
    if ((border_x - chunk_start_x) % spacing != 0 && (border_y - chunk_start_y) % spacing != 0) {
        gen_mountains_pixel(border_x, border_y);
    }

    // Na závěr zavoláme upravenou interpolaci pro tento chunk
    interpolate_chunk_heightmap(cx, cy, spacing);
}
void init_gen_mountains(){
    baseNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    baseNoise.SetFractalOctaves(6);
    baseNoise.SetFractalLacunarity(2.0f);
    baseNoise.SetFractalGain(0.5f);
    baseNoise.SetFrequency(0.01f);
    baseNoise.SetSeed(MAP_SEED);
    lowNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    lowNoise.SetFrequency(0.002f);
    lowNoise.SetSeed(MAP_SEED);
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    detailNoise.SetFractalOctaves(4);
    detailNoise.SetFrequency(0.05f);
    detailNoise.SetSeed(MAP_SEED);
    veryDetailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    veryDetailNoise.SetFractalOctaves(4);
    veryDetailNoise.SetFrequency(0.05f);
    veryDetailNoise.SetSeed(MAP_SEED);
}



void gen_chunk_heightmap(int cx, int cy){
    // printf("generating chunk %d %d\n", cx, cy);
    chunks[cx][cy]->generated=true;

    if (selected_map==1){
    } else if (selected_map==0){
    } else if (selected_map==2){
        gen_heightmap_mountains_chunk(cx,cy);
    }
    // printf("chunk generation done\n");
    return;
}
void init_gen_heightmap(){
    if (selected_map==1){
        gen_heightmap_redmap();

    } else if (selected_map==0){

        gen_heightmap_testmap();
        x_pos=40.0f;
        y_pos=1.0f;
        z_pos=35.0f;
    } else if (selected_map==2){
        init_gen_mountains();
    }
}
std::vector<FlatCandidate> find_flat_locations(
    int map_size,              // Rozměr mapy v pixelech (např. 2048)
    float map_scale_meters,    // Kolik metrů měří 1 pixel (např. 1.0f pro 1px = 1m)
    float density_per_km2,     // Požadovaný počet vzorků na km^2 (např. 20.0f)
    float max_allowed_variance,// Horní limit pro varianci (co už je moc prudké)
    int search_radius          // Okruh kontroly rovnosti kolem bodu (např. 4)
) {
    std::vector<FlatCandidate> candidates;

    // --- 1. Výpočet kroku (check_step) podle hustoty na km^2 ---
    float map_area_km2 = (map_size * map_scale_meters / 1000.0f) * (map_size * map_scale_meters / 1000.0f);
    int total_samples_needed = (int)(density_per_km2 * map_area_km2);
    
    // Zabraňme dělení nulou nebo nesmyslně malým číslům
    if (total_samples_needed < 1) total_samples_needed = 1;

    // Určíme mřížkový krok (step) v pixelech
    int samples_per_side = (int)std::sqrt(total_samples_needed);
    int check_step = map_size / std::max(1, samples_per_side);
    check_step = std::max(1, check_step); // Krok musí být alespoň 1px

    printf("[FIND FLAT] Skenuji mapu (%dx%d px)... Krok: %d px (~%d vzorků celkem)\n", 
           map_size, map_size, check_step, total_samples_needed);

    // --- 2. Skenování mapy ---
    for (int y = search_radius; y < map_size - search_radius; y += check_step) {
        for (int x = search_radius; x < map_size - search_radius; x += check_step) {
            
            // JEDNOPRŮCHODOVÝ výpočet průměru a variance:
            // Var(X) = E[X^2] - (E[X])^2
            float sum = 0.0f;
            float sum_sq = 0.0f;
            int count = 0;

            for (int dy = -search_radius; dy <= search_radius; ++dy) {
                for (int dx = -search_radius; dx <= search_radius; ++dx) {
                    // DŮLEŽITÉ: Použijeme tvou lehkou funkci/pointer na heightmapu
                    float h = mapgen_get_heightmap_height(x + dx, y + dy); 
                    sum += h;
                    sum_sq += h * h;
                    count++;
                }
            }

            float mean = sum / count;
            float variance = (sum_sq / count) - (mean * mean);

            // Pokud je místo pod limitem náročnosti terénu, uložíme ho
            if (variance < max_allowed_variance) {
                candidates.push_back({x, y, mean, variance});
            }
        }
    }

    // --- 3. Seřadíme kandidáty od NEJROVNĚJŠÍHO po nejhorší ---
    std::sort(candidates.begin(), candidates.end(), [](const FlatCandidate& a, const FlatCandidate& b) {
        return a.variance < b.variance;
    });

    printf("[FIND FLAT] Nalezeno %zu vhodných rovných kandidátů.\n", candidates.size());
    return candidates;
}