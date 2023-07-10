#ifndef MOVABLEELEMENTS_H
#define MOVABLEELEMENTS_H

#include "nonSteppable.h"
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

#include "chamber.h"
//#include "objectTypes.h"
#include "audibleElement.h"

class movableElements :   virtual public audibleElement
{
public:
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    movableElements();
    explicit movableElements(std::shared_ptr<chamber> board);
    virtual ~movableElements();
    virtual bool moveInDirection(direction dir);
    virtual bool moveInDirectionSpeed(direction dir,int speed);
    virtual int getType();

    virtual bool dragInDirection(direction dragIntoDirection);
    virtual bool dragInDirectionSpeed(direction dragIntoDirection,int speed);
    virtual coords getOffset();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<movableElements> sbe);

    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);

};

#endif // MOVABLEELEMENTS_H
