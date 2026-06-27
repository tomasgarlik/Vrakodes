# CXXFLAGS=-std=c++11 -Flibs -Ofast -march=native -flto -funroll-loops -fomit-frame-pointer -DNDEBUG
# LDFLAGS=-Flibs -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -Wl,-rpath,@executable_path/libs -framework OpenGL -lm -DGL_SILENCE_DEPRECATION -w
# SRC=src/main.cpp
# OUT=program

# all:
# 	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

# clean:
# 	rm -f $(OUT)
# Detekce operačního systému
ifeq ($(OS),Windows_NT)
    # --- WINDOWS NASTAVENÍ ---
    CXX = g++
    SDL_PATH = SDL2_windows
    OUT = program_win64.exe
    CXXFLAGS = -std=c++11 -I$(SDL_PATH)/include -I$(SDL_PATH)/include/SDL2 -Ofast -march=native -DNDEBUG
    LDFLAGS = -L$(SDL_PATH)/lib -lglew32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lopengl32 -lm
    RM = del
else
    # --- MAC NASTAVENÍ ---
    CXX = clang++
    OUT = program
    CXXFLAGS = -std=c++11 -Flibs -Ofast -march=haswell -flto -funroll-loops -fomit-frame-pointer -DNDEBUG   #  -fsanitize=address -g
    LDFLAGS = -Flibs -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -Wl,-rpath,@executable_path/libs -framework OpenGL -lm -DGL_SILENCE_DEPRECATION -w
    RM = rm -f
endif

SRC = src/main.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	$(RM) $(OUT)
