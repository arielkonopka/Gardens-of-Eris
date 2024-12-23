
/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef COMMONS_H
#define COMMONS_H
#define ALLEGRO_UNSTABLE 1
#define ALLEGRO_SRC

#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>

#include <array>
#include "elementFactory.h"
#include <cmath>

/**
 * @namespace bElemTypes
 * @brief A delightful collection of arbitrary constants for game element types.
 *
 * This namespace houses the sacred numbers of Discordia, representing types of
 * game elements that will inevitably confuse and amaze you. These constants are
 * the mystical return values of the great bElem class and its numerous chaotic offspring.
 *
 * All constants here are not just numbers, they are glimpses into the cosmic prank
 * that is the game world. Use them wisely, or Eris herself may throw a golden apple
 * into your codebase. Have fun!
 *
 * The sacred categories are:
 * - **Environment elements** (e.g., floor, wall, stash): Static, predictable, and boring... unless they aren't.
 * - **Characters** (e.g., player, monsters, drones): Beware, for they may not be what they seem.
 * - **Items** (e.g., key, door, missiles): Objects of power, or maybe just distractions in Eris' grand jest.
 *
 * Remember, these constants may be arbitrary, but so is life. Hail Eris!
 */
namespace bElemTypes {
    // Base types - the foundation of chaos
    constexpr int _belemType = -1;          ///< The anti-type, representing nothingness, or perhaps everything.
    constexpr int _floorType = 0;           ///< A floor, to walk on... or maybe to slip on. Who can say?
    constexpr int _rubishType = 2;          ///< Debris or trash? One creature's trash is another creature's treasure.
    constexpr int _wallType = 4;            ///< A wall, standing tall, until it doesn't.

    // Characters - friends, foes, or indifferent cosmic entities?
    constexpr int _monster = 6;             ///< Monsters, they exist to be feared... or mocked.
    constexpr int _patrollingDrone = 7;     ///< A drone, forever patrolling, but to what end? Who gave it orders?
    constexpr int _puppetMasterType = 77;   ///< Puppet Master - the strings are pulled, but by whom? Probably Eris.
    constexpr int _brickClusterType = 8;    ///< A brick cluster - it does absolutely nothing, can't be killed, but can be destroyed and pushed around. An embodiment of stubborn inertia.
    constexpr int _player = 100;            ///< You, the player! The chosen one, or maybe just a pawn in a cosmic joke.

    // Items - the true prizes in the game... or are they?
    constexpr int _key = 51;                ///< A key! It opens doors, unless the door doesn't exist. Then it's just a key.
    constexpr int _door = 52;               ///< A door! A way in or a way out? Who can truly know?

    // Weapons and their chaotic projectiles
    constexpr int _plainMissile = 201;      ///< A missile, born of the plain gun's fiery ambition.
    constexpr int _plainGun = 200;          ///< The plain gun - simple, direct, and as subtle as a flying brick.
    constexpr int _bazookaMissileType = 205;///< The bazooka missile - bigger, louder, and the herald of chaos.
    constexpr int _bazookaType = 204;       ///< The bazooka itself - because sometimes you need a bit more boom.

    constexpr int _bunker = 250;            ///< A bunker - safe haven or tomb? Only Eris knows.
    constexpr int _teleporter = 400;        ///< A teleporter - taking you where you need to go, or where you don't.

    // Special items - more mysterious than the Golden Apple itself
    constexpr int _goldenAppleType = 900;   ///< A golden apple - oh yes, the chaos bringer. You want this, or do you?
    constexpr int _stash = 600;             ///< A stash - what's inside? Probably more chaos.
    constexpr int _simpleBombType = 602;    ///< A simple bomb - it's simple, but don't underestimate the explosion.

    // Special characters - are they Bouba or are they Kiki? Only the fates can tell.
    constexpr int _boubaType = 555;         ///< Bouba - soft and round, yet full of surprises.
    constexpr int _kikiType = 556;          ///< Kiki - sharp and edgy, just like your last existential crisis.
}


/**
 * @namespace GoEConstants
 * @brief A collection of constants that define the laws of time, space, and other trivialities.
 *
 * This namespace holds the sacred numbers that govern movement, weapon mechanics, and the passing of time
 * in the world of Discordia. These constants are the unwritten rules by which all must abide... unless
 * Eris decides to rewrite them. Proceed with caution, for these values may seem reasonable, but they are
 * the embodiment of controlled chaos.
 *
 * Categories include:
 * - **Movement delays**: Because nothing should ever happen exactly when you expect it.
 * - **Weapon mechanics**: Speed, ammo, and everything else that fuels your chaotic arsenal.
 * - **Time management**: How long things last... or don't.
 *
 * In the grand cosmic jest, these values hold the universe together. Or maybe they just slow it down. Hail Eris!
 */
namespace GoEConstants {
    // Movement delays - to keep you on your toes (or to trip you up)
    constexpr int _mov_delay = 5+3;                   ///< The default delay between movements. Because haste makes waste.
    constexpr int _mov_delay_push = 5+2+2;              ///< A little more delay when pushing something. Physics, or just Eris messing with you?

    // Weapon mechanics - the stats that drive your instruments of chaos
    constexpr int _bazookaMaxSteps = 40;            ///< Maximum steps a bazooka missile can take before it gets bored and explodes.
    constexpr int _plainGunAmmo = 5+5;               ///< Ammo for the plain gun. Plain, simple, and just enough to get into trouble.
    constexpr int _plainMissileSpeed = 5-1;           ///< Speed of the plain missile - fast enough to cause chaos, slow enough for suspense.
    constexpr int _bazookaMissileSpeed = 5;         ///< Speed of the bazooka missile - because big explosions need dramatic buildup.
    constexpr int _plainGunCharge = 5*2;             ///< Charge time for the plain gun. Patience is a virtue, but chaos doesn't care.

    // Teleportation - because who has time to walk?
    constexpr int _teleportationTime = (_mov_delay * 15); ///< Time required to teleport. Instant, but not really.
    constexpr int _teleportStandTime = 5*2*5+5;          ///< How long you must stand before teleportation. Eris likes to make you wait.

    // Time constants - because sometimes things take longer than expected
    constexpr int _maxWaitingTtime = 65535;         ///< Maximum time you can wait. Infinite? Almost.
    constexpr int _defaultKillTime = 5*5;            ///< Time it takes to "kill" something. But are they really dead? Eris might disagree.
    constexpr int _defaultDestroyTime = 5*5*5-5*5-1;         ///< Time it takes to destroy something. Slow destruction builds anticipation.

    // Miscellaneous - where the real chaos happens
    constexpr static int _dividerCloak = int(5+3);    ///< The mystical divider for the cloak. Divides what? Who knows.
    constexpr int _interactedTime = 5+5;             ///< Time of interaction. Meaningful, or just a fleeting moment of connection?
}



#define confFname1 "./data/skins.json"
#define confFname2 "./GoEoOL/data/skins.json"



/**
 * @struct coords
 *
 * @brief A structure representing two-dimensional coordinates on a game board.
 *
 * This structure contains two integers: `x` and `y` to represent horizontal (x-axis) and vertical (y-axis) positions on a board.
 * It also includes several operator overloads and a function to allow for intuitive manipulation of the coordinates.
 * Most of the operators are not vector operators as in Cartesian space, these are helpers to make things a bit easier later
 *
 * @var int x
 * Horizontal position.
 *
 * @var int y
 * Vertical position.
 *
 * @fn bool operator==(coords a) constexpr
 * Check if two coordinates are equal. Returns `true` if both `x` and `y` values are equal.
 *
 * @fn bool operator!=(coords a) constexpr
 * Check if two coordinates are not equal. Returns `true` if `x` or `y` values are different.
 *
 * @fn coords operator+(coords a)
 * Add two coordinates. Returns a new `coords` object with `x` and `y` values being the sum of `x` and `y` values of the operands.
 *
 * @fn coords operator-(coords a)
 * Subtract a coordinate from another. Returns a new `coords` object with `x` and `y` values being the difference of `x` and `y` values of the operands.
 *
 * @fn float distance(constexpr coords& a) constexpr
 * Calculate the Euclidean distance between two points represented by `coords` objects.
 * Returns the square root of the sum of squares of differences in `x` and `y` coordinates.
 *
 * @fn coords operator%(coords a)
 * Perform element-wise modulus operation on coordinates. Returns a new `coords` object with `x` and `y` values being the modulus of the `x` and `y` values of the operands.
 *
 * @fn coords operator*(int a)
 * Multiply a coordinate by an integer. Returns a new `coords` object with `x` and `y` values multiplied by the integer `a`.
 *
 *  @fn coords operator/(int a)
 *  Divide by a both coordinates, returns coords object
 *
 *  @fn coords operator/(coords a)
 *  Divide cooridinates of two vectors, so the result is coors(floor(x/a.x),floor(y/a.y))
 */

typedef struct coords
{
    int x=-1,y=-1;
    inline coords validate (coords bottom)
    {
        if (x<0 || x>=bottom.x || y<0 || y>=bottom.y)
            return coords(-65535,65535);
        return coords(x,y);
    }

    inline bool operator==(coords a) const
    {
        if (a.x==x && a.y==y)
            return true;
        else
            return false;
    };

    inline bool operator!=(coords a) const
    {
        if (a.x!=x || a.y!=y)
            return true;
        return false;
    }
    inline coords operator+(coords a)
    {
        return coords(x+a.x,y+a.y);
    }
    inline coords operator-(coords a)
    {
        return coords(x-a.x, y-a.y);
    }

    int sum2d() const
    {
        return x+y;
    }

    float distance(const coords& a) const
    {
        coords n=coords(x,y)-a;

        return ::sqrt(static_cast<float>(n.x * n.x + n.y * n.y));
    }
    inline coords operator%(coords a)
    {
        return coords(x%a.x,y%a.y);

    }
    inline coords operator*(coords a)
    {
        return coords(x*a.x,y*a.y);
    }
    inline coords operator*(int a)
    {
        return coords(x*a,y*a);
    }

    inline coords operator+(int a)
    {
        return coords(x+a,y+a);
    }
    inline coords operator-(int a)
    {
        return coords(x-a,y-a);
    }
    inline coords operator/(int a)
    {
        return coords(int(x/a),int(y/a));
    }

    inline coords operator/(coords a)
    {
        return coords(int(floor(x/a.x)),int(floor(y/a.y)));
    }

} coords;
constexpr coords NOCOORDS = coords(-65535, -65535);

/**
 * @class enum dir::direction
 *
 * @brief An enumeration representing cardinal directions.
 * This is used in various operations, including movement and interaction.
 * There is a dirToCoords function provided
 */
namespace dir
{
    enum class direction {UP=0,LEFT=1,DOWN=2,RIGHT=3,NODIRECTION=4};
    const std::array<direction,5> allDirections={direction::UP,direction::LEFT,direction::DOWN,direction::RIGHT,direction::NODIRECTION};
    const std::array<direction, 5> allDirectionsOpposite = {direction::DOWN,direction::RIGHT,direction::UP,direction::LEFT,direction::NODIRECTION};
    const std::array<coords, 5> directionToCoordsMap = {
            coords(0, -1),  // UP
            coords(-1, 0),  // LEFT
            coords(0, 1),   // DOWN
            coords(1, 0),    // RIGHT
            coords(0,0),      //NOP
      };
    /**
     * @brief dirToCoords, allows easily calculate new position
     * just add direction vector to location
     * @param d
     * @return coords - direction vector
     */
    inline coords dirToCoords(direction d){
        return directionToCoordsMap[static_cast<int>(d)];
    }
    inline dir::direction getOppositeDirection(dir::direction d) {
        return dir::allDirectionsOpposite[static_cast<int>(d)];
    }

}






/**
 * @struct controlItem
 *
 * @brief A structure representing an action to be performed by a game entity.
 *
 * @var int type
 * This variable signifies the type of command, where -1 indicates no command,
 * 0 is for movement, 1 is for shooting, 2 is for interaction, 3 is for gun cycling,
 * 4 is for using an item in inventory, 5 is for cycling through inventory,
 * 6 is for dying and 7 is for exiting.
 *
 * @var direction dir
 * This variable specifies the direction for the action. It uses the direction enumeration.
 */
typedef struct controlItem
{
    int type; /*-1 - nocommand, 0-move,1-shoot,2-interact,3-gun cycle,4-use element in inventory,5 - cycle inventory,6 - die, 7 - exit */
    dir::direction dir;

} controlItem;

/**
 * @enum modType
 *
 * @brief An enumeration representing the type of modifier.
 * This includes NONE (no modifier), GUN, AMMO, ARMOR, and POWERUP.
 */
typedef enum { NONE=0,GUN=1,AMMO=2,ARMOR=3,POWERUP=4} modType;

/**
 * @enum mechanicResult
 *
 * @brief An enumeration representing the result of a mechanic operation.
 * The values can be CHANGED, UNCHANGED, ELEMENTOUT, and ELEMENTIN.
 */
typedef enum { CHANGED=0,UNCHANGED=1,ELEMENTOUT=2,ELEMENTIN=3 } mechanicResult;

/**
 * @struct sNeighboorhood
 *
 * @brief A structure representing the properties of a game entity's neighborhood.
 *
 * @var bool steppableClose[4]
 * An array representing whether the immediately adjacent directions are steppable (UP, LEFT, DOWN, RIGHT).
 *
 * @var bool steppableOther[4]
 * An array representing whether the diagonally adjacent directions are steppable (UP-LEFT, DOWN-LEFT, DOWN-RIGHT, UP-RIGHT).
 *
 * @var bool steppable[8]
 * An array representing whether the entire neighborhood is steppable, starting from UP, then UP-LEFT, LEFT, DOWN-LEFT, DOWN, DOWN-RIGHT, RIGHT, UP-RIGHT.
 *
 * @var int nTypes[8]
 * An array representing the types of entities in the entire neighborhood, following the same order as the steppable array.
 */
typedef struct neighboorhood
{
    bool steppableClose[4]; //up,left,down,right
    bool steppableOther[4]; //up-left,down-left,down-right,up-right
    bool steppable[8]; // whole neighboorhood, starting from up, then up-left, left, down-left, down, down-right, right, up-right
    int  nTypes[8];
} sNeighboorhood;

/**
 * @struct crd3d
 *
 * A structure representing three-dimensional coordinates in space.
 * Each coordinate (x, y, z) is an integer and defaults to -1.
 *
 * @var crd3d::x
 * The X-axis coordinate.
 * @var crd3d::y
 * The Y-axis coordinate.
 * @var crd3d::z
 * The Z-axis coordinate.
 *
 * @function operator==
 * Equality operator. Returns true if the coordinates (x, y, z) of both structures are equal.
 *
 * @function distance
 * Calculates the Euclidean distance between the current coordinates and another point in three-dimensional space.
 * It uses the standard formula for distance in Euclidean space sqrt((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2).
 */

using coords3d=struct crd3d
{
    float x=-1;
    float y=-1;
    float z=-1;
    inline bool operator==(crd3d a) const
    {
        if (a.x==x && a.y==y && a.z==z)
            return true;
        else
            return false;
    };
    inline float distance(const crd3d& a) const
    {
        crd3d n = {x - a.x, y - a.y, z - a.z};
        return ::sqrt(static_cast<float>(n.x * n.x + n.y * n.y + n.z * n.z));
    };

};

/**
 * @brief Enum used as key for the statistics map in bElemStats.
 *
 * This enum defines various types of statistics that can be stored
 * in a bElemStats object. Each enum value is a key that can be used
 * to set and retrieve values from the statistics map.
 */
using pointsType = enum pt
{
    TOTAL = 0,    ///< Represents the total score.
    SHOOT = 1,    ///< Represents the shooting score.
    STEPS = 2,    ///< Represents the steps score.
    COLLECTS = 3  ///< Represents the collections score.
};


#endif
