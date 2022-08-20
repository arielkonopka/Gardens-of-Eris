#ifndef USABLE_H
#define USABLE_H

//#include "commons.h"
#include "collectible.h"


class usable : public collectible
{
public:
    usable();
    virtual ~usable();

    usable(chamber *board);
    usable(chamber *board,bool registerEl);
    usable(chamber *board,int x, int y);
    usable(chamber *board,int x, int y, bool registerEl);
    virtual bool use(bElem *who);
    virtual bool use(bElem *who,direction dir);
    virtual bool isCollectible();
    virtual bool isUsable();

protected:

private:
};

#endif // USABLE_H
