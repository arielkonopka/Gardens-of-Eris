#ifndef SIMPLEBOMB_H
#define SIMPLEBOMB_H
#include "bElem.h"
#include "explosives.h"
#include "nonSteppable.h"
#include "killableElements.h"

class simpleBomb: public explosives, public nonSteppable, public killableElements, public movableElements
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    simpleBomb(chamber* board);
    virtual ~simpleBomb();
    virtual bool hurt(int points);
    virtual bool kill();
    virtual bool destroy();
    virtual int getType();
    virtual bool mechanics();
protected:

private:
    bool exploded;
    bool triggered;
};

#endif // SIMPLEBOMB_H