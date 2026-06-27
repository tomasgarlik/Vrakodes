CXX=clang++
CXXFLAGS=-std=c++11 `sdl2-config --cflags` -Ofast
LDFLAGS=`sdl2-config --libs` -lSDL2_image -lSDL2_ttf
SRC=src/main.cpp src/tinyfiledialogs.cpp
OUT=Vehicle_editor

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)

