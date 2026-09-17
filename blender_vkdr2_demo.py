import os
import sys

import bpy
import bgl
import numpy as np
from mathutils import Matrix

ROOT = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else ""
if not os.path.isfile(os.path.join(ROOT, "vkdr2_renderer.so")):
    ROOT = "/Users/tomasgarlik/Documents/C/Nic/renderer-master"
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)

import vkdr2_renderer

renderer = None
_draw_handle = None

# Set these to real files before registering for textured output.
ALBEDO_PATH = "/Users/tomasgarlik/Documents/C/Nic/renderer-master/assets/cars/fecilia/texture.png"
ROUGHNESS_PATH = "/Users/tomasgarlik/Documents/C/Nic/renderer-master/assets/cars/fecilia/roughness.png"
DEFAULT_ROUGHNESS = 0.8


def cube_geometry():
    # Position, normal, UV. Faces use separate vertices where normals/UVs differ.
    vertices = np.array([
        (-1, -1,  1,  0,  0,  1, 0, 0), ( 1, -1,  1, 0, 0,  1, 1, 0),
        ( 1,  1,  1, 0,  0,  1, 1, 1), (-1,  1,  1, 0, 0,  1, 0, 1),
        ( 1, -1, -1, 0,  0, -1, 0, 0), (-1, -1, -1, 0,  0, -1, 1, 0),
        (-1,  1, -1, 0,  0, -1, 1, 1), ( 1,  1, -1, 0,  0, -1, 0, 1),
        (-1, -1, -1, -1, 0,  0, 0, 0), (-1, -1,  1, -1, 0, 0, 1, 0),
        (-1,  1,  1, -1, 0, 0, 1, 1), (-1,  1, -1, -1, 0, 0, 0, 1),
        ( 1, -1,  1,  1, 0,  0, 0, 0), ( 1, -1, -1,  1, 0, 0, 1, 0),
        ( 1,  1, -1,  1, 0, 0, 1, 1), ( 1,  1,  1, 1, 0, 0, 0, 1),
        (-1,  1,  1, 0,  1,  0, 0, 0), ( 1,  1,  1, 0, 1, 0, 1, 0),
        ( 1,  1, -1, 0, 1, 0, 1, 1), (-1,  1, -1, 0, 1, 0, 0, 1),
        (-1, -1, -1, 0, -1, 0, 0, 0), ( 1, -1, -1, 0, -1, 0, 1, 0),
        ( 1, -1,  1, 0, -1, 0, 1, 1), (-1, -1,  1, 0, -1, 0, 0, 1),
    ], dtype=np.float32)
    indices = np.array([
        0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
    ], dtype=np.uint32)
    return vertices, indices


def blender_matrix(matrix):
    return np.array(Matrix(matrix), dtype=np.float32)


def current_framebuffer():
    value = bgl.Buffer(bgl.GL_INT, 1)
    bgl.glGetIntegerv(bgl.GL_FRAMEBUFFER_BINDING, value)
    return int(value[0])


def draw_callback():
    if renderer is None:
        return

    context = bpy.context
    region = context.region
    region_data = context.region_data
    if region_data is None:
        return

    view = blender_matrix(region_data.view_matrix)
    projection = blender_matrix(region_data.window_matrix)
    framebuffer = current_framebuffer()
    renderer.set_camera(view, projection)
    renderer.render_frame(region.width, region.height, framebuffer)


def register():
    global renderer, _draw_handle
    if _draw_handle is not None:
        return

    renderer = vkdr2_renderer.BlenderRenderer(1024, 1024)
    renderer.init()

    vertices, indices = cube_geometry()
    renderer.upload_mesh(1, vertices, indices)
    renderer.upload_material(1, ALBEDO_PATH, DEFAULT_ROUGHNESS, ROUGHNESS_PATH)

    _draw_handle = bpy.types.SpaceView3D.draw_handler_add(
        draw_callback, (), "WINDOW", "POST_VIEW"
    )
    for area in bpy.context.screen.areas:
        if area.type == "VIEW_3D":
            area.tag_redraw()


def unregister():
    global renderer, _draw_handle
    if _draw_handle is not None:
        bpy.types.SpaceView3D.draw_handler_remove(_draw_handle, "WINDOW")
        _draw_handle = None
    if renderer is not None:
        renderer.cleanup()
        renderer = None


if __name__ == "__main__":
    register()
