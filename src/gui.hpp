#ifndef GUI_H
#define GUI_H
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
SDL_Surface* TTF_RenderUTF8_Multiline(TTF_Font* font,const char* text,SDL_Color color) {
    // 1) spočítat počet řádků
    int lines = 1;
    for (const char* c = text; *c; c++)
        if (*c == '\n') lines++;

    // 2) rozdělit text na řádky
    char** rows = (char**)malloc(sizeof(char*) * lines);
    int row = 0;
    const char* start = text;

    for (const char* c = text; ; c++) {
        if (*c == '\n' || *c == '\0') {
            int len = c - start;
            rows[row] = (char*)malloc(len + 1);
            memcpy(rows[row], start, len);
            rows[row][len] = 0;
            row++;
            start = c + 1;
        }
        if (*c == '\0') break;
    }

    // 3) vyrenderovat každý řádek
    SDL_Surface** surfaces = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * lines);

    int max_w = 0;
    int total_h = 0;

    for (int i = 0; i < lines; i++) {
        surfaces[i] = TTF_RenderUTF8_Blended(font, rows[i], color);
        if (!surfaces[i]) return NULL;

        if (surfaces[i]->w > max_w) max_w = surfaces[i]->w;
        total_h += surfaces[i]->h;
    }

    // 4) cílový surface
    SDL_Surface* result =
        SDL_CreateRGBSurfaceWithFormat(
            0, max_w, total_h, 32, SDL_PIXELFORMAT_RGBA32
        );

    // 5) blit řádků pod sebe
    int y = 0;
    for (int i = 0; i < lines; i++) {
        SDL_Rect dst = {0, y, surfaces[i]->w, surfaces[i]->h};
        SDL_BlitSurface(surfaces[i], NULL, result, &dst);
        y += surfaces[i]->h;

        SDL_FreeSurface(surfaces[i]);
        free(rows[i]);
    }

    free(rows);
    free(surfaces);

    return result;
}

bool display_button(int x, int y, int w, int h, const char* text) {
    SDL_Rect buttonRect = {gpx(x), gpx(y), gpx(w), gpx(h)};
    
    // Draw button background (icons[6])
    if (SDL_PointInRect(&mousepoint, &buttonRect)) {
        SDL_SetSurfaceColorMod(icons[6], 200, 200, 200);  // Hover dim
        if (clic) {
            return true;  // Button clicked
        }
    } else {
        SDL_SetSurfaceColorMod(icons[6], 255, 255, 255);  // Normal color
    }
    SDL_BlitScaled(icons[6], NULL, hud_surf, &buttonRect);
    
    // Draw text over button
    SDL_Surface* textSurf = TTF_RenderUTF8_Blended(font, text, (SDL_Color){255, 255, 255, 255});
    if (textSurf) {
        SDL_Rect textPos = {
            buttonRect.x + (buttonRect.w/5),
            buttonRect.y + (buttonRect.h/6),
            buttonRect.w - (buttonRect.w/5)*2,
            buttonRect.h - (buttonRect.h/6)*2
        };
        SDL_BlitScaled(textSurf, NULL, hud_surf, &textPos);
        SDL_FreeSurface(textSurf);
    }
    
    return false;
}
// Pomocná funkce, která bezpečně vrátí ikonu na daném indexu
SDL_Surface* get_current_icon(int index) {
    int global_idx = -1;
    if (current_category_index == -1) {
        if (index >= 0 && index < (int)current_root_items.size()) {
            global_idx = current_root_items[index];
        }
    } else {
        const MenuNode& cat = menu_items[current_category_index];
        if (index >= 0 && index < (int)cat.sub_items.size()) {
            global_idx = cat.sub_items[index];
        }
    }
    if (global_idx != -1 && global_idx < (int)menu_items.size()) {
        return menu_items[global_idx].icon;
    }
    return NULL;
}

bool display_car_choose_menu() {
    // Zjistíme počet dostupných položek v aktuálním zobrazení
    int total_available = (current_category_index == -1) 
        ? (int)current_root_items.size() 
        : (int)menu_items[current_category_index].sub_items.size();



    // Pozadí a Blur
    dest = {0, 0, gpx(800), gpx(600)};
    if (blur_time < BLUR_SPEED) {
        blur_time += 1.0f / fps;
    }
    SDL_SetSurfaceAlphaMod(icons[1], clamp((int)(blur_time * 255.0f), 0, 255));
    SDL_BlitScaled(icons[1], NULL, hud_surf, &dest);

    // Výpočet pozice pro animaci slidingu
    dest = { gpx(100 - menu_delta), gpx(75), gpx(600), gpx(450) };
    
    SDL_Surface* current_surf = get_current_icon(selected_car);
    SDL_Surface* prev_surf = get_current_icon(selected_car - 1);
    SDL_Surface* next_surf = get_current_icon(selected_car + 1);

    if (moving_menu == 1) {
        if (prev_surf) SDL_BlitScaled(prev_surf, NULL, hud_surf, &dest);
        dest = { gpx(900 - menu_delta), gpx(75), gpx(600), gpx(450) };
        if (current_surf) SDL_BlitScaled(current_surf, NULL, hud_surf, &dest);
        
        menu_delta += MENU_ANIMATION_SPEED / fps;
        if (menu_delta > 800) { menu_delta = 0; moving_menu = 0; }
    } 
    else if (moving_menu == -1) {
        if (next_surf) SDL_BlitScaled(next_surf, NULL, hud_surf, &dest);
        dest = { gpx(-700 - menu_delta), gpx(75), gpx(600), gpx(450) };
        if (current_surf) SDL_BlitScaled(current_surf, NULL, hud_surf, &dest);
        
        menu_delta -= MENU_ANIMATION_SPEED / fps;
        if (menu_delta < -800) { menu_delta = 0; moving_menu = 0; }
    } 
    else {
        // Kliknutí na ikonu
        if (SDL_PointInRect(&mousepoint, &dest) && current_surf) {
            SDL_SetSurfaceColorMod(current_surf, 200, 200, 200);
            if (clic) {
                clic = false;
                
                int global_idx = (current_category_index == -1) 
                    ? current_root_items[selected_car] 
                    : menu_items[current_category_index].sub_items[selected_car];
                
                if (menu_items[global_idx].is_category) {
                    // VSTUP DO KATEGORIE
                    current_category_index = global_idx;
                    selected_car = 0;
                    menu_delta = 0;
                } else {
                    // VÝBĚR AUTA -> Start hry
                    blur_time = 0.0f;
                    printf("Spouštím auto z cesty: %s\n", menu_items[global_idx].path.c_str());
                    start_lock = false;
                    return true;
                }
            }
        } else if (current_surf) {
            SDL_SetSurfaceColorMod(current_surf, 255, 255, 255);
        }
        if (current_surf) SDL_BlitScaled(current_surf, NULL, hud_surf, &dest);
    }

    // Šipka VPRAVO
    dest = { gpx(710), gpx(260), gpx(80), gpx(80) };
    if (SDL_PointInRect(&mousepoint, &dest)) {
        SDL_SetSurfaceColorMod(icons[3], 200, 200, 200);
        if (clic && selected_car < total_available - 1 && moving_menu == 0) {
            selected_car += 1;
            menu_delta = 0;
            moving_menu = 1;
            clic = false;
        }
    } else {
        SDL_SetSurfaceColorMod(icons[3], 255, 255, 255);
    }
    SDL_BlitScaled(icons[3], NULL, hud_surf, &dest);

    // Šipka VLEVO
    dest = { gpx(10), gpx(260), gpx(80), gpx(80) };
    if (SDL_PointInRect(&mousepoint, &dest)) {
        SDL_SetSurfaceColorMod(icons[4], 200, 200, 200);
        if (clic && selected_car > 0 && moving_menu == 0) {
            selected_car -= 1;
            menu_delta = 0;
            moving_menu = -1;
            clic = false;
        }
    } else {
        SDL_SetSurfaceColorMod(icons[4], 255, 255, 255);
    }
    SDL_BlitScaled(icons[4], NULL, hud_surf, &dest);

    // --- TLAČÍTKO ZPĚT ---
    if (current_category_index != -1) {
        if (display_button(gpx(10), gpx(10), gpx(60), gpx(40), "<")) {
            current_category_index = -1; 
            selected_car = 0;            
            menu_delta = 0;
            moving_menu = 0;
            clic = false;
            return false;
        }
    }

    return false;
}
bool display_map_choose_menu(){
	dest={0,0,gpx(800),gpx(600)};
	if (blur_time<BLUR_SPEED){
		blur_time+=1.0f/fps;
	}
	SDL_SetSurfaceAlphaMod(icons[1], clamp((int)(blur_time*255.0f),0,255));
	SDL_BlitScaled(icons[1], NULL, hud_surf, &dest);
	dest={ gpx(100-menu_delta), gpx(75), gpx(600), gpx(450) };
	if (moving_menu==1){
		SDL_BlitScaled(map_icons[selected_map-1], NULL, hud_surf, &dest);
		dest={gpx(900-menu_delta), gpx(75), gpx(600), gpx(450) };
		SDL_BlitScaled(map_icons[selected_map], NULL, hud_surf, &dest);
		menu_delta+=MENU_ANIMATION_SPEED/fps;
		if (menu_delta>800){menu_delta=0;moving_menu=0;}
	} else if (moving_menu==-1){
		SDL_BlitScaled(map_icons[selected_map+1], NULL, hud_surf, &dest);
		dest={gpx(-700-menu_delta), gpx(75), gpx(600), gpx(450) };
		SDL_BlitScaled(map_icons[selected_map], NULL, hud_surf, &dest);
		menu_delta-=MENU_ANIMATION_SPEED/fps;
		if (menu_delta<-800){menu_delta=0;moving_menu=0;}
	} else {
		if (SDL_PointInRect(&mousepoint, &dest)){
			SDL_SetSurfaceColorMod(map_icons[selected_map], 200, 200, 200);
			if (clic){
				blur_time=0.0f;
                printf("selected map: %d\n", selected_map);
                clic=false;
				return true;
			}
		} else {
			SDL_SetSurfaceColorMod(map_icons[selected_map], 255, 255, 255);
		}
		SDL_BlitScaled(map_icons[selected_map], NULL, hud_surf, &dest);
	}
	dest={gpx(710),gpx(260),gpx(80),gpx(80)};
	if (SDL_PointInRect(&mousepoint, &dest)){
		SDL_SetSurfaceColorMod(icons[3], 200, 200, 200);
		if (menu_type==2 && clic){
			selected_map+=1;
			menu_delta=0;
			moving_menu=1;
		}
	} else {
		SDL_SetSurfaceColorMod(icons[3], 255, 255, 255);
	}
	SDL_BlitScaled(icons[3], NULL, hud_surf,&dest);
	dest={gpx(10),gpx(260),gpx(80),gpx(80)};
	if (SDL_PointInRect(&mousepoint, &dest)){
		SDL_SetSurfaceColorMod(icons[4], 200, 200, 200);
		if (menu_type==2 && clic){
			selected_map-=1;
			menu_delta=0;
			moving_menu=-1;
		}
	} else {
		SDL_SetSurfaceColorMod(icons[4], 255, 255, 255);
	}
	SDL_BlitScaled(icons[4], NULL, hud_surf,&dest);
	return false;
}
bool display_intro_screen(){
	dest={0,0,gpx(800),gpx(600)};
	SDL_BlitScaled(icons[0], NULL, hud_surf, &dest);
	dest={gpx(250),gpx(300),gpx(300),gpx(150)};
	if (SDL_PointInRect(&mousepoint,&dest)){
		SDL_SetSurfaceColorMod(icons[2], 200, 200, 200);
		if (clic){
            change_menu(MENU_TYPE_MAP_SELECTION);
			return true;
		}
	} else {
		SDL_SetSurfaceColorMod(icons[2], 255, 255, 255);
	}
	SDL_BlitScaled(icons[2], NULL, hud_surf, &dest);
	
	// Settings button
	dest={gpx(650),gpx(500),gpx(100),gpx(80)};
	if (SDL_PointInRect(&mousepoint,&dest)){
		SDL_SetSurfaceColorMod(icons[5], 200, 200, 200);
		if (clic){
            change_menu(MENU_TYPE_SETTINGS);
			return true;
		}
	} else {
		SDL_SetSurfaceColorMod(icons[5], 255, 255, 255);
	}
	SDL_BlitScaled(icons[5], NULL, hud_surf, &dest);
    



SDL_Color barva = {0, 0, 0}; // Bílá barva (R, G, B)
SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, VERSION_TEXT, barva);


// Přenesení textu na obrazovku
SDL_Rect textPos = { gpx(5), gpx(570), gpx(130), gpx(25) }; // Pozice textu na HUDu (v pixelech)
SDL_BlitScaled(textSurface, NULL, hud_surf, &textPos);
SDL_FreeSurface(textSurface);


	return false;
}
#endif


inline void putPixel(SDL_Surface* surf, int x, int y, Uint32 color)
{
    if (x < 0 || y < 0 || x >= surf->w || y >= surf->h) return;
    Uint32* pixels = (Uint32*)surf->pixels;
    int pitch = surf->pitch / 4;
    pixels[y * pitch + x] = color;
}
Uint32 getProcessColor(SDL_PixelFormat* fmt, int i)
{
    // generuje různé barvy podle indexu
    Uint8 r = (i * 97) % 255;
    Uint8 g = (i * 57) % 255;
    Uint8 b = (i * 137) % 255;
    return SDL_MapRGB(fmt, r, g, b);
}


void drawProcessPie(int cx, int cy, int size,
                     uint64_t* times,
                     char names[][256],
                     int count,
                     TTF_Font* font)
{
    if (!hud_surf || !times || count <= 0) return;

    if (SDL_MUSTLOCK(hud_surf)) SDL_LockSurface(hud_surf);

    int radius = size / 2;

    // 🧠 spočítat celkový čas
    int total = 0;
    for (int i = 0; i < count; i++) total += times[i];
    if (total == 0) total = 1;

    // 🎯 kreslení koláče
    float angleStart = 0.0f;

    for (int i = 0; i < count; i++)
    {
        float fraction = (float)times[i] / total;
        float angleEnd = angleStart + fraction * 2.0f * PI;

        Uint32 color = getProcessColor(hud_surf->format, i);

        // vykreslit výseč
        for (int py = -radius; py <= radius; py++)
        {
            for (int px = -radius; px <= radius; px++)
            {
                float dist = sqrtf(px*px + py*py);
                if (dist > radius) continue;

                float ang = atan2f(py, px);
                if (ang < 0) ang += 2.0f * PI;

                if (ang >= angleStart && ang < angleEnd)
                    putPixel(hud_surf, cx + px, cy + py, color);
            }
        }

        angleStart = angleEnd;
    }

    if (SDL_MUSTLOCK(hud_surf)) SDL_UnlockSurface(hud_surf);

    // 🏷️ Kreslení popisků
    angleStart = 0.0f;
    for (int i = 0; i < count; i++)
    {
        float fraction = (float)times[i] / total;
        float angleMid = angleStart + (fraction * PI);

        int tx = cx + cosf(angleMid) * (radius * 0.6f);
        int ty = cy + sinf(angleMid) * (radius * 0.6f);

        SDL_Color white = {255,255,255,255};

        char label[128];
        sprintf(label, "%s", names[i]);

        SDL_Surface* text = TTF_RenderText_Blended(font, label, white);
        if (text && fraction>=0.1f)
        {
            SDL_Rect dst = { tx - text->w/2, ty - text->h/2, 0, 0 };
            SDL_BlitSurface(text, NULL, hud_surf, &dst);
            SDL_FreeSurface(text);
        }

        angleStart += fraction * 2.0f * PI;
    }
}
SDL_Surface* render_text_with_shadow_surface(TTF_Font* font, const char* text,
                                             SDL_Color textColor,
                                             SDL_Color shadowColor,
                                             int shadowOffset)
{
    // povrch pro hlavní text
    SDL_Surface* textSurface = TTF_RenderUTF8_Multiline(font, text, textColor);
    if (!textSurface) return nullptr;

    // povrch pro stín
    SDL_Surface* shadowSurface = TTF_RenderUTF8_Multiline(font, text, shadowColor);
    if (!shadowSurface) {
        SDL_FreeSurface(textSurface);
        return nullptr;
    }

    // vytvoříme nový povrch s dostatečnou velikostí pro stín
    int w = textSurface->w + shadowOffset;
    int h = textSurface->h + shadowOffset;

    SDL_Surface* outSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!outSurface) {
        SDL_FreeSurface(textSurface);
        SDL_FreeSurface(shadowSurface);
        return nullptr;
    }

    // vyplnit průhledností
    SDL_FillRect(outSurface, NULL, SDL_MapRGBA(outSurface->format, 0, 0, 0, 0));

    // překopírujeme stín
    SDL_Rect shadowRect = { shadowOffset, shadowOffset, shadowSurface->w, shadowSurface->h };
    SDL_BlitSurface(shadowSurface, NULL, outSurface, &shadowRect);

    // překopírujeme hlavní text
    SDL_Rect textRect = { 0, 0, textSurface->w, textSurface->h };
    SDL_BlitSurface(textSurface, NULL, outSurface, &textRect);

    // cleanup
    SDL_FreeSurface(textSurface);
    SDL_FreeSurface(shadowSurface);

    return outSurface;
}
int rend_text(int x, int y, int size, std::string text, SDL_Color color={255,255,255,255}, bool show=true){
    SDL_Surface* textSurf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    
    if (textSurf) {
        // Chceme, aby text byl nad barem a zachoval si svůj poměr stran
        float textAR = (float)textSurf->w / (float)textSurf->h;
        
        // Nastavíme fixní výšku textu (třeba 25px v gpx) a šířku dopočítáme
        SDL_Rect textDest;
        textDest.h = size;
        textDest.w = (int)(textDest.h * textAR);
        
        // Vycentrování textu horizontálně
        textDest.x = x;
        // Pozice kousek nad barem
        textDest.y = y;
        if (show){
            SDL_BlitScaled(textSurf, NULL, hud_surf, &textDest);
        }
        // DŮLEŽITÉ: Tady po sobě musíme uklidit "hovínka", jinak ti vyteče RAM!
        SDL_FreeSurface(textSurf);
        return textDest.w; // Vrátíme šířku textu, abychom mohli posunout další položku
    }
}
int itemy=0;
int setting_item_size=15;
int slider_size=200;
void display_check_box(int x, int y, bool& variable, std::string text){
    int box_border=2;
    rend_text(x+setting_item_size, y, setting_item_size, text);
    SDL_Rect boxRect = { x, y, setting_item_size, setting_item_size };
    SDL_Color boxColor = { 255, 255, 255, 255 };
    SDL_FillRect(hud_surf, &boxRect, SDL_MapRGBA(hud_surf->format, boxColor.r, boxColor.g, boxColor.b, boxColor.a));
    if (variable) {
        SDL_Rect checkRect = { x + box_border, y + box_border, setting_item_size - 2 * box_border, setting_item_size - 2 * box_border };
        SDL_Color checkColor = { 0, 255, 0, 255 };
        SDL_FillRect(hud_surf, &checkRect, SDL_MapRGBA(hud_surf->format, checkColor.r, checkColor.g, checkColor.b, checkColor.a));
    } else {
        SDL_Rect checkRect = { x + box_border, y + box_border, setting_item_size - 2 * box_border, setting_item_size - 2 * box_border };
        SDL_Color checkColor = { 255, 0, 0, 255 };
        SDL_FillRect(hud_surf, &checkRect, SDL_MapRGBA(hud_surf->format, checkColor.r, checkColor.g, checkColor.b, checkColor.a));
    }
    if (SDL_PointInRect(&mousepoint, &boxRect)){
        SDL_SetSurfaceColorMod(hud_surf, 200, 200, 200);
        if (clic){
            variable = !variable;
        }
    }
    itemy+=setting_item_size;
}
// --- NASTAVENÍ ROZMĚRŮ (Tady si to laď) ---
int slider_w = 200;      // Celková šířka lišty
int slider_h = 16;       // Výška ovládací zóny (hitboxu)
int bar_h = 6;           // Tloušťka té vodorovné čáry
int mark_h = 12;         // Výška těch svislých čárek
int knob_w = 8;          // Šířka jezdce
int knob_h = 20;         // Výška jezdce
int padding_x = 15;      // Mezera mezi textem a sliderem
// ------------------------------------------
void display_slider_int(int x, int y, int& variable, int min, int max, std::string text, float exp = 1.0f) {
    // --- NASTAVENÍ ROZMĚRŮ ---
    int slider_w = 200;
    int bar_h = 6;
    int mark_h = 12;
    int knob_w = 8;
    int knob_h = 20;
    int padding_x = 15;

    // 1. Texty a zjištění fixní šířky podle 'max'
    rend_text(x, y, setting_item_size, text + ": " + std::to_string(variable));
    int text_width = rend_text(x, y, setting_item_size, text + ": " + std::to_string(max), {255,255,255,255}, false);
    
    int sliderX = x + text_width + padding_x;
    int sliderY = y + (setting_item_size / 2) - (bar_h / 2);
    SDL_Rect clickRect = { sliderX, y, slider_w, setting_item_size };
    
    // 2. Pozadí lišty
    SDL_Rect barRect = { sliderX, sliderY, slider_w, bar_h };
    SDL_FillRect(hud_surf, &barRect, SDL_MapRGBA(hud_surf->format, 80, 80, 80, 255));
    
    // 3. Vykreslení čárek
    int range = max - min;
    if (range > 0) {
        if (range <= 20) {
            // MALÝ ROZSAH: Vykreslíme čárku pro každou celočíselnou hodnotu
            for (int i = 0; i <= range; i++) {
                // Musíme značku posunout podle exponenciály, aby seděla k logice pohybu!
                float pct = (float)i / (float)range;
                float display_pct = powf(pct, 1.0f / exp); // Inverze exp
                
                int markX = sliderX + (int)(display_pct * slider_w);
                int markY = y + (setting_item_size / 2) - (mark_h / 2);
                SDL_Rect markRect = { markX, markY, 1, mark_h };
                SDL_FillRect(hud_surf, &markRect, SDL_MapRGBA(hud_surf->format, 150, 150, 150, 255));
            }
        } else {
            // VELKÝ ROZSAH: Vykreslíme jen 5 fixních orientačních bodů (0%, 25%, 50%, 75%, 100%)
            for (int i = 0; i <= 1; i++) {
                int markX = sliderX + (i * slider_w / 4);
                int markY = y + (setting_item_size / 2) - (mark_h / 2);
                SDL_Rect markRect = { markX, markY, 1, mark_h };
                SDL_FillRect(hud_surf, &markRect, SDL_MapRGBA(hud_surf->format, 150, 150, 150, 255));
            }
        }
    }

    // 4. Logika ovládání s FLOAT EXPONENCIÁLOU
    if (SDL_PointInRect(&mousepoint, &clickRect)) {
        if (prk) { // Doporučuju prk pro plynulost i u intů
            float pct = (float)(mousepoint.x - sliderX) / (float)slider_w;
            if (pct < 0) pct = 0;
            if (pct > 1) pct = 1;
            
            // Exponenciální přepočet
            float adjusted_pct = powf(pct, exp);
            
            // Výpočet výsledného intu s korektním zaokrouhlením
            variable = min + (int)(adjusted_pct * (float)range + 0.5f);
            
            if (variable < min) variable = min;
            if (variable > max) variable = max;
        }
    }

    // 5. Jezdec (Zpětný výpočet pozice)
    float val_pct = (range > 0) ? (float)(variable - min) / (float)range : 0;
    float display_pct = powf(val_pct, 1.0f / exp); // Inverzní k pow(pct, exp)

    int knobX = sliderX + (int)(display_pct * (float)slider_w);
    int knobY = y + (setting_item_size / 2) - (knob_h / 2);
    SDL_Rect knobRect = { knobX - (knob_w / 2), knobY, knob_w, knob_h };
    
    if (SDL_PointInRect(&mousepoint, &clickRect)) {
        SDL_FillRect(hud_surf, &knobRect, SDL_MapRGBA(hud_surf->format, 100, 255, 100, 255));
    } else {
        SDL_FillRect(hud_surf, &knobRect, SDL_MapRGBA(hud_surf->format, 0, 200, 0, 255));
    }

    itemy += setting_item_size + 5;
}
void display_slider_float(int x, int y, float& variable, float min, float max, std::string text, float exp = 1.0f) {
    // --- NASTAVENÍ ROZMĚRŮ ---
    int slider_w = 200;      
    int bar_h = 6;           
    int mark_h = 12;         
    int knob_w = 8;          
    int knob_h = 20;         
    int padding_x = 15;      
    // -------------------------

    // 1. Texty a stabilní šířka (pomocí tvého triku s 'max' a 'show=false')
    std::stringstream ss;
    ss << text << ": " << std::fixed << std::setprecision(2) << variable;
    rend_text(x, y, setting_item_size, ss.str());
    
    std::stringstream ss2;
    ss2 << text << ": " << std::fixed << std::setprecision(2) << max;
    int text_width = rend_text(x, y, setting_item_size, ss2.str(), {255,255,255,255}, false);

    int sliderX = x + text_width + padding_x;
    int sliderY = y + (setting_item_size / 2) - (bar_h / 2);
    SDL_Rect clickRect = { sliderX, y, slider_w, setting_item_size };
    
    // 2. Pozadí lišty
    SDL_Rect barRect = { sliderX, sliderY, slider_w, bar_h };
    SDL_FillRect(hud_surf, &barRect, SDL_MapRGBA(hud_surf->format, 80, 80, 80, 255));
    // 3. carky
    float range = max - min;
    if (range > 0) {
        if (range <= 20) {
            // MALÝ ROZSAH: Vykreslíme čárku pro každou celočíselnou hodnotu
            for (int i = 0; i <= range; i++) {
                // Musíme značku posunout podle exponenciály, aby seděla k logice pohybu!
                float pct = (float)i / (float)range;
                float display_pct = powf(pct, 1.0f / exp); // Inverze exp
                
                int markX = sliderX + (int)(display_pct * slider_w);
                int markY = y + (setting_item_size / 2) - (mark_h / 2);
                SDL_Rect markRect = { markX, markY, 1, mark_h };
                SDL_FillRect(hud_surf, &markRect, SDL_MapRGBA(hud_surf->format, 150, 150, 150, 255));
            }
        } else {
            // VELKÝ ROZSAH: Vykreslíme jen 5 fixních orientačních bodů (0%, 25%, 50%, 75%, 100%)
            for (int i = 0; i <= 1; i++) {
                int markX = sliderX + (i * slider_w / 4);
                int markY = y + (setting_item_size / 2) - (mark_h / 2);
                SDL_Rect markRect = { markX, markY, 1, mark_h };
                SDL_FillRect(hud_surf, &markRect, SDL_MapRGBA(hud_surf->format, 150, 150, 150, 255));
            }
        }
    }

    // 4. Logika ovládání s EXPONENCIÁLOU
    if (SDL_PointInRect(&mousepoint, &clickRect)) {
        if (prk) { 
            // Lineární procento (kde je myš)
            float pct = (float)(mousepoint.x - sliderX) / (float)slider_w;
            if (pct < 0) pct = 0;
            if (pct > 1) pct = 1;
            
            // Aplikace exponenciály
            // Pokud exp = 1.0, nic se nezmění (lineární)
            // Pokud exp = 2.0, pct se umocní (na začátku slideru jsou malé změny)
            float adjusted_pct = powf(pct, exp);
            
            variable = min + adjusted_pct * (max - min);
        }
    }

    // 5. Výpočet pozice jezdce (musíme jít opačným směrem – z hodnoty na pixely)
    float val_pct = (variable - min) / range;
    if (val_pct < 0) val_pct = 0;
    
    // Zpětná transformace pro zobrazení (odmocnina)
    // pct = val_pct^(1/exp)
    float display_pct = powf(val_pct, 1.0f / exp);
    
    int knobX = sliderX + (int)(display_pct * (float)slider_w);
    int knobY = y + (setting_item_size / 2) - (knob_h / 2);
    SDL_Rect knobRect = { knobX - (knob_w / 2), knobY, knob_w, knob_h };
    
    // Vykreslení jezdce
    if (SDL_PointInRect(&mousepoint, &clickRect)) {
        SDL_FillRect(hud_surf, &knobRect, SDL_MapRGBA(hud_surf->format, 100, 255, 100, 255));
    } else {
        SDL_FillRect(hud_surf, &knobRect, SDL_MapRGBA(hud_surf->format, 0, 200, 0, 255));
    }

    itemy += setting_item_size + 5; 
}
void display_settings(){
    dest={gpx(0),gpx(0),gpx(800),gpx(600)};
    SDL_BlitScaled(icons[0], NULL, hud_surf, &dest);
	if (blur_time<BLUR_SPEED){
		blur_time+=1.0f/fps;
	}
	SDL_SetSurfaceAlphaMod(icons[1], clamp((int)(blur_time*255.0f),0,255));
	SDL_BlitScaled(icons[1], NULL, hud_surf, &dest);
    itemy=100;
    int itemx=10;
    // --- LEVÝ SLOUPEC ---
    display_check_box(itemx, itemy, t_vsync, "Vsync");
    display_check_box(itemx, itemy, t_shadows, "Shadows");
    display_check_box(itemx, itemy, t_antiastropic_filtering, "Antiastropic filtering");
    display_check_box(itemx, itemy, t_fullscreen, "Fullscreen");
    display_check_box(itemx, itemy, t_highdpi, "High DPI");
    display_slider_int(itemx, itemy, t_shader_level, 0, 1, "Shader level");
    display_slider_int(itemx, itemy, t_renderer, 0, 1, "Renderer");
    display_slider_int(itemx, itemy, t_tex_pres, 1, 20, "Texture precision");
    display_slider_int(itemx, itemy, t_hud_render_interval, 1, 1000, "HUD render interval");
    display_slider_int(itemx, itemy, t_STARTPAUSELENGTH, 5, 60, "Start pause length");
    display_slider_int(itemx, itemy, t_TREE_QUALITY_DISTANCE, 10, 100, "Tree quality distance");
    display_slider_int(itemx, itemy, t_WIDTH, 640, 3840, "Screen width");
    display_slider_int(itemx, itemy, t_HEIGHT, 480, 2160, "Screen height");
    display_slider_int(itemx, itemy, t_MAP_SIZE, 5000, 100000, "Map size");
    display_slider_int(itemx, itemy, t_HUD_WIDTH, 400, 3200, "HUD width");
    display_slider_int(itemx, itemy, t_HUD_HEIGHT, 300, 2400, "HUD height");
    itemy+=100;
    display_slider_int(itemx, itemy, t_collision_skip_rate, 1, 20, "Collision skip rate (frames)");
    // display_slider_float(itemx, itemy, t_RAD_DEG, 1.0f, 100.0f, "RAD_DEG");
    // display_slider_float(itemx, itemy, t_PI, 1.0f, 3.1415926535f, "Pi");
    display_slider_float(itemx, itemy, t_SIMULATION_FREQ, 10.0f, 30000.0f, "Sim Frequency (Hz)", 2.0f);
    display_slider_float(itemx, itemy, t_shift_interval, 50.0f, 5000.0f, "Shift Interval (M)");

    // --- PRAVÝ SLOUPEC ---
    itemx = 405;
    itemy = 100;
    display_slider_float(itemx, itemy, t_min_fps, 1.0f, 120.0f, "Min FPS for real time");
    display_slider_float(itemx, itemy, t_render_distance, 1.0f, 100000.0f, "Render Distance (M)", 2.0f);
    display_slider_float(itemx, itemy, t_slomo_slowness, 1.5f, 1000.0f, "Slomo Factor");
    display_slider_float(itemx, itemy, t_light_ambient, 0.0f, 1.0f, "Ambient Light");
    display_slider_float(itemx, itemy, t_zoom_fov, 0.1f, 120.0f, "Zoom FOV");
    display_slider_float(itemx, itemy, t_shadow_range, 1.0f, 100000.0f, "Shadow Range", 2.0f);
    display_slider_float(itemx, itemy, t_shadow_jump, 1.0f, 1000.0f, "Shadow map jump (M)", 1.5f);
    display_slider_float(itemx, itemy, t_exposure_multiplyer, 0.1f, 20.0f, "Exposure", 1.5f);
    display_slider_int(itemx, itemy, t_SHADOW_RESOLUTION, 64, 32768, "Shadowmap Resolution", 1.5f);

    // --- TLAČÍTKO ---
    if (display_button(200, HUD_HEIGHT - 50, 80, 30, "Pouzit")) {
        vsync = t_vsync;
        shadows = t_shadows;
        antiastropic_filtering = t_antiastropic_filtering;
        fullscreen = t_fullscreen;
        highdpi = t_highdpi;
        shader_level = t_shader_level;
        renderer = t_renderer;
        tex_pres = t_tex_pres;
        hud_render_interval = t_hud_render_interval;
        STARTPAUSELENGTH = t_STARTPAUSELENGTH;
        TREE_QUALITY_DISTANCE = t_TREE_QUALITY_DISTANCE;
        WIDTH = t_WIDTH;
        HEIGHT = t_HEIGHT;
        MAP_SIZE = t_MAP_SIZE;
        HUD_WIDTH = t_HUD_WIDTH;
        HUD_HEIGHT = t_HUD_HEIGHT;
        collision_skip_rate = t_collision_skip_rate;
        SHADOW_RESOLUTION = t_SHADOW_RESOLUTION;
        RAD_DEG = t_RAD_DEG;
        PI = t_PI;
        SIMULATION_FREQ = t_SIMULATION_FREQ;
        shift_interval = t_shift_interval;
        retina_scale = t_retina_scale;
        min_fps = t_min_fps;
        render_distance = t_render_distance;
        slomo_slowness = t_slomo_slowness;
        light_ambient = t_light_ambient;
        zoom_fov = t_zoom_fov;
        shadow_range = t_shadow_range;
        shadow_jump = t_shadow_jump;
        exposure_multiplyer = t_exposure_multiplyer;
        
        setup_renderer(WIDTH, HEIGHT);
    }

    if (display_button(20, HUD_HEIGHT-50, 80, 30, "Ulozit")){
        save_settings();
    }
    if (display_button(110, HUD_HEIGHT-50, 80, 30, "Reset")){
        reset_settings();
    }
    if (display_button(20, 20, 80, 30, "<")){
        menu_go_back();
    }


}
void display_pause_menu(){
    rend_text(300, 50, 50, "Game menu", {255,255,255,255});
    if (display_button(100, 300, 250, 80, "Resume")){
        last_menu_type=MENU_TYPE_NONE;
        menu_type=MENU_TYPE_NONE;
    }
    if (display_button(100, 400, 250, 80, "Settings")){
        change_menu(MENU_TYPE_SETTINGS);
    }
    if (display_button(450, 400, 250, 80, "Quit")){
        exit(0);
    }
    if (display_button(450, 200, 250, 80, "Add vehicle")){
        if (menu_type==3){
			menu_type=0;
		} else {
			menu_type=3;
			blur_time=0.0f;
			permission_to_add_car=true;
		}
    }
    if (display_button(100, 200, 250, 80, "Change vehicle")){
        if (menu_type==3){
			menu_type=0;
		} else {
			menu_type=3;
			blur_time=0.0f;
		}
    }
    if (display_button(450, 300, 250, 80, "Change map")){
        if (menu_type==MENU_TYPE_MAP_SELECTION){
			menu_type=0;
		} else {
			menu_type=MENU_TYPE_MAP_SELECTION;
			blur_time=0.0f;
		}
    }
}
bool render_hud(){
    log("rendering hud");
    if (menu_type==0){
        hud_render_interval=default_hud_render_interval;
    } else {
        hud_render_interval=10;
    }
    if ((SDL_GetTicks64()-last_hud_render_time>=hud_render_interval) || force_hud_render){
        last_hud_render_time=SDL_GetTicks64();
        bool ret=false;
        // Vymaže plochu na černou barvu (0, 0, 0)
        SDL_FillRect(hud_surf, NULL, SDL_MapRGBA(hud_surf->format, 0, 0, 0,0));
        if (hud_stuff_render_mode==2){
            snprintf(
                hud_text,
                sizeof(hud_text),
                "Vrakodes 1.0\nRenderer: %s, %s\nOpenGL version: %s\n \nFPS: %.1f\nChecked boxes: %d\nSimulation steps: %d\nStep dt: %f\nActive cars: %d\nDriving car: %d\nJoints count: %d\nDebug mode: %d\nPos: %.2f, %.2f, %.2f\nCar pos: %.2f, %.2f, %.2f\nCar rot: %.2f, %.2f, %.2f\nRot: %.2f, %.2f\nEngine f: %.1f\n \nSpeed: %.1fKM/H\nMach: %.4f\nDamage: %.4f\nTime: %.2f\n \n%s",
                info_vkdr,
                info_renderer,
                info_glversion,
                fps,
                checked_collision_boxes,
                simulation_steps,
                step_dt,
                cars.size(),
                driving_car,
                cars[driving_car].joints_count,
                debug,
                x_pos,
                y_pos,
                z_pos,
                cars[driving_car].pos_x+cars[driving_car].x_shift,
                cars[driving_car].pos_y,
                cars[driving_car].pos_z+cars[driving_car].z_shift,
                cars[driving_car].rot_x*RAD_DEG,
                cars[driving_car].rot_y*RAD_DEG,
                cars[driving_car].rot_z*RAD_DEG,
                x_rot,
                y_rot,
                engine_force,
                cars[driving_car].velocity,
                cars[driving_car].velocity/1200.0f,
                cars[driving_car].breaking_score,
                game_time,
                warning_text
            );
            wrap_text(hud_text,30);

            // textSurface =
            //     TTF_RenderUTF8_Multiline(font, text,
            //         (SDL_Color){255,0,0,255});
            textSurface=render_text_with_shadow_surface(font,hud_text,(SDL_Color){0,0,0,255}, (SDL_Color){255,255,255,255}, 4);
            if (!textSurface) {
                printf("TTF error: %s\n", TTF_GetError());
            }
            if (textSurface) {
                SDL_Rect textPos = { gpx(5), gpx(5), gpx(300), gpx(500) }; // Pozice textu na HUDu (v pixelech)
                SDL_BlitScaled(textSurface, NULL, hud_surf, &textPos);
            }
            SDL_FreeSurface(textSurface);
            drawProcessPie(70,550,100,process_times,process_names,PROCESS_COUNT,font1);
            drawProcessPie(190,550,100,render_process_times,render_process_names,RENDER_PROCESS_COUNT,font1);
        }
        if (menu_type==3){
            if (title){
                dest={gpx(0),gpx(0),gpx(800),gpx(600)};
                SDL_BlitScaled(icons[0], NULL, hud_surf, &dest);
            }
            if (display_car_choose_menu()){
                ret=true;
                last_menu_type=MENU_TYPE_NONE;
                menu_type=MENU_TYPE_NONE;
                title=false;
                if (map_to_be_generated){
                    map_to_be_generated=false;
                    generate_map=true;
                }
            }
        } else if (menu_type==2){
            if (title){
                dest={gpx(0),gpx(0),gpx(800),gpx(600)};
                SDL_BlitScaled(icons[0], NULL, hud_surf, &dest);
            }
            if (display_map_choose_menu()){
                ret=true;
                change_menu(MENU_TYPE_CAR_SELECTION);
                title=false;
                map_to_be_generated=true;
            }
        } else if (menu_type==1){
            display_intro_screen();
        } else if (menu_type==MENU_TYPE_SETTINGS){
            display_settings();
        } else if (menu_type==MENU_TYPE_PAUSE){
            display_pause_menu();
        }
        // rend_text(10, 10, 20, std::to_string(menu_type), {255,255,255,255});
        if (paused && menu_type==MENU_TYPE_NONE){
            SDL_Surface* texturface=render_text_with_shadow_surface(font, "PAUSED", {255,255,255,255}, {0,0,0,255}, 2);
            if (texturface) {
                SDL_Rect textPos = { gpx(680), gpx(10), gpx(100), gpx(30) }; // Pozice textu na HUDu (v pixelech)
                SDL_BlitScaled(texturface, NULL, hud_surf, &textPos);
            }
            SDL_FreeSurface(texturface);
        }
        if ((SDL_GetTicks64() - cam_change_time < CAM_CHANGE_TEXT_TIME) && menu_type==MENU_TYPE_NONE){
            std::string cam_text;
            switch (in_car_mode){
                case 0:
                    cam_text="Free camera";
                    break;
                case 1:
                    cam_text="3rd person view";
                    break;
                case 2:
                    cam_text="1st person view";
            }
            SDL_Surface* texturface=render_text_with_shadow_surface(font, cam_text.c_str(), {255,255,255,255}, {0,0,0,255}, 2);
            if (texturface) {
                SDL_Rect textPos = { gpx(630), gpx(40), gpx(150), gpx(30) }; // Pozice textu na HUDu (v pixelech)
                SDL_BlitScaled(texturface, NULL, hud_surf, &textPos);
            }
            SDL_FreeSurface(texturface);
        }
        if (hud_stuff_render_mode!=0 && menu_type==MENU_TYPE_NONE){
            std::string text=std::to_string(cars[driving_car].velocity)+"km/h";
            SDL_Surface* texturface=render_text_with_shadow_surface(font, text.c_str(), {255,255,255,255}, {0,0,0,255}, 2);
            if (texturface) {
                SDL_Rect textPos = { gpx(350), gpx(560), gpx(100), gpx(30) }; // Pozice textu na HUDu (v pixelech)
                SDL_BlitScaled(texturface, NULL, hud_surf, &textPos);
            }
            SDL_FreeSurface(texturface);
        }
        // display_check_box(10, 10, clic, "clic"); // for debug
        clic=false;
        log("render hud done");
        return ret;
    }
    force_hud_render=false;
    clic=false;
    log("render hud done");
    return false;
}
void display_background(bool blured=true){
    dest={0,0,gpx(800),gpx(600)};
	SDL_BlitScaled(icons[1], NULL, hud_surf, &dest);
}
void render_progress_bar(float progress, const std::string& text) {
    // 1. Základní nastavení rozměrů
    int screenW = gpx(800);
    int screenH = gpx(600);

    // 3. Definice Progress Baru (vycentrovaný)
    int barW = gpx(400);
    int barH = gpx(30);
    int barX = (screenW - barW) / 2;
    int barY = (screenH - barH) / 2;

    // Pozadí baru (Tmavě fialová)
    SDL_Rect bgRect = { barX, barY, barW, barH };
    Uint32 bgColor = SDL_MapRGBA(hud_surf->format, 40, 0, 60, 200);
    SDL_FillRect(hud_surf, &bgRect, bgColor);

    // Vnitřní výplň
    if (progress > 0.01f) {
        if (progress > 1.0f) progress = 1.0f;
        SDL_Rect fillRect = { barX, barY, (int)(barW * progress), barH };
        Uint32 fillColor = SDL_MapRGBA(hud_surf->format, 160, 32, 240, 255);
        SDL_FillRect(hud_surf, &fillRect, fillColor);
    }

    // 4. Renderování textu pomocí tvého objektu 'font'
    if (!text.empty()) {
        SDL_Color textColor = { 255, 255, 255, 255 }; // Bílá
        // Vytvoříme surface s textem (používám Blended pro hezké vyhlazování)
        SDL_Surface* textSurf = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
        
        if (textSurf) {
            // Chceme, aby text byl nad barem a zachoval si svůj poměr stran
            float textAR = (float)textSurf->w / (float)textSurf->h;
            
            // Nastavíme fixní výšku textu (třeba 25px v gpx) a šířku dopočítáme
            SDL_Rect textDest;
            textDest.h = gpx(25); 
            textDest.w = (int)(textDest.h * textAR);
            
            // Vycentrování textu horizontálně
            textDest.x = (screenW - textDest.w) / 2;
            // Pozice kousek nad barem
            textDest.y = barY - textDest.h - gpx(10);

            SDL_BlitScaled(textSurf, NULL, hud_surf, &textDest);

            // DŮLEŽITÉ: Tady po sobě musíme uklidit "hovínka", jinak ti vyteče RAM!
            SDL_FreeSurface(textSurf);
        }
    }
}
void progress_bar(float progress, const std::string& text) {
    display_background();
    render_progress_bar(progress, text);
    run_essencials();
}