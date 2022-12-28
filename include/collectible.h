#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "nonSteppable.h"
#include "killableElements.h"

class collectible : virtual public bElem
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    collectible();
    explicit collectible(std::shared_ptr<chamber> board);
    virtual bool isCollectible();

};

#endif // COLLECTIBLE_H
