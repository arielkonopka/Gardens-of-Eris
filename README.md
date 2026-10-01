
# Garden of Eris or Obnoxious Labyrinth

Greetings and welcome to me little passion project, so. I began this adventure with [Python](https://github.com/arielkonopka/pyLurker), but soon enough I found meself realizin' that Python might not have the swiftness needed for the grand number of elements I'd like to be workin' with simultaneously.

Now, from the get-go, this project isn't focused on creating a product, but rather it's about takin' a wee bit of time to build somethin' for the joy of it. I hung up me coding hat a good few years back, and I simply wished to feel that delightful thrill of crafting code once more. Since me life doesn't revolve 'round computers, I can only spare a modest bit of time for the project, which means it's movin' along at a leisurely pace.

So, here we have a work in progress. It tends to function well enough, but consider yerself warned, me friend.

# Recent build status

Main branch build status:

[![CI](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/ci.yml/badge.svg)](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/ci.yml)

Every push builds the game and runs the unit tests on Linux and on Windows (MSYS2), and packs each build as a download on the run's page. Pushing a version tag also publishes both builds as a GitHub release:

```
git tag v0.1.0
git push origin v0.1.0
```

## Why the idea

Ever since I was a young lad, I've dreamt of creating a game of this sort. A few years back, I contributed some code to the [GNU Robbo project](http://gnurobbo.sourceforge.net/), which was a grand bit of fun, but that was purely C and SDL1.2.

Time passed, and I didn't write a single line of code. Then one day, I resolved to do a bit of coding and thought of a game to create. I pondered over the game's story, which I've only just begun to grasp.

In the repo, you'll find an ubrello5 file that outlines me vision for the game's classes. Mind you, this diagram is far from complete as well.

I considered a random level generator for two reasons:

I wouldn't need to create a level editor
I wouldn't need to design the levels
In the meantime, I've discovered a third reason:

I can test the game quite swiftly, without the need for loading and saving level data. Of course, I'll eventually have to implement that feature.
[Feel free to check out a relatively recent video of the gameplay.](https://youtu.be/sntxioo-ZFc?si=E77h_9FG4BRN3sUD)

## The game story

Once upon a time, the [Goddes](https://en.wikipedia.org/wiki/Eris_(mythology))  ventured into her garden, only to be gobsmacked when she discovered that all her golden apples had vanished. As was her nature, she flew into a rage.

Now, as a [Discordian Pope](https://en.wikipedia.org/wiki/Discordianism), it falls upon you to recover the lost apples scattered throughout various realities.

The Goddess has dispersed your avatars throughout this peculiar world you now find yourself in. The missing apples are hidden here – collect them with great care, for they may explode or break. The garden has no end, and neither has the search: the maze keeps growing wherever you go, so there is no winning, only your score.

Every time your avatar perishes, you'll respawn in the first unused and activated avatar on your path. If you haven't activated any avatars, you'll meet your end: the game shows your score and takes you back to the title screen.

The labyrinth is inhabited by an assortment of creatures and contraptions. You'll encounter gun modules and worker drones, as well as guard drones, doors, keys, movable turrets, and immobile turrets. Keep an eye out for other malevolent entities (TBC).



## Playing the game

The game opens with a title screen: **Start game**, **Config** and **Exit**. When the save folder holds a save the game can read, it shows **Continue** and **New game** instead of Start game: Continue picks that game up where it was saved, and New game builds a new world that takes the old save's place at once, so the old game is gone. Config sets the folder the game is saved to, the music and sound effects volumes (Left and Right change them in steps of 5%), the story scroller, and the controls; all of it is kept in `settings.json` next to the game. Starting a game builds the maze around you, and the maze keeps growing in every direction as you walk: it never ends.

Leave the main menu alone for a while (Demo after, 60 seconds by default, plus a random part of it again) and the game plays itself: a demo builds a world of its own and drops an autopilot far out in the maze, 23 to 63 chunks from the start, where the maze is at its hardest, so it looks like a game well under way. The autopilot collects what it passes, tries doors and shoots monsters and drones in its line once it holds a gun. After Demo length (30 seconds by default) the hall of fame shows for Hall of fame shown (10 seconds by default), then the menu comes back. Any key, pad button or stick push brings the menu back at once and the demo world is thrown away. The demo never writes, replaces or deletes a save, and never enters the hall of fame. The three times are in Config and in `settings.json` (`demoWait`, `demoLength`, `hallOfFameLength`, 5 to 3600 seconds).

The hall of fame keeps the ten best games, kept in `halloffame.json` in the save folder. When a lost game's best score earns a place, the Game over screen asks for a name (up to 16 characters; none gives "Anonymous") and shows the list with the new entry marked.

Each time the maze grows by a new chunk, a random Discordian story scrolls along the top of the screen, unless one is still scrolling. Config switches the story scroller on or off and picks the stories file: Left and Right step through the files next to it (`data/txt/stories.json` in English, `stories.pl.json` in Polish, `stories.ro.json` in Romanian), or type the path of your own. A stories file is a JSON list of `{"title": "...", "body": "..."}`.

These are the default keys. Config, Controls lets you choose other keys (two per action) and a pad button for each action: pick the action, then press the new key or pad button. Backspace leaves an action without keys, and Reset to defaults brings this layout back. The pad's stick always walks.

| Key | What it does |
|---|---|
| Arrows or W A S D | walk |
| Shift + direction | shoot the selected gun |
| Ctrl + direction | interact with, or pick up, what is next to you |
| Alt + direction | drag what is behind you |
| X | select the next usable item |
| Z | select the next kind of gun (the HUD shows how many of the selected kind you carry) |
| Space | use the selected item |
| R | drop the selected item |
| Esc or F10 | save the game and go back to the title screen; nothing is lost, and Continue picks it up again |
| Backspace | give up this avatar (you come back in the next activated one) |
| Shift + Backspace | quit the game without saving (the shoot key and the give-up key together) |
| F5 / F9 | save / load the game (`savegame.goe` in the save folder) |

`GardenOfEris --load <file>` starts straight from a saved game, without the title screen.

Every new game prints its world seed (`World seed: ...`). `GardenOfEris --seed <number>` builds that same world again, which helps when reporting a bug in the maze.

The HUD shows your score (**P**), your level (**Dex**, see Stats) and the current difficulty (**D**, see Difficulty).

Settings saved by an older version kept Esc for giving up; the game moves it to Backspace and gives Esc to save and exit, unless you chose your keys after this change.

When the last avatar is gone, a Game over screen shows the best score of all the avatars you played in that game, and its save is deleted: a lost game cannot be continued. Enter takes you back to the title screen, where Start game begins a new world.

### If the game crashes
The game writes a crash report, `crash-<date>-<time>.log`, into the save folder (the game's folder unless Config says otherwise); on Windows a message box says where it went. It holds what went wrong, the world seed, a stack trace and the last lines the game printed. Please attach it to the bug report, with what you were doing.

## Elements of the gardens

| Element | What it does |
|---|---|
| Floor, wall | The maze itself. Floors come in a few looks; walls stand until they don't. |
| Brick cluster | Can't be killed, but can be pushed around and blown up. Also used to block passages instead of a door. |
| Door and key | A door opens for a key of its colour. Rooms are often locked, and the key is usually placed somewhere else. |
| Golden apple | The thing you are here for. Shoot one and it becomes a healing item that drains itself and finally explodes in your inventory. |
| Plain gun, bazooka | Weapons to pick up. Shooting fast makes weaker shots. |
| Simple bomb | Explodes when shot or hit by another explosion. |
| Landmine | Looks almost like floor: a pentagon plate with the Sacred Chao on it. Goes off when anything steps on it and takes that thing with it. Only the maze further from the start has them, and the further out, the more. |
| Monster | Roams the maze. Monsters collect things if their skin allows it (`canCollect` in `skins.json`), and some have no inventory on purpose. |
| Bunker | A fixed turret that looks along its four lines and fires at you. |
| Kiki and bouba | Kiki is a death-ray emitter; the beam is made of boubas, which hurt whatever stands in them. |
| Patrolling drone and puppet master | A drone does nothing until you hand it a puppet master (a controller). The controller decides how it moves: **patrol** (wanders, and becomes an extra camera for you), **collector** (goes for collectibles it can see), **hunter** (chases you around walls when it sees you, goes to where it saw you last when you slip away, and patrols the walls otherwise), **wall follower** (keeps a hand on the wall and walks the maze). When a controller takes over a drone it says "controller enabled" in a robot voice, each kind in its own language: patrol in English, collector in Polish, hunter in German, wall follower in French, a camera's guardian in Russian and the Hound in Latin. |
| Security camera | Watches for you. When it sees you, its guardian drones come to check the spot. Guardians fight you when they see you, shoot along clear lines, go to where you were last seen when you slip away, patrol the walls around the camera when there is nobody to find, and never go further than 55 cells from their camera. |
| The Hound | A red drone with the golden apple on its hull. It is sent after you when you stay in one 64x64 area too long; it patrols the walls until it sees you, then bites, and gives up when you leave that area. |
| Teleporter | Local teleporters lead somewhere in the same region of the world (5 x 5 chunks); global ones (subtype 0), each in its own locked room, can take you anywhere in the maze built so far. The pairing is random and made when a teleporter is first used. |
| Player avatar | You. Interact with an unused avatar (Ctrl + direction) to activate it; when you die you come back in the next activated one. |

## Building the game

You need a C++23 compiler (GCC 13 or newer), CMake, and these libraries:
* liballegro5 - better install all of it along with dev packages,
* googletest (libgtest-dev) - for the unit tests
* rapidjson 
* openAL - we use it to play sound
* libsndfile - we use it to decode audio files

Build and test with CMake:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the game from the `GoEoOL` folder, where the `data` folder is. On Windows, the game builds with MSYS2 (UCRT64) and the same libraries; `.github/workflows/ci.yml` lists the exact packages for both systems.

The `goe-bench` target times building the world (the start, then one chunk at a time), a game tick, save/load, and a walk that swaps chunks to disk and back: `goe-bench 61 3000 60` from `GoEoOL` builds 61 chunks, about as many cells as one of the old 500x500 levels, then walks 60 chunks east and back, reporting the chunks in memory, the time per chunk written or read, and the memory used.

### Agents and Python

The game can also be played by a program, without a window, sound or keyboard, for example by a learning agent such as [exRelaxer](https://github.com/arielkonopka/exRelaxer)'s. The `goe-agent` library (`agent/`) runs the game headless; the `goe` Python package (`agent/python`) wraps it as a ViZDoom-like `goe.Game` and a Gymnasium `goe.GoeEnv`. The agent sees a circle of cells around the player (or around a fixed cell), the player's numbers and its inventory, and for each it chooses which features to see: types, subtypes, qualities and states. It acts with the game's own controls, and the reward is the score gained.

```
pip install ./agent/python                    # or: cmake -S . -B build -DGOE_BUILD_PYTHON=ON
python3 -c "import goe; env = goe.GoeEnv(); print(env.reset(seed=1)[0]['vision'].shape)"
```

Chunks can also be built from a fixed pattern of elements instead of a random maze, chunk by chunk or all of them, so an agent trains on a world chosen for it (`goe.ChunkPattern`; an example is `agent/patterns/rooms.json`).

`agent/python/README.md` describes the observation, the actions, the options and the patterns.

The repository also has the older build.sh shell script (Bash):

```
./build.sh --help
Garden of Eris or Obnoxious Labirynth build script for GNU Linux
usage:
./build.sh [-m moduleName] [-g] [-a] [-t]
-gh - install necessary packages for Ubuntu
-rt - run the tests while you build them, combine with -t and -m
-a - build all
-g - build only the game elements and link the game
-t - build only the tests
-m moduleName - build only one cpp file, if combined with -t, then the test case file is build.
examples:
# all of these examples will also link all the changes to the binaries
# Build all
./build.sh -a
# Build unit tests for bElem
./build.sh -t -m bElem-test`
# Build bElem, soundManager, presenter, bElem-tests
./build.sh -m bElem -m soundManager -m presenter -t -m bElem-test
```

### Diagrams

[docs/diagrams](docs/diagrams/README.md) holds PlantUML diagrams of the code: class diagrams of the elements, the world, the application and the agent library, a component diagram, activity diagrams of the application, a game tick and chunk generation, sequence diagrams of a player step, saving and an agent step, and state diagrams of the title screen, an element's life and a guardian drone. Render them again with `plantuml -tsvg *.puml` in that folder after a change.

## Main assumptions

1. The game features only a randomly generated maze, so it does.

2. Everythin' should be placed willy-nilly, with no discernible pattern.

3. It must be possible to walk from any steppable spot to any other, if we do away with all the doors and teleporters.

4. The game has no end: one endless maze, built chunk by chunk as the player walks (see The endless world). The further from the start, the fewer holes in the maze walls, and the harder it gets, in five steps (see Difficulty).

5. Teleporters add shortcuts.
  * Two types of teleporters exist: local and global.
    - Global teleporters are a special subtype 0, each in its own locked room, in about every fifth chunk. They pair with any other global teleporter in the world.
    - Local teleporters share a subtype within a region of 5 x 5 chunks and are walk-in teleporters.
6. When elements on the board move, they don't replace each other but step on top of one another. We start with a board chock-full of empty elements, then create new elements that step onto the empty ones. With mechanics, we manage a vector of live elements (those in need of their mechanics to run). The vector is inspected, and each element's mechanics are executed. 
7. The destruction of elements occurs by adding their ID and a timestamp to a separate vector. Later on, this vector is scanned, and when their time elapses, the disposeElement() method is executed on the respective element.
8. We strive to avoid code duplications whenever we can. That said, this rule has been bent a few times, especially with newly introduced code.


## Random maze generator

My implementation of [recursive division](https://en.wikipedia.org/wiki/Maze_generation_algorithm) has some deliberate modifications. For eg. first few divisions are made to be more or less equal - the dividing walls can be set only in certin range of places, instead all.

# The endless world
The whole game happens on one board that has no edges. Its cells are kept in chunks of 64 x 64 (`chamber`), and cell coordinates may be negative. A new game builds the start chunk, where the player is, and every chunk up to two chunks away (`worldBuilder`). While you play, the nearest missing chunk within those two chunks is built, one per tick; a chunk takes a few milliseconds. Beyond that the world is not there yet, and nothing can step into it.

Each chunk owns the wall along its west and north edges. The gaps in a wall come from the world seed and the wall's place only, so both chunks beside a wall know where it is open whichever is built first. The cells next to every gap are cleared, so every gap leads into the maze of both chunks. Inside, a chunk is the recursive-division maze above, filled like the old levels were. A chunk is built behind a fence (`chamber::fence`): nothing placed while it is built can reach into the chunks next to it, so a chunk comes out the same for one world seed, in whatever order the player makes the chunks appear.

Only elements within two chunks of the player run; the ones further away wait until the player comes back.

A chunk takes about 3 MB of memory, so chunks more than three chunks from the player go to disk, one per tick, into a folder in the system's temporary folder that is removed when the game closes (`gameSerializer::swapOutChunk`). When the player comes within two chunks of one again, it is read back as it was left, with its fog of war, its monsters where they were and whatever you dropped there; a chunk on disk is never built anew. So memory stays at about 7 x 7 chunks however far you walk (`goe-bench` walking 120 chunks out and back: about 155 MB, where keeping every chunk took about 500 MB after only 30). A chunk where an avatar stands stays in memory. Teleporters and golden apples in a chunk on disk still count: a teleporter can pick one on disk as its other end, and a link to one on disk reads that chunk back when it is used. A save keeps the chunks on disk too.

# Random element placement

During the labirynth creation, we create a [spanning tree](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/docs/spanningtree.gif). Every node can have multiple children (usually 4), and every node has a parent, except for the head, which has no parent. You can check the diagram in docs folder.

Every node has a surface (a number of available spaces), which is calculated using algorythm: 

If the node has children, the surface is a sum of the children's surfaces, otherwise calculate the surface by the node dimensions (width x height).

On node deletion:

We delete all the children, and recalculate the surface - we travel to the root, and update the sums. If the node is the last child, we delete the parent.

First we construct a start list of the objects:

* player (possibly multiple elements)
* a gun - one
* two keys of the type 0
* the space must be closed with door type 0

and then we search the spanning tree for the locations that would be sufficient to contain the elements.

Of these locations we pick randomly a location, where we would scatter the objects.

On the object scatter, we get the list of available fields (we check the neighboorhood if we do not try to take the place in a passage.)

We lock the space with the doors of the type 0

We set the parent node to deny more doors in the node.

We delete the node that we just filled from the spanning tree.

Now we construct lists of elements to be placed on the board, we also calculate, if we want to close the spaces, and we search the appropriate spaces, until there is no more space left.

# skins.json file

This file contains the skin definition for the game. It has quite a flexible design, you can for eg. have different animations for different subtypes of element, that are turned in different directions.

It also contains the music and sound information.

`FogBitmap` sets what covers the places nobody sees, e.g. `"FogBitmap": "data/graph/fog.png"`. The bitmap is tiled over the world and stays put while the view scrolls; a power-of-two size such as 256x256 is safest on phones. Without the entry, or when the file cannot be loaded, the fog is the plain dark colour. The default fog is drawn by `tools/make-fog.py`.


**TBD**

# Mechanics
## Moving and collecting
When an element walks into a cell, the rules are tried in this order (`include/motion.h`): collect what is there, step onto it, push it one cell further, interact with it. A collector always collects first and only then steps onto the cell the collectible uncovered, in the same move; if the collect fails, it does not step. Nothing is ever placed on top of a collectible it could pick up.

A cell holds a stack of elements (a floor, maybe something lying on it, maybe someone standing there). Only `chamber::place` and `chamber::lift` change a stack.

## Live elements
There exists a vector containing "mechanical" elements. We add elements that possess certain mechanics, such as shooting, walking, or performing actions autonomously. However, the animation phases are managed differently, enabling objects without mechanics to still have animated sprites.
There are two methods:

 1. void registerLiveElement(std::shared_ptr<bElem> who);
 2. void deregisterLiveElement(unsigned int instanceId);

The first method is for registering a mechanical object (which requires an implemented mechanics method), while the second method is for deregistering the object.

## Apples
When an apple is unbeschädigt, it acts as a collectible token that must be gathered. However, if it becomes damaged (for example, by being shot at), it transforms into a healing device. When a player (or any other element capable of collecting) acquires the item, it will heal the collector. But be warned: the apple will deplete its own energy. When the energy reaches zero, the apple will explode in the inventory, resulting in the untimely demise of the collector.
## Teleporters
Every new teleporter is added to a registry (a vector of weak pointers, guarded by a mutex, so it is). A chunk being built publishes its teleporters only once it is complete, so a teleporter never links into a half-built chunk. As soon as our player interacts with a teleporter, we're checkin' if it has an attached link to its corresponding teleporter mate. We take a gander at the type of the teleporter, and we follow these steps:

 * If there's no established link, we pick a random teleporter from our list and remove the interacted one along with the chosen one. We set the chosen one to be "LEFT" (it will become a receiver). Teleporters play no music: there are no separate chambers any more, only the endless maze.
 * We then set the chosen teleporter as the other end of the connection. 
 * Once the other end is all set up, we inspect the teleporter for any steppable fields. 
 * If we find one, 
    - we whisk the object that interacted away from their original location 
    - and place 'em right into that steppable field we found in the teleporter.


## Shooting guns
A gun that runs out of ammo is thrown away. The next gun of the same kind (type and subtype) in the inventory takes over; if there is none, the next gun of any kind does.


A plain gun shoots plain missiles. It is used by an element that can collect it (or create it like bunker) and can use it. A gun takes the operators dexterity, then finds a random value that will be used to decrease missile energy.
Then after the shot, the guns energy is halved. It restores with mechanics() calls. So the faster you shoot, the weaker shots you produce.

# Stats

Element stats is a class that will be responsible for element's stats. You can have a monster, that could shoot and gain better skills with time. :)

The player's level is **Dex** in the HUD: floor(log5(hits + 1)), where hits counts everything your shots and explosions hit. Level 1 takes 4 hits, level 2 takes 24, level 3 takes 124, and so on in fives. Your sight radius grows with the steps you take.

# Unit tests

The unit tests use GoogleTest and live in the unitTests directory, one test program per *.cpp file (22 of them now). CMake builds each one, and `ctest` runs them all from the `GoEoOL` folder. The agent library has its own: `agent-test` (GoogleTest) and, with `-DGOE_BUILD_PYTHON=ON`, `python-tests` (pytest). regression-test keeps one test for every bug fixed, so it doesn't come back.


# Sound

Our game now possesses the capability to generate sounds. You can utilize the playSound method with two arguments: eventType and event. These will be employed to locate your sound, along with additional data that will be processed for you.

We are using openAL as our audio driver, allowing us control over object positions and providing further versatility. The configuration of sound data is stored in the "samples" field of the configManager data. It is organized similarly to the sound configuration in the JSON file:

    "Samples": [
        {
           "SubType":0,
           "stEvents":[
            {
                "EventType": "walk",
                "eventData": [
                    {
                         "Event": "walkOn",
                         "fname": "fname",
                         "name": "Step sounds of a player",
                         "description": "This is a sample with step sound of a player",
                         "allowMulti": false,
                         "modeOfAction":0,
                         "stacking": false
                    }
                  ]
             }
          ]
        }       
     ]

Sample data is contained in a structure that you can access like configObject->samples[typeId][subtypeId][EventType][Event]. We have a singleton soundManager class. It includes a registerSound method. You pass the element instance ID, element type ID, element subtype ID, event type, and event. The method will load the sample, if necessary, and we construct a sample tree-like structure (with hashmaps).

However, there are limitations:

 * The distance must be accurate; it is in the config file MaxSoundDistance
 * The sound must originate from the same chamber as the listener (player)

Sounds are placed around the player in board cells (include/soundSpace.h): right on the screen is the right ear, up is in front and down is behind, and the volume halves with each doubling of the distance. OpenAL can only place mono sounds, so stereo samples are mixed down to mono when they are loaded; music keeps its stereo.

The music follows the difficulty D: song k of the music list in skins.json plays from D = k on, and the last song plays on past the end of the list, so the list is ordered from the calmest to the wildest. A new song fades in over the old one for 5 seconds, and a song plays for at least 23 seconds before D can change it, so walking back and forth over a distance step does not flip the music (include/difficulty.h, include/difficultyMusic.h).

When the element that generated the sound is removed or disposed of, only looping sounds are stopped, while others have the opportunity to cease playing by themselves.

We manage sounds by maintaining a pool of sources (openAL) in a circular buffer, which aids in locating the oldest samples. When we register the sample (play it), we first search for unregistered samples; if unsuccessful, we look for samples played in a loop.

There are control switches that modify sound handling:

 * allowMulti - Do we permit multiple instances of the sound? If set to false, attempting to play the sound while it is already playing will have no effect, suitable for looping sounds like humming noises activated by proximity or similar.
 * modeOfAction - 0 for regular, 1 for looping
 * stacking - If we allow multiple sounds, do we let them play, or should we stop the sound currently playing and start anew upon request (false), or permit all instances to play while avoiding collisions by applying a delay if the previous sound did not have the chance to play?

## Save and load
F5 saves the whole world (every chunk built so far, also the ones on disk, every element with its inventory and timers, the random generator's state) and the world seed, so chunks built after a load fit the ones built before it, to one binary file, and F9 loads it back. Esc saves the same way before going back to the title screen; if saving fails, a message says so and the game goes on. The file starts with a format version; a newer game still loads older saves.

## Difficulty
The game gets harder the better you get and the further you go. The difficulty D, shown as "D:" next to "Dex:" in the HUD, is the sum of:

 * the player level: floor(log5(hits + 1)), the same number as Dex,
 * the distance: floor(log2(1 + d / 64)), where d is how far the player is from the starting room.

The maze gets harder with distance too. A chunk's depth is the same distance rule counted from the start chunk, capped at 4: 0 at the start, 1 from the next chunk, 2 from three chunks out, 3 from seven, 4 from fifteen. Its maze walls have 5 - depth holes, and it gets 23 landmines per step of depth in its pick table.

Every rule that depends on D or depth lives in include/difficulty.h: bunker range and rest between shots, camera and guardian sight, guardians per camera, kiki beam damage, maze holes and landmines per chunk, and the Hound. The Hound is a drone sent after a player who stays in one 64x64 area too long (230 seconds at D 1, 23 seconds less per step, never under 55 seconds); it bites, and gives up when the player leaves the area. The tunings follow the Law of Fives: every step, cap and floor is built from 5 or 23.

- Refactor sound engine
- Refactor chamber, to contain bElem container, which then would have the stepOnElement routines???
~~- Refactor the engine, to have only elements on the same board to be active. This will make a lot ot things very tricky, especially teleporting elements between boards with active elements in the inventory.~~
~~- Add sound gain on music and samples~~
~~- Add new type of a gun, that would shoot bombs - grenade launcher~~
~~- Add landmine, a steppable, that would kill you~~
~~- Add a bot and a camera, when a player is near a camera, all bots are notified about the position~~
- Add fire/electric door that can be switched with a switch (indestructable)
- Add switches that will be linked to the energy doors
  
  - Add reasonable mechanism for linking switches with doors. Like doors should have two sitches on both sides, unless the sub-chamber contains a teleport, then it could have only one switch inside.
  - Different switches

    - Locking - you step on it, it changes state, you step out it does not change anything
    - nonLocking - when you step on it, it changes state, when you step out it changes the state back - you need to push something on it
  - What to do when the switch is destroyed:

    - the switch should be removed from the board, therefore no more switching would be possible. the door that it controlled would be set to closed for ever.

- multiStacked object, like a tank with a turret, that would rotate on it's own. The presenter class should already be able to handle to some point such an object.


## bElem attributes
The config file now will have entries to configure elements attributes, like being steppable, being movable and so on. 
    int subType=-1;
    bool killable=false;
    bool destroyable=false;
    bool steppable=false;
    bool isMovable=false;
    bool isInteractive=false;
    bool isCollectible=false;
    bool canPush=false;
    bool canBePushed=false;
    bool canCollect=false;
    bool isWeapon=false;
    bool isOpen=false;
    bool isLocked=false;
    int energy=100;
    int maxEnergy=100;
    int ammo=0;
    int maxAmmo=0;



- Planned next (see the design notes): energy doors with switches, crumbling floor, laser gate, armor as a player stat, the Friend, Hostile and Neutral NPCs (trading on bump, scaling as 5^n with your level), and the Altar of Eris.
- The endless world, next steps (see the chunked world design): regions of chunks with their own name, colour and music; global teleporters that open a far away chunk.

## Art and numbers
New tiles use Discordian symbols: the golden apple, the Sacred Chao, pentagons, and Eris' gold and red. Gameplay numbers follow the Law of Fives: they are built from 5 or 23.

## ChangeLog
* PlantUML diagrams in docs/diagrams: classes, components, activities, sequences and states.
* A hall of fame: the ten best games with name, score and date, kept in the save folder. A lost game good enough for it asks for a name. The title screen shows it after the demo.
* The demo starts far out in the maze, where it is hardest, and stops after 30 seconds. The wait before it, its length and how long the hall of fame shows are set in Config.
* A demo on the title screen: after 60 to 120 seconds without a press on the main menu, an autopilot plays a world of its own until any key, pad button or stick push brings the menu back. The demo world is discarded and saves are left alone.
* Agents can build chunks from a fixed pattern of elements (`goe.ChunkPattern`, from rows and a legend, arrays or a JSON file), for one chunk or for every new chunk. A pattern smaller than a chunk is repeated to fill it, and it is laid out the same whatever the seed.
* Game over shows the best score among the avatars of the lost game, and deletes that game's save, so Continue cannot bring a lost game back. Saves keep the best score of avatars already lost (save format 7; older saves still load).
* The title screen offers New game next to Continue when a save exists. A new game overwrites the old save right away, so Continue always brings back the game you played last.
* Agents can play: a headless `goe-agent` library and a `goe` Python package (a ViZDoom-like game and a Gymnasium environment), with a circle of vision around the player, the player's numbers and inventory, each with its features chosen by the agent. The same seed plays the same game.
* Chasers play fair: the hunter, guardians, cameras and the Hound only know where you are by seeing you, and walls, brick clusters, bunkers, teleporters and closed doors block their view. When you slip out of sight they go to where they saw you last; when you are not there, or they never saw you, they patrol along the walls of the maze.
* Esc saves the game and goes back to the title screen without losing an avatar, and the title screen has Continue while a readable save exists. Giving up an avatar moved to Backspace. Saves keep the world seed now (save format 6; older saves still load).
* The music changes with the difficulty: each step of D brings the next song of the music list, with a crossfade. It replaces the songs that were placed around the start of the map.
* A story scroller: a random story from `data/txt/stories.json` scrolls along the top of the screen whenever a new chunk of the maze is made. Config switches it on or off and picks the stories file (English, Polish or Romanian, or your own). Fixed a stray bracket that made `stories.pl.json` unreadable.
* Doors stand in the holes they close: at the edges of chunks a door used to stand one cell in front of the hole. Global teleporters no longer play music, since there are no separate chambers any more.
* The build is warning-free with `-Wall -Wextra -Wpedantic -Wshadow`, which are now on by default. Fixed along the way: handing an inventory to a new owner made every item its own collector.
* Explosions are heard again: landmines, bombs and bazooka missiles went off in silence because the blast never asked for its sound.
* Puppet masters speak: each kind of controller says "controller enabled" in its own language, in a S.A.M.-like robot voice (made with espeak-ng by `tools/voices/make-controller-voices.sh`).
* Config has music and sound effects volumes, and a Controls screen to choose the keys and pad buttons for every action.
* The fog is a bitmap now, set by `FogBitmap` in skins.json (a dark Discordian haze with gold pentagons by default), and the light is centred on whoever sees. The fog shaders are lighter and GLSL ES compatible for a later phone port: a small visibility mask is painted on the CPU and read with one filtered lookup, instead of every pixel looping over every view point.
* Far chunks go to disk: chunks more than three chunks from you are written to a temporary folder and read back, as you left them, when you come near again, so memory stays flat however far you walk. Saves keep them (save format 5; older saves still load).
* Fixed: after a load or a new game, the old world could stay in memory for the rest of the game, because an explosive kept holding on to its board.
* One endless maze instead of separate levels: the maze is built in 64x64 chunks around you as you walk and never ends, so no other levels are built in the background any more. There is no winning; collecting every apple found so far no longer ends the game. The maze gets harder the further you go from the start (fewer holes in its walls, more landmines), and D is now your level plus your distance from the start. Global teleporters link anywhere in the maze built so far. Older saves still load.
* Walking into a collectible picks it up and steps onto its cell in the same move; before, the collector stayed where it was and had to move again.
* Fixed: every missile fired stayed in memory, and in the save file, for the rest of the game. Save files are smaller now; older saves still load.
* Tidier elements: the base element class lost a dozen unused methods, and moving, sounds, drawing offsets and cell stacks each live in one small place of their own.
* Choosing guns: Z jumps to the next kind of gun instead of stepping through identical ones, and the HUD shows how many of the selected kind you carry. When a gun runs out of ammo, another gun of the same kind takes over, or any other gun if there is none, without losing the shot.
* Losing the last avatar no longer closes the window without a word (it looked like a crash): a Game over screen shows the score and returns to the title screen for a new game. Quitting closes the window at once instead of freezing while a background level finishes.
* Crash reports: a crash writes `crash-<date>-<time>.log` with a stack trace into the save folder, on Linux and Windows, no debugger needed.
* Fixed thread races between the game and the levels built in the background: golden apples, music registration, the game clock, and a new level's player taking over the game.
* Separate randomness for the game, level building, music and visual effects, so building levels in the background no longer shares a random generator with the running game. One world seed rebuilds the same levels (`--seed <number>`).
* Sounds come from where they happen: a shot to the left is heard on the left.
* Difficulty D (player level + chamber depth + distance) shown in the HUD; bunkers, cameras, guardians and kiki beams get tougher with it. New landmine and the Hound, with Discordian tiles. Tunings follow the Law of Fives.
* Title screen with Start, Config and Exit; the save folder is set in Config and kept in settings.json.
* Save and load (F5 / F9, `--load <file>`).
* Controllers: a drone's behaviour comes from the puppet master driving it (patrol, collector, hunter, wall follower, guardian). Security cameras with guardian drones.
* Teleporters pair at random through a thread-safe registry; local and global teleporters.
* Much faster: a 500x500 level builds in about half a second instead of over a minute, and a game tick takes about a third of a millisecond.
* Modern C++23 with no raw pointers or new/delete; Boost is gone, tests use GoogleTest; one CI for Linux and Windows.
* Now kiki si not placed so danesly, still glitches happen.
* Fixed monster - more to go, whole monster mechanics must be rewritten
* Added death ray contraption system kiki & bouba
* Constants have their namespace
* Now bElem types have their own namespace
* A bit of code cleaning, that sound manager desperately needs refactoring. now the sounds are created with proper coordinates, normalized, that openAL expects
* Removed that awful requirement for self pointer in additionalProvisioning
* Teleporter: theOtherEnd is now a weak pointer, this should mitigate problems with teleport destruction from the memory. It the destructed is a receiver, it will be disabled forever, if sender, it will be connected with another available node.
* Teleporters now are all active at the beginning, then the ones that are chosen to be the receiver, get inactivated, and their music stops, if they are of type 0 (interchamber teleports)
* Added normalization of the sounf effects coordinates, velocities and position of the listener. 
* Introduced a GL pixelshader that now renders our cloak
* Did some cleaning up, especially the singletons. Now they are more or less written in compliant way, at least should be thread safe.
* removed two unneded classes: movableElements and mechanical and non steppable
* Extended the interface, now we can drop usables, like broken apples, and we also see the broken apple, and how much more energy does it have
* Added a bazooka weapon, the graphics is pancerfaust60.
* Explosives were refactored a bit, now the explode method can be used instead of destroy.
* Joystick initial support. Now the InputManager is a singleton with its own thread, that can be started/stopped only once, it's purpose is to get the controller state to any app, that demands it.
* The cloaking mechanism is mostly done, what is left is optimizations. Removed one bitmap from use. Introduced drawing bitmaps with different alpha(transparency), now we do not beed blur75/50/25, and therefore are removed from the code.
* Introduced partial tile copying to create roundish views
* Introduced multi uncovered points. We simply add an object to a list with viewpoint::get_instance()->addViewPoint(std::shared_ptr<bElem> in). We also can change the main viewpoint, to make the "view to follow" different objects
* I reformatted the skins.json file. This time, I had to create a formatting tool for the file. It's an awk script and can be used much like any other awk script.
* I further refactored the code, removing references to textures and video configuration. Instead, there is now a specialized singleton class responsible for obtaining the right texture.
* I refactored the bunker and added sounds for collecting collectibles.
* I started a major refactoring process, which resulted in the creation of two new classes: bElemStats and bElemAttrs. These classes will likely flatten the structure of bElems in the future as I plan to implement mechanics and other hooks using hashmaps of lambdas.
* I began creating configurable element attributes.
* I introduced an object called puppetMaster, which is missing the graphical hooks but compiles. The patrolling drone will have different brains that will control it.
* I made some changes to the patrolling drone, and now it requests a brain object on interaction.
* I made changes to the Inventory, and now we can request a single token.
* I introduced board cloaking, which works on two levels. First, we only draw those elements that are or were in the field of view. Then, we apply a cloaking mask with a hole cut out for the player. The radius of the hole can be controlled with a variable.
* Now, every object can have its own animations for death, teleportation, destruction, and fading out. The last one is not supported yet.
* I changed the way tiles are displayed on the screen. In the first phase, we only draw still tiles, whether they are on the floor standing still or they make up the floor. Then, all the moving elements are displayed in the second round, and the active player is drawn at the end. This sequence may seem strange, but since I implemented the animation of transitioning between tiles, some situations became weird. For example, when we drag an object while turning in circles, sometimes the object obscures the player, and sometimes it's the opposite. We don't have an isometric view, and that looks awkward. It's also worth noting that in the second pass, only moved objects are drawn, and in the first one, only still objects. This has its drawbacks. For instance, we can't handle a situation where two fields close to each other have a still object stepping on a moved object that would move under a still object on the other field. So if we'd like to build a tunnel where, for example, trains could pass by and be seen obscured by some kind of semitransparent roof, then we would fail.
* I changed pointers from raw to managed.
* I removed the garbage collector as it is no longer necessary.
* I enabled the first sounds but encountered the first issue: there is a limited number of available sources, so I cannot have as many as I want. I still have plenty, but I will have to sort them by the distance to the player and remove the farthest ones.








