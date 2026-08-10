#include <stdio.h>
#include <SDL2/SDL.h>
#include <unistd.h>
int main(){

    SDL_Init(SDL_INIT_VIDEO);
    int flags = SDL_WINDOW_FULLSCREEN;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetSwapInterval(1);
	SDL_Window* screen = SDL_CreateWindow("Fixing screen resolution...",
						-2,
						-36,
						1920,
						1080,
						flags);

    SDL_Renderer* renderer=SDL_CreateRenderer(screen,-1,SDL_RENDERER_ACCELERATED);

		SDL_RenderPresent(renderer);

}
