SRC = src/main.cpp

# Detekce operačního systému a nastavení proměnných
ifeq ($(OS),Windows_NT)
    # --- WINDOWS NASTAVENÍ ---
    CXX = C:/Users/Tomas/Documents/clang/bin/clang++.exe
    SDL_PATH = SDL2_windows
    OUT = program_win64.exe
    CXXFLAGS = -std=gnu++20 -I$(SDL_PATH)/include -I$(SDL_PATH)/include/SDL2 -Ofast -march=native -DNDEBUG --target=x86_64-w64-mingw32 -w
    LDFLAGS = -static -L$(SDL_PATH)/lib -lglew32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lopengl32 -lcomdlg32 -lole32 -luuid -lgdi32 -limm32 -lwinmm -lversion -lcfgmgr32 -lsetupapi -lrpcrt4 -loleaut32 -lm #-mwindows
    RM = del /Q /F
    
    COMPILE_AND_LINK = \
        $(CXX) $(CXXFLAGS) -c src/main.cpp -o main.o && \
        g++ main.o -o $(OUT) $(LDFLAGS)
    
    CLEAN_FILES = $(OUT) *.o
else
    # Detekce Unix systémů (Mac vs Raspberry Pi / Linux)
    UNAME_S := $(shell uname -s)
    UNAME_M := $(shell uname -m)

    ifeq ($(UNAME_S),Darwin)
        # --- MAC NASTAVENÍ ---
        CXX = clang++
        OUT = program
        CXXFLAGS = -std=c++20 -Flibs -Ofast -march=haswell -flto -funroll-loops -fomit-frame-pointer -DNDEBUG
        LDFLAGS = -Flibs -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -Wl,-rpath,@executable_path/libs -framework OpenGL -lm -DGL_SILENCE_DEPRECATION -w
        RM = rm -f
        
        COMPILE_AND_LINK = $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
        CLEAN_FILES = $(OUT)
    else
        # --- RASPBERRY PI / LINUX NASTAVENÍ ---
        CXX = g++
        OUT = program_rpi
        # Pro starší 32-bit RPi OS použij -march=armv7-a, pro 64-bit (RPi 4/5) -march=armv8-a. 'native' to vyřeší samo.
        CXXFLAGS = -std=c++2a -Ofast -march=native -flto -funroll-loops -fomit-frame-pointer -DNDEBUG -w
        # Na Linuxu/RPi se knihovny linkují standardně ze systému
        LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lGL -lGLEW -lm
        RM = rm -f
        
        COMPILE_AND_LINK = $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
        CLEAN_FILES = $(OUT)
    endif
endif

# --- SPOLEČNÁ PRAVIDLA PRO OBAL SYSTÉMY ---
all:
	$(COMPILE_AND_LINK)

clean:
	$(RM) $(CLEAN_FILES)
