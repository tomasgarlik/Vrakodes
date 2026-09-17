#!/bin/bash
set -euo pipefail

clang++ -std=c++20 -O2 -DGL_SILENCE_DEPRECATION \
  -I. -Isrc \
  -Ilibs/SDL2.framework/Versions/A/Headers \
  -Ilibs/SDL2_image.framework/Versions/A/Headers \
  -Ilibs/SDL2_ttf.framework/Versions/A/Headers \
  vkdr2_preview.cpp \
  -F./libs \
  -framework OpenGL \
  -framework SDL2 \
  -framework SDL2_image \
  -framework SDL2_ttf \
  -Wl,-rpath,@executable_path/libs \
  -o vkdr2_preview
