
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

Ones upon a time, the [Goddes](https://en.wikipedia.org/wiki/Eris_(mythology)) went to her garden, and to all the shock, she notices, that all her golden apples were gone. She as usual got furious. 

Now you, the [Discordian Pope](https://en.wikipedia.org/wiki/Discordianism) got yourself a mission. The goddes scattered your avatars across the strange world, that she had created for her apples. You must collect all of them, and bring them to your Goddess.



## Main assumptions

1. The game has only random generated levels. 
2. Everything should be randomly placed. 
3. It must be possible to walk from any steppable place on a board [class chamber](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/chamber.h) to any other steppable place, if we remove all the doors and teleports.
4. The game actually should never end. Player gets to a teleport to another chamber at some point. When the player reaches that, new chamber is created, and a link between chambers is established - the best option would be to have the teleport class, that would contain that link. The newly created world should contain a teleport to the prevoius one, this way we can get the player to go between chambers. This way we could theoretically plan element placements for more than one chamber. Like we can place a key to specific door in different chamber, and behind that door, we can place golden apples.
5. Elements on board do not replace each other when they are moved, they instead step on each other. So we start with a board full of [empty](https://github.com/arielkonopka/Gardens-of-Eris/blob/main/include/bElem.h) elements. 
6. No code duplications, whenever possible. Now that rule is broken with the video engine, but the engine is to be modified anyway, so this will get eliminated.

## Random maze generator

My implementation of [recursive division](https://en.wikipedia.org/wiki/Maze_generation_algorithm) has some deliberate modifications. For eg. first few divisions are made to be more or less equal - the dividing walls can be set only in certin range of places, instead all. 

Placing the items can be a bit tricky. I thought of few strategies:

1. Scatter randomly across all the steppable places on the maze (maze generated first)
2. Because of how the maze is generated, we can exactly locate the moment, when the space is not divided anymore. That is where we can place our stuff, that gives us nice options, like place stuff by te walls, place stuff in the middle of free space.
3. Place elements in walls.

I chose 1 and 3 (not implemented yet). Option one is a bit modified. When we place an element on a board, we call it's method selfAlign(), and it should self-align in the given neighborhood.










## skins.json file 

This file contains the skin definition for the game. It has quite a flexible design, you can for eg. have different animations for different subtypes of element, that are turned in different directions.


TBD



