#ifndef BRICKCLUSTER_H
#define BRICKCLUSTER_H

#pragma once
#include "commons.h"
#include "bElem.h"
#include "chamber.h"
#include "memory.h"
#include "movableElements.h"
class brickCluster: public movableElements
{
public:
    static videoElement::videoElementDef* vd;
    videoElement::videoElementDef* getVideoElementDef() final;
    brickCluster(std::shared_ptr<chamber> board);
    brickCluster();
    int getType() const;
    bool additionalProvisioning() ;
    bool additionalProvisioning(int subtype,std::shared_ptr<brickCluster> sbe) ;
    bool additionalProvisioning(int subtype,int typeId) ;

};

#endif
