#ifdef _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_ttf.h>
    #include <SDL_mixer.h>
    #include <sys/stat.h> // not realy a sdl part, just some windows stuff
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_mixer.h>
#endif
// Pokud kompiluješ na starším SDL, které nemá SDL_GetTicks64
#if !SDL_VERSION_ATLEAST(2, 0, 18)
#define SDL_GetTicks64 SDL_GetTicks
#endif
