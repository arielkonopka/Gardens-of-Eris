#ifndef SIMPLEBOMB_H
#define SIMPLEBOMB_H
#include "bElem.h"
#include "explosives.h"
#include "killableElements.h"

class simpleBomb: public explosives,  public killableElements
{
public:

    simpleBomb()=default;
    explicit simpleBomb(std::shared_ptr<chamber> board);
    virtual ~simpleBomb()=default;
    virtual bool hurt(int points);
    virtual bool kill();
    virtual bool destroy();
    virtual int getType() const;
    virtual bool mechanics();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<simpleBomb> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    float getViewRadius() const;
private:
    bool triggered=false;
};

#endif // SIMPLEBOMB_H
