
# Garden of Eris or Obnoxious Labyrinth

Welcome to my pet project. I started it in [Python](https://github.com/arielkonopka/pyLurker), but got to the conclusion, that Python would not deliver sufficient speed with the number of elements I plan to have at the same time.

From the beginning, this project is not *product oriented*, but rather it is a way to spend little time on building something. I stopped writing code many years ago, and I simply wanted to feel the joy of coding again. Since my life is rather not that *computer centric*, I spend fairly little time one the project, and therefore it advances slowly.

**This is work in progress. It usually works, but be warned.**

# Recent build status

Main branch build status: tests with the game:

[![C/C++ CI](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/c-cpp.yml)

Sonarcloud static analysis:

[![SonarCloud](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/sonarcloud_main.yml/badge.svg)](https://github.com/arielkonopka/Gardens-of-Eris/actions/workflows/sonarcloud_main.yml)

## Why the idea

I thought of writting similar kind of a game, since I was a kid. Few years back, I added some code to [GNU Robbo project](http://gnurobbo.sourceforge.net/), and it was fun, but that was pure C and SDL1.2.

Years have passed and I did not write any code. So one day I decided to do some coding, and thought of a game to make. I thought of the game's story, which I yet cought only briefly. 

In the repo you can find an ubrello5 file with the idea, how the classes in the game should look like. This diagram is far from being finished as well.

I thought of a random level generator because of two reasons:

- I would not have to write level editor
- I would not have to design the levels

In the meanwhile I got a third reason:

   I can test it fairly fast, without even loading and saving the level data. Which by the way I will have to implement eventually.
[You can check the fairly recent video of the gameplay.](https://youtu.be/H6MmLlQes_0)

## The game story

Once upon a time, the [Goddes](https://en.wikipedia.org/wiki/Eris_(mythology)) went to her garden, and to all the shock, she had noticed, that all her golden apples had been gone. She as usual got furious.

Now you, a [Discordian Pope](https://en.wikipedia.org/wiki/Discordianism) have got to recover all the apples lost in different reality. 

The Goddes scattered your avatars across the strange world you found yourself in. All the lost apples are there. Collect them, but be extra carefull, they might explode, they can be broken. Then bring them back to the Goddes, you will be rewarded. 

Everytime your avatar is killed, you are respawned in first unused and activated avatar on your way. If you did not activate any avatars, you die.

The Labirynth is occupied by various creatures and devices. You can find gun modules, and worker drones, as guarding drones, doors, keys, movable turrets, immobile turrets. Also other nasty things (TBC).

## Building the game

To build the game, you need following libraries:
* liballegro5 - better install all of it along with dev packages,
* libboost - I would recommend all of it - including unit-test-framework
* rapidjson 
* openAL - we use it to play sound
* libsndfile - we use it to decode audio files

The repository is equipped with build.sh shell script (Bash):

`./build.sh --help
Garden of Erif or Obnoxious Labirynth build script for GNU Linux
usage:
./build.sh [-sq] [-m moduleName] [-g] [-a] [-t]
-gh - install necessary packages for Ubuntu
-rt - run the tests while you build them, combine with -t and -m
-a - build all
-g - build only the game elements and link the game
-t - build only the tests
-m moduleName - build only one cpp file, if combinet with -t, then the test cse file is build.
-sq - build and run the tests just for the sonar qube analysis.
examples:
# all of these examples will also link all the changes to the binaries
# Build all
./build.sh -a
# Build unit tests for bElem
./build.sh -t -m bElem-test`
# Build bElem, soundManager, presenter, bElem-tests
./build.sh -m bElem -m soundManager -m presenter -t -m bElem-test`

## Main assumptions

1. The game has only random generated levels.
2. Everything should be randomly placed.
3. It must be possible to walk from any steppable place on a board [class chamber](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/chamber.h) to any other steppable place, if we remove all the doors and teleports.
4. The game actually should be very very large. 5 different chambers are created with different number of holes in the walls - that is something like the difficulty level. 
   * The chambers are connected with teleports. 
   * There are two types of teleports: Internal and inter-chamber
      Inter-chamber is the same kind of a teleport, but of special subtype 0. There will be one teleporter of that subtype on each chamber.
      Internal teleporters share common subtype per chamber. These are walk in teleporters. 
5. Elements on board do not replace each other when they are moved, they instead step on each other. 
    So we start with a board full of [empty](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/floorElement.h) elements. We then create new elements and step on the board empty elements. 
    
    With mechanics we deal with a vecotr of live elements (that require their mechanics to be run), then the vecotr is inspected, and each element's mechanics is run. That is how we deal with destruction of inanimate objects, they enable the mechanics by registering a live object.
    
    With that the mechanics method will deal with removing the object from the board. If we do not register mechanics, it will only look like it is being destroyed, but will survive.
    
6. No code duplications, whenever possible. Now that rule is bent few times, especially with newlly introduced code.



## Random maze generator

My implementation of [recursive division](https://en.wikipedia.org/wiki/Maze_generation_algorithm) has some deliberate modifications. For eg. first few divisions are made to be more or less equal - the dividing walls can be set only in certin range of places, instead all.

# Random element placement

During the labirynth creation, we create a spanning tree. Every node can have multiple children (usually 4), and every node has a parent, except for the head, which has no parent. You can check the diagram in docs folder.

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

There is a vector with "mechanical" elements. We add elements that have some mechanics (like they shoot, walk, do something on their own), but the animation phases are handled differently, so objects without mechanics still can have animated sprites.
There are methods:

  1. void registerLiveElement(bElem* who);
  2. void deregisterLiveElement(bElem* who);

One is for registering a mechanical object (there has to be implemeted mechanics method), the other one deregisters the object.

## Apples
When an apple is intact it is a collectible token, that must be collected. But when it is broken (like shot at), then it becomes a healing device.
When a player (or any other element, that can collect), collects the item, it will heal the collector.
But it would deplay its own energy, when energy reaches 0, then the apple explodes in the inventory, killing the collector.

## Teleporters

Every new teleporter is placed in a vector (actually it is a vector of pointers). As soon, as the player interacts with a teleporter it checks if it has attached link to a corresponding teleporter.
We check the type of a teleporter, if it is even, we:
    If not, we take randomly chosen teleporter from the list, and we remove the interacted element from the list. We set the chosen teleport as the other end.
    If other end is established, we check if the teleporter has any steppable fields in it. If so, we take the player from the original location, and place it into the steppable field found in the teleport.
When it is odd:
    We find the first teleport of the same type but on different chamber. Then we establish connection, where the counterpart will direct to the first teleport.

## Shooting guns

A plain gun shoots plain missiles. It is used by an element that can collect it (or create it like bunker) and can use it. A gun takes the operators dexterity, then finds a random value that will be used to decrease missile energy.
Then after the shot, the guns energy is halved. It restores with mechanics() calls. So the faster you shoot, the weaker shots you produce.

# Stats

Element stats is a class that will be responsible for element's stats. You can have a monster, that could shoot and gain better skills with time. :)

# Unit tests

The unit tests should be written in *.cpp files that should be located in unitTests directory.
When running build.sh, the unit tests would be built as well. You can then run them, they are built as separate executables.


# Sound

Our game now has the ability to produce sounds.
Just inherit after audibleElement, and then you cn use playSound method with two arguments: eventType, and event. These will be used to locate your sound, along with other data, that will be done for you.
We are using [openAL](https://www.openal.org/documentation/openal-1.1-specification.pdf) as our audiodriver, it is because we can have control over object positions, and just because we can. 
The configuration of the sound data is contained in "samples" field of the configManager data. It is ordered similarily to the sound configuration in JSON file:

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

The sampleData is contained in structure that you can access like configObje->samples[typeId][subtypeId][EventType][Event]
We got a singleton soundManager class. It has a method register sound. You pass element instance id, element type id, element subtypeid, event type, and event. The method will load the sample - if necessary, we build sample tree like structure (with hashmaps)- and then would play it.
But there are limitations:

* the distance must be right, it is in config file _MaxSoundDistance_
* the sound must come from the same chamber as the listener (player)
When the element, that generated sound is removed disposed, only looping sounds are stopped, other have the chance to stop playing by themselvs.

We handle the sounds by having the pool of sources (openAL), which we keep in a circular buffer. That helps us to find oldest samples. When we register the sample (play it) we first seek unregistered samples, if we fail with that, we seek samples that are played in loop (to kill them), and if we fail at that, we kill any first sample we try to overtake.

There are controlling switches, that change the sound handling:

* allowMulti - do we allow multiple instances of the sound? If set to false, try to play the sound, while it already plays, will have no effect, good for looping sounds like humming noises activated by proximity or something alike
* modeOfAction - 0 - regular, 1 - looping
* stacking - if we allow multiple sounds, do we allow them to play, or should we just kill the sound that is playing, and start over on request (false), or allow all instances to play, but to avoid collisions, apply delay, if the last sound did not have the chance to play - to avoid unnecessary amplicifactions of the sound (true)


## TODO

- Add new type of a gun, that would shoot bombs - grenade launcher
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


## ChangeLog

Now every object can have its own animations of death, teleport, destruction and fadingOut, the last one is not supported yet



- changed the way tiles are shown on the screen. Now in the first phase we draw only still tiles. Because they are either on the floor standing still, of they are the floor. Then all the moving elements are being displayed in second round, and at the end active player is drawn. 

    The sequence may seem weird, but since I implemented the animateion of progressing between the tiles, some situations got weird. Like when we drag an object with but turning in circles. Sometimes the object obscures player, sometimes it is the opposite. We do not have an isometric view, and that looks awkward.    
    We have alse to point out, that in second pass only moved objects are drawn, and in the first one only stills. This has it's cons. Like we now cannot handle a situation like this, we got two fields close to each other, that have a still object, that is stepping on a moved object, that would move under a still object on the other field. So if we'd like to build a tunnel, where for eg. trains could go by, and we could see it obscured by somekind of semitransparent roof, then we would fail.
    

- changed pointers from raw to managed
- removed Garbage collector, as it is not necessary anymore
- enabled first sounds, stumbled upon first issue, there is a limited amount of sources, available, so I cannot really have as many as I want... Still have plenty
    I will have to sort them by the distance to the player and remove the farthest ones.



