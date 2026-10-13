#!/usr/bin/env python3
# Copyright (c) 2026, Ariel Konopka. MIT licence, see the repository's LICENSE.
#
# Exports every mesh of a Blender file to its own .glb, standing on the origin, with modifiers
# applied: for model packs that come only as a .blend (Fruit Pack 01). Needs Blender's Python
# module: pip install bpy
#
#   python3 tools/models/blend-to-glb.py GoEoOL/data/models/fruit-pack-01/fruit-pack-01.blend \
#       GoEoOL/data/models/fruit-pack-01
#
# For Fruit Pack 01 it also writes golden_apple.glb, the apple with its skin painted gold.
import os
import sys

import bpy
import mathutils


def export(name, path):
    bpy.ops.object.select_all(action='DESELECT')
    o = bpy.data.objects[name]
    keep = o.location.copy()
    # in the middle of the origin, standing on it: the game puts a model's ground on the floor
    o.location = (0, 0, 0)
    bpy.context.view_layer.update()
    bottom = min((o.matrix_world @ mathutils.Vector(c)).z for c in o.bound_box)
    o.location = (0, 0, -bottom)
    o.select_set(True)
    bpy.context.view_layer.objects.active = o
    bpy.ops.export_scene.gltf(filepath=path, use_selection=True, export_apply=True, export_format='GLB',
                              export_cameras=False, export_lights=False)
    o.location = keep


def main(src, out):
    os.makedirs(out, exist_ok=True)
    bpy.ops.wm.open_mainfile(filepath=src)
    for o in [o for o in bpy.data.objects if o.type == 'MESH']:
        export(o.name, os.path.join(out, o.name + ".glb"))
        print("wrote", o.name + ".glb")
    if "apple" in bpy.data.objects and "Skin" in bpy.data.materials:
        skin = bpy.data.materials["Skin"]
        bsdf = next(n for n in skin.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
        bsdf.inputs["Base Color"].default_value = (1.0, 0.72, 0.12, 1.0)
        export("apple", os.path.join(out, "golden_apple.glb"))
        print("wrote golden_apple.glb")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("usage: blend-to-glb.py FILE.blend OUT_FOLDER")
    main(sys.argv[1], sys.argv[2])
