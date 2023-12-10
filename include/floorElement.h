#ifndef FLOOR_H
#define FLOOR_H

#include "bElem.h"


class floorElement : public bElem
{
public:

    floorElement();
    explicit floorElement(std::shared_ptr<chamber> board);
    virtual ~floorElement()=default;
    virtual  int getType() const;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<floorElement> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
};

#endif // FLOOR_H
