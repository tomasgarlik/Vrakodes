#!/bin/bash
set -euo pipefail

BLENDER_PYTHON="/Applications/Blender_3.6.app/Contents/Resources/3.6/python/bin/python3.10"
PYTHON_INCLUDE="${PYTHON_INCLUDE:-/Library/Frameworks/Python.framework/Versions/3.10/include/python3.10}"

if [[ ! -x "$BLENDER_PYTHON" ]]; then
  echo "Blender Python not found: $BLENDER_PYTHON" >&2
  exit 1
fi

if [[ ! -f "$PYTHON_INCLUDE/Python.h" ]]; then
  echo "Python.h not found: $PYTHON_INCLUDE/Python.h" >&2
  echo "Install matching CPython 3.10 development headers and update PYTHON_INCLUDE." >&2
  exit 1
fi

PYBIND11_INCLUDE="$($BLENDER_PYTHON -c 'import pybind11; print(pybind11.get_include())')"

clang++ -std=c++20 -O2 -bundle -undefined dynamic_lookup -DBLENDER_BUILD \
  -I"$PYTHON_INCLUDE" \
  -I"$PYBIND11_INCLUDE" \
  -I. -Isrc \
  -Ilibs/SDL2.framework/Versions/A/Headers \
  -Ilibs/SDL2_image.framework/Versions/A/Headers \
  -Ilibs/SDL2_ttf.framework/Versions/A/Headers \
  blender_api.cpp \
  -F./libs \
  -framework OpenGL \
  -framework SDL2 \
  -framework SDL2_image \
  -framework SDL2_ttf \
  -Wl,-rpath,@loader_path/libs \
  -o vkdr2_renderer.so