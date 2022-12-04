#ifndef EXPLOSIVES_H
#define EXPLOSIVES_H

#include <bElem.h>


class explosives : virtual public bElem
{
public:

    explosives(chamber *board);
    virtual ~explosives();

    virtual bool explode();

protected:

private:
};

#endif // EXPLOSIVES_H
