
# Garden of Eris

Welcome to my pet project. I started it in [Python](https://github.com/arielkonopka/pyLurker), but got to the conclusion, that Python would not deliver sufficient speed with the number of elements I plan to have at the same time.

From the beginning, this project is not *product oriented*, but rather it is a training. I stopped writting code many years ago, and I simply wanted to feel the joy of coding again. Since my life is rather not that *computer centric*, I spend fairly little time one the project, and therefore it advances slowly.

**This is work in progress. It usually works, but be warned.**


## Why the idea

I thought of writting similar kind of a game, since I was a kid. Few years back, I added some code to [GNU Robbo project](http://gnurobbo.sourceforge.net/), and it was fun, but that was pure C and SDL1.2. 

So as soon as I decided to do some coding, I thought of a game to make. I thought of the game's story, and now I'm coding it. In the repo you can find a [Dia](https://en.wikipedia.org/wiki/Dia_(software)) [file](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/Diagram1.dia?raw=true), with the idea, how the classes in the game should look like. This diagram is far from being finished as well.
I thought of a random level generator because of two reasons:

- I would not have to write level editor
- I would not have to design the levels

In the meanwhile I got a third reason, I can test it fairly fast, without even loading and saving the level data.
 



## The game story

Ones upon a time, the [Goddes](https://en.wikipedia.org/wiki/Eris_(mythology)) went to her garden, and to all the shock, she had noticed, that all her golden apples had been gone. She as usual got furious. 

Now you, the [Discordian Pope](https://en.wikipedia.org/wiki/Discordianism) got yourself a mission. The goddes scattered your avatars across the strange world, that she had created for her apples. You must collect all of them, and bring them to your Goddess.



## Main assumptions

1. The game has only random generated levels. 
2. Everything should be randomly placed. 
3. It must be possible to walk from any steppable place on a board [class chamber](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/chamber.h) to any other steppable place, if we remove all the doors and teleports.
4. The game actually should never end. Player gets to a teleport to another chamber at some point. When the player reaches that, new chamber is created, and a link between chambers is established - the best option would be to have the teleport class, that would contain that link. The newly created world should contain a teleport to the prevoius one, this way we can get the player to go between chambers. This way we could theoretically plan element placements for more than one chamber. Like we can place a key to specific door in different chamber, and behind that door, we can place golden apples.
5. Elements on board do not replace each other when they are moved, they instead step on each other. So we start with a board full of [empty](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/bElem.h) elements. We then create new elements and step on the board empty elements. It goes deeper. If we call ellement's mechanics, it would call the mechanics of all the elements that are below (with a flag suggesting, that something is standing on it). I also plan, to have the mechanics being run on collected elements (that could allow to make a shield, or fake apples, which would kill the player, if not used - thrown - possibilities are endless)
6. No code duplications, whenever possible. Now that rule is broken with the video engine, but the engine is to be modified anyway, so this will get eliminated.

## Random maze generator

My implementation of [recursive division](https://en.wikipedia.org/wiki/Maze_generation_algorithm) has some deliberate modifications. For eg. first few divisions are made to be more or less equal - the dividing walls can be set only in certin range of places, instead all. 

Placing the items can be a bit tricky. I thought of few strategies:

1. Scatter randomly across all the steppable places on the maze (maze generated first)
2. Because of how the maze is generated, we can exactly locate the moment, when the space is not divided anymore. That is where we can place our stuff, that gives us nice options, like place stuff by te walls, place stuff in the middle of free space.
3. Place elements in walls.
4. Search for the best fit for spaces requested, we use the way maze was generated to identify appropriate spaces, which we then have scattered with objects. - there is a lot to be done here


Since we use the recursive division methon, I thought that the subchambers can have their own *coordinates*.
First rectangle would be **B**, as beginning, then it would be divided in 4 rectangles: a, b, c, d. Those rectengles would be divided into a, b, c, d, and so on...
THen we notice, that the rectangles processed by the algorythm would look like this: 

Ba, Baa, Baaab, Baaac, Baaad, Bab, Bac, Bad, Bb, Bc, Bd... 

We can now decide where to place our objects, we assign an object to coordinates, and then scatter it across the rectangle.

# Random element placement
I finally started implementing this. Now it works like this, until there is 10me space available, randomly pick objects to be placed, then find a chamber that fits the best in size. Mark that chamber banned, so it will not be used another time.
Sometimes close the chamber with doors and loose the key somewhere.



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
If not, we take randomly chosen teleporter from the list, and we remove the interacted element from the list. We set the chosen teleport as the other end.
If other end is established, we check if the teleporter has any steppable fields in it. If so, we take the player from the original location, and place it into the steppable field found in the teleport.


# Shooting guns
A plain gun shoots plain missiles. It is used by an element that can collect it (or create it like bunker) and can use it. A gun takes the operators dexterity, then finds a random value that will be used to decrease missile energy. 
Then after the shot, the guns energy is halved. It restores with mechanics() calls. So the faster you shoot, the weaker shots you produce.



## Random Level Generator

## TODO
* there is probably a bug with mechanics deregistration, which causes a segfault from time to time. I removed most of the references to deregistration. We should not remove the objects from vector, while we are using iterators.
