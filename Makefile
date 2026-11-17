SRC = src/main.cpp \
      src/imgui/imgui.cpp \
      src/imgui/imgui_draw.cpp \
      src/imgui/imgui_tables.cpp \
      src/imgui/imgui_widgets.cpp \
      src/imgui/backends/imgui_impl_sdl2.cpp \
      src/imgui/backends/imgui_impl_opengl3.cpp

# Detekce operačního systému a nastavení proměnných
ifeq ($(OS),Windows_NT)
    # --- WINDOWS NASTAVENÍ ---
    CXX = g++
    SDL_PATH = SDL2_windows
    OUT = program_win64.exe
    CXXFLAGS = -std=c++20 -I$(SDL_PATH)/include -I$(SDL_PATH)/include/SDL2 -Isrc/imgui -Isrc/imgui/backends -Ofast -march=native -DNDEBUG -w
    LDFLAGS = -static -L$(SDL_PATH)/lib -lglew32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lopengl32 -lcomdlg32 -lole32 -luuid -lgdi32 -limm32 -lwinmm -lversion -lcfgmgr32 -lsetupapi -lrpcrt4 -loleaut32 -lm
    RM = rm -f
    
    COMPILE_AND_LINK = $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
    CLEAN_FILES = $(OUT) *.o
else
    # Detekce Unix systémů (Mac vs Raspberry Pi / Linux)
    UNAME_S := $(shell uname -s)

    ifeq ($(UNAME_S),Darwin)
        # --- MAC NASTAVENÍ ---
        CXX = clang++
        OUT = program
        CXXFLAGS = -std=c++20 -Flibs -Ilibs/SDL2.framework/Headers -Isrc/imgui -Isrc/imgui/backends -Ofast -march=haswell -flto -funroll-loops -fomit-frame-pointer -DNDEBUG
        LDFLAGS = -Flibs -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -Wl,-rpath,@executable_path/libs -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -DGL_SILENCE_DEPRECATION -w
        RM = rm -f
        
        COMPILE_AND_LINK = $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
        CLEAN_FILES = $(OUT)
    else
        # --- RASPBERRY PI / LINUX NASTAVENÍ ---
        CXX = g++
        OUT = program_rpi
        CXXFLAGS = -std=c++2a -Isrc/imgui -Isrc/imgui/backends -Ofast -march=native -flto -funroll-loops -fomit-frame-pointer -DNDEBUG -w
        LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lGL -lGLEW -lm
        RM = rm -f
        
        COMPILE_AND_LINK = $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
        CLEAN_FILES = $(OUT)
    endif
endif

# --- SPOLEČNÁ PRAVIDLA PRO OBA SYSTÉMY ---
.PHONY: all clean

all:
	$(COMPILE_AND_LINK)

clean:
	$(RM) $(CLEAN_FILES)