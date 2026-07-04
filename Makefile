# ifeq ($(OS),Windows_NT)
#     # --- WINDOWS NASTAVENÍ ---
#     CXX = g++
#     SDL_PATH = SDL2_windows
#     OUT = program_win64.exe
#     CXXFLAGS = -std=c++11 -I$(SDL_PATH)/include -I$(SDL_PATH)/include/SDL2 -Ofast -march=native -DNDEBUG
#     LDFLAGS = -L$(SDL_PATH)/lib -lglew32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lopengl32 -lm
#     RM = del
# else
#     # --- MAC NASTAVENÍ ---
#     CXX = clang++
#     OUT = program
#     CXXFLAGS = -std=c++11 -Flibs -Ofast -march=haswell -flto -funroll-loops -fomit-frame-pointer -DNDEBUG   #  -fsanitize=address -g
#     LDFLAGS = -Flibs -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -Wl,-rpath,@executable_path/libs -framework OpenGL -lm -DGL_SILENCE_DEPRECATION -w
#     RM = rm -f
# endif

# SRC = src/main.cpp

# all:
# 	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

# clean:
# 	$(RM) $(OUT)
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
    
    # Tady definujeme příkaz pro kompilaci (na Windows dvoukrokově)
    COMPILE_AND_LINK = \
        $(CXX) $(CXXFLAGS) -c src/main.cpp -o main.o && \
        g++ main.o -o $(OUT) $(LDFLAGS)
    
    CLEAN_FILES = $(OUT) *.o
else
    # --- MAC NASTAVENÍ ---
    CXX = clang++
    OUT = program
    CXXFLAGS = -std=c++20 -Flibs -Ofast -march=haswell -flto -funroll-loops -fomit-frame-pointer -DNDEBUG
    LDFLAGS = -Flibs -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -Wl,-rpath,@executable_path/libs -framework OpenGL -lm -DGL_SILENCE_DEPRECATION -w
    RM = rm -f
    
    # Na Macu to Clang sfoukne rovnou všechno v jednom kroku
    COMPILE_AND_LINK = $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
    CLEAN_FILES = $(OUT)
endif

# --- SPOLEČNÁ PRAVIDLA PRO OBAL SYSTÉMY ---
# (Pravidla musí být venku z ifeq podmínek)

all:
	$(COMPILE_AND_LINK)

clean:
	$(RM) $(CLEAN_FILES)