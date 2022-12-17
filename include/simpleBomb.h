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
    simpleBomb();
    simpleBomb(std::shared_ptr<chamber> board);
    virtual bool hurt(int points);
    virtual bool kill();
    virtual bool destroy();
    virtual int getType();
    virtual bool mechanics();
private:
    bool triggered=false;
};

#endif // SIMPLEBOMB_H
