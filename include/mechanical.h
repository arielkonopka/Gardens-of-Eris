#ifndef MECHANICAL_H
#define MECHANICAL_H

#include <movableElements.h>
#include "commons.h"


class mechanical : virtual public bElem
{
public:
    mechanical();
    explicit mechanical(std::shared_ptr<chamber> board);
    explicit mechanical(std::shared_ptr<chamber> board,bool rEl);
    virtual ~mechanical() = default;
    virtual bool additionalProvisioning() ;
    virtual bool additionalProvisioning(int subtype,int typeId) ;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<mechanical> sbe);

private:
    bool registerEl=true;

};

#endif // MECHANICAL_H
