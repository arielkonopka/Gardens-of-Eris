#!/usr/bin/env python3
"""Draws GoEoOL/data/graph/fog.png, the default fog bitmap: a dark, seamlessly tiling haze
with five faint golden pentagons (Law of Fives). Run from the repository root."""
import math
import numpy as np
from PIL import Image

SIZE = 256
rng = np.random.default_rng(23)


def periodic_noise(period):
    """Value noise that wraps every SIZE pixels, with `period` lattice cells across."""
    grid = rng.random((period, period))
    t = np.arange(SIZE) * period / SIZE
    i0 = np.floor(t).astype(int)
    f = t - i0
    f = f * f * (3 - 2 * f)
    i1 = (i0 + 1) % period
    a = grid[np.ix_(i0, i0)]
    b = grid[np.ix_(i0, i1)]
    c = grid[np.ix_(i1, i0)]
    d = grid[np.ix_(i1, i1)]
    fy = f[:, None]
    fx = f[None, :]
    return (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy


haze = sum(periodic_noise(p) / w for p, w in ((4, 1), (8, 2), (16, 4), (32, 8), (64, 16)))
haze = (haze - haze.min()) / (haze.max() - haze.min())

# darker than the plain fog colour (15, 25, 45) at the darkest, a dim bruised violet at the lightest
dark = np.array([9, 13, 26], float)
light = np.array([34, 26, 54], float)
img = dark + (light - dark) * haze[..., None]

# five faint gold pentagons, drawn with wrap-around so the tile stays seamless
gold = np.array([212, 175, 55], float)
yy, xx = np.mgrid[0:SIZE, 0:SIZE].astype(float)
for k in range(5):
    cx, cy = rng.random(2) * SIZE
    r = 13 + 5 * k
    rot = rng.random() * 2 * math.pi
    pts = [(cx + r * math.cos(rot + j * 2 * math.pi / 5), cy + r * math.sin(rot + j * 2 * math.pi / 5)) for j in range(5)]
    dist = np.full((SIZE, SIZE), 1e9)
    for j in range(5):
        (x0, y0), (x1, y1) = pts[j], pts[(j + 1) % 5]
        for ox in (-SIZE, 0, SIZE):
            for oy in (-SIZE, 0, SIZE):
                px, py = xx - x0 - ox, yy - y0 - oy
                vx, vy = x1 - x0, y1 - y0
                t = np.clip((px * vx + py * vy) / (vx * vx + vy * vy), 0, 1)
                dist = np.minimum(dist, np.hypot(px - t * vx, py - t * vy))
    line = np.clip(1.2 - dist, 0, 1) * 0.18
    img = img * (1 - line[..., None]) + gold * line[..., None]

Image.fromarray(np.clip(img, 0, 255).astype(np.uint8), "RGB").save("GoEoOL/data/graph/fog.png", optimize=True)
