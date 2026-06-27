#ifdef _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_ttf.h>
    #include <SDL_mixer.h>
    #include <sys/stat.h> // not realy a sdl part, just some windows stuff ( i dont like windows )
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_mixer.h>
#endif