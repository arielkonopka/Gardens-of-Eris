#ifndef INVENTORY_H
#define INVENTORY_H
#include "commons.h"
//#include "bElem.h"
#include <vector>
#include <map>
class bElem;
class chamber;

typedef struct tType
{
    int tokenType;
    int tokenSubtype;
    auto operator=(const tType& it) const
    {
        return (this->tokenType==it.tokenType && this->tokenSubtype==it.tokenSubtype);
    };
    auto operator<(const tType& it) const
    {
        if(it.tokenType==this->tokenType)
            return this->tokenSubtype<it.tokenSubtype;

        return this->tokenType<it.tokenType;
    };
    auto operator>(const tType& it) const
    {
        if(it.tokenType==this->tokenType)
            return this->tokenSubtype<it.tokenSubtype;

        return this->tokenType>it.tokenType;
    };



} tType;

class inventory
{
public:
    inventory(bElem *owner);
    virtual ~inventory();
    virtual void updateBoard();
    virtual void changeOwner(bElem* who);
    virtual bElem* getKey(int type,int subType,bool removeIt);
    virtual bElem* getUsable();
    virtual bool nextUsable();
    virtual bElem* getActiveWeapon();
    virtual int countTokens(int type, int subtype);
    virtual bool nextGun();
    virtual bool addToInventory(bElem* what);  // this will qualify the category of the object, that will be collected, if object could not be collected, return false
    virtual int requestTokens(int number,int type,int subType); // this will "burn" number of tokens, that are of a type and a subType, if there are less tokens, it will return number of tokens remaining, otherwise 0
    virtual bool removeActiveWeapon();
    virtual bool mergeInventory(inventory* theOtherInventory);
    virtual bool removeToken(int position);
    virtual bool isEmpty();
    virtual bool removeCollectibleFromInventory(int instance);
    virtual bool findInInventory(int instanceId);
    std::vector<bElem*> weapons;
    std::vector<bElem*> mods;
    std::vector<bElem*> tokens;
    std::vector<bElem*> usables;
    std::vector<bElem*> keys;
protected:
    int wPos;
    int uPos;
private:
    virtual void incrementTokenNumber(tType token);
    virtual void decrementTokenNumber(tType token);
    bElem *owner;
    std::map<tType,int> tokenNumbers;
};

#endif // INVENTORY_H
