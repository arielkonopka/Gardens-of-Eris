#include "brickCluster.h"


videoElement::videoElementDef* brickCluster::vd=nullptr;

videoElement::videoElementDef* brickCluster::getVideoElementDef()
{
    return brickCluster::vd;
}

brickCluster::brickCluster(std::shared_ptr<chamber> board):movableElements(board),nonSteppable(board)
{
}
brickCluster::brickCluster():movableElements(),nonSteppable()
{
}