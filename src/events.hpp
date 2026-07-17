#ifndef EVENTS_HPP
#define EVENTS_HPP
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
void quit(int code)
{
    SDL_Quit();
    exit(code);
}
int handle_key(SDL_Keysym keysym, SDL_Window* screen)
{

    switch (keysym.sym) {
	case SDL_WINDOWEVENT:
		if (event.window.event==SDL_WINDOWEVENT_CLOSE){
			SDL_Quit();
			running=false;
			SDL_DestroyWindow(window);
			exit(0);
		}
		break;
	case SDLK_LSHIFT:
		shift=1;
		break;
	case SDLK_q:
		x_rot=0.0;
		y_rot=0.0;
		break;
	case SDLK_x:
		slomo=!slomo;
		break;
	case SDLK_c:
		//nothig happens here
		break;
	case SDLK_z:
		shadowmap_debug=!shadowmap_debug;
		break;
	case SDLK_F2:
		rubish = system("scrot");
		break;
	case SDLK_F3:
		debug++;
		if (debug>DEBUG_MODES-1){
			debug=0;
		}
		break;
	case SDLK_TAB:
		printf("driving car %d\n", driving_car);
		driving_car++;
		if (driving_car>=cars.size()){
			driving_car=0;
		}
		printf("driving car %d\n", driving_car);
		change_driving_car_vars();
		break;
	case SDLK_h:
		in_car_mode++;
		if (in_car_mode>2){in_car_mode=0;}
		if (in_car_mode==0 || in_car_mode==1){
			NEAR=0.5f;
		} else if (in_car_mode==2){
			NEAR=0.1f;
		}
		SDL_WarpMouseInWindow(screen,WD2,HD2);
		cam_change_time=SDL_GetTicks();
		force_hud_render=true;
		break;
	case SDLK_p:
		paused=!paused;
		    printf("PAUSING: faces=%d vertices=%d points=%d joints=%d\n", 
           cars[driving_car].faces_count, cars[driving_car].vertices_count, cars[driving_car].points_count, cars[driving_car].joints_count);
		force_hud_render=true;
		break;
	// case SDLK_l:
	// 	slomo=!slomo;
	// 	break;
	case SDLK_RIGHT:
		if (menu_type==3){
			selected_car++;
			menu_delta=0;
			moving_menu=1;
			break;
		}
		if (menu_type==2){
			selected_map++;
			menu_delta=0;
			moving_menu=1;
			break;
		}
		nextframe=true;
		break;
	case SDLK_LEFT:
		if (menu_type==3){
			selected_car--;
			menu_delta=0;
			moving_menu=-1;
			break;
		}
		if (menu_type==2){
			selected_map--;
			menu_delta=0;
			moving_menu=-1;
			break;
		}
		break;
	case SDLK_r:
		if (menu_type==3){
			menu_type=0;
		} else {
			menu_type=3;
			blur_time=0.0f;
			permission_to_add_car=true;
		}
		break;
	case SDLK_v:
		if (menu_type==3){
			menu_type=0;
		} else {
			menu_type=3;
			blur_time=0.0f;
		}
		break;
	case SDLK_o:
		hud_debug_shall_be_rendered=!hud_debug_shall_be_rendered;
		break;
	case SDLK_1:
		// x_pos=chaloupka_x;
		// z_pos=chaloupka_z;
		// y_pos=chaloupka_y;
		x_pos=chaloupky[1].x;
		z_pos=chaloupky[1].y;
		y_pos=get_heightmap_height(x_pos, z_pos)+2.0f;
		break;
	case SDLK_2:
		// x_pos=chaloupka_x;
		// z_pos=chaloupka_z;
		// y_pos=chaloupka_y;
		x_pos=3000;
		z_pos=3000;
		y_pos=get_heightmap_height(x_pos, z_pos)+2.0f;
		break;
	case SDLK_F4:
		printf("FPS: %f\nrender process times\n", fps);
		for (i=0;i<RENDER_PROCESS_COUNT;i++){
			printf("    %s: %d \n", render_process_names[i], render_process_times[i]);
		}
		printf("\n\nprocess times\n");
		for (i=0;i<PROCESS_COUNT;i++){
			printf("    %s: %d \n", process_names[i], process_times[i]);
		}
		break;
	case SDLK_ESCAPE:

		// quit(0);
		escape=true;
		if (menu_type==MENU_TYPE_NONE){
			menu_type=MENU_TYPE_PAUSE;
			pause_zaloha=paused;
			paused=true;
		} else if (menu_type==MENU_TYPE_PAUSE){
			menu_type=MENU_TYPE_NONE;
			paused=pause_zaloha;
		}
		break;
    }
    return ret;
}
int process_events(SDL_Window* screen)
{
	ret=0;
	steering=false;
	// clic=false;
	// While at least one event present
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				quit(0);
				ret=10;
				break;
			case SDL_KEYDOWN:
				ret=handle_key(event.key.keysym,screen);
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym==SDLK_LSHIFT){
					shift=0;
				}
				break;
			case SDL_MOUSEMOTION:
				x_mouse=event.motion.x;
				y_mouse=event.motion.y;
				SDL_GetMouseState(&mouseX, &mouseY);
				// mousepoint={(int)((float)mouseX*retina_scale), (int)((float)mouseY*retina_scale)};
				mousepoint={(int)(((float)mouseX/(float)WIDTH)*(float)HUD_WIDTH), (int)(((float)mouseY/(float)HEIGHT)*(float)HUD_HEIGHT)};
				if (menu_type==0){
					if (in_car_mode){
						// volant_pos=-((x_mouse-WD2)*CITLIVOST_RIZENI);

					} else {
						x_rot+=(float)(x_mouse-WD2)/WIDTH*ROT_SPEED;
						y_rot-=(float)(y_mouse-HD2)/WIDTH*ROT_SPEED;
					}
				    SDL_ShowCursor(SDL_FALSE);
				} else {
				    SDL_ShowCursor(SDL_TRUE);
				}
				// deltaX = event.motion.x - lastMouseX;
                // deltaY = event.motion.y - lastMouseY;
                // y_mouse += deltaX * 0.1f;  // Rotace do stran
                // x_mouse -= deltaY * 0.1f;  // Rotace nahoru/dolů
                // if (x_mouse > 90.0f){x_mouse = 90.0f;}
                // if (x_mouse < -90.0f){ x_mouse = -90.0f;}
                // lastMouseX = event.motion.x;
                // lastMouseY = event.motion.y;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button==1){
					prk=1;
					clic=true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button==1){
					prk=0;
					// clic=true;
				}
				break;

		}
	}

	float mult=1.03f;
	if (prk){
		if (shift) {
			// Let dozadu: zvětšíme zápornou rychlost a ještě trochu přidáme
			speed = (speed * mult) - SP; 
		} else {
			// Let dopředu: zvětšíme kladnou rychlost a ještě trochu přidáme
			speed = (speed * mult) + SP;
		}
	} else {
		speed=0.0f;
	}
	//printf("%f",speed);
	//printf("%d",event.button.button);
	if (menu_type==0){
		if (!in_car_mode){
			x_pos-=sin_deg(x_rot+90)*sin_deg(y_rot-90)*speed/fps;
			y_pos+=sin_deg(y_rot)*speed/fps;
			z_pos-=sin_deg(x_rot)*sin_deg(y_rot-90)*speed/fps;
			SDL_WarpMouseInWindow(screen,WD2,HD2);
		}
	}



	const Uint8* keys = SDL_GetKeyboardState(NULL);

	float step = steer_speed*2.0f/fps;

if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        cars[driving_car].engine_force = cars[driving_car].engine_power;
        if (!last_time_gas){
                audio_switch_to_sound2();
                last_time_gas=true;
        }
} else {
        if (last_time_gas){
                audio_switch_to_sound1();
                last_time_gas=false;
        }
        cars[driving_car].engine_force = 0.0f;
}

if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]){
        cars[driving_car].engine_force=-cars[driving_car].engine_power;
}

if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
        cars[driving_car].volant_pos -= step;
        steering=true;
}

if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
        cars[driving_car].volant_pos += step;
        steering=true;
}
	if (keys[SDL_SCANCODE_F]) {
		FOV=zoom_fov;
	} else {
		FOV=default_fov;
	}
	if (!steering){
		if (cars[driving_car].volant_pos>0.0f){
			cars[driving_car].volant_pos -= step;
		} else if (cars[driving_car].volant_pos<0.0f){
			cars[driving_car].volant_pos += step;
		}
	}
	if (cars.size()>=2){
		cardata& c=(driving_car==1)?cars[0]:cars[1];
		step = c.steer_speed*2.0f/fps;
		if (keys[SDL_SCANCODE_I]) {
			c.engine_force = c.engine_power;
			if (!last_time_gas){
				audio_switch_to_sound2();
				last_time_gas=true;
			}
		} else {
			if (last_time_gas){
				audio_switch_to_sound1();
				last_time_gas=false;
			}
			c.engine_force = 0.0f;
		}
		if (keys[SDL_SCANCODE_K]){
			c.engine_force=-c.engine_power;
		}
		if (keys[SDL_SCANCODE_L]) {
			c.volant_pos -= step;
			steering=true;
		}
		if (keys[SDL_SCANCODE_J]) {
			c.volant_pos += step;
			steering=true;
		}
		if (!steering){
			if (c.volant_pos>0.0f){
				c.volant_pos -= step;
			} else if (c.volant_pos<0.0f){
				c.volant_pos += step;
			}
		}
		c.volant_pos=clamp(c.volant_pos,-1.0f, 1.0f);
	}
	cars[driving_car].volant_pos=clamp(cars[driving_car].volant_pos,-1.0f, 1.0f);
	selected_car=clamp(selected_car,0,number_of_vehicles-1);



	return ret;
}
void run_essencials(){
    process_events(screen);
    // render_hud();
    render();
    SDL_GL_SwapWindow(window);
}
#endif
