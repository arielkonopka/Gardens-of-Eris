#ifndef PLAINMISSILE_H
#define PLAINMISSILE_H
#include "killableElements.h"
#include "videoElementDef.h"
#include <math.h>
class plainMissile : public killableElements, public movableElements, public mechanical
{
public:

    virtual int getType() const;
    plainMissile();
    explicit plainMissile(std::shared_ptr<chamber> mychamber);
    explicit plainMissile(std::shared_ptr<chamber> mychamber,int energy);
    virtual ~plainMissile();
    virtual bool mechanics();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<plainMissile> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    virtual bool stepOnAction(bool step, std::shared_ptr<bElem> who);
private:
    std::shared_ptr<bElem> statsOwner;

};

#endif // PLAINMISSILE_H
