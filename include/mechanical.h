#ifndef MECHANICAL_H
#define MECHANICAL_H

#include <movableElements.h>
#include "commons.h"


class mechanical : virtual public bElem
{
public:
    mechanical();
    mechanical(std::shared_ptr<chamber> board);
    mechanical(std::shared_ptr<chamber> board,bool rEl);
    virtual ~mechanical();
    virtual bool additionalProvisioning() ;
private:
   bool registerEl=true;

};

#endif // MECHANICAL_H
