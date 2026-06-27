if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Somthing fucked up during SDL2 initalization.",
        SDL_GetError(),
        nullptr // žádné okno zatím nemáme
    );
    return 1;
}
if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "SDL2_image initialization failed",
        IMG_GetError(),
        nullptr
    );
    SDL_Log("IMG_Init failed: %s", IMG_GetError());
    return 1;
}

SDL_Rect rect;
TTF_Init();
font = TTF_OpenFont("fonts/font3.ttf", 100);
SDL_Window* splashscreen = SDL_CreateWindow("Splash Screen",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    600, 275, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI);
if (!splashscreen) {
    SDL_Log("Window creation failed: %s", SDL_GetError());
    return 1;
}
SDL_Renderer* rendere = SDL_CreateRenderer(splashscreen, -1, SDL_RENDERER_ACCELERATED);
if (!rendere) {
    SDL_Log("Renderer creation failed: %s", SDL_GetError());
    SDL_DestroyWindow(splashscreen);
    return 1;
}
SDL_Texture* texture = loadTexture("icons/splashscreen.png", rendere);
SDL_RenderClear(rendere);
SDL_RenderCopy(rendere, texture, NULL, NULL);
SDL_RenderPresent(rendere);
Uint32 startTime = SDL_GetTicks();
running=true;
while (running) {
    SDL_Event event0;
    while (SDL_PollEvent(&event0)) {
        if (event0.type == SDL_QUIT) {
            running = false;
        }
    }
    if (SDL_GetTicks() - startTime > SPLASHSCREENTIME) {
        running = false;
    }
}
SDL_DestroyTexture(texture);
SDL_DestroyRenderer(rendere);
// SDL_DestroyWindow(splashscreen);
#ifdef RETINA_DISPLAY
Uint32 window_flags = SDL_WINDOW_SHOWN | 
                    SDL_WINDOW_RESIZABLE | 
                    SDL_WINDOW_MAXIMIZED | 
                    SDL_WINDOW_ALLOW_HIGHDPI;
retina=true;
#else 
Uint32 window_flags = SDL_WINDOW_SHOWN | 
                    SDL_WINDOW_RESIZABLE | 
                    SDL_WINDOW_MAXIMIZED;
retina=false;
#endif
if (retina){
    scale=2.0;
} else {
    scale=1.0;
}
SDL_Window* window = SDL_CreateWindow(
    "Vrakodes vehicle editor",
    SDL_WINDOWPOS_CENTERED, 
    SDL_WINDOWPOS_CENTERED,
    800, 600,  // při MAXIMIZED se ignoruje
    window_flags
);
if (!window) {
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Something fucked up while creating a window.",
        SDL_GetError(),
        nullptr
    );
    SDL_Quit();
    return 1;
}
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

if (!renderer) {
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Something fucked up while trying to show something in the window.",
        SDL_GetError(),
        nullptr
    );
    SDL_Quit();
    return 1;
}
dropmenu filemenu = {
    .functions={
        newfunc,
        openfunc,
        saveasfunc,
        load_obj,
        deduplicate_func,
        quitfunc
    },
    .textures={
        createTextTexture(renderer,"New"),
        createTextTexture(renderer,"Open ..."),
        createTextTexture(renderer,"Save as..."),
        createTextTexture(renderer,"Import OBJ..."),
        createTextTexture(renderer,"Deduplicate"),
        createTextTexture(renderer,"Quit")
    },
    .lenght=6,
    .title=createTextTexture(renderer, "File"),
    .titletype="defined",
    .xpos=0,
    .ypos=0,
    .width=getTextureWidth(createTextTexture(renderer,"Import OBJ...    "), STANDARTPICEHEIGHT-STANDARTBORDER*2),
    .topwidth=60,
    .dashable=false
};
// textbox testbox = {
//     .xpos=300,
//     .ypos=200,
//     .text="Hello guis today we have a list of",
//     .active=false,
//     .textsize=30
// };
SDL_Color cc={255,100,20,255};
SDL_Event event;
running=true;
SDL_StartTextInput();
SDL_SetWindowIcon(window, IMG_Load("icons/icon.png"));
making_new_part=false;