import os
import struct
import subprocess

import bpy
from mathutils import Vector

ROOT = "/Users/tomasgarlik/Documents/C/Nic/renderer-master"
PREVIEW_EXE = os.path.join(ROOT, "vkdr2_preview")
PREVIEW_FILE = os.path.join(ROOT, "vkdr2_preview.vkd2mesh")
PREVIEW_LOG = os.path.join(ROOT, "vkdr2_preview.log")
ALBEDO_PATH = os.path.join(ROOT, "assets/cars/fecilia/texture.png")
ROUGHNESS_PATH = os.path.join(ROOT, "assets/cars/fecilia/roughness.png")
_preview_process = None


def selected_meshes():
    objects = [obj for obj in bpy.context.selected_objects if obj.type == "MESH"]
    if not objects and bpy.context.active_object and bpy.context.active_object.type == "MESH":
        objects = [bpy.context.active_object]
    if not objects:
        objects = [obj for obj in bpy.context.scene.objects
                   if obj.type == "MESH" and obj.visible_get()]
    if not objects:
        raise RuntimeError("Select at least one mesh object before exporting")
    return objects


def image_path_from_input(material, input_name, fallback):
    if material is None or not material.use_nodes:
        return fallback
    shader = next((node for node in material.node_tree.nodes
                   if node.type == "BSDF_PRINCIPLED"), None)
    if shader is None:
        return fallback
    socket = shader.inputs.get(input_name)
    if socket is None or not socket.is_linked:
        return fallback
    pending = [socket.links[0].from_node]
    visited = set()
    while pending:
        source = pending.pop()
        if source.name in visited:
            continue
        visited.add(source.name)
        image = getattr(source, "image", None)
        if image is not None and image.filepath:
            return bpy.path.abspath(image.filepath)
        for input_socket in source.inputs:
            for link in input_socket.links:
                pending.append(link.from_node)
    return fallback


def material_texture_paths(objects):
    for obj in objects:
        for slot in obj.material_slots:
            material = slot.material
            if material is None:
                continue
            albedo = image_path_from_input(material, "Base Color", ALBEDO_PATH)
            roughness = image_path_from_input(material, "Roughness", ROUGHNESS_PATH)
            print("VKDR2 material: %s" % material.name)
            print("VKDR2 albedo: %s" % albedo)
            print("VKDR2 roughness: %s" % roughness)
            return albedo, roughness
    return ALBEDO_PATH, ROUGHNESS_PATH


def renderer_vector(vector):
    return vector.x, vector.z, -vector.y


def export_mesh(path):
    mesh_records = []
    depsgraph = bpy.context.evaluated_depsgraph_get()
    objects = selected_meshes()

    for obj in objects:
        vertices = []
        indices = []
        albedo_path, roughness_path = material_texture_paths([obj])
        evaluated = obj.evaluated_get(depsgraph)
        mesh = evaluated.to_mesh()
        try:
            mesh.calc_loop_triangles()
            uv_layer = mesh.uv_layers.active
            normal_matrix = obj.matrix_world.to_3x3().inverted().transposed()

            for triangle in mesh.loop_triangles:
                for loop_index in triangle.loops:
                    loop = mesh.loops[loop_index]
                    position = obj.matrix_world @ mesh.vertices[loop.vertex_index].co
                    normal = normal_matrix @ mesh.polygons[triangle.polygon_index].normal
                    position = renderer_vector(position)
                    normal = renderer_vector(normal)
                    uv = uv_layer.data[loop_index].uv if uv_layer else Vector((0.0, 0.0))
                    vertices.extend((
                        position[0], position[1], position[2],
                        normal[0], normal[1], normal[2],
                        uv.x, uv.y,
                    ))
                    indices.append(len(indices))
        finally:
            evaluated.to_mesh_clear()

        if indices:
            mesh_records.append((vertices, indices, albedo_path, roughness_path))

    if not mesh_records:
        raise RuntimeError("Selected meshes contain no triangles")

    all_vertices = [vertex for record in mesh_records for vertex in record[0]]
    coordinates = list(zip(all_vertices[0::8], all_vertices[1::8], all_vertices[2::8]))
    min_x = min(value[0] for value in coordinates)
    min_y = min(value[1] for value in coordinates)
    min_z = min(value[2] for value in coordinates)
    max_x = max(value[0] for value in coordinates)
    max_y = max(value[1] for value in coordinates)
    max_z = max(value[2] for value in coordinates)
    center_x = (min_x + max_x) * 0.5
    center_y = (min_y + max_y) * 0.5
    center_z = (min_z + max_z) * 0.5
    largest_extent = max(max_x - min_x, max_y - min_y, max_z - min_z)
    scale = 2.0 / largest_extent if largest_extent > 1e-6 else 1.0
    normalized_records = []
    for vertices, indices, albedo_path, roughness_path in mesh_records:
        for index in range(0, len(vertices), 8):
            vertices[index + 0] = (vertices[index + 0] - center_x) * scale
            vertices[index + 1] = (vertices[index + 1] - center_y) * scale
            vertices[index + 2] = (vertices[index + 2] - center_z) * scale
        normalized_records.append((vertices, indices, albedo_path, roughness_path))

    with open(path, "wb") as output:
        output.write(b"VKD2MESH")
        output.write(struct.pack("<I", len(normalized_records)))
        for vertices, indices, albedo_path, roughness_path in normalized_records:
            output.write(struct.pack("<II", len(vertices) // 8, len(indices)))
            output.write(struct.pack("<%df" % len(vertices), *vertices))
            output.write(struct.pack("<%dI" % len(indices), *indices))
            for texture_path in (albedo_path, roughness_path):
                encoded = os.path.abspath(texture_path).encode("utf-8")
                output.write(struct.pack("<I", len(encoded)))
                output.write(encoded)

        camera = bpy.context.scene.camera
        if camera is None:
            camera_position = (0.0, 1.5, 6.0)
            camera_target = (0.0, 0.0, 0.0)
            camera_fov = 60.0
            camera_near = 0.1
            camera_far = 100.0
        else:
            rotation = camera.matrix_world.to_3x3()
            position = camera.matrix_world.translation
            target = position + rotation @ Vector((0.0, 0.0, -1.0))
            position = renderer_vector(position)
            target = renderer_vector(target)
            camera_position = tuple((position[index] - (center_x, center_y, center_z)[index]) * scale
                                    for index in range(3))
            camera_target = tuple((target[index] - (center_x, center_y, center_z)[index]) * scale
                                  for index in range(3))
            camera_fov = camera.data.angle_y * 180.0 / 3.141592653589793
            camera_near = camera.data.clip_start
            camera_far = camera.data.clip_end
        output.write(struct.pack(
            "<9f", *(camera_position + camera_target +
                      (camera_fov, camera_near, camera_far))
        ))
        render = bpy.context.scene.render
        output.write(struct.pack(
            "<2I",
            int(render.resolution_x * render.resolution_percentage / 100),
            int(render.resolution_y * render.resolution_percentage / 100),
        ))

    return sum(len(record[1]) for record in normalized_records) // 3


def launch_preview():
    global _preview_process
    if not os.path.isfile(PREVIEW_EXE):
        raise RuntimeError("Build vkdr2_preview first: " + PREVIEW_EXE)
    triangle_count = export_mesh(PREVIEW_FILE)
    if _preview_process is not None and _preview_process.poll() is None:
        _preview_process.terminate()
    log_file = open(PREVIEW_LOG, "w")
    _preview_process = subprocess.Popen(
        [PREVIEW_EXE, PREVIEW_FILE], stdout=log_file, stderr=subprocess.STDOUT
    )
    print("VKDR2 preview launched with %d triangles" % triangle_count)
    print("Preview file: %s" % PREVIEW_FILE)
    print("Preview log: %s" % PREVIEW_LOG)


def stop_preview():
    global _preview_process
    if _preview_process is not None and _preview_process.poll() is None:
        _preview_process.terminate()
    _preview_process = None


if __name__ == "__main__":
    launch_preview()
