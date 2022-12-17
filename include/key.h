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
    key(std::shared_ptr<chamber> board);
    key(std::shared_ptr<chamber> board,int x,int y);
    key(std::shared_ptr<chamber> board,int subtype);
    key();


};

#endif // KEY_H
