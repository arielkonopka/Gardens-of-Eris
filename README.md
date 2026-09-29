
# Garden of Eris or Obnoxious Labyrinth

Greetings and welcome to me little passion project, so. I began this adventure with [Python](https://github.com/arielkonopka/pyLurker), but soon enough I found meself realizin' that Python might not have the swiftness needed for the grand number of elements I'd like to be workin' with simultaneously.

Now, from the get-go, this project isn't focused on creating a product, but rather it's about takin' a wee bit of time to build somethin' for the joy of it. I hung up me coding hat a good few years back, and I simply wished to feel that delightful thrill of crafting code once more. Since me life doesn't revolve 'round computers, I can only spare a modest bit of time for the project, which means it's movin' along at a leisurely pace.

So, here we have a work in progress. It tends to function well enough, but consider yerself warned, me friend.

# Recent build status

Main branch build status:

[![CI](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/ci.yml/badge.svg)](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/ci.yml)
[![SonarCloud](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/sonar.yml/badge.svg)](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/sonar.yml)

Every push builds the game and runs the unit tests on Linux and on Windows (MSYS2), and packs each build as a download on the run's page. Pushing a version tag also publishes both builds as a GitHub release:

```
git tag v0.1.0
git push origin v0.1.0
```

The SonarCloud analysis with test coverage runs as its own workflow, so its badge is separate from the build badge.

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

The Goddess has dispersed your avatars throughout this peculiar world you now find yourself in. The missing apples are hidden here – collect them with great care, for they may explode or break. Return the apples to the Goddess, and she'll reward you handsomely.

Every time your avatar perishes, you'll respawn in the first unused and activated avatar on your path. If you haven't activated any avatars, you'll meet your end.

The labyrinth is inhabited by an assortment of creatures and contraptions. You'll encounter gun modules and worker drones, as well as guard drones, doors, keys, movable turrets, and immobile turrets. Keep an eye out for other malevolent entities (TBC).



## Playing the game

The game opens with a title screen: **Start**, **Config** and **Exit**. Config sets the folder the game is saved to; it is kept in `settings.json` next to the game. Starting a game builds the first maze right away, and the other levels are built in the background while you play.

| Key | What it does |
|---|---|
| Arrows or W A S D | walk |
| Shift + direction | shoot the selected gun |
| Ctrl + direction | interact with, or pick up, what is next to you |
| Alt + direction | drag what is behind you |
| X | select the next usable item |
| Z | select the next gun |
| Space | use the selected item |
| R | drop the selected item |
| Esc | give up this avatar (you come back in the next activated one) |
| F5 / F9 | save / load the game (`savegame.goe` in the save folder) |

`GardenOfEris --load <file>` starts straight from a saved game, without the title screen.

The HUD shows your score (**P**), your level (**Dex**, see Stats) and the current difficulty (**D**, see Difficulty).

## Elements of the gardens

| Element | What it does |
|---|---|
| Floor, wall | The maze itself. Floors come in a few looks; walls stand until they don't. |
| Brick cluster | Can't be killed, but can be pushed around and blown up. Also used to block passages instead of a door. |
| Door and key | A door opens for a key of its colour. Rooms are often locked, and the key is usually placed somewhere else. |
| Golden apple | The thing you are here for. Shoot one and it becomes a healing item that drains itself and finally explodes in your inventory. |
| Plain gun, bazooka | Weapons to pick up. Shooting fast makes weaker shots. |
| Simple bomb | Explodes when shot or hit by another explosion. |
| Landmine | Looks almost like floor: a pentagon plate with the Sacred Chao on it. Goes off when anything steps on it and takes that thing with it. Only the harder levels have them, and the harder the level, the more. |
| Monster | Roams the maze. Monsters collect things if their skin allows it (`canCollect` in `skins.json`), and some have no inventory on purpose. |
| Bunker | A fixed turret that looks along its four lines and fires at you. |
| Kiki and bouba | Kiki is a death-ray emitter; the beam is made of boubas, which hurt whatever stands in them. |
| Patrolling drone and puppet master | A drone does nothing until you hand it a puppet master (a controller). The controller decides how it moves: **patrol** (wanders, and becomes an extra camera for you), **collector** (goes for collectibles it can see), **hunter** (chases you around walls when you are near), **wall follower** (keeps a hand on the wall and walks the maze). |
| Security camera | Watches for you. When it sees you, its guardian drones come to check the spot. Guardians fight you when they see you, shoot along clear lines, and never go further than 55 cells from their camera. |
| The Hound | A red drone with the golden apple on its hull. It is sent after you when you stay in one 64x64 area too long; it bites, and gives up when you leave that area. |
| Teleporter | Local teleporters lead somewhere in the same level; global ones (subtype 0) can take you to another level. The pairing is random and made when a teleporter is first used. |
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

The `goe-bench` target times level generation, a game tick, and save/load on a large level: `goe-bench 500 3000` from `GoEoOL`.

The repository also has the older build.sh shell script (Bash):

```
./build.sh --help
Garden of Eris or Obnoxious Labirynth build script for GNU Linux
usage:
./build.sh [-sq] [-m moduleName] [-g] [-a] [-t]
-gh - install necessary packages for Ubuntu
-rt - run the tests while you build them, combine with -t and -m
-a - build all
-g - build only the game elements and link the game
-t - build only the tests
-m moduleName - build only one cpp file, if combined with -t, then the test case file is build.
-sq - build and run the tests just for the sonar qube analysis.
examples:
# all of these examples will also link all the changes to the binaries
# Build all
./build.sh -a
# Build unit tests for bElem
./build.sh -t -m bElem-test`
# Build bElem, soundManager, presenter, bElem-tests
./build.sh -m bElem -m soundManager -m presenter -t -m bElem-test
```

## Main assumptions

1. The game features only randomly generated levels, so it does.

2. Everythin' should be placed willy-nilly, with no discernible pattern.

3. In a  chamber, it must be possible to traverse from any steppable spot to another, if we do away with all the doors and teleporters.

4. The game ought to be vast, with chambers in five levels of challenge, each with a varyin' number of holes in the walls. Fewer holes make a deeper, harder chamber (see Difficulty).

5. The chambers connect through teleporters.
  * Two types of teleporters exist: internal and inter-chamber.
    - Inter-chamber teleporters are a special subtype 0, with one such teleporter in each chamber.
    - Internal teleporters share a common subtype within a chamber (the chamber's id + 1) and are walk-in teleporters.
6. When elements on the board move, they don't replace each other but step on top of one another. We start with a board chock-full of empty elements, then create new elements that step onto the empty ones. With mechanics, we manage a vector of live elements (those in need of their mechanics to run). The vector is inspected, and each element's mechanics are executed. 
7. The destruction of elements occurs by adding their ID and a timestamp to a separate vector. Later on, this vector is scanned, and when their time elapses, the disposeElement() method is executed on the respective element.
8. We strive to avoid code duplications whenever we can. That said, this rule has been bent a few times, especially with newly introduced code.


## Random maze generator

My implementation of [recursive division](https://en.wikipedia.org/wiki/Maze_generation_algorithm) has some deliberate modifications. For eg. first few divisions are made to be more or less equal - the dividing walls can be set only in certin range of places, instead all.

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


**TBD**

# Mechanics
There exists a vector containing "mechanical" elements. We add elements that possess certain mechanics, such as shooting, walking, or performing actions autonomously. However, the animation phases are managed differently, enabling objects without mechanics to still have animated sprites.
There are two methods:

 1. void registerLiveElement(std::shared_ptr<bElem> who);
 2. void deregisterLiveElement(unsigned int instanceId);

The first method is for registering a mechanical object (which requires an implemented mechanics method), while the second method is for deregistering the object.

## Apples
When an apple is unbeschädigt, it acts as a collectible token that must be gathered. However, if it becomes damaged (for example, by being shot at), it transforms into a healing device. When a player (or any other element capable of collecting) acquires the item, it will heal the collector. But be warned: the apple will deplete its own energy. When the energy reaches zero, the apple will explode in the inventory, resulting in the untimely demise of the collector.
## Teleporters
Every new teleporter is added to a registry (a vector of weak pointers, guarded by a mutex, so it is). A level being built publishes its teleporters only once it is complete, so a teleporter never links into a half-built level. As soon as our player interacts with a teleporter, we're checkin' if it has an attached link to its corresponding teleporter mate. We take a gander at the type of the teleporter, and we follow these steps:

 * If there's no established link, we pick a random teleporter from our list and remove the interacted one along with the chosen one. We set the chosen one to be "LEFT" (it will become a receiver) and pause its song. We could unpause them, but I don't think it makes sense.. 
 * We then set the chosen teleporter as the other end of the connection. 
 * Once the other end is all set up, we inspect the teleporter for any steppable fields. 
 * If we find one, 
    - we whisk the object that interacted away from their original location 
    - and place 'em right into that steppable field we found in the teleporter.


## Shooting guns

A plain gun shoots plain missiles. It is used by an element that can collect it (or create it like bunker) and can use it. A gun takes the operators dexterity, then finds a random value that will be used to decrease missile energy.
Then after the shot, the guns energy is halved. It restores with mechanics() calls. So the faster you shoot, the weaker shots you produce.

# Stats

Element stats is a class that will be responsible for element's stats. You can have a monster, that could shoot and gain better skills with time. :)

The player's level is **Dex** in the HUD: floor(log5(hits + 1)), where hits counts everything your shots and explosions hit. Level 1 takes 4 hits, level 2 takes 24, level 3 takes 124, and so on in fives. Your sight radius grows with the steps you take.

# Unit tests

The unit tests use GoogleTest and live in the unitTests directory, one test program per *.cpp file (14 of them now). CMake builds each one, and `ctest` runs them all from the `GoEoOL` folder. regression-test keeps one test for every bug fixed, so it doesn't come back.


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

When the element that generated the sound is removed or disposed of, only looping sounds are stopped, while others have the opportunity to cease playing by themselves.

We manage sounds by maintaining a pool of sources (openAL) in a circular buffer, which aids in locating the oldest samples. When we register the sample (play it), we first search for unregistered samples; if unsuccessful, we look for samples played in a loop.

There are control switches that modify sound handling:

 * allowMulti - Do we permit multiple instances of the sound? If set to false, attempting to play the sound while it is already playing will have no effect, suitable for looping sounds like humming noises activated by proximity or similar.
 * modeOfAction - 0 for regular, 1 for looping
 * stacking - If we allow multiple sounds, do we let them play, or should we stop the sound currently playing and start anew upon request (false), or permit all instances to play while avoiding collisions by applying a delay if the previous sound did not have the chance to play?

## Save and load
F5 saves the whole world (every level, every element with its inventory and timers, the random generator's state) to one binary file, and F9 loads it back. The file starts with a format version; a newer game still loads older saves.

## Difficulty
The game gets harder the better you get and the further you go. The difficulty D, shown as "D:" next to "Dex:" in the HUD, is the sum of:

 * the player level: floor(log5(hits + 1)), the same number as Dex,
 * the depth of the chamber: 0 for the easiest levels up to 4 for the ones with the fewest holes,
 * the distance: floor(log2(1 + d / 64)), where d is how far the player is from the chamber's starting room.

Every rule that depends on D lives in include/difficulty.h: bunker range and rest between shots, camera and guardian sight, guardians per camera, kiki beam damage, landmines per level, and the Hound. The Hound is a drone sent after a player who stays in one 64x64 area too long (230 seconds at D 1, 23 seconds less per step, never under 55 seconds); it bites, and gives up when the player leaves the area. The tunings follow the Law of Fives: every step, cap and floor is built from 5 or 23.

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
- One endless world made of chunks instead of separate levels.

## Art and numbers
New tiles use Discordian symbols: the golden apple, the Sacred Chao, pentagons, and Eris' gold and red. Gameplay numbers follow the Law of Fives: they are built from 5 or 23.

## ChangeLog
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








