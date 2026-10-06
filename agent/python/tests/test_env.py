"""The goe package: Gardens of Eris for agents. One game per process, so every test closes its own."""
import numpy as np
import pytest

import goe


@pytest.fixture
def game():
    g = goe.Game()
    yield g
    g.close()


def test_the_default_observation_has_everything(game):
    game.new_episode(555)
    s = game.get_state()
    assert game.cell_features == goe.ELEMENT_FEATURES + goe.CELL_FEATURES
    assert game.player_features == goe.ELEMENT_FEATURES + goe.PLAYER_FEATURES
    assert game.inventory_sections == goe.SECTIONS
    assert s.vision.shape == (len(game.cell_features), 17, 17)
    assert s.vision.dtype == np.float32
    assert s.player.shape == (len(game.player_features),)
    assert s.inventory.shape == (5, 5, len(game.item_features))
    assert s.inventory_counts.shape == (5,)
    assert s.game_variables is s.player
    # the player is in the middle of its vision
    t = game.cell_features.index("type")
    assert s.vision[t, 8, 8] == 100
    assert s.vision[t, 0, 0] == -1  # outside the circle
    assert "GIVE_UP" not in game.available_actions
    assert any(d.startswith("energy: ") for d in goe.describe_features())


def test_the_composition_can_be_chosen():
    with goe.Game(vision_radius=2, circle=False, cell_features=["type", "subtype", "in_sight"],
                  player_features=["x", "y", "energy", "score"], inventory_sections=["weapons", "keys"],
                  inventory_slots=2, item_features=["type", "selected"]) as g:
        g.new_episode(555)
        s = g.get_state()
        assert s.vision.shape == (3, 5, 5)
        assert (s.vision[0] != -1).all()  # not a circle: every cell near the start is built
        assert s.player.shape == (4,)
        assert s.inventory.shape == (2, 2, 2)
        assert g.inventory_sections == ["weapons", "keys"]


def test_cells_remember_what_the_player_saw():
    assert goe.CELL_FEATURES == ["exists", "in_sight", "visits", "seen", "novelty"]
    with goe.Game(cell_features=["visits", "seen", "novelty"]) as g:
        g.new_episode(555)
        first = g.get_state().vision
        assert first[0, 8, 8] == 1 and first[1, 8, 8] == 1
        assert first[2, 0, 8] == 1  # out of sight: never seen
        g.make_action("NOOP")
        second = g.get_state().vision
        # standing still, the view is the same but more familiar
        assert second[1, 8, 8] == 2
        assert second[2, 8, 8] < first[2, 8, 8]


def test_the_vision_can_stay_on_a_fixed_cell():
    with goe.Game(follow_player=False, fixed_centre=(3, -2), vision_radius=1, cell_features=["type"]) as g:
        g.new_episode(555)
        assert g.get_state().centre == (3, -2)
        for _ in range(5):
            g.make_action("MOVE_LEFT")
        assert g.get_state().centre == (3, -2)


def test_unknown_names_are_refused():
    with pytest.raises(ValueError):
        goe.Game(cell_features=["colour"])
    with pytest.raises(ValueError):
        goe.Game(inventory_sections=["pockets"])


def test_one_game_per_process(game):
    with pytest.raises(RuntimeError):
        goe.Game()


def test_actions_by_index_name_or_buttons(game):
    game.new_episode(555)
    for a in ("MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT"):
        game.make_action(a)
        game.make_action(0)
    buttons = np.zeros(len(game.available_actions))
    buttons[game.available_actions.index("MOVE_LEFT")] = 1
    game.make_action(buttons)
    game.make_action(goe.ACTIONS.index("NOOP"))
    assert game.episode_tick == 10 * 8
    assert game.action_taken
    with pytest.raises(IndexError):
        game.make_action(len(game.available_actions))


def test_the_same_seed_plays_the_same_game(game):
    rng = np.random.default_rng(1)
    plan = rng.integers(0, len(game.available_actions), 60)

    def play(seed):
        game.new_episode(seed)
        out = []
        for a in plan:
            r = game.make_action(int(a))
            s = game.get_state()
            out.append((r, s.vision.copy(), s.player.copy(), s.inventory.copy()))
        return out

    a, b, c = play(99), play(99), play(100)
    assert all(ra == rb and (va == vb).all() and (pa == pb).all() and (ia == ib).all()
               for (ra, va, pa, ia), (rb, vb, pb, ib) in zip(a, b))
    assert any((va != vc).any() for (_, va, _, _), (_, vc, _, _) in zip(a, c))


def test_episodes_can_be_cut_short():
    with goe.Game(episode_ticks=24, ticks_per_step=8) as g:
        g.new_episode(555)
        for _ in range(3):
            assert not g.is_episode_finished()
            g.make_action("NOOP")
        assert g.is_truncated() and g.is_episode_finished() and not g.is_player_dead()


def test_the_gymnasium_environment():
    gymnasium = pytest.importorskip("gymnasium")
    from gymnasium.utils.env_checker import check_env
    env = goe.GoeEnv(render_mode="ansi", vision_radius=3, episode_ticks=400)
    try:
        assert isinstance(env, gymnasium.Env)
        check_env(env, skip_render_check=True)
        obs, info = env.reset(seed=3)
        start = info["score"]
        assert env.observation_space.contains(obs)
        assert info["seed"] == 3
        assert env.render().splitlines()[3][3] == "@"
        done, steps, total = False, 0, 0.0
        while not done:
            obs, reward, terminated, truncated, info = env.step(env.action_space.sample())
            total += reward
            steps += 1
            done = terminated or truncated
        assert truncated and steps == 50
        if info["avatars_lost"] == 0:
            assert total == info["score"] - start  # the rewards add up to the score gained
    finally:
        env.close()


def test_the_reward_weighs_events():
    assert set(goe.SHAPED_REWARD) <= set(goe.EVENTS)
    assert set(goe.EVENT_MEANINGS) == set(goe.EVENTS)
    with pytest.raises(ValueError):
        goe.Game(reward_weights={"nonsense": 1.0})
    weights = dict(goe.SHAPED_REWARD, score=0.5)
    with goe.Game(vision_radius=2, episode_ticks=50 * 30, reward_weights=weights) as g:
        g.new_episode(555)
        rng = np.random.default_rng(0)
        total = 0.0
        while not g.is_episode_finished():
            reward = g.make_action(int(rng.integers(1, 9)))
            step = g.step_events
            assert set(step) == set(goe.EVENTS)
            assert reward == pytest.approx(sum(weights.get(k, 0.0) * v for k, v in step.items()))
            total += reward
        episode = g.episode_events
        assert episode["score"] > 0
        assert total == pytest.approx(sum(weights.get(k, 0.0) * v for k, v in episode.items()))


def test_the_environment_reports_the_events():
    env = goe.GoeEnv(vision_radius=2, episode_ticks=80, reward_weights={"hurt": -1.0, "score": 1.0})
    env.reset(seed=555)
    _, reward, _, _, info = env.step(1)
    assert set(info["events"]) == set(goe.EVENTS)
    assert reward == pytest.approx(info["events"]["score"] - info["events"]["hurt"])
    env.close()
