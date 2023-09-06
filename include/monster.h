#ifndef MONSTER_H
#define MONSTER_H
#include "objectTypes.h"
#include "chamber.h"
#include "videoElementDef.h"
#include "killableElements.h"
#include "plainGun.h"

class monster : public killableElements,  public mechanical, public movableElements
{
public:

    monster();
    explicit monster(std::shared_ptr<chamber> board);
    explicit monster(std::shared_ptr<chamber> board,int newSubtype);
    ~monster();
    int getType() const;
    virtual bool mechanics();
    virtual bool checkNeigh();
    virtual bool steppableNeigh();
    bool additionalProvisioning(int subtype,int typeId);
    bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<monster> sbe);

private:
    std::shared_ptr<plainGun> weapon=nullptr;
    bool inited=false;
    int rotA=3;
    int rotB=1;

};

#endif // MONSTER_H
