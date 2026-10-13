#!/bin/bash
# Fetches CC0 3D model packs for the 3D view into GoEoOL/data/models/<pack>/, each with its licence.
# Run it from the repository root. Needs git; --zip also needs unzip.
#
#   tools/models/fetch-models.sh                  every KayKit pack below, cloned from GitHub
#   tools/models/fetch-models.sh dungeon skeletons  only these
#   tools/models/fetch-models.sh --zip NAME FILE  a pack you downloaded yourself (Kenney, Quaternius):
#                                                 its glTF/GLB files, their .bin and textures and its
#                                                 licence go to GoEoOL/data/models/NAME
#
# KayKit packs (Kay Lousberg, CC0, https://github.com/KayKit-Game-Assets):
#   dungeon      KayKit Dungeon Remastered     walls, floors, doors, keys, crates, traps
#   adventurers  KayKit Adventurers            animated characters (player) and their weapons
#   skeletons    KayKit Skeletons              animated skeleton monsters and their weapons
#   spacebase    KayKit Space Base Bits        sci-fi modules, landers, cargo, solar panels
#   prototype    KayKit Prototype Bits         barrels, boxes, coins, doors, simple shapes
#
# Packs to download by hand and add with --zip (both CC0; their sites block scripted downloads):
#   Kenney: https://kenney.nl/assets/space-kit, /blaster-kit, /food-kit   (pick the GLB format)
#   Quaternius: https://quaternius.com (Ultimate Space Kit, Animated Robot, Lowpoly Animated Monsters)
#
# Files already in a pack's folder are kept, so the script can run again. Commit only the
# models skins.json uses, and add each new folder to the 3D models table in CREDITS.md.
set -euo pipefail

models="GoEoOL/data/models"
if [ ! -d "GoEoOL/data" ]; then
    echo "Run me from the repository root" >&2
    exit 1
fi

repoOf() {
    case "$1" in
        dungeon) echo "KayKit-Dungeon-Remastered-1.0 kaykit-dungeon" ;;
        adventurers) echo "KayKit-Character-Pack-Adventures-1.0 kaykit-adventurers" ;;
        skeletons) echo "KayKit-Character-Pack-Skeletons-1.0 kaykit-skeletons" ;;
        spacebase) echo "KayKit-Space-Base-Bits-1.0 kaykit-space-base" ;;
        prototype) echo "KayKit-Prototype-Bits-1.0 kaykit-prototype" ;;
        *) return 1 ;;
    esac
}

# copies the glTF/GLB files under $1, with the .bin and images next to them, flat into $2
copyModels() {
    local from="$1" to="$2" count=0
    mkdir -p "$to"
    while IFS= read -r -d '' dir; do
        for f in "$dir"/*; do
            case "${f,,}" in
                *.glb | *.gltf | *.bin | *.png | *.jpg | *.jpeg)
                    # KayKit names its binaries wall.gltf.glb; the game calls them wall.glb
                    local name
                    name=$(basename "$f")
                    name="${name%.gltf.glb}"
                    [ "$name" = "$(basename "$f")" ] || name="$name.glb"
                    [ -e "$to/$name" ] || cp "$f" "$to/$name"
                    count=$((count + 1)) ;;
            esac
        done
    done < <(find "$from" -type f \( -iname '*.glb' -o -iname '*.gltf' \) -printf '%h\0' | sort -zu)
    # a .gltf may keep its texture in a folder of its own, like KayKit's texture/
    while IFS= read -r -d '' img; do
        [ -e "$to/$(basename "$img")" ] || cp "$img" "$to/"
    done < <(find "$from" -type d -iname 'textures' -o -type d -iname 'texture' | while read -r d; do
        find "$d" -maxdepth 1 -type f \( -iname '*.png' -o -iname '*.jpg' \) -print0; done)
    echo "$count"
}

copyLicence() {
    local from="$1" to="$2"
    local licence
    licence=$(find "$from" -maxdepth 3 -type f \( -iname 'license*' -o -iname 'licence*' \) | head -1)
    if [ -n "$licence" ]; then
        cp "$licence" "$to/LICENSE.txt"
    else
        echo "  no licence file found: check the pack's page and add LICENSE.txt to $to" >&2
    fi
}

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

if [ "${1:-}" = "--zip" ]; then
    [ $# -eq 3 ] || { echo "usage: $0 --zip NAME FILE.zip" >&2; exit 1; }
    name="$2" zip="$3"
    case "$name" in */* | .* | "") echo "NAME is a folder name, like kenney-space-kit" >&2; exit 1 ;; esac
    unzip -q "$zip" -d "$work/pack"
    n=$(copyModels "$work/pack" "$models/$name")
    copyLicence "$work/pack" "$models/$name"
    echo "$name: $n files in $models/$name"
    exit 0
fi

packs=("$@")
[ ${#packs[@]} -gt 0 ] || packs=(dungeon adventurers skeletons spacebase prototype)
for pack in "${packs[@]}"; do
    if ! read -r repo folder < <(repoOf "$pack"); then
        echo "Unknown pack $pack (dungeon, adventurers, skeletons, spacebase, prototype)" >&2
        exit 1
    fi
    echo "fetch  $pack ($repo)"
    git clone -q --depth 1 --filter=blob:none --sparse "https://github.com/KayKit-Game-Assets/$repo" "$work/$pack"
    git -C "$work/$pack" sparse-checkout set --no-cone '/LICENSE.txt' '/addons/*/Assets/' '/addons/*/Characters/' \
        '!/addons/*/Assets/fbx/' '!/addons/*/Assets/obj/' '!/addons/*/Characters/fbx/' '!/addons/*/Characters/obj/'
    n=$(copyModels "$work/$pack" "$models/$folder")
    copyLicence "$work/$pack" "$models/$folder"
    echo "       $n files in $models/$folder"
done
echo "Done. skins.json names a model by its path, like data/models/kaykit-dungeon/wall.glb"
