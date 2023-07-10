#include "brickCluster.h"

videoElement::videoElementDef *brickCluster::vd = nullptr;

videoElement::videoElementDef *brickCluster::getVideoElementDef()
{
    return brickCluster::vd;
}

brickCluster::brickCluster(std::shared_ptr<chamber> board) : brickCluster()
{
    this->setBoard(board);
}
brickCluster::brickCluster() : movableElements(), nonSteppable()
{
}

int brickCluster::getType() const

{
    return _brickClusterType;
};


bool brickCluster::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool brickCluster::additionalProvisioning(int subtype, std::shared_ptr<brickCluster>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool brickCluster::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}
