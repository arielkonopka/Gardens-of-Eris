
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

#include <stddef.h>
#include <memory>
#include <vector>
#include <algorithm>

#include <array>
#include "elementFactory.h"
#include <cmath>

#define _belemType -1
#define _floorType 0
#define _stillElemType 1
#define _rubishType 2
#define _wallType       4
#define _movableType    5
#define _monster 6
#define _patrollingDrone 7
#define _puppetMasterType 77
#define _brickClusterType 8
#define _player  (100)
#define _collectible 50
#define _key 51
#define _door 52
#define _plainMissile 201
#define _plainGun 200
#define _bunker 250

#define _bazookaMissileType 205
#define _bazookaType 204
#define _bazookaMaxSteps 40
#define _teleporter 400
#define _teleportationTime (_mov_delay*15)
#define _teleportStandTime 55

#define _goldenAppleType 900
#define _stash 600
#define _explosivesType 601
#define _simpleBombType 602


#define _maxWaitingTtime 65535
#define _mov_delay   8
#define _mov_delay_push   9

#define _plainGunAmmo 10
#define _plainMissileSpeed 4
#define _bazookaMissileSpeed 4
#define _plainMissileEnergy 100
#define _plainGunCharge 10

#define _boubaType 555
#define _kikiType 556

#define _dexterityLevels 64

#define _initialDexterity 1

#define _defaultEnergy 80
#define _defaultKillTime 25
#define _defaultDestroyTime 99

#define _defaultRecurrenceDepth 35


#define _interactedTime 10


#define confFname1 "./data/skins.json"
#define confFname2 "./GoEoOL/data/skins.json"

const static int _dividerCloak=8;


/**
 * @struct coords
 *
 * @brief A structure representing two-dimensional coordinates on a game board.
 *
 * This structure contains two integers: `x` and `y` to represent horizontal (x-axis) and vertical (y-axis) positions on a board.
 * It also includes several operator overloads and a function to allow for intuitive manipulation of the coordinates.
 *
 * @var int x
 * Horizontal position.
 *
 * @var int y
 * Vertical position.
 *
 * @fn bool operator==(coords a) const
 * Check if two coordinates are equal. Returns `true` if both `x` and `y` values are equal.
 *
 * @fn bool operator!=(coords a) const
 * Check if two coordinates are not equal. Returns `true` if `x` or `y` values are different.
 *
 * @fn coords operator+(coords a)
 * Add two coordinates. Returns a new `coords` object with `x` and `y` values being the sum of `x` and `y` values of the operands.
 *
 * @fn coords operator-(coords a)
 * Subtract a coordinate from another. Returns a new `coords` object with `x` and `y` values being the difference of `x` and `y` values of the operands.
 *
 * @fn float distance(const coords& a) const
 * Calculate the Euclidean distance between two points represented by `coords` objects.
 * Returns the square root of the sum of squares of differences in `x` and `y` coordinates.
 *
 * @fn coords operator%(coords a)
 * Perform element-wise modulus operation on coordinates. Returns a new `coords` object with `x` and `y` values being the modulus of the `x` and `y` values of the operands.
 *
 * @fn coords operator*(int a)
 * Multiply a coordinate by an integer. Returns a new `coords` object with `x` and `y` values multiplied by the integer `a`.
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


} coords;
const coords NOCOORDS = coords(-65535, -65535);

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
    const std::array<coords, 5> directionToCoordsMap = {
            (coords){0, -1},  // UP
            (coords){-1, 0},  // LEFT
            (coords){0, 1},   // DOWN
            (coords){1, 0},    // RIGHT
            (coords){0,0},      //NOP
      };
    /**
     * @brief dirToCoords, allows easily calculate new position
     * just add direction vector to location
     * @param d
     * @return coords - direction vector
     */
    inline coords dirToCoords(direction d){
        return directionToCoordsMap[(int)d];
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
