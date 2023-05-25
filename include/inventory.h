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
    inventory() = default;
    virtual ~inventory() = default;
    virtual void updateBoard();
    virtual void changeOwner(std::shared_ptr<bElem> who);
    virtual std::shared_ptr<bElem> getKey(int type,int subType,bool removeIt);
    virtual std::shared_ptr<bElem> getUsable();
    virtual bool nextUsable();
    virtual std::shared_ptr<bElem> getActiveWeapon();
    virtual int countTokens(int type, int subtype);
    virtual bool nextGun();
    virtual bool addToInventory(std::shared_ptr<bElem> what);  // this will qualify the category of the object, that will be collected, if object could not be collected, return false
    virtual int requestTokens(int number,int type,int subType); // this will "burn" number of tokens, that are of a type and a subType, if there are less tokens, it will return number of tokens remaining, otherwise 0
    virtual std::shared_ptr<bElem> requestToken(int type,int subType); // this will take one collectible from the inventory, and return it, can be used to transfer elements
    virtual bool removeActiveWeapon();
    virtual bool mergeInventory(std::shared_ptr<inventory> theOtherInventory);
    virtual bool removeToken(int position);
    virtual bool isEmpty();
    virtual bool removeCollectibleFromInventory(int instance);
    virtual bool findInInventory(int instanceId);
    std::vector<std::shared_ptr<bElem>> weapons;
    std::vector<std::shared_ptr<bElem>> mods;
    std::vector<std::shared_ptr<bElem>> tokens;
    std::vector<std::shared_ptr<bElem>> usables;
    std::vector<std::shared_ptr<bElem>> keys;

private:
    virtual void incrementTokenNumber(tType token);
    virtual void decrementTokenNumber(tType token);
    std::shared_ptr<bElem> owner=nullptr;
    std::map<tType,int> tokenNumbers;
    int wPos=0;
    int uPos=0;
};

#endif // INVENTORY_H
