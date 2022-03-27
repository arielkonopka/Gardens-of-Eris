#ifndef USABLE_H
#define USABLE_H

//#include "commons.h"
#include "collectible.h"


class usable : public collectible
{
public:
    usable();
    virtual ~usable();

    usable(chamber *board,gCollect *garbage);
    usable(chamber *board,gCollect *garbage,int x, int y);
    virtual bool use(bElem *who);
    virtual bool use(bElem *who,direction dir);
    virtual bool isCollectible();
    virtual bool isUsable();

protected:

private:
};

#endif // USABLE_H
