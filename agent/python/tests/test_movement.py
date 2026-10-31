"""The movement switch (movement=8|4): the same actions in both, diagonal ones masked in 4."""
import hashlib

import numpy as np
import pytest

import goe

DIAGONALS = [a for a in goe.ACTIONS if a.startswith(("MOVE_", "SHOOT_")) and a.count("_") == 2]

# The fingerprint of 400 steps of seed 4242 with actions drawn from default_rng(7), taken with
# the game as it was before the movement switch (main at 215b1c6, Linux, GCC 13): eight-way
# movement must play exactly as it did. A change that alters the game on purpose changes it.
EIGHT_WAY_FINGERPRINT = "a0e4c68f1426067020438688c07dd54f1c031eb9efc9244a45c1be904b90d046"


def fingerprint(**options):
    h = hashlib.sha256()
    with goe.Game(**options) as g:
        g.new_episode(4242)
        rng = np.random.default_rng(7)
        n = len(g.available_actions)
        for _ in range(400):
            g.make_action(int(rng.integers(0, n)))
            s = g.get_state()
            h.update(s.vision.tobytes())
            h.update(s.player.tobytes())
            h.update(s.inventory.tobytes())
            h.update(repr((s.score, s.tick, s.centre, sorted(g.step_events.items()), g.action_taken)).encode())
            if g.is_episode_finished():
                break
    return h.hexdigest()


def test_the_diagonal_actions_are_the_eight():
    assert DIAGONALS == ["MOVE_UP_LEFT", "MOVE_UP_RIGHT", "MOVE_DOWN_LEFT", "MOVE_DOWN_RIGHT",
                         "SHOOT_UP_LEFT", "SHOOT_UP_RIGHT", "SHOOT_DOWN_LEFT", "SHOOT_DOWN_RIGHT"]


def test_movement_must_be_four_or_eight():
    with pytest.raises(ValueError):
        goe.Game(movement=6)


@pytest.mark.parametrize("give_up", [False, True])
def test_the_actions_are_the_same_in_both_modes(give_up):
    with goe.Game(movement=8, allow_give_up=give_up) as g:
        eight = list(g.available_actions)
        assert g.action_mask.all()
        assert g.action_mask.shape == (len(eight),)
    with goe.Game(movement=4, allow_give_up=give_up) as g:
        assert list(g.available_actions) == eight
        mask = g.action_mask
        assert mask.dtype == bool and mask.shape == (len(eight),)
        assert [a for a, ok in zip(eight, mask) if not ok] == DIAGONALS


def test_in_four_way_mode_a_diagonal_action_never_moves_the_player():
    with goe.Game(movement=4, player_features=["x", "y"]) as g:
        rng = np.random.default_rng(5)
        for seed in (555, 4242, 31337):
            g.new_episode(seed)
            for _ in range(150):
                # mostly diagonals, with side moves now and then so the player gets about
                a = DIAGONALS[int(rng.integers(0, 4))] if rng.random() < 0.7 else \
                    ["MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT"][int(rng.integers(0, 4))]
                before = tuple(g.get_state().player)
                g.make_action(a)
                if g.is_episode_finished():
                    break
                if a in DIAGONALS:
                    assert not g.action_taken
                    after = tuple(g.get_state().player)
                    dx, dy = abs(after[0] - before[0]), abs(after[1] - before[1])
                    # nothing the player did; a teleporter or an explosion may still move it,
                    # but never by one diagonal step
                    assert not (dx == 1 and dy == 1)


def test_eight_way_mode_plays_exactly_as_before():
    assert fingerprint() == EIGHT_WAY_FINGERPRINT
    assert fingerprint(movement=8) == EIGHT_WAY_FINGERPRINT


def test_the_env_hands_out_the_mask():
    env = goe.GoeEnv(movement=4)
    try:
        _, info = env.reset(seed=555)
        assert info["action_mask"].shape == (env.action_space.n,) if hasattr(env, "action_space") else True
        assert not info["action_mask"].all()
        _, _, _, _, info = env.step(goe.ACTIONS.index("MOVE_UP_LEFT"))
        assert not info["action_taken"]
        assert list(info["action_mask"]) == list(env.game.action_mask)
    finally:
        env.close()
