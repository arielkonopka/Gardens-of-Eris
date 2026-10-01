"""Chunks built from a fixed pattern of elements instead of a random maze."""
import os

import numpy as np

from . import _goe

# the element types a pattern may place, by name: {"wall": 4, "player": 100, ...}
ELEMENT_TYPES = dict(_goe.ELEMENT_TYPES)
# the size of a chunk: a pattern is at most this many cells on each side
CHUNK_SIZE = _goe.CHUNK_SIZE
NOTHING = -1


class ChunkPattern:
    """A fixed layout of elements for one chunk of the world (64 x 64 cells).

    types       2-D (rows, columns) element types: ELEMENT_TYPES values, -1 for nothing (the
                floor as the game made it), 0 for the floor itself with the subtype as its look
    subtypes    the same shape; None: all 0

    A pattern smaller than a chunk is repeated to fill it, from the chunk's top left cell. A
    patterned chunk has no maze and no walls of its own: walls are where the pattern puts them.
    In the start chunk the first player in the pattern (row by row) is the one the game starts
    with; without one, the player goes on the free floor nearest the chunk's middle. Players in
    other chunks are spare avatars.
    """

    def __init__(self, types, subtypes=None):
        if isinstance(types, _goe.ChunkPattern):
            self._native = types
            return
        types = np.asarray(types, dtype=np.int64)
        if types.ndim != 2:
            raise ValueError(f"types must be 2-D (rows, columns), not shape {types.shape}")
        subtypes = np.zeros_like(types) if subtypes is None else np.asarray(subtypes, dtype=np.int64)
        if subtypes.shape != types.shape:
            raise ValueError(f"subtypes have shape {subtypes.shape}, types {types.shape}")
        height, width = types.shape
        self._native = _goe.ChunkPattern(width, height, types.ravel().tolist(), subtypes.ravel().tolist())

    @classmethod
    def from_rows(cls, rows, legend):
        """From rows of characters and what each character stands for: None (nothing), a type (its
        number or name from ELEMENT_TYPES), or (type, subtype).

            ChunkPattern.from_rows(["#####",
                                    "#@.k#",
                                    "#####"], {"#": "wall", "@": "player", "k": ("key", 1), ".": None})
        """
        cells = {}
        for char, entry in legend.items():
            if len(char) != 1:
                raise ValueError(f"legend keys are single characters, not {char!r}")
            cells[char] = _cell(entry, char)
        if not rows or any(len(r) != len(rows[0]) for r in rows):
            raise ValueError("rows must be a non-empty list of strings of the same length")
        missing = sorted({c for r in rows for c in r} - set(cells))
        if missing:
            raise ValueError(f"not in the legend: {missing}")
        types = [[cells[c][0] for c in r] for r in rows]
        subtypes = [[cells[c][1] for c in r] for r in rows]
        return cls(types, subtypes)

    @classmethod
    def from_json(cls, text):
        """From JSON text: {"legend": {"#": "wall", "k": ["key", 1], ".": null}, "rows": ["#k.#", ...]}."""
        return cls(_goe.ChunkPattern.from_json(text))

    @classmethod
    def load(cls, path):
        """from_json with the file's text."""
        return cls(_goe.ChunkPattern.load(os.fspath(path)))

    @property
    def width(self):
        return self._native.width

    @property
    def height(self):
        return self._native.height

    @property
    def types(self):
        """(rows, columns) element types."""
        return np.asarray(self._native.types, dtype=np.int64).reshape(self.height, self.width)

    @property
    def subtypes(self):
        return np.asarray(self._native.subtypes, dtype=np.int64).reshape(self.height, self.width)

    def __eq__(self, other):
        return isinstance(other, ChunkPattern) and self._native == other._native

    def __repr__(self):
        return f"ChunkPattern({self.width} x {self.height})"


def _cell(entry, char):
    if entry is None:
        return (NOTHING, 0)
    if isinstance(entry, (tuple, list)):
        if not 1 <= len(entry) <= 2:
            raise ValueError(f"{char!r} must be a type or (type, subtype)")
        return (_type(entry[0], char), int(entry[1]) if len(entry) == 2 else 0)
    return (_type(entry, char), 0)


def _type(t, char):
    if isinstance(t, str):
        if t not in ELEMENT_TYPES:
            raise ValueError(f"unknown type name {t!r} for {char!r}; known: {sorted(ELEMENT_TYPES)}")
        return ELEMENT_TYPES[t]
    return int(t)


def as_pattern(p):
    """A ChunkPattern from a ChunkPattern or the path of a JSON pattern file; None stays None."""
    if p is None or isinstance(p, ChunkPattern):
        return p
    if isinstance(p, (str, os.PathLike)):
        return ChunkPattern.load(p)
    raise TypeError(f"expected a ChunkPattern or a path, not {type(p).__name__}")
