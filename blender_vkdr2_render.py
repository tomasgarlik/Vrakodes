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

ENGINE_ID = "VKDR2_RENDER"
renderer = None
_registered = False

ALBEDO_PATH = "/Users/tomasgarlik/Documents/C/Nic/renderer-master/assets/cars/fecilia/texture.png"
ROUGHNESS_PATH = "/Users/tomasgarlik/Documents/C/Nic/renderer-master/assets/cars/fecilia/roughness.png"
DEFAULT_ROUGHNESS = 0.8


def cube_geometry():
    vertices = np.array([
        (-1, -1,  1, 0, 0, 1, 0, 0), (1, -1, 1, 0, 0, 1, 1, 0),
        (1, 1, 1, 0, 0, 1, 1, 1), (-1, 1, 1, 0, 0, 1, 0, 1),
        (1, -1, -1, 0, 0, -1, 0, 0), (-1, -1, -1, 0, 0, -1, 1, 0),
        (-1, 1, -1, 0, 0, -1, 1, 1), (1, 1, -1, 0, 0, -1, 0, 1),
        (-1, -1, -1, -1, 0, 0, 0, 0), (-1, -1, 1, -1, 0, 0, 1, 0),
        (-1, 1, 1, -1, 0, 0, 1, 1), (-1, 1, -1, -1, 0, 0, 0, 1),
        (1, -1, 1, 1, 0, 0, 0, 0), (1, -1, -1, 1, 0, 0, 1, 0),
        (1, 1, -1, 1, 0, 0, 1, 1), (1, 1, 1, 1, 0, 0, 0, 1),
        (-1, 1, 1, 0, 1, 0, 0, 0), (1, 1, 1, 0, 1, 0, 1, 0),
        (1, 1, -1, 0, 1, 0, 1, 1), (-1, 1, -1, 0, 1, 0, 0, 1),
        (-1, -1, -1, 0, -1, 0, 0, 0), (1, -1, -1, 0, -1, 0, 1, 0),
        (1, -1, 1, 0, -1, 0, 1, 1), (-1, -1, 1, 0, -1, 0, 0, 1),
    ], dtype=np.float32)
    indices = np.array([
        0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
    ], dtype=np.uint32)
    return vertices, indices


def create_fbo(width, height):
    fbo = bgl.Buffer(bgl.GL_INT, 1)
    color = bgl.Buffer(bgl.GL_INT, 1)
    depth = bgl.Buffer(bgl.GL_INT, 1)

    bgl.glGenFramebuffers(1, fbo)
    bgl.glGenTextures(1, color)
    bgl.glGenRenderbuffers(1, depth)

    bgl.glBindTexture(bgl.GL_TEXTURE_2D, color[0])
    bgl.glTexImage2D(
        bgl.GL_TEXTURE_2D, 0, bgl.GL_RGBA32F, width, height, 0,
        bgl.GL_RGBA, bgl.GL_FLOAT, None
    )
    bgl.glTexParameteri(bgl.GL_TEXTURE_2D, bgl.GL_TEXTURE_MIN_FILTER, bgl.GL_LINEAR)
    bgl.glTexParameteri(bgl.GL_TEXTURE_2D, bgl.GL_TEXTURE_MAG_FILTER, bgl.GL_LINEAR)

    bgl.glBindRenderbuffer(bgl.GL_RENDERBUFFER, depth[0])
    bgl.glRenderbufferStorage(bgl.GL_RENDERBUFFER, bgl.GL_DEPTH_COMPONENT24, width, height)

    bgl.glBindFramebuffer(bgl.GL_FRAMEBUFFER, fbo[0])
    bgl.glFramebufferTexture2D(
        bgl.GL_FRAMEBUFFER, bgl.GL_COLOR_ATTACHMENT0,
        bgl.GL_TEXTURE_2D, color[0], 0
    )
    bgl.glFramebufferRenderbuffer(
        bgl.GL_FRAMEBUFFER, bgl.GL_DEPTH_ATTACHMENT,
        bgl.GL_RENDERBUFFER, depth[0]
    )
    if bgl.glCheckFramebufferStatus(bgl.GL_FRAMEBUFFER) != bgl.GL_FRAMEBUFFER_COMPLETE:
        raise RuntimeError("VKDR2 Blender render framebuffer is incomplete")
    bgl.glBindFramebuffer(bgl.GL_FRAMEBUFFER, 0)
    return int(fbo[0]), int(color[0]), int(depth[0])


def destroy_fbo(resources):
    if not resources:
        return
    fbo, color, depth = resources
    bgl.glDeleteFramebuffers(1, bgl.Buffer(bgl.GL_INT, 1, [fbo]))
    bgl.glDeleteTextures(1, bgl.Buffer(bgl.GL_INT, 1, [color]))
    bgl.glDeleteRenderbuffers(1, bgl.Buffer(bgl.GL_INT, 1, [depth]))


def camera_matrices(scene, width, height):
    camera = scene.camera
    if camera is None:
        raise RuntimeError("Set a camera in the Blender scene before rendering")
    depsgraph = bpy.context.evaluated_depsgraph_get()
    view = camera.matrix_world.inverted()
    projection = camera.calc_matrix_camera(depsgraph, x=width, y=height)
    return np.array(view, dtype=np.float32), np.array(projection, dtype=np.float32)


def read_pixels(width, height):
    pixels = bgl.Buffer(bgl.GL_FLOAT, width * height * 4)
    bgl.glReadPixels(0, 0, width, height, bgl.GL_RGBA, bgl.GL_FLOAT, pixels)
    image = np.array(pixels[:], dtype=np.float32).reshape((height, width, 4))
    return np.flipud(image).reshape(-1).tolist()


class VKDR2RenderEngine(bpy.types.RenderEngine):
    bl_idname = ENGINE_ID
    bl_label = "VKDR2"
    bl_use_preview = False

    def render(self, depsgraph):
        global renderer
        scene = depsgraph.scene
        width = max(1, int(scene.render.resolution_x * scene.render.resolution_percentage / 100))
        height = max(1, int(scene.render.resolution_y * scene.render.resolution_percentage / 100))

        if renderer is None:
            renderer = vkdr2_renderer.BlenderRenderer(width, height)
            renderer.init()
            vertices, indices = cube_geometry()
            renderer.upload_mesh(1, vertices, indices)
            renderer.upload_material(1, ALBEDO_PATH, DEFAULT_ROUGHNESS, ROUGHNESS_PATH)

        view, projection = camera_matrices(scene, width, height)
        fbo = create_fbo(width, height)
        try:
            renderer.set_camera(view, projection)
            renderer.render_frame(width, height, fbo[0])
            pixels = read_pixels(width, height)
        finally:
            destroy_fbo(fbo)

        result = self.begin_result(0, 0, width, height)
        result.layers[0].passes["Combined"].rect = pixels
        self.end_result(result)


def register():
    global _registered
    if not _registered:
        bpy.utils.register_class(VKDR2RenderEngine)
        _registered = True
    bpy.context.scene.render.engine = ENGINE_ID


def unregister():
    global renderer, _registered
    if renderer is not None:
        renderer.cleanup()
        renderer = None
    if _registered:
        bpy.utils.unregister_class(VKDR2RenderEngine)
        _registered = False


if __name__ == "__main__":
    register()
