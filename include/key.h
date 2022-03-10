#ifndef KEY_H
#define KEY_H

#include "collectible.h"
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

class key : public collectible
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    key(chamber *board,gCollect *garbage);
    key(chamber *board,gCollect *garbage,int x,int y);
    key(chamber *board,gCollect *garbage,int subtype);
    virtual ~key();

protected:

private:
};

#endif // KEY_H
