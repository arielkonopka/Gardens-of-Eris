#ifndef EXPLOSIVES_H
#define EXPLOSIVES_H

#include <bElem.h>


class explosives : virtual public bElem
{
public:

    explicit explosives(std::shared_ptr<chamber> board);
    explosives();
    virtual ~explosives();

    virtual bool explode();
    virtual bool explode(float radius);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<explosives> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
private:
};

#endif // EXPLOSIVES_H
