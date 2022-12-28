#ifndef USABLE_H
#define USABLE_H

//#include "commons.h"
#include "collectible.h"


class usable : virtual public bElem
{
public:
    usable();
    explicit usable(std::shared_ptr<chamber> board);
    virtual bool use(std::shared_ptr<bElem> who);
    virtual bool use(std::shared_ptr<bElem> who,direction dir);
    virtual bool isUsable();

};

#endif // USABLE_H
