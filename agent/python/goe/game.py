"""The game, shaped like ViZDoom's DoomGame, which exRelaxer's Doom experiments drive."""
import os
from dataclasses import dataclass

import numpy as np

from . import _goe
from .pattern import as_pattern

# everything the agent may do; GIVE_UP only with allow_give_up=True
ACTIONS = list(_goe.ACTIONS)
# what the reward can weigh (Game's reward_weights); see EVENT_MEANINGS
EVENTS = list(_goe.EVENTS)
EVENT_MEANINGS = {
    "score": "points gained: the game's score (new cells visited, items collected, damage dealt)",
    "collect": "items collected, each item once an episode (golden apples count as apple)",
    "apple": "golden apples collected, each once an episode",
    "use": "the usable in hand used (a broken golden apple eaten for energy)",
    "open": "doors opened, each door once an episode",
    "teleport": "trips through a teleporter",
    "kill": "monsters, drones and puppet masters killed by the player's missiles and blasts",
    "mine": "mines and bombs set off by the player's missiles and blasts",
    "hurt": "energy lost",
    "death": "avatars lost (the last one included)",
}
# a reward from events, as a starting point: things done are rewarded, harm is penalised
SHAPED_REWARD = {"collect": 5.0, "apple": 20.0, "use": 2.0, "open": 10.0, "teleport": 5.0, "kill": 10.0,
                 "mine": 5.0, "hurt": -0.2, "death": -50.0}


def default_data_dir():
    """Where the game's data is (the folder holding data/skins.json): $GOE_DATA_DIR, else the
    GoEoOL folder of the checkout the package was built from, else the working directory."""
    if os.environ.get("GOE_DATA_DIR"):
        return os.environ["GOE_DATA_DIR"]
    if _goe.DEFAULT_DATA_DIR and os.path.isfile(os.path.join(_goe.DEFAULT_DATA_DIR, "data", "skins.json")):
        return _goe.DEFAULT_DATA_DIR
    return ""


@dataclass
class GameState:
    """One observation.

    vision            float32 (channels, rows, columns): channel c is cell_features[c]; the cell at
                      (row, column) is centre + (column - radius, row - radius), y grows downwards
    player            float32 (player_features,)
    inventory         float32 (sections, slots, item_features); an empty slot has type -1
    inventory_counts  float32 (sections,): how many items each section holds, all of them
    centre            (x, y) of the vision's centre, counted from the middle of the start area
    tick              game ticks since the episode started (50 a second)
    score             the active avatar's total points
    """
    vision: np.ndarray
    player: np.ndarray
    inventory: np.ndarray
    inventory_counts: np.ndarray
    centre: tuple
    tick: int
    score: int

    @property
    def game_variables(self):
        """The player's numbers, under ViZDoom's name for them."""
        return self.player


class Game:
    """Gardens of Eris without a window.

    data_dir            folder with data/skins.json; default: default_data_dir()
    vision_radius       the vision grid has 2 * radius + 1 cells on each side
    circle              cells further than the radius from the centre stay empty
    follow_player       centre the vision on the active player; False: on fixed_centre
    fixed_centre        (x, y) counted from the middle of the start area
    cell_features       what each vision cell holds: names from ELEMENT_FEATURES and CELL_FEATURES
                        (exists, in_sight); None: all of them
    player_features     names from ELEMENT_FEATURES and PLAYER_FEATURES; None: all of them
    inventory_sections  names from SECTIONS (weapons, usables, keys, mods, tokens); None: all
    inventory_slots     items shown per section
    item_features       names from ELEMENT_FEATURES and ITEM_FEATURES (selected);
                        None: type, subtype, energy, ammo, max_ammo, selected
    ticks_per_step      game ticks per make_action; the action reaches the player once, in the
                        first tick it can act (a move takes 8 ticks)
    episode_ticks       the episode is cut after this many ticks; 0: never (the maze never ends)
    allow_give_up       adds the GIVE_UP action (the avatar dies)
    reward_weights      {event: weight}: a step's reward is the sum of its event counts (EVENTS)
                        times these weights; None: the score gained ({"score": 1}).
                        SHAPED_REWARD rewards collecting, using, opening, teleporting, apples,
                        kills and mines, and penalises hurt and death
    chunk_patterns      {(chunk x, chunk y): ChunkPattern or path of a JSON pattern}: those chunks
                        are built from the pattern instead of a random maze. (0, 0) is the start
                        chunk, (1, 0) the one east of it, (0, -1) the one north; chunk_at says
                        which chunk a cell is in. A chunk is CHUNK_SIZE (64) cells on each side
    default_pattern     the pattern of every other chunk; None: a random maze

    describe_features() says what every feature means.
    """

    def __init__(self, data_dir=None, vision_radius=8, circle=True, follow_player=True, fixed_centre=(0, 0),
                 cell_features=None, player_features=None, inventory_sections=None, inventory_slots=5,
                 item_features=None, ticks_per_step=8, episode_ticks=0, allow_give_up=False,
                 reward_weights=None, chunk_patterns=None, default_pattern=None):
        c = _goe.Config()
        c.data_dir = default_data_dir() if data_dir is None else str(data_dir)
        c.vision_radius = vision_radius
        c.circle = circle
        c.follow_player = follow_player
        c.fixed_centre = tuple(fixed_centre)
        c.cell_features = list(cell_features or [])
        c.player_features = list(player_features or [])
        c.inventory_sections = list(inventory_sections or [])
        c.inventory_slots = inventory_slots
        c.item_features = list(item_features or [])
        c.ticks_per_step = ticks_per_step
        c.episode_ticks = episode_ticks
        c.allow_give_up = allow_give_up
        if reward_weights is not None:
            unknown = set(reward_weights) - set(EVENTS)
            if unknown:
                raise ValueError(f"unknown events in reward_weights: {sorted(unknown)}; known: {EVENTS}")
            c.reward_weights = {k: float(v) for k, v in reward_weights.items()}
        c.chunk_patterns = {_chunk(k): as_pattern(p)._native for k, p in (chunk_patterns or {}).items()}
        if default_pattern is not None:
            c.default_pattern = as_pattern(default_pattern)._native
        self.vision_radius = vision_radius
        self.inventory_slots = inventory_slots
        self._game = _goe.Game(c)

    # --- the episode ---
    def new_episode(self, seed=None):
        """A new world. The same seed builds the same world, and the same actions play the same game."""
        self._game.new_episode(None if seed is None else int(seed) & 0xFFFFFFFF)

    def make_action(self, action):
        """Plays one action for ticks_per_step ticks and returns the reward (the score gained).

        action: an index into available_actions, its name ("MOVE_UP"), or a sequence with one
        number per action (ViZDoom's buttons; the largest one is played)."""
        return self._game.make_action(self._index(action))

    def get_state(self):
        return GameState(**self._game.get_state())

    def is_episode_finished(self):
        """The last avatar is gone, or episode_ticks passed."""
        return self._game.is_episode_finished()

    def is_player_dead(self):
        """The last avatar is gone: game over."""
        return self._game.is_player_dead()

    def is_truncated(self):
        """episode_ticks passed."""
        return self._game.is_truncated()

    def close(self):
        """Lets go of the world, so another Game can start in this process."""
        self._game = None

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    # --- fixed patterns ---
    def set_chunk_pattern(self, chunk, pattern):
        """Builds the chunk (x, y) from the pattern (a ChunkPattern or a JSON file's path; None: a
        random maze again). It applies to chunks made from now on: those this episode has not
        built yet, and every chunk of the next episodes."""
        p = as_pattern(pattern)
        self._game.set_chunk_pattern(_chunk(chunk), None if p is None else p._native)

    def set_default_pattern(self, pattern):
        """The pattern of every chunk without its own (None: a random maze), as set_chunk_pattern."""
        p = as_pattern(pattern)
        self._game.set_default_pattern(None if p is None else p._native)

    def clear_chunk_patterns(self):
        """Every chunk a random maze again, as set_chunk_pattern."""
        self._game.clear_chunk_patterns()

    def chunk_at(self, x, y):
        """The (x, y) of the chunk holding the cell, the cell given as the agent sees it (counted
        from the middle of the start area, as GameState.centre and the player's x and y)."""
        return tuple(self._game.chunk_at(int(x), int(y)))

    # --- what the last step did ---
    @property
    def action_taken(self):
        """Whether the last action reached the player (False when it was busy the whole step)."""
        return self._game.action_taken

    @property
    def step_events(self):
        """{event: count} of the last step (EVENTS; hurt is energy lost)."""
        return self._game.step_events

    @property
    def episode_events(self):
        """{event: count} of the episode so far."""
        return self._game.episode_events

    @property
    def avatars_lost(self):
        """How many times this episode a spare avatar took over from a lost one."""
        return self._game.avatars_lost

    @property
    def episode_tick(self):
        return self._game.episode_tick

    @property
    def score(self):
        return self._game.score

    @property
    def seed(self):
        return self._game.seed

    # --- the shape of things ---
    @property
    def available_actions(self):
        return ACTIONS[:self._game.action_count]

    @property
    def cell_features(self):
        return list(self._game.cell_feature_names)

    @property
    def player_features(self):
        return list(self._game.player_feature_names)

    @property
    def item_features(self):
        return list(self._game.item_feature_names)

    @property
    def inventory_sections(self):
        return list(self._game.inventory_section_names)

    def _index(self, action):
        if isinstance(action, str):
            return ACTIONS.index(action)
        if isinstance(action, (int, np.integer)):
            return int(action)
        buttons = np.asarray(action)
        if buttons.shape != (self._game.action_count,):
            raise ValueError(f"expected {self._game.action_count} buttons, got shape {buttons.shape}")
        return int(np.argmax(buttons))


def _chunk(c):
    x, y = c
    return (int(x), int(y))
