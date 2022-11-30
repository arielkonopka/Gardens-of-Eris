
# Garden of Eris or Obnoxious labirynth.

Welcome to my pet project. I started it in [Python](https://github.com/arielkonopka/pyLurker), but got to the conclusion, that Python would not deliver sufficient speed with the number of elements I plan to have at the same time.

From the beginning, this project is not *product oriented*, but rather it is a way to spend little time on building somthing. I stopped writting code many years ago, and I simply wanted to feel the joy of coding again. Since my life is rather not that *computer centric*, I spend fairly little time one the project, and therefore it advances slowly.

**This is work in progress. It usually works, but be warned.**


## Why the idea

I thought of writting similar kind of a game, since I was a kid. Few years back, I added some code to [GNU Robbo project](http://gnurobbo.sourceforge.net/), and it was fun, but that was pure C and SDL1.2. 

So as soon as I decided to do some coding, I thought of a game to make. I thought of the game's story, and now I'm coding it. In the repo you can find an ubrello5 file with the idea, how the classes in the game should look like. This diagram is far from being finished as well.
I thought of a random level generator because of two reasons:

- I would not have to write level editor
- I would not have to design the levels

In the meanwhile I got a third reason, I can test it fairly fast, without even loading and saving the level data.
 
[You can chek the fairly recent video of the gameplay.](https://youtu.be/wQ-Lpc4TViM)


## The game story

Ones upon a time, the [Goddes](https://en.wikipedia.org/wiki/Eris_(mythology)) went to her garden, and to all the shock, she had noticed, that all her golden apples had been gone. She as usual got furious. 

Now you, the [Discordian Pope](https://en.wikipedia.org/wiki/Discordianism) got yourself a mission. The goddes scattered your avatars across the strange world, that she had created for her apples. You must collect all of them, and bring them to your Goddess.

## Building the game.
To build the game, you need following libraries:
liballegro5 - better install all of it along with dev packages,
libboost - I would recommend all of it - including unit-test-framework
The repository is equipped with build.sh shell script (Bash), you can use it to build the game and the unit tests.


## Main assumptions

1. The game has only random generated levels. 
2. Everything should be randomly placed. 
3. It must be possible to walk from any steppable place on a board [class chamber](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/chamber.h) to any other steppable place, if we remove all the doors and teleports.
4. The game actually should be very very large. 5 different chambers are created with different number of holes in the walls - that is something like the difficulty level. The chambers are connected with teleports. There are two types of teleports: stable one - always teleports you to other chamber, than you are currently in, but are bidirectional, you can always come back. Unstable teleports (subType==0) - they are not bidirectional like the stable ones, they randomly get the counter part, and then the counterpart will randomly choose its counter part and so on...
5. Elements on board do not replace each other when they are moved, they instead step on each other. So we start with a board full of [empty](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/bElem.h) elements. We then create new elements and step on the board empty elements. It goes deeper. If we call ellement's mechanics, it would call the mechanics of all the elements that are below (with a flag suggesting, that something is standing on it). I also plan, to have the mechanics being run on collected elements (that could allow to make a shield, or fake apples, which would kill the player, if not used - thrown - possibilities are endless)
6. No code duplications, whenever possible. Now that rule is broken with the video engine, but the engine is to be modified anyway, so this will get eliminated.

## Random maze generator

My implementation of [recursive division](https://en.wikipedia.org/wiki/Maze_generation_algorithm) has some deliberate modifications. For eg. first few divisions are made to be more or less equal - the dividing walls can be set only in certin range of places, instead all. 


# Random element placement
During the labirynth creation, we create a spanning tree. Every node can have multiple children (usually 4), and every node has a parent, except for the head, which has no parent.
Every node has a surface (a number of available elements), which is calculated like this: if the node has children, the surface is a sum of the children's surfaces, otherwise calculate the surface by the node dimensions.
On node deletion, we delete all the children, and recalculate the surface - we travel to the root, and update the sums. If the node is the last child, we delete the parent.
First we construct a start list of the objects:
* player (possibly multiple elements)
* a gun - one
* two keys of the type 0
and then we search the spanning tree for the locations that would be sufficient to contain the elements.
Of these locations we pick randomly a location, where we would scatter the objects.
On the object scatter, we get the list of available fields (we check the neighboorhood if we do not try to take the place in a passage.
We lock the space with the doors of the type 0
We set the parent node to deny more doors in the node.
We delete the node that we just filled

Now we construct lists of elements to be placed on the board, we also calculate, if we want to close the spaces, and we search the appropriate spaces, until there is no more space left.

## skins.json file 

This file contains the skin definition for the game. It has quite a flexible design, you can for eg. have different animations for different subtypes of element, that are turned in different directions.


TBD

## Mechanics
The game uses two timers. One is used for scrolling and general screen refresh rate. The other is used to perform game mechanics calculations. This way we do not have to review the whole board every frame. It should be good to think of another method of 
updating fames.
There is a vector with "mechanical" elements. We add elements that have some mechanics (like they shoot, walk, do something on their own), but the animation phases are handled differently, so objects without mechanics still can have animated sprites.
There are methods:
  1. void registerLiveElement(bElem* who);
  2. void deregisterLiveElement(bElem* who);
One is for registering a mechanical object (there has to be implemeted mechanics method), the other one deregisters the object.


# Teleporters
Every new teleporter is placed in a vector (actually it is a vector of pointers). As soon, as the player interacts with a teleporter it checks if it has attached link to a corresponding teleporter.
We check the type of a teleporter, if it is even, we:
    If not, we take randomly chosen teleporter from the list, and we remove the interacted element from the list. We set the chosen teleport as the other end.
    If other end is established, we check if the teleporter has any steppable fields in it. If so, we take the player from the original location, and place it into the steppable field found in the teleport.
When it is odd:
    We find the first teleport of the same type but on different chamber. Then we establish connection, where the counterpart will direct to the first teleport.





# Shooting guns
A plain gun shoots plain missiles. It is used by an element that can collect it (or create it like bunker) and can use it. A gun takes the operators dexterity, then finds a random value that will be used to decrease missile energy. 
Then after the shot, the guns energy is halved. It restores with mechanics() calls. So the faster you shoot, the weaker shots you produce.

# Stats 
Element stats is a class that will be responsible for element's stats. You can have a monster, that could shoot and gain better skills with time. :)

## Unit tests.
The unit tests should be written in *.cpp files that should be located in unitTests directory.
When running build.sh, the unit tests would be built as well. You can then run them, they are built as separate executables.


## TODO
* add blocking mechanism for the garbage collector, so we could like attach foreign stats (or whole bElems) to other objects, these objects could have intependent activity and timespan.
  like imagine a case:
    a player shoots a shot and dies just right after.
    the bullet goes for a while and hits something. (flies longer than the player lives), we would not want the garbage collector to destroy the player, until the bullet finishes its existance.
    The player would be removed from the board, just the garbage collector would not clean it out.



## ChangeLog
Now every object can have its own animations of death, teleport, destruction and fadingOut, the last one is not supported yet

