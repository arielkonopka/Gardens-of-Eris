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

#define _plainGunAmmo 10
#define _plainMissileSpeed 3
#define _plainMissileEnergy 100



#define _defaultEnergy 80
#define _defaultKillTime 20
#define _defaultDestroyTime 20

#define _interactedTime 5


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




 #endif
