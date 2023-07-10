#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "nonSteppable.h"
#include "killableElements.h"

class collectible : virtual public audibleElement
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    collectible();
    explicit collectible(std::shared_ptr<chamber> board);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<collectible> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);

};

#endif // COLLECTIBLE_H
