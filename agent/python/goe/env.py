"""A Gymnasium-style environment around Game. Works without gymnasium installed (then it is a
plain class with the same reset/step); with gymnasium it is a gymnasium.Env with spaces."""
import numpy as np

from .game import Game

try:
    import gymnasium
    from gymnasium import spaces
    _Base = gymnasium.Env
except ImportError:  # pragma: no cover - exercised only without gymnasium
    gymnasium = spaces = None
    _Base = object

# what render() draws for a cell, by element type; anything else is "?"
GLYPHS = {-1: " ", 0: ".", 2: ",", 4: "#", 6: "M", 7: "d", 8: "B", 51: "k", 52: "D", 77: "p", 78: "c",
          100: "@", 200: "g", 201: "*", 204: "G", 205: "*", 250: "U", 400: "T", 555: "o", 556: "x",
          600: "$", 602: "b", 603: "_", 900: "A"}


class GoeEnv(_Base):
    """Gardens of Eris as a Gymnasium environment.

    Observation (a dict of float32 arrays, see GameState):
        vision            (cell features, 2r + 1, 2r + 1)
        player            (player features,)
        inventory         (sections, slots, item features)
        inventory_counts  (sections,)
    Action: Discrete, the game's controls (Game.available_actions).
    Reward: the score gained in the step.
    terminated: the last avatar is gone. truncated: episode_ticks passed.

    Every keyword goes to Game (vision_radius, cell_features, follow_player, ...).
    """
    metadata = {"render_modes": ["ansi"]}

    def __init__(self, render_mode=None, **game_options):
        self.render_mode = render_mode
        self.game = Game(**game_options)
        self._last_state = None
        if spaces is not None:
            self.action_space = spaces.Discrete(len(self.game.available_actions))
            self.observation_space = spaces.Dict(
                {k: spaces.Box(-np.inf, np.inf, shape=shape, dtype=np.float32) for k, shape in self._shapes().items()})

    def _shapes(self):
        side = 2 * self.game.vision_radius + 1
        sections = len(self.game.inventory_sections)
        slots = self.game.inventory_slots
        return {"vision": (len(self.game.cell_features), side, side),
                "player": (len(self.game.player_features),),
                "inventory": (sections, slots, len(self.game.item_features)),
                "inventory_counts": (sections,)}

    def _observation(self):
        s = self._last_state = self.game.get_state()
        return {"vision": s.vision, "player": s.player, "inventory": s.inventory,
                "inventory_counts": s.inventory_counts}

    def _info(self):
        s = self._last_state
        return {"score": s.score, "tick": s.tick, "centre": s.centre, "seed": self.game.seed,
                "action_taken": self.game.action_taken, "avatars_lost": self.game.avatars_lost}

    def reset(self, *, seed=None, options=None):
        """A new world. With a seed, the same seed gives the same world; without one, a fresh world
        from the environment's random generator (seeded by the first reset's seed)."""
        if _Base is not object:
            super().reset(seed=seed)
            world = seed if seed is not None else int(self.np_random.integers(0, 2 ** 32))
        else:
            world = seed
        self.game.new_episode(world)
        return self._observation(), self._info()

    def step(self, action):
        reward = float(self.game.make_action(int(action)))
        obs = self._observation()
        terminated = self.game.is_player_dead()
        truncated = self.game.is_truncated() and not terminated
        return obs, reward, terminated, truncated, self._info()

    def render(self):
        if self.render_mode != "ansi" or self._last_state is None:
            return None
        names = self.game.cell_features
        if "type" not in names:
            return None
        types = self._last_state.vision[names.index("type")]
        return "\n".join("".join(GLYPHS.get(int(t), "?") for t in row) for row in types)

    def close(self):
        self.game.close()
