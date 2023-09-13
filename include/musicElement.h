#ifndef MUSICELEMENT_H
#define MUSICELEMENT_H

#include "audibleElement.h"
#include "mechanical.h"


class musicElement : public mechanical
{
public:
    musicElement();
    musicElement(std::shared_ptr<chamber> board);
    virtual ~musicElement();
    bool mechanics();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<musicElement> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
protected:

private:
};

#endif // MUSICELEMENT_H
