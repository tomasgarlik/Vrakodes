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
#include <thread>
#include <iomanip>
#include <sstream>
#include "sdl_includes.h"
#include "opengl_include.h"
#include <algorithm>
#include "../libs/fastnoiselite.h"
#include "constants.h"
#include "types.hpp"
#include "global_vars.hpp"
#define INCLUDES
#include "function_definitions.h"
#include "debug.hpp"
#include "general_functions.hpp"
#include "events.hpp"
#include "gui.hpp"
#include "renderer.hpp"
#include "map.hpp"
#include "heightmap.hpp"
#include "road_mesh.hpp"
#include "heightmap_roads.hpp"
#include "car.hpp"
#include "softsoft.hpp"
#include "fysics.hpp"
#include "sound.hpp"
float bin;
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
int main(int argc, char *argv[]) {
	load_settings();
	printf("sdl init...\n");
    // Inicializace SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
	if (renderer==0){
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	} else {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	}
    // Vytvoření okna
	printf("ttf init...\n");

	TTF_Init();

	font = TTF_OpenFont("fonts/font.ttf", 50);
	font1 = TTF_OpenFont("fonts/font.ttf", 10);

	if (!font) {
        printf("TTF error: %s\n", TTF_GetError());
    } else {
		printf("TTF font loaded succesfully.\n");
	}
	printf("window init...\n");
	Uint32 flags = SDL_WINDOW_SHOWN | 
				SDL_WINDOW_OPENGL | 
				SDL_WINDOW_MAXIMIZED;
	if (fullscreen) {
		flags=flags | SDL_WINDOW_FULLSCREEN;
	}
	if (highdpi){
		flags=flags | SDL_WINDOW_ALLOW_HIGHDPI;
	}
    window = SDL_CreateWindow("Vrakodes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, flags);
    if (!window) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
	printf("glcontex init...\n");
    // Vytvoření OpenGL kontextu
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        printf("SDL_GL_CreateContext error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

	#ifndef __APPLE__
		printf("initing glew (windows)...\n");
		glewExperimental = GL_TRUE; // Nutné pro moderní ovladače
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			printf("Chyba: GLEW se nepovedlo nastartovat: %s\n", glewGetErrorString(err));
			return 1;
		}
	#endif


	if (vsync){
		SDL_GL_SetSwapInterval(1);
	}
    SDL_Init(SDL_INIT_AUDIO);
	audio_init();
	Mix_Chunk* test = Mix_LoadWAV("assets/cars/traktor/engine1.wav");
	if (!test) {
		printf("Load failed: %s\n", Mix_GetError());
	} else {
		printf("Loaded successfully\n");
		int ch = Mix_PlayChannel(-1, test, 0); // Play once, not looping
		printf("Playing on channel: %d\n", ch);
		// SDL_Delay(3000); // Wait 3 seconds to hear it
	}
	glClearColor(col(10),col(10),col(190),1.0);
	setup_renderer(WIDTH, HEIGHT);
	init_map_memory();
	load_textures();
	// gen_heightmap();
	GLuint *texttextures = NULL;
	x_pos=100.0f;
	y_pos=1.0f;
	z_pos=100.0f;
	x_rot=45.0f;
	driving_car=0;
	start_lock=true;
	GLuint map_list_terrain;
	GLuint map_list_other;
	GLuint white_list;
    SDL_ShowCursor(SDL_TRUE);
	load_textures();
    SDL_Event event;
	discoverVehicles();
	loadVehicleIcons();
	load_map_icons();
	load_icons();
	hud_surf = SDL_CreateRGBSurfaceWithFormat(0, 800, 600, 32, SDL_PIXELFORMAT_BGRA32);
	volant_pos=0.0f;
	printf("creating car\n");
	// cars.push_back({});
	cars.emplace_back();
	printf("naming car\n");
	strcpy(cars[driving_car].name, "sportcar");
	printf("\n");
	reset_vehicle(cars[driving_car]);
	load_car(cars[driving_car]);
	load_car_mesh(cars[driving_car]);
	load_objects();
	// gen_map_other();
	cars[driving_car].deformation_system=0;
	// calculate_car_deformation();
	// init_vehicle(40.0f, 35.0f);
	// init_collision_boxes();
	last_x_pos=x_pos;
	last_z_pos=z_pos;
	if (renderer==0){
		white_list = rend_white();
	}
	get_gl_info();
    running = 1;
	map_to_be_generated=false;
	printf("starting main loop\n");
	x_pos=30;
	z_pos=30;
	y_pos=5;
    while (running) {
        timestart = SDL_GetTicks64();
		uint64_t last_t=timestart;
		generate_map=false;
		nextframe=false;
		cars[driving_car].niceni=niceni;
		debug_data.clear();
		if (startpause){
			if (startpauseframes>STARTPAUSELENGTH){
				startpause=false;
				paused=pause_zaloha;
			} else {
				paused=true;
			}
			startpauseframes++;
		}
		log("processing events");
		result = process_events(screen);
		process_times[0] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		log("update car mesh");
		update_car_mesh();
		log("recalculate normals");
		recalculate_normals();
		process_times[1] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		log("render hud");
		if (render_hud()){
			printf("reloading game stuff\n");
			if (generate_map){
				mapgen=true;
				printf("regenerating map\n");
				load_settings();
				if (selected_map==0){
					tex_pres=1;
					render_distance=300.0f;
					rerender_interval=0.7f;
				}
				obj_textures_count=0;
				objects.clear();
				lqobjects.clear();
				load_objects();
				clear_chunks();
				init_gen_heightmap();
				if (selected_map!=MAP_TESTMAP){
        			gen_roads();
				}
				// gen_heightmap();
				gen_map_other();
				update_static_objects();
				load_textures();
				generate_map=false;
				if (selected_map==2){
					clear_chunks_heightmap();
				}
				mapgen=false;
				gen_heightmap_roads();
				init_collision_boxes();
				if (selected_map==1){
					x_pos=3000;
					z_pos=3000;
					y_pos=get_heightmap_height(x_pos, z_pos)+2.0f;
				} else if (selected_map==0){
					x_pos=40.0f;
					y_pos=1.0f;
					z_pos=35.0f;
				} else if (selected_map==2){
					x_pos=3000;
					z_pos=3000;
					y_pos=get_heightmap_height(x_pos, z_pos)+2.0f;
				}
				in_car_mode=1;
				update_static_objects();
			}
			if (permission_to_add_car){
				printf("adding car");
				permission_to_add_car=false;
				driving_car++;
				// cars.push_back({});
				cars.emplace_back();
			}
			printf("copying car name (car_names.size() returns %d)\n", car_names.size());
			int global_idx = (current_category_index == -1) 
				? current_root_items[selected_car] 
				: menu_items[current_category_index].sub_items[selected_car];

			// 2. A až teď vytáhneme správné jméno pro zobrazení v HUDu
			car_name = car_names[global_idx];
			printf("reseting car\n");
			reset_vehicle(cars[driving_car]);
			// x_shift=0.0f;
			// z_shift=0.0f;
			printf("load car");
			load_car(cars[driving_car]);
			printf("load car mesh");
			load_car_mesh(cars[driving_car]);
			calculate_car_deformation();
			init_vehicle(cars[driving_car], z_pos,x_pos,y_pos);
			bool spawned=false;
			float upshift=0.0f;
			while (!spawned){
				spawned=true;
				for (auto& p:cars[driving_car].points){
					Vec3 pos;
					pos.x=p.x;
					pos.y=p.y;
					pos.z=p.z;
					if (p.collide){
						calculate_terrain_collisions(p, 0.1f);
						calculate_OBB_collisions(p, 0.1f, cars[driving_car].x_shift, cars[driving_car].z_shift);
					}
					if (!(p.x==pos.x && p.y==pos.y && p.z==pos.z)){
						spawned=false;
						p.x=pos.x;
						p.y=pos.y;
						p.z=pos.z;
						break;
					}
				}
				break;
				for (auto& p:cars[driving_car].points){
					p.y+=upshift;
				}
				printf("upshifting %f\n", upshift);
				upshift+=0.2f;
			}
			create_car_buffers();

			printf("initing this cars audio stuff\n");
			audio_load_sound1();
			audio_load_sound2();
			audio_switch_to_sound1();
			audio_start_loop();
			game_start=(float)SDL_GetTicks64()/1000.0f;
			printf("starting game\n");
			pause_zaloha=paused;
			startpause=true;
			startpauseframes=0;
			paused=true;
		}
		log("render hud done, setting camera view");
		if (in_car_mode!=0){
			if (in_car_mode==1){
				float target_x_rot = -get_car_rotation_y(cars[driving_car]) * RAD_DEG - 270.0f;
				float rot_smoothness = 2.0f/fps;
				float max_sync_limit = 30.0f; // Maximální povolený rozdíl ve stupních
				float diff = target_x_rot - smooth_x_rot;
				while (diff < -180.0f) diff += 360.0f;
				while (diff > 180.0f)  diff -= 360.0f;
				if (diff > max_sync_limit)  diff = max_sync_limit;
				if (diff < -max_sync_limit) diff = -max_sync_limit;
				smooth_x_rot += diff * rot_smoothness;
				x_rot = smooth_x_rot; // Tady to přepíšeme, aby zbytek tvého kódu fungoval
				y_rot = -10.0f;       // To máš zafixované pro sklon
				z_pos = cars[driving_car].pos_z + (sin(-x_rot / RAD_DEG) * -cars[driving_car].v3rd.x);
				x_pos = cars[driving_car].pos_x - (cos(x_rot / RAD_DEG) * -cars[driving_car].v3rd.x);
				y_pos = cars[driving_car].pos_y + cars[driving_car].v3rd.y;


				if (y_pos<get_heightmap_height(x_pos,z_pos)+0.2f){
					y_pos=get_heightmap_height(x_pos,z_pos)+0.2f;
				}
			} else if (in_car_mode==2){
				float target_x_rot = -get_car_rotation_y(cars[driving_car]) * RAD_DEG - 270.0f;
				float rot_smoothness = 10.0f/fps; // 0.05 = plynulé, 1.0 = okamžité (locked)
				float max_sync_limit = 30.0f; // Maximální povolený rozdíl ve stupních
				float diff = target_x_rot - smooth_x_rot;
				while (diff < -180.0f) diff += 360.0f;
				while (diff > 180.0f)  diff -= 360.0f;
				if (diff > max_sync_limit)  diff = max_sync_limit;
				if (diff < -max_sync_limit) diff = -max_sync_limit;
				smooth_x_rot += diff * rot_smoothness;
				x_rot = smooth_x_rot; // Tady to přepíšeme, aby zbytek tvého kódu fungoval


				float target_y_rot = -get_raw_car_rotation_x(cars[driving_car]) * RAD_DEG - 270.0f;
				diff = target_y_rot - smooth_y_rot;
				while (diff < -180.0f) diff += 360.0f;
				while (diff > 180.0f)  diff -= 360.0f;
				if (diff > max_sync_limit)  diff = max_sync_limit;
				if (diff < -max_sync_limit) diff = -max_sync_limit;
				smooth_y_rot += diff * rot_smoothness;
				y_rot = -smooth_y_rot+90.0f;


				// z_pos = cars[driving_car].pos_z +	(sin(-x_rot / RAD_DEG) * -cars[driving_car].vcab.x)+
				// 									(sin((-x_rot-90.0f) / RAD_DEG) * -cars[driving_car].vcab.z);

				// x_pos = cars[driving_car].pos_x -	(cos(x_rot / RAD_DEG) * -cars[driving_car].vcab.x)+
				// 									(cos((x_rot-90.0f) / RAD_DEG) * -cars[driving_car].vcab.z);

				// y_pos = cars[driving_car].pos_y + cars[driving_car].vcab.y;

				float yaw   =  x_rot / RAD_DEG;
float pitch = -y_rot / RAD_DEG;

float cx = -cars[driving_car].vcab.x;
float cy =  cars[driving_car].vcab.y;
float cz = -cars[driving_car].vcab.z;

z_pos = cars[driving_car].pos_z
      + sin(-x_rot / RAD_DEG)      * cx
      + sin((-x_rot - 90.0f) / RAD_DEG) * cz * cos(pitch)
      + sin(pitch) * cy;           // pitch tilts z toward y

x_pos = cars[driving_car].pos_x
      - cos(x_rot / RAD_DEG)       * cx
      + cos((x_rot - 90.0f) / RAD_DEG)  * cz * cos(pitch)
      + 0;                         // pitch doesn't affect x directly

y_pos = cars[driving_car].pos_y
      + cy
      + cz * sin(pitch);           // forward offset tilts up/down with pitch
				if (y_pos<get_heightmap_height(x_pos,z_pos)+0.2f){
					y_pos=get_heightmap_height(x_pos,z_pos)+0.2f;
				}


				
			}

		}
		log("done");
		render();
		log("swapwindow");
		SDL_GL_SwapWindow(window);
		process_times[2] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		int render_process_sum=0;
		for (i=0;i<RENDER_PROCESS_COUNT-1;i++){
			render_process_sum+=render_process_times[i];
		}
		render_process_times[RENDER_PROCESS_COUNT-1]=process_times[2]-render_process_sum;
		process_times[3] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		log("audio");
		audio_set_speed(1.0f+cars[driving_car].velocity/cars[driving_car].sound_divider);
		audio_update();
		process_times[4] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		//draw_hud(data_on_screen);
		if (frames_from_reload!=-1){
			fps=last_fps;
		}
		log("simulation");
		if (!paused && menu_type==0){
			if (slomo){
				step_simulation((1.0f/fps)/slomo_slowness);
			} else {
				if (fps>=20.0f){
					step_simulation(1.0f/fps);
				} else {
					step_simulation(0.05);
					snprintf(warning_text, sizeof(warning_text), "! Your FPS is to low to run the simulation in real time !\nThe minimum FPS to run in real time is 20.");
				}
			}
		} else if (nextframe && menu_type==0){
			step_simulation(1.0f/SIMULATION_FREQ);
		}
		process_times[5] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		game_time=((float)timestart/1000.0f)-game_start;
		if (abs(last_x_pos-x_pos)>render_distance*rerender_interval || abs(last_z_pos-z_pos)>render_distance*rerender_interval){
			update_static_objects();
			last_x_pos=x_pos;
			last_z_pos=z_pos;
		}
		if (frames_from_reload!=-1){
			if (frames_from_reload<10){
				frames_from_reload++;
			} else {
				frames_from_reload=-1;
			}
		}
		voxel_debug=(debug==3);
		process_times[6] = SDL_GetTicks64() - last_t;
		last_t = SDL_GetTicks64();
		framelenght = SDL_GetTicks64() - timestart;
		fps_update();
    }
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
