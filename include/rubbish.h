#ifndef RUBBISH_
#define RUBBISH_
#include "bElem.h"
#include "objectTypes.h"

class rubbish:  public bElem
{
public:

    rubbish();
    explicit rubbish(std::shared_ptr<chamber> board);
    virtual ~rubbish() = default;
    bool mechanics() final;
    int getType()  const ;
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<rubbish> sbe);

};

#endif // RUBBISH_
