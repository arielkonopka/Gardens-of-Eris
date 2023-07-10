#ifndef BRICKCLUSTER_H
#define BRICKCLUSTER_H

#pragma once
#include "commons.h"
#include "bElem.h"
#include "chamber.h"
#include "memory.h"
#include "movableElements.h"
class brickCluster: public movableElements, nonSteppable
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    brickCluster(std::shared_ptr<chamber> board);
    brickCluster();
    int getType() const;
    bool additionalProvisioning() ;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<brickCluster> sbe);
    virtual bool additionalProvisioning(int subtype,int typeId);
private:

};

#endif
