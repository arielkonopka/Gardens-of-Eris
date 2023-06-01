#ifndef PUPPETMASTERFR_H
#define PUPPETMASTERFR_H

#include <collectible.h>



class puppetMasterFR : public collectible, public mechanical, public killableElements, public nonSteppable
{
    public:
        puppetMasterFR();
        puppetMasterFR(std::shared_ptr<chamber> board);
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        virtual void setCollected(std::shared_ptr<bElem> who);
        virtual ~puppetMasterFR();
        virtual bool mechanics();
        int getType();
    protected:

    private:

        bool mechanicsPatrollingDrone();


};

#endif // PUPPETMASTERFR_H
