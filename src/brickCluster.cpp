#include "brickCluster.h"



brickCluster::brickCluster(std::shared_ptr<chamber> board) : brickCluster()
{
    this->setBoard(board);
}
brickCluster::brickCluster() : movableElements()
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
