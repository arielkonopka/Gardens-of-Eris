#ifndef MOVABLEELEMENTS_H
#define MOVABLEELEMENTS_H

#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

#include "chamber.h"
//#include "objectTypes.h"
#include "bElem.h"

class movableElements : virtual public bElem
{
public:

    movableElements();
    explicit movableElements(std::shared_ptr<chamber> board);
    virtual ~movableElements()=default;
    virtual bool moveInDirection(direction dir);
    virtual bool moveInDirectionSpeed(direction dir,int speed);
    virtual int getType() const;

    virtual bool dragInDirection(direction dragIntoDirection);
    virtual bool dragInDirectionSpeed(direction dragIntoDirection,int speed);
    virtual coords getOffset() const;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<movableElements> sbe);

    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);

};

#endif // MOVABLEELEMENTS_H
