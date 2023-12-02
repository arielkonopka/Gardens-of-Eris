#ifndef WALL_H
#define WALL_H

#include "videoElementDef.h"

#include "objectTypes.h"
#include "bElem.h"


class wall : public bElem
{
public:

    wall();
    virtual ~wall()=default;
    bool stepOnElement(std::shared_ptr<bElem> elem);
    std::shared_ptr<bElem> removeElement();
    explicit wall(std::shared_ptr<chamber> board);
    explicit wall(std::shared_ptr<chamber> board,int subtype);
    int getType() const;

    bool additionalProvisioning() ;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<wall> sbe);
    virtual bool additionalProvisioning(int subtype,int typeId);
};

#endif // WALL_H
