#ifndef COMMONS_H
#define COMMONS_H

#define _belemType 0
#define _stillElemType 1
#define _rubishType 2
#define _nonSteppableType 3
#define _wallType       4
#define _movableType    5
#define _monster 6
#define _player  (100)
#define _collectible 50
#define _key 51
#define _door 52
#define _plainMissile 201
#define _plainGun 200
#define _bunker 250
#define _teleporter 400
#define _stash 600


#define _mov_delay   2
#define _mov_delay_push   3

#define _plainGunAmmo 10
#define _plainMissileSpeed 0
#define _plainMissileEnergy 100


#define _dexterityLevels 50

#define _initialDexterity 5

#define _defaultEnergy 80
#define _defaultKillTime 10
#define _defaultDestroyTime 10

#define _interactedTime 3
#define NOCOORDS   ((coords){-1,-1})
#define NOSTATS ((stats){-1,-1})
typedef struct coords
{
    int x=-1,y=-1;
    inline bool operator==(coords a)
    {
        if (a.x==x && a.y==y)
            return true;
        else
            return false;
    };
    inline bool operator!=(coords a)
    {
        if (a.x!=x || a.y!=y)
            return true;
        return false;
    }

} coords;

typedef enum { UP=0,LEFT=1,DOWN=2,RIGHT=3} direction;
 typedef struct controlItem
 {
     int type; /*-1 - nocommand, 0-move,1-shoot,2-interact,3-gun cycle,4-use element in inventory,5 - cycle inventory,6 - die, 7 - exit */
     direction dir;

 } controlItem;

typedef enum { NONE=0,GUN=1,AMMO=2,ARMOR=3,POWERUP=4} modType;

/* when the element mechanics is run, we need the result of what happened to be passable further down the inheritance hierarchy, that is why we need more status types */
typedef enum { CHANGED=0,UNCHANGED=1,ELEMENTOUT=2,ELEMENTIN=3 } mechanicResult;

typedef struct stats
{
    int strength;
    int energy;
    int dexterity;

} stats;

typedef struct neighboorhood
{
    bool steppableClose[4]; //up,left,down,right
    bool steppableOther[4]; //up-left,down-left,down-right,up-right
    bool steppable[8]; // whole neighboorhood, starting from up, then up-left, left, down-left, down, down-right, right, up-right

} sNeighboorhood;
 #endif
