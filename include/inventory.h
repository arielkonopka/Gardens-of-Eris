#ifndef INVENTORY_H
#define INVENTORY_H
#include "commons.h"
#include "bElem.h"
#include <vector>
class bElem;

class inventory
{
    public:
        inventory();
        virtual ~inventory();
        virtual bElem* getKey(int type,int subType,bool removeIt);
        virtual bElem* getUsable();
        virtual bool nextUsable();
        virtual bElem* getActiveWeapon();
        virtual void mechanics();
        virtual bool nextGun();
        virtual bool addToInventory(bElem* what);  // this will qualify the category of the object, that will be collected, if object could not be collected, return false
        virtual int requestTokens(int number,int type,int subType); // this will "burn" number of tokens, that are of a type and a subType, if there are less tokens, it will return number of tokens remaining, otherwise 0
        virtual bool removeActiveWeapon();
        virtual bool mergeInventory(inventory* theOtherInventory);
        virtual bool removeToken(int position);
        std::vector<bElem*> weapons;
        std::vector<bElem*> mods;
        std::vector<bElem*> tokens;
        std::vector<bElem*> usables;
        std::vector<bElem*> keys;
    protected:
        int wPos;
        int uPos;
    private:
};

#endif // INVENTORY_H
