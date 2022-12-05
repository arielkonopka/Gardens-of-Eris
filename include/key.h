#ifndef KEY_H
#define KEY_H

#include "collectible.h"
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

class key : public collectible, public nonSteppable
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    key(chamber *board);
    key(chamber *board,int x,int y);
    key(chamber *board,int subtype);


};

#endif // KEY_H
