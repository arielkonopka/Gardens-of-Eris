#ifndef EXPLOSIVES_H
#define EXPLOSIVES_H

#include <bElem.h>


class explosives : virtual public bElem
{
public:

    explosives(std::shared_ptr<chamber> board);
    explosives();
    virtual ~explosives();

    virtual bool explode();

};

#endif // EXPLOSIVES_H
