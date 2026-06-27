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


//made to interpolate heightmap values, so it can be generated at a lower presicion to save time and this will take care of the rest
// void interpolate_chunk_heightmap(int cx, int cy, int spacing) {
//     printf("interpolating heightmap with spacing %d...\n", spacing);

//     int num_threads = 3;
//     std::vector<std::thread> threads;
//     std::vector<int> progress_per_thread(num_threads, 0); // 0..100 per thread

//     // each thread handles a horizontal strip of the map
//     auto worker = [&](int thread_id) {
//         int strip_start = (MAP_SIZE / num_threads) * thread_id;
//         int strip_end   = (thread_id == num_threads - 1) ? MAP_SIZE : strip_start + (MAP_SIZE / num_threads);

//         for (int i = strip_start; i < strip_end; i++) {
//             for (int j = 0; j < MAP_SIZE; j++) {
//                 int gx  = (i / spacing) * spacing;
//                 int gy  = (j / spacing) * spacing;
//                 int gx2 = gx + spacing;
//                 int gy2 = gy + spacing;

//                 if (gx2 > MAP_SIZE) gx2 = MAP_SIZE;
//                 if (gy2 > MAP_SIZE) gy2 = MAP_SIZE;

//                 float tx = (float)(i - gx) / (float)spacing;
//                 float ty = (float)(j - gy) / (float)spacing;

//                 float h00 = get_heightmap_pixel(gx,  gy);
//                 float h10 = get_heightmap_pixel(gx2, gy);
//                 float h01 = get_heightmap_pixel(gx,  gy2);
//                 float h11 = get_heightmap_pixel(gx2, gy2);

//                 float h = h00 * (1-tx) * (1-ty)
//                         + h10 * tx     * (1-ty)
//                         + h01 * (1-tx) * ty
//                         + h11 * tx     * ty;

//                 set_heightmap_height(i, j, h);
//                 set_heightmap_texture(i, j, get_heightmap_texture(gx, gy));
//             }
//             // update this thread's progress
//             progress_per_thread[thread_id] = (int)(100.0f * (float)(i - strip_start) / (float)(strip_end - strip_start));
//         }
//         progress_per_thread[thread_id] = 100;
//     };

//     // launch threads 1..3 (thread 0 is main thread)
//     for (int t = 1; t < num_threads; t++) {
//         threads.push_back(std::thread(worker, t));
//     }

//     // run thread 0 work on main thread + show progress bar
//     int strip_start = 0;
//     int strip_end   = MAP_SIZE / num_threads;
//     float last_updated_progress = 0.0f;

//     for (int i = strip_start; i < strip_end; i++) {
//         for (int j = 0; j < MAP_SIZE; j++) {
//             int gx  = (i / spacing) * spacing;
//             int gy  = (j / spacing) * spacing;
//             int gx2 = gx + spacing;
//             int gy2 = gy + spacing;

//             if (gx2 > MAP_SIZE) gx2 = MAP_SIZE;
//             if (gy2 > MAP_SIZE) gy2 = MAP_SIZE;

//             float tx = (float)(i - gx) / (float)spacing;
//             float ty = (float)(j - gy) / (float)spacing;

//             float h00 = get_heightmap_pixel(gx,  gy);
//             float h10 = get_heightmap_pixel(gx2, gy);
//             float h01 = get_heightmap_pixel(gx,  gy2);
//             float h11 = get_heightmap_pixel(gx2, gy2);

//             float h = h00 * (1-tx) * (1-ty)
//                     + h10 * tx     * (1-ty)
//                     + h01 * (1-tx) * ty
//                     + h11 * tx     * ty;

//             set_heightmap_height(i, j, h);
//             set_heightmap_texture(i, j, get_heightmap_texture(gx, gy));
//         }
//         progress_per_thread[0] = (int)(100.0f * (float)(i - strip_start) / (float)(strip_end - strip_start));

//         // progress bar based on average of all threads
//         float avg = 0.0f;
//         for (int t = 0; t < num_threads; t++) avg += progress_per_thread[t];
//         avg /= (float)(num_threads * 100);

//         if (avg - last_updated_progress >= 0.02f || i == 0) {
//             display_background();
//             render_progress_bar(avg, "Interpolating heightmap");
//             run_essencials();
//             last_updated_progress = avg;
//         }
//     }
//     progress_per_thread[0] = 100;

//     // wait for all threads to finish
//     for (auto& th : threads) th.join();

//     printf("interpolation done\n");
// }
/*
void interpolate_chunk_heightmap(int cx, int cy, int spacing) {
    log("interpolating heightmap");
    // Spočítáme si počáteční a koncové globální souřadnice pro tento chunk
    int chunk_start_x = cx * CHUNK_SIZE;
    int chunk_start_y = cy * CHUNK_SIZE;
    int chunk_end_x   = chunk_start_x + CHUNK_SIZE;
    int chunk_end_y   = chunk_start_y + CHUNK_SIZE;

    int num_threads = 3;
    std::vector<std::thread> threads;

    // Lambda worker, který zpracovává horizontální pruh v rámci CHUNKU
    auto worker = [&](int thread_id) {
        int strip_start = chunk_start_x + (CHUNK_SIZE / num_threads) * thread_id;
        int strip_end   = (thread_id == num_threads - 1) ? chunk_end_x : strip_start + (CHUNK_SIZE / num_threads);

        for (int i = strip_start; i < strip_end; i++) {
            for (int j = chunk_start_y; j < chunk_end_y; j++) {
                // Výpočet mřížky pro interpolaci (zůstává globální)
                int gx  = (i / spacing) * spacing;
                int gy  = (j / spacing) * spacing;
                int gx2 = gx + spacing;
                int gy2 = gy + spacing;

                // Tx a Ty vyjadřují pozici uvnitř dané spacing buňky
                float tx = (float)(i - gx) / (float)spacing;
                float ty = (float)(j - gy) / (float)spacing;

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
    };

    // Spustíme vlákna 1 a 2 na pozadí
    for (int t = 1; t < num_threads; t++) {
        threads.push_back(std::thread(worker, t));
    }

    // Vlákno 0 (hlavní vlákno) odpracuje svůj díl práce hned tady
    worker(0);

    // Počkáme, až ostatní dvě vlákna dokončí práci na tomto chunku
    for (auto& th : threads) {
        th.join();
    }
    log("interpolate heightmap done");
}
*/
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

// void gen_heightmap_mountains(int cx, int cy){
//     int margin=200;
//     FastNoiseLite baseNoise;
//     baseNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     baseNoise.SetFractalOctaves(6);
//     baseNoise.SetFractalLacunarity(2.0f);
//     baseNoise.SetFractalGain(0.5f);
//     baseNoise.SetFrequency(0.01f); // velké kopce
//     FastNoiseLite lowNoise;
//     lowNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     lowNoise.SetFrequency(0.002f); // velké kopce
//     FastNoiseLite detailNoise;
//     detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     detailNoise.SetFractalOctaves(4);
//     detailNoise.SetFrequency(0.05f); // menší detaily

//     FastNoiseLite veryDetailNoise;
//     veryDetailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     veryDetailNoise.SetFractalOctaves(4);
//     veryDetailNoise.SetFrequency(0.05f); // menší detaily

//     float last_updated_progress=0.0f;
//     int presicion=10;
//     for (int i = -margin; i < MAP_SIZE+margin; i+=presicion) {
//         { // progress bar
//             // 1. Správný výpočet 0.0 až 1.0 (přetypuj aspoň jedno číslo na float)
//             float progress = (float)(i+margin) / (float)(MAP_SIZE+margin - 1);

//             // 2. Kontrola, jestli uplynulo aspoň 10 % (0.1f)
//             if (progress - last_updated_progress >= 0.02f || i == 0) {
//                 display_background();
//                 render_progress_bar(progress, "Generating heightmap");
//                 run_essencials();
                
//                 // 3. DŮLEŽITÉ: Musíš si zapamatovat, kdy jsi naposledy kreslil!
//                 last_updated_progress = progress;
//             }
//         }
//         for (int j = -margin; j < MAP_SIZE+margin; j+=presicion) {
//             // map end blending
//             float weight=1.0f;
//             if (i < margin) {
//                 weight *= (float)(i + margin) / (float)(margin * 2);
//             }
//             if (j < margin) {
//                 weight *= (float)(j + margin) / (float)(margin * 2);
//             }
//             if (i > MAP_SIZE - margin) {
//                 weight *= (float)(MAP_SIZE + margin - i) / (float)(margin * 2);
//             }
//             if (j > MAP_SIZE - margin) {
//                 weight *= (float)(MAP_SIZE + margin - j) / (float)(margin * 2);
//             }



//             float x = (float)i / 10.0f; // větší měřítko
//             float z = (float)j / 10.0f;

//             // základní kopečky
//             float b = lowNoise.GetNoise(x,z)+1.0f;

//             //megalow
//             float megalow = pow(lowNoise.GetNoise(x/3.0f,z/3.0f)+1.0f, 1.5f);
            

//             float h = baseNoise.GetNoise(x, z); // -1..1
//             h = pow((h + 1.0f) * 0.5f, 1.2f) * 1000.0f; // 0..1000, vyhlazeno exponenciálně
//             h*=b;
//             if (h>600.0f){
//                 h=h+(h*((h-600.0f)/600.0f));
//             }


//             float d = detailNoise.GetNoise(x * 2.0f, z * 2.0f); // -1..1

//             h += d * h/50.0f; // +-10m detaily

//             float vd = veryDetailNoise.GetNoise(x * 20.0f, z * 20.0f); // -1..1
//             vd*=clamp(h-400.0f,0.0f,1000000.0f)/100.0f;
//             h += vd;
//             // vyhlazení strmých spádů (bez 20m stěn)
//             // if (i > 0 && j > 0) {
//             //     float prevH = get_heightmap_height(i-1, j);
//             //     float prevH2 = get_heightmap_height(i, j-1);
//             //     float diff = (h - prevH + h - prevH2) * 0.5f;
//             //     if (diff > 5.0f) h = prevH + 5.0f;  // max 5m krok
//             //     if (diff < -5.0f) h = prevH - 5.0f;
//             // }

//             if (h>600.0f){
//                 h=terrace(h, 20.0f+detailNoise.GetNoise(x, z)*10.0f, 600.0f, 1200.0f);
//             }

//             add_heightmap_height(i, j, (h*weight)*0.7);

//             // --- textura podle výšky a strmosti ---
//             float th=h+((baseNoise.GetNoise(x, z)+1.0f)*50.0f);
//             int tex = 0; // default tráva
//             if (th > 750.0f) tex = 2;            // kámen nahoře
//             else if (th > 600.0f) tex = 5;       // hlína
//             else if (th < 500.0f) tex = 0;       // nízko tráva
//             else {
//                 // jemný mix podle detailu
//                 tex = (d > 0.5f) ? 2 : 0;  
//             }
//             set_heightmap_texture(i, j, tex);
//         }
//     }
//     interpolate_chunk_heightmap(cx, cy, presicion);
//     printf("heightmap generation mountains done\n");
// }
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
    baseNoise.SetFractalOctaves(6); baseNoise.SetFractalLacunarity(2.0f); baseNoise.SetFractalGain(0.5f); baseNoise.SetFrequency(0.01f);
    lowNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin); lowNoise.SetFrequency(0.002f);
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin); detailNoise.SetFractalOctaves(4); detailNoise.SetFrequency(0.05f);
    veryDetailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin); veryDetailNoise.SetFractalOctaves(4); veryDetailNoise.SetFrequency(0.05f);

}

#include <vector>
#include <cmath>
#include <algorithm>



// Pomocná funkce pro výpočet barycentrických souřadnic trojúhelníku
// Slouží k přesné interpolaci výšky a textury uvnitř trojúhelníku


// void gen_heightmap_roads6767() {
//     printf("\ngenerating heightmap roads (RASTERIZED + BEVEL JOINTS + OVERLAP)... %d\n", roadparts_len);
    
//     std::vector<float> height_accum(MAP_SIZE * MAP_SIZE, 0.0f);
//     std::vector<float> weight_accum(MAP_SIZE * MAP_SIZE, 0.0f);
//     std::vector<float> texture_accum(MAP_SIZE * MAP_SIZE, 0.0f);
    
//     std::vector<float> original_heights(MAP_SIZE * MAP_SIZE);
//     for(int i=0; i < MAP_SIZE * MAP_SIZE; i++) {
//         int x = i % MAP_SIZE;
//         int z = i / MAP_SIZE;
//         original_heights[i] = get_heightmap_height(x, z);
//         height_accum[i] = original_heights[i];
//     }

//     float last_updated_progress = 0.0f;

//     for (int i = 0; i < roadparts_len; i++) {
//         float progress = (float)i / (float)(roadparts_len - 1);
//         if (progress - last_updated_progress >= 0.1f || i == 0) {
//             display_background();
//             render_progress_bar(progress, "Rasterizing gapless road geometry");
//             run_essencials();
//             last_updated_progress = progress;
//         }

//         roadpoint& p1 = roadpoints[roadparts[i].p1];
//         roadpoint& p2 = roadpoints[roadparts[i].p2];
        
//         float x1 = p1.y; float z1 = p1.x; 
//         float x2 = p2.y; float z2 = p2.x;
//         float h1 = p1.h; float h2 = p2.h;
        
//         float road_w = roadparts[i].width;
//         float blend_w = 8.0f; 
        
//         float dx = x2 - x1;
//         float dz = z2 - z1;
//         float len = sqrtf(dx * dx + dz * dz);
//         if (len < 0.1f) continue;
        
//         float ux = dx / len;
//         float uz = dz / len;
//         float nx = -uz;
//         float nz = ux;

//         // --- PODÉLNÝ PŘESAH ---
//         // Natáhneme geometrické souřadnice o půl metru/pixelu na každou stranu
//         float length_overlap = 0.5f;
//         float ox1 = x1 - ux * length_overlap;
//         float oz1 = z1 - uz * length_overlap;
//         float ox2 = x2 + ux * length_overlap;
//         float oz2 = z2 + uz * length_overlap;
        
//         // Vygenerování 4 bodů na začátku (používáme o kousek posunuté ox1, oz1)
//         RoadVertex p1_left_blend  = { ox1 + nx * (road_w * 0.5f + blend_w), oz1 + nz * (road_w * 0.5f + blend_w), h1, 0.0f };
//         RoadVertex p1_left_road   = { ox1 + nx * (road_w * 0.5f),           oz1 + nz * (road_w * 0.5f),           h1, 1.0f };
//         RoadVertex p1_right_road  = { ox1 - nx * (road_w * 0.5f),           oz1 - nz * (road_w * 0.5f),           h1, 1.0f };
//         RoadVertex p1_right_blend = { ox1 - nx * (road_w * 0.5f + blend_w), oz1 - nz * (road_w * 0.5f + blend_w), h1, 0.0f };

//         // Vygenerování 4 bodů na konci (používáme posunuté ox2, oz2)
//         RoadVertex p2_left_blend  = { ox2 + nx * (road_w * 0.5f + blend_w), oz2 + nz * (road_w * 0.5f + blend_w), h2, 0.0f };
//         RoadVertex p2_left_road   = { ox2 + nx * (road_w * 0.5f),           oz2 + nz * (road_w * 0.5f),           h2, 1.0f };
//         RoadVertex p2_right_road  = { ox2 - nx * (road_w * 0.5f),           oz2 - nz * (road_w * 0.5f),           h2, 1.0f };
//         RoadVertex p2_right_blend = { ox2 - nx * (road_w * 0.5f + blend_w), oz2 - nz * (road_w * 0.5f + blend_w), h2, 0.0f };

//         // Vykreslení hlavního tělesa
//         rasterize_triangle(p1_left_road, p2_left_road, p1_right_road, height_accum, weight_accum, texture_accum, original_heights);
//         rasterize_triangle(p1_right_road, p2_left_road, p2_right_road, height_accum, weight_accum, texture_accum, original_heights);
//         rasterize_triangle(p1_left_blend, p2_left_blend, p1_left_road, height_accum, weight_accum, texture_accum, original_heights);
//         rasterize_triangle(p1_left_road, p2_left_blend, p2_left_road, height_accum, weight_accum, texture_accum, original_heights);
//         rasterize_triangle(p1_right_road, p2_right_road, p1_right_blend, height_accum, weight_accum, texture_accum, original_heights);
//         rasterize_triangle(p1_right_blend, p2_right_road, p2_right_blend, height_accum, weight_accum, texture_accum, original_heights);

//         // --- DOPLŇOVÁNÍ MEZER V ZATÁČKÁCH (S PŘESYHY) ---
//         int next_idx = -1;
//         if (i + 1 < roadparts_len && roadparts[i + 1].p1 == roadparts[i].p2) {
//             next_idx = i + 1;
//         } else {
//             for (int k = 0; k < roadparts_len; k++) {
//                 if (roadparts[k].p1 == roadparts[i].p2) {
//                     next_idx = k;
//                     break;
//                 }
//             }
//         }

//         if (next_idx != -1) {
//             roadpoint& p3 = roadpoints[roadparts[next_idx].p2];
//             float x3 = p3.y; float z3 = p3.x;
            
//             float dx2 = x3 - x2;
//             float dz2 = z3 - z2;
//             float len2 = sqrtf(dx2 * dx2 + dz2 * dz2);
            
//             if (len2 > 0.1f) {
//                 float ux2 = dx2 / len2;
//                 float uz2 = dz2 / len2;
//                 float nx2 = -uz2;
//                 float nz2 = ux2;

//                 // U výplňových trojúhelníků bereme výchozí p2 základ, ale o kousek je zvětšíme
//                 // použitím mírně upravených normál pro dokonalé prolnutí hran
//                 RoadVertex next_start_left_blend  = { x2 + nx2 * (road_w * 0.5f + blend_w), z2 + nz2 * (road_w * 0.5f + blend_w), h2, 0.0f };
//                 RoadVertex next_start_left_road   = { x2 + nx2 * (road_w * 0.5f),           z2 + nz2 * (road_w * 0.5f),           h2, 1.0f };
//                 RoadVertex next_start_right_road  = { x2 - nx2 * (road_w * 0.5f),           z2 - nz2 * (road_w * 0.5f),           h2, 1.0f };
//                 RoadVertex next_start_right_blend = { x2 - nx2 * (road_w * 0.5f + blend_w), z2 - nz2 * (road_w * 0.5f + blend_w), h2, 0.0f };

//                 float cross = ux * uz2 - uz * ux2;
//                 RoadVertex v_center = { x2, z2, h2, 1.0f };

//                 if (cross > 0.0001f) {
//                     rasterize_triangle(v_center, p2_right_road, next_start_right_road, height_accum, weight_accum, texture_accum, original_heights);
//                     rasterize_triangle(p2_right_road, p2_right_blend, next_start_right_blend, height_accum, weight_accum, texture_accum, original_heights);
//                     rasterize_triangle(p2_right_road, next_start_right_blend, next_start_right_road, height_accum, weight_accum, texture_accum, original_heights);
//                 } 
//                 else if (cross < -0.0001f) {
//                     rasterize_triangle(v_center, next_start_left_road, p2_left_road, height_accum, weight_accum, texture_accum, original_heights);
//                     rasterize_triangle(p2_left_road, next_start_left_blend, p2_left_blend, height_accum, weight_accum, texture_accum, original_heights);
//                     rasterize_triangle(p2_left_road, next_start_left_road, next_start_left_blend, height_accum, weight_accum, texture_accum, original_heights);
//                 }
//             }
//         }
//     }
    
//     // Finální zápis do heightmapy
//     for (int z = 0; z < MAP_SIZE; z++) {
//         for (int x = 0; x < MAP_SIZE; x++) {
//             int idx = z * MAP_SIZE + x;
//             if (weight_accum[idx] > 0.001f) { 
//                 set_heightmap_height(x, z, height_accum[idx]);
//                 set_chunk_permanency(x,z,true);
//                 if (texture_accum[idx] > 0.85f) { 
//                     set_heightmap_texture(x, z, 1); 
//                 } else {
//                     set_heightmap_texture(x, z, 6); 
//                 }
//             }
//         }
//     }
//     printf("Perfect, gapless roads generated!\n");
// }
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