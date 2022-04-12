#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "nonSteppable.h"
#include "killableElements.h"

class collectible : public killableElements
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    collectible(chamber *board,gCollect *garbage);
    collectible(chamber *board,gCollect *garbage,int x, int y);
    virtual bool isSteppable();
    virtual ~collectible();
    virtual bool isCollectible();
protected:

private:
};

#endif // COLLECTIBLE_H
