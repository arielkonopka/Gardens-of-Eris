#ifndef EXPLOSIVES_H
#define EXPLOSIVES_H

#include <movableElements.h>


class explosives : public movableElements
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    explosives(chamber *board);
    explosives(chamber *board,int x, int y);
    virtual ~explosives();
    virtual bool canBeDestroyed();
    virtual bool canBeKilled();
    virtual bool explode();
    virtual bool hurt(int points);
    virtual bool kill();
    virtual bool destroy();

protected:

private:
};

#endif // EXPLOSIVES_H
