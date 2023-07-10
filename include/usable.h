#ifndef USABLE_H
#define USABLE_H

//#include "commons.h"
#include "audibleElement.h"


class usable : virtual public audibleElement
{
public:
    usable();
    explicit usable(std::shared_ptr<chamber> board);
    virtual bool use(std::shared_ptr<bElem> who);
    virtual bool use(std::shared_ptr<bElem> who,direction dir);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<usable> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
};

#endif // USABLE_H
