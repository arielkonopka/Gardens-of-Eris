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
    virtual bool explode(float radius);

};

#endif // EXPLOSIVES_H
