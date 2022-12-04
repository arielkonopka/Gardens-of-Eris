#ifndef USABLE_H
#define USABLE_H

//#include "commons.h"
#include "collectible.h"


class usable : virtual public bElem
{
public:
    usable();
    virtual ~usable();
    usable(chamber *board);
    virtual bool use(bElem *who);
    virtual bool use(bElem *who,direction dir);
    virtual bool isUsable();

protected:

private:
};

#endif // USABLE_H
