
# Garden of Eris or Obnoxious Labyrinth

Greetings and welcome to me little passion project, so. I began this adventure with [Python](https://github.com/arielkonopka/pyLurker), but soon enough I found meself realizin' that Python might not have the swiftness needed for the grand number of elements I'd like to be workin' with simultaneously.

Now, from the get-go, this project isn't focused on creating a product, but rather it's about takin' a wee bit of time to build somethin' for the joy of it. I hung up me coding hat a good few years back, and I simply wished to feel that delightful thrill of crafting code once more. Since me life doesn't revolve 'round computers, I can only spare a modest bit of time for the project, which means it's movin' along at a leisurely pace.

So, here we have a work in progress. It tends to function well enough, but consider yerself warned, me friend.

# Recent build status

Main branch build status: tests with the game:

[![C/C++ CI](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/c-cpp.yml)


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



## Building the game

To build the game, you need following libraries:
* liballegro5 - better install all of it along with dev packages,
* libboost - I would recommend all of it - including unit-test-framework
* rapidjson 
* openAL - we use it to play sound
* libsndfile - we use it to decode audio files

The repository is equipped with build.sh shell script (Bash):

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

4. The game ought to be vast, with five distinct chambers creatin' different levels of challenge, each with a varyin' number of holes in the walls.

5. The chambers connect through teleporters.
  * Two types of teleporters exist: internal and inter-chamber.
    - Inter-chamber teleporters are a special subtype 0, with one such teleporter in each chamber.
    - Internal teleporters share a common subtype within a chamber and are walk-in teleporters.
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

 1. void registerLiveElement(bElem* who);
 2. void deregisterLiveElement(bElem* who);

The first method is for registering a mechanical object (which requires an implemented mechanics method), while the second method is for deregistering the object.

## Apples
When an apple is unbeschädigt, it acts as a collectible token that must be gathered. However, if it becomes damaged (for example, by being shot at), it transforms into a healing device. When a player (or any other element capable of collecting) acquires the item, it will heal the collector. But be warned: the apple will deplete its own energy. When the energy reaches zero, the apple will explode in the inventory, resulting in the untimely demise of the collector.
## Teleporters
Every new teleporter is added to a vector (in reality, it's a vector of pointers, so it is). As soon as our player interacts with a teleporter, we're checkin' if it has an attached link to its corresponding teleporter mate. We take a gander at the type of the teleporter, and we follow these steps:

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

# Unit tests

The unit tests should be written in *.cpp files that should be located in unitTests directory.
When running build.sh, the unit tests would be built as well. You can then run them, they are built as separate executables.


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
When the element that generated the sound is removed or disposed of, only looping sounds are stopped, while others have the opportunity to cease playing by themselves.

We manage sounds by maintaining a pool of sources (openAL) in a circular buffer, which aids in locating the oldest samples. When we register the sample (play it), we first search for unregistered samples; if unsuccessful, we look for samples played in a loop.

There are control switches that modify sound handling:

 * allowMulti - Do we permit multiple instances of the sound? If set to false, attempting to play the sound while it is already playing will have no effect, suitable for looping sounds like humming noises activated by proximity or similar.
 * modeOfAction - 0 for regular, 1 for looping
 * stacking - If we allow multiple sounds, do we let them play, or should we stop the sound currently playing and start anew upon request (false), or permit all instances to play while avoiding collisions by applying a delay if the previous sound did not have the chance to play?

## TODO
- Refactor sound engine
- Refactor chamber, to contain bElem container, which then would have the stepOnElement routines???
~~- Refactor the engine, to have only elements on the same board to be active. This will make a lot ot things very tricky, especially teleporting elements between boards with active elements in the inventory.~~
~~- Add sound gain on music and samples~~
~~- Add new type of a gun, that would shoot bombs - grenade launcher~~
- Add landmine, a steppable, that would kill you
- Add a bot and a camera, when a player is near a camera, all bots are notified about the position
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



## ChangeLog
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








