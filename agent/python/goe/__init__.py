"""Gardens of Eris for agents: the game without a window, a sound or a keyboard.

    Game     shaped like ViZDoom's DoomGame: new_episode, get_state, make_action, is_episode_finished
    GoeEnv   Gymnasium-style: reset and step, with observation and action spaces

There is one game per process (the game keeps its world in static state); run games in
parallel in separate processes. See README.md next to this package.
"""
from ._goe import (ACTIONS, CELL_FEATURES, ELEMENT_FEATURES, ITEM_FEATURES, PLAYER_FEATURES, SECTIONS,
                   describe_features)
from .game import Game, GameState, default_data_dir
from .env import GoeEnv

__all__ = ["ACTIONS", "SECTIONS", "ELEMENT_FEATURES", "PLAYER_FEATURES", "CELL_FEATURES", "ITEM_FEATURES",
           "describe_features", "Game", "GameState", "GoeEnv", "default_data_dir"]
