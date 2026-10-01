"""Chunks built from a fixed pattern of elements. One game per process, so every test closes its own."""
import json

import numpy as np
import pytest

import goe

ROOM = ["########",
        "#......#",
        "#.k..A.#",
        "#......#",
        "#..@...#",
        "#......#",
        "#......#",
        "########"]
LEGEND = {"#": "wall", ".": None, "k": ("key", 1), "A": "golden_apple", "@": "player"}


def test_patterns_are_made_from_rows_arrays_and_json(tmp_path):
    p = goe.ChunkPattern.from_rows(ROOM, LEGEND)
    assert (p.width, p.height) == (8, 8)
    assert p.types[4, 3] == goe.ELEMENT_TYPES["player"] == 100
    assert p.types[2, 2] == 51 and p.subtypes[2, 2] == 1
    assert p.types[1, 1] == -1
    assert goe.ChunkPattern(p.types, p.subtypes) == p
    path = tmp_path / "room.json"
    path.write_text(json.dumps({"legend": {k: (list(v) if isinstance(v, tuple) else v) for k, v in LEGEND.items()},
                                "rows": ROOM}))
    assert goe.ChunkPattern.load(path) == p
    assert goe.ChunkPattern.from_json(path.read_text()) == p
    assert goe.CHUNK_SIZE == 64


def test_bad_patterns_are_refused():
    with pytest.raises(ValueError):
        goe.ChunkPattern.from_rows(["#?"], {"#": "wall"})
    with pytest.raises(ValueError):
        goe.ChunkPattern.from_rows(["#"], {"#": "dragon"})
    with pytest.raises(ValueError):
        goe.ChunkPattern(np.zeros((65, 1), dtype=int))
    with pytest.raises(ValueError):
        goe.ChunkPattern([[201]])  # a missile
    with pytest.raises(ValueError):
        goe.ChunkPattern([[4, 4]], [[0]])
    with pytest.raises(ValueError):
        goe.ChunkPattern.from_json('{"rows": []}')


def test_the_agent_sees_the_pattern():
    room = goe.ChunkPattern.from_rows(ROOM, LEGEND)
    with goe.Game(chunk_patterns={(0, 0): room}, vision_radius=3, circle=False, cell_features=["type"]) as g:
        g.new_episode(7)
        s = g.get_state()
        # the player starts on the pattern's first player, at (3, 4) of the room
        expected = np.where(room.types == -1, 0, room.types)
        rows = [(4 + dy) % 8 for dy in range(-3, 4)]
        cols = [(3 + dx) % 8 for dx in range(-3, 4)]
        assert (s.vision[0] == expected[np.ix_(rows, cols)]).all()
        assert g.chunk_at(0, 0) == (0, 0)
        assert g.chunk_at(61, 0) == (1, 0)


def test_every_chunk_can_take_one_pattern_and_the_game_plays_on(tmp_path):
    rows = [r.replace("@", ".") for r in ROOM]
    rows[1] = "#.M..c.#"  # a monster and a camera, so things move
    legend = dict(LEGEND, M="monster", c="camera")
    path = tmp_path / "rooms.json"
    path.write_text(json.dumps({"legend": {k: (list(v) if isinstance(v, tuple) else v) for k, v in legend.items()},
                                "rows": rows}))
    env = goe.GoeEnv(default_pattern=str(path), vision_radius=4, circle=False, cell_features=["type"],
                     episode_ticks=8 * 40)
    first, _ = env.reset(seed=1)
    second, _ = env.reset(seed=2)
    # any seed: the same world around the player (who starts near the start chunk's middle)
    assert (first["vision"] == second["vision"]).all()
    done = False
    while not done:
        _, _, terminated, truncated, _ = env.step(env.action_space.sample())
        done = terminated or truncated
    env.close()


def test_patterns_can_be_changed_and_cleared():
    room = goe.ChunkPattern.from_rows([r.replace("@", ".") for r in ROOM], LEGEND)
    with goe.Game(cell_features=["type"], vision_radius=2, circle=False) as g:
        g.set_default_pattern(room)
        g.new_episode(3)
        boxed = g.get_state().vision
        g.set_chunk_pattern((0, 0), goe.ChunkPattern(np.full((1, 1), -1)))
        g.new_episode(3)
        # an empty pattern: nothing but floor around the player
        around = g.get_state().vision[0]
        assert around[2, 2] == 100
        around[2, 2] = 0
        assert (around == 0).all()
        g.set_chunk_pattern((0, 0), None)
        g.new_episode(3)
        assert (g.get_state().vision == boxed).all()
        g.clear_chunk_patterns()
        g.new_episode(3)
        assert not (g.get_state().vision == boxed).all()


def test_generate_chunk_creates_chunks_with_patterns():
    """Agents can directly generate chunks with fixed patterns."""
    room = goe.ChunkPattern.from_rows(ROOM, LEGEND)
    empty = goe.ChunkPattern(np.full((1, 1), -1))
    with goe.Game(cell_features=["type"], vision_radius=2, circle=False) as g:
        g.new_episode(5)
        # generate_chunk returns True when successful
        assert g.generate_chunk((0, 0), room) is True
        assert g.generate_chunk((1, 0), empty) is True
        assert g.generate_chunk((1, 1)) is True  # no pattern: random or default
    # generate_chunk returns False when no episode has started
    with goe.Game() as g:
        assert g.generate_chunk((2, 2)) is False
