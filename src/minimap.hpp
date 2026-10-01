#ifndef INCLUDES
#define INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <map>
#include "sdl_includes.h"
#include "opengl_include.h"
#include <algorithm>
#include "../libs/fastnoiselite.h"
#include "constants.h"
#include "types.hpp"
#include "global_vars.hpp"
#include "general_functions.hpp"
#include "gui.hpp"
#include "renderer.hpp"
#include "map.hpp"
#include "heightmap.hpp"
#include "car.hpp"
#include "softsoft.hpp"
#include "fysics.hpp"
#include "sound.hpp"
#endif
void init_minimap(int size){
    printf("initing minimap at size %d\n", size);
    minimap_surf=SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA8888);
}
void minimap_draw_pixel(int x, int y, int r, int g, int b){
    // Zamknutí paměti před zápisem

    Uint32* pixels = (Uint32*)minimap_surf->pixels;
    int stride = minimap_surf->pitch / 4; // Počet pixelů na jeden řádek
    Uint32 color = SDL_MapRGBA(minimap_surf->format, r, g, b, 255); // R, G, B, A

    // Zápis jednoho pixelu na souřadnice [x, y]
    pixels[y * stride + x] = color;

}
void minimap_draw_circle(int center_x, int center_y, int radius, int r, int g, int b, bool fill = false) {
    if (radius <= 0) return;

    if (fill) {
        // VYPLNĚNÉ KOLEČKO: Čtvercový cyklus + Pythagorova věta BEZ odmocniny (dx^2 + dy^2 <= r^2)
        int r2 = radius * radius;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy <= r2) {
                    minimap_draw_pixel(center_x + dx, center_y + dy, r, g, b);
                }
            }
        }
    } else {
        // OBRYS KOLEČKA: Midpoint/Bresenhamův algoritmus (extrémně rychlý, využívá zrcadlení 8 výsečí)
        int x = radius;
        int y = 0;
        int err = 0;

        while (x >= y) {
            minimap_draw_pixel(center_x + x, center_y + y, r, g, b);
            minimap_draw_pixel(center_x + y, center_y + x, r, g, b);
            minimap_draw_pixel(center_x - y, center_y + x, r, g, b);
            minimap_draw_pixel(center_x - x, center_y + y, r, g, b);
            minimap_draw_pixel(center_x - x, center_y - y, r, g, b);
            minimap_draw_pixel(center_x - y, center_y - x, r, g, b);
            minimap_draw_pixel(center_x + y, center_y - x, r, g, b);
            minimap_draw_pixel(center_x + x, center_y - y, r, g, b);

            if (err <= 0) {
                y += 1;
                err += 2 * y + 1;
            }
            if (err > 0) {
                x -= 1;
                err -= 2 * x + 1;
            }
        }
    }
}

void minimap_draw_line(int x0, int y0, int x1, int y1, int r, int g, int b) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        minimap_draw_pixel(x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) { 
            err += dy; 
            x0 += sx; 
        }
        if (e2 <= dx) { 
            err += dx; 
            y0 += sy; 
        }
    }
}
#include <cmath>

void draw_player_pointer(float x_pos, float z_pos, float x_rot, float scale, int player_r) {
    // 1. Přepočet 3D pozice [X, Z] na pixely minimapy
    int px = (int)(x_pos * scale);
    int py = (int)(z_pos * scale);

    // 2. Vykreslení těla hráče (červené plné kolečko)
    minimap_draw_circle(px, py, player_r, 255, 0, 0, true);

    // 3. Převod stupňů na radiány
    const float DEG2RAD = 3.14159265f / 180.0f;
    float rad = x_rot * DEG2RAD;

    // 4. Délka směrové ručičky (např. 2.5x větší než poloměr)
    float pointer_len = player_r * 2.5f;

    // 5. Výpočet koncového bodu čáry
    int end_x = px + (int)(cosf(rad) * pointer_len);
    int end_y = py + (int)(sinf(rad) * pointer_len);

    // 6. Nakreslení žlutého ukazatele směru
    minimap_draw_line(px, py, end_x, end_y, 255, 0, 0);
}
void draw_minimap_surf(){
    int minimap_size=minimap_surf->w;
    float scale=(float)(minimap_size)/(float)MAP_SIZE;
    if (SDL_MUSTLOCK(minimap_surf)) SDL_LockSurface(minimap_surf);
    float minimap_heightmap[minimap_size][minimap_size];
    float min=10000000.0f;
    float max=0.0f;
    //get the heights into the array
    for (int x=0;x<minimap_size;x++){
        for (int y=0;y<minimap_size;y++){
            float h=mapgen_get_heightmap_height((float)y/(float)scale, (float)x/(float)scale);
            minimap_heightmap[y][x]=h;
            if (h<min){min=h;}
            if (h>max){max=h;}
        }
    }
    //normalize the array
    float mult=1.0f/(max-min);
    for (int x=0;x<minimap_size;x++){
        for (int y=0;y<minimap_size;y++){
            float c=(minimap_heightmap[y][x]-=min)*=mult;
            c*=255.0f;
            minimap_draw_pixel(y, x, c,c,c);
        }
    }
    int player_rad=3;
    draw_player_pointer(x_pos, z_pos, x_rot, scale, player_rad);
    for (auto&fjdk:chaloupky){
        minimap_draw_circle(fjdk.y*scale, fjdk.x*scale, 3, 255,255,0,true);
    }
    // 2. Varianta: Pokud roadparts odkazuje na indexy v roadpoints
    for (int i = 0; i < roadparts_len; i++) {
        const auto& pt1 = roadpoints[roadparts[i].p1]; // nebo .p1 / .idx1
        const auto& pt2 = roadpoints[roadparts[i].p2];   // nebo .p2 / .idx2

        int x0 = (int)(pt1.y * scale);
        int y0 = (int)(pt1.x * scale); // nebo .z
        int x1 = (int)(pt2.y * scale);
        int y1 = (int)(pt2.x * scale);

        minimap_draw_line(x0, y0, x1, y1, 255, 0, 0);
    }
    if (debug!=0){
        for (auto&fjdk:major_road_points){
            minimap_draw_circle(fjdk.y*scale, fjdk.x*scale, 3, 50,50,255,true);
        }
        // Projde všechny úseky, 'start' je první Vec2, 'end' je druhý Vec2
        for (const auto& [type, endpoints] : major_road_segments) {
            const auto& [start, end] = endpoints;

            // Přepočet souřadnic na pixely minimapy
            int x0 = (int)(start.x * scale);
            int y0 = (int)(start.y * scale);
            int x1 = (int)(end.x * scale);
            int y1 = (int)(end.y * scale);

            // Výběr barvy podle typu silnice
            int r = 255, g = 255, b = 0; // 0: Žlutá (výchozí)

            if (type == 1) {
                r = 255; g = 100; b = 0;   // 1: Oranžová
            } else if (type == 2) {
                r = 100; g = 255; b = 0;   // 2: Žlutozelená
            }

            minimap_draw_line(x0, y0, x1, y1, r, g, b);
        }
    }





    if (SDL_MUSTLOCK(minimap_surf)) SDL_UnlockSurface(minimap_surf);
}
void draw_minimap_to_hud(int x, int y){
    SDL_Rect dst_rect = { x, y, 0, 0 }; // U standardního blitu stačí určit x, y (w a h se ignorují)

    // SDL_BlitSurface(zdroj, zdrojový_výřez, cíl, cílová_pozice)
    SDL_BlitSurface(minimap_surf, NULL, hud_surf, &dst_rect);
}