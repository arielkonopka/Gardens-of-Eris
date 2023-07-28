#ifndef PUPPETMASTERFR_H
#define PUPPETMASTERFR_H

#include <bElem.h>
#include <killableElements.h>
#include <mechanical.h>


class puppetMasterFR :  public mechanical, public killableElements
{
public:
    puppetMasterFR();
    puppetMasterFR(std::shared_ptr<chamber> board);
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    //i  virtual void setCollected(std::shared_ptr<bElem> who);
    virtual ~puppetMasterFR();
    virtual bool mechanics();
    int getType();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<puppetMasterFR> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
protected:

private:
    bool collectorMechanics();
    bool mechanicsPatrollingDrone();
    std::shared_ptr<bElem> findObjectInDirection(direction dir);


};

#endif // PUPPETMASTERFR_H
