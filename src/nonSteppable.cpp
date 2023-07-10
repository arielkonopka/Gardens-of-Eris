#include "nonSteppable.h"


videoElement::videoElementDef* nonSteppable::vd=nullptr;


nonSteppable::nonSteppable(std::shared_ptr<chamber> board):bElem(board)
{


}
nonSteppable::nonSteppable():bElem()
{

}



videoElement::videoElementDef* nonSteppable::getVideoElementDef()
{
    return nonSteppable::vd;
}




bool nonSteppable::additionalProvisioning(int subtype, std::shared_ptr<nonSteppable>sbe)
{
     return this->additionalProvisioning(subtype,sbe->getType());

}

bool nonSteppable::additionalProvisioning()
{
     return this->additionalProvisioning(0,this->getType());

}

bool nonSteppable::additionalProvisioning(int subtype,int typeId)
{
     return bElem::additionalProvisioning(subtype,typeId);

}
