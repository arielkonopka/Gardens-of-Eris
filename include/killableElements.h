#ifndef KILLABLEELEMENTS_H
#define KILLABLEELEMENTS_H

#include "mechanical.h"
#include "commons.h"



class killableElements : virtual public bElem
{
public:
    explicit killableElements(std::shared_ptr<chamber> board);
    killableElements() ;
    virtual ~killableElements()=default;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<killableElements> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
};

#endif // KILLABLEELEMENTS_H
