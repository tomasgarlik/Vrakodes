int covered[COVERED_SIZE];
int i;
int j;
int k;
int l;
unsigned int textures[20];

std::vector<cardata> cars;
point p1;
point p2;
point p3;
point p4;

// float heightmap[MAP_SIZE][MAP_SIZE];
int mountain_texture=9;
float last_ch;
float volant_pos;
SDL_Window * screen;
SDL_Window * screenm;
SDL_Event event;
TTF_Font * font;
TTF_Font * font1;
int load_num_textures=1;
int display_text=0;
int ret=0;
float fps_time;
float y_rot=0.0f;
float x_rot=0.0f;
float x_pos;
float y_pos=5;
float z_pos;
float speed=0;
float last_time=0;
float now_time;
float this_y;
float ifl;
float jfl;
float kfl;
float lfl;
float shi;
int crashed;
int rubish;
int frame;
int prk=0;
int x_mouse;
int y_mouse;
int shift=0;
int num_textures[20];
int in_car_mode=0;
int bin1;
int bin2;
int bin3;
int bin4;
int bin5;
int bin6;
int bin7;
int bin8;
int bin9;
int bin69;
int result;
uint32_t timestart;
int framelenght;
float height0;
float height1;
float height2;
float height3;
int mouseX;
int mouseY;
int lastMouseX;
int lastMouseY;
int deltaX;
int deltaY;
Uint32 fps_last_time = 0;
int fps_frames = 0;
float fps = 0.0f;
float engine_force=0;
bool gas_down;
bool steering;
int generated;
// float h_heightmap[MAX_MAP_SIZE][MAX_MAP_SIZE];
float smooth_x_rot=0.0f;
float smooth_y_rot=0.0f;
collision_box collision_boxes[MAX_COLLISION_BOXES];
int collision_boxes_len=0;
int debug=0;
bool paused=false;
bool nextframe=false;
int menu_type=1;
SDL_Surface* vehicle_icons[MAX_VEHICLES];
SDL_Surface* icons[MAX_ICONS];
SDL_Surface* map_icons[MAX_MAPS];
SDL_Surface* hud_surf;
std::vector<std::string> car_names={};
int number_of_vehicles;
int selected_car=0;
int selected_map=0;
int moving_menu=0;
int menu_delta=0;
SDL_Rect dest;
int running;
SDL_Window *window;
SDL_Point mousepoint;
bool downed=false;
SDL_Surface * textSurface;
GLuint texture;
bool clic;
float blur_time=0.0f;
bool title=true;
GLfloat metal_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat other_specular[] = { 0.05f, 0.05f, 0.05f, 1.0f };
float w1;
float w2;
float w3;
float w4;
float tree_h;
float game_time=0.0f;
float game_start;
float last_x_pos;
float last_z_pos;
int last_fps_check_time=0.0f;;
float min_render_distance=50.0f;
int a;
int b;
int c;
int d;
int r;
int g;
int x_mousse;
int y_mousse;
float fi;
std::vector<roadpoint> roadpoints;
int roadpoints_len=0;
std::vector<roadpart> roadparts;
int roadparts_len=0;
char info_vendor[256]="meh";
char info_renderer[256]="some kind of GPU";
char info_vkdr[256]="IDK";
char info_glversion[256]="more than 1.0";
char warning_text[512]=" ";
std::vector<mesh> objects;
std::vector<mesh> lqobjects;
uint64_t process_checkpoints[50];
uint64_t process_times[50];
char process_names[][256]={
    "process_events",
    "car_mesh_deformation",
    "render",
    "some_stuff",
    "audio_update",
    "simulation",
    "bits_and_bobs"
};
uint64_t render_process_times[50];
uint64_t temp_render_process_times[50];
char render_process_names[][256]={
    "lighting_setup",
    "other_objects",
    "shadows",
    "car",
    "debug",
    "HUD",
    "waiting_for_GPU"
};
int checked_collision_boxes;
int collision_boxes_count;
bool generate_map;
bool map_to_be_generated;
int obj_textures[1000]={};
int obj_textures_count=0;
bool hud_debug_shall_be_rendered=false;
int last_hud_render_time=0;
int hud_render_interval=10;
bool force_hud_render=false;
float chaloupka_x=0.0f;
float chaloupka_z=0.0f;
float chaloupka_y=0.0f;
std::vector<mesh> static_objects;
float rerender_interval=0.1;
int frames_from_reload=-1;
float last_fps=50.0f;
std::vector<Vec2> chaloupky;
bool slomo=false;
int simulation_steps=100;
int driving_car=0;
bool last_time_gas=false;
float engine_power=200.0f;
int steering_type=0;
float max_steer=0.2f;
float steer_speed=1.0f;
std::string car_name="";
float niceni=0.0;
bool start_lock=false;
bool permission_to_add_car=false;
bool voxel_debug=true;
GLuint voxel_list;
float voxel_size=0.3f;
std::vector<Vec3> voxels;
char hud_text[1024];
GLuint white_list;
std::vector<MeshBuffer> staticObjectBuffers;
std::vector<MeshBuffer> staticBuffers;
GLuint normal_fallback;
bool shadowmap_debug=false;
GLuint generic_texture;
float NEAR=0.5f;
std::vector<std::vector<int>> cars_collided_faces;
float step_dt=1.0f;
bool auto_exposure=false;
float FOV=80.0f;
float default_fov=90.0f;
bool shall_calculate_collisions=true;
int collision_skip_rate=4;
int col_skip_frame=0;
bool post_process=true;
bool startpause=false;
int startpauseframes=0;
bool pause_zaloha=false;
std::vector<debug_obj> debug_data;
std::vector<RoadDebugTriangle> road_debug_triangles;
std::vector<RoadDebugSegment> road_debug_segments;
bool escape=false;
int last_menu_type=MENU_TYPE_INTRO;
float lod_factor=2.0f;
int terrain_lod_level_size=800;
int other_lod_level_size=100;
int chunks_loaded=0;
int max_chunks_loaded=10000000;
int chunk_deload_bulk=100;
bool mapgen=true; //true when the map is generating, affects the heightmap
int mapgen_spacing=50;
FastNoiseLite baseNoise;
FastNoiseLite lowNoise;
FastNoiseLite detailNoise;
FastNoiseLite veryDetailNoise; //for the mountain map
uint64_t cam_change_time=0;

std::vector<MenuNode> menu_items;
std::vector<int> current_root_items; // Indexy položek pro aktuální zobrazení

int current_category_index = -1;




// settings
bool vsync=true;
bool shadows=true;
bool antiastropic_filtering=true;
bool fullscreen=false;
bool highdpi=false;
int shader_level=1;
int renderer=1;
int default_hud_render_interval=200;
int tex_pres=5;
int STARTPAUSELENGTH=1;
int TREE_QUALITY_DISTANCE=20;
int MAP_SIZE=20000;
int HUD_WIDTH=800;
int HUD_HEIGHT=600;
int WIDTH=1280;
int HEIGHT=720;
int HD2=HEIGHT / 2;
int WD2=WIDTH / 2;
int SHADOW_RESOLUTION=2048;
float RAD_DEG=57.2957795131;
float PI=3.14159265358979323846264338327950288;
float SIMULATION_FREQ=2400.0f;
float shift_interval=500.0f;
float retina_scale=2.0f;
float min_fps=30.0f;
float render_distance=5000.0f;
float slomo_slowness=10.0f;
float light_ambient=0.3f;
float zoom_fov=25.0f;
float shadow_range=100.0f;
float shadow_jump=20.0f;
float exposure_multiplyer=1.5;





bool t_vsync = vsync, t_shadows = shadows, t_antiastropic_filtering = antiastropic_filtering, t_fullscreen = fullscreen, t_highdpi = highdpi;
int t_shader_level = shader_level, t_renderer = renderer, t_tex_pres = tex_pres, t_hud_render_interval = hud_render_interval, t_STARTPAUSELENGTH = STARTPAUSELENGTH, t_TREE_QUALITY_DISTANCE = TREE_QUALITY_DISTANCE, t_WIDTH = WIDTH, t_HEIGHT = HEIGHT, t_MAP_SIZE = MAP_SIZE, t_HUD_WIDTH = HUD_WIDTH, t_HUD_HEIGHT = HUD_HEIGHT, t_collision_skip_rate = collision_skip_rate, t_SHADOW_RESOLUTION = SHADOW_RESOLUTION;
float t_RAD_DEG = RAD_DEG, t_PI = PI, t_SIMULATION_FREQ = SIMULATION_FREQ, t_shift_interval = shift_interval, t_retina_scale = retina_scale, t_min_fps = min_fps, t_render_distance = render_distance, t_slomo_slowness = slomo_slowness, t_light_ambient = light_ambient, t_zoom_fov = zoom_fov, t_shadow_range = shadow_range, t_shadow_jump = shadow_jump, t_exposure_multiplyer = exposure_multiplyer;






int CHUNKS_SIZE=(MAP_SIZE/CHUNK_SIZE);
std::vector<std::vector<chunk*>> chunks;
std::vector<Vec2> major_road_points={};
