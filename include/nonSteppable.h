#ifndef NONSTEPPABLE_H
#define NONSTEPPABLE_H

#include "bElem.h"
//#include "gCollect.h"
//#include "chamber.h"


#include "objectTypes.h"


class nonSteppable : virtual public bElem
{
public:
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    nonSteppable();
    explicit nonSteppable(std::shared_ptr<chamber> board);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<nonSteppable> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);

};

#endif // NONSTEPPABLE_H
