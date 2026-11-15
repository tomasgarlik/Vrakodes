#include <iostream>
#include <SDL.h>
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

// Hlavičkové soubory ImGui (podle toho, kde máš složku imgui)
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

int main(int argc, char* argv[]) {
    // 1. Inicializace SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Chyba inicializace SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Nastavení OpenGL atributů před vytvořením okna (OpenGL 3.3 Core)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Vytvoření SDL okna s OpenGL kontextem
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Moje ImGui + SDL2 + OpenGL Aplikace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (!window) {
        std::cerr << "Chyba při vytváření SDL okna: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Zapnout Vsync (synchronizace s obnovovací frekvencí monitoru)

    // 2. Inicializace Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Povolit ovládání klávesnicí

    // Nastavení vzhledu (tmavý motiv)
    ImGui::StyleColorsDark();

    // Inicializace backendů pro SDL2 a OpenGL 3
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Hlavní smyčka programu
    bool running = true;
    while (running) {
        // Zpracování událostí (vstupy z klávesnice, myši, zavření okna)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // DŮLEŽITÉ: Předat eventy do ImGui
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                running = false;
        }

        // 3. Začátek nového ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // --- TADY SI DEFINUJEŠ SVÉ UI ---

        // A) Oficiální demo okno (ukáže všechno, co ImGui umí)
        ImGui::ShowDemoWindow();

        // B) Tvoje vlastní ukázkové okno
        {
            static float slider_value = 0.5f;
            static int click_counter = 0;

            ImGui::Begin("Moje Herni Menu / Inventar"); // Název okna

            ImGui::Text("Ahoj, tohle je moje custom okno v OpenGL!");
            ImGui::SliderFloat("Rychlost", &slider_value, 0.0f, 1.0f);

            if (ImGui::Button("Klikni na mne!")) {
                click_counter++;
            }
            ImGui::SameLine();
            ImGui::Text("Pocet kliknuti: %d", click_counter);

            ImGui::Text("Aktualni FPS: %.1f", io.Framerate);

            ImGui::End();
        }

        // ---------------------------------

        // 4. Vykreslení ImGui dat
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.15f, 1.00f); // Barva pozadí okna (tmavě modrá/šedá)
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Prohození bufferů (zobrazení na obrazovku)
        SDL_GL_SwapWindow(window);
    }

    // 5. Úklid při ukončování programu
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}