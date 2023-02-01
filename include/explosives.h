#ifndef EXPLOSIVES_H
#define EXPLOSIVES_H

#include <audibleElement.h>


class explosives : virtual public audibleElement
{
public:

    explicit explosives(std::shared_ptr<chamber> board);
    explosives();
    virtual ~explosives();

    virtual bool explode();

};

#endif // EXPLOSIVES_H
