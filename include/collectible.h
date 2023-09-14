#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "nonSteppable.h"
#include "killableElements.h"

class collectible : virtual public audibleElement
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    collectible();
    explicit collectible(std::shared_ptr<chamber> board);
    virtual bool isCollectible();
    virtual void setCollected(std::shared_ptr<bElem> who);

};

#endif // COLLECTIBLE_H
