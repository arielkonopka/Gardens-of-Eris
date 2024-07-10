#include "bElemContainer.h"


bElemContainer::~bElemContainer()
{
    al_destroy_mutex(this->eMutex);
}



bElemContainer::bElemContainer(): eMutex(al_create_mutex_recursive())
{

}

std::shared_ptr<bElem>  bElemContainer::removeFromTop() {
    std::shared_ptr<bElem> exEl=this->eCollection.back();
    this->eCollection.pop_back();
    return exEl;}

bool bElemContainer::addOnTop(std::shared_ptr<bElem> elem) {
    this->eCollection.push_back( elem);
    return true;
}

bool bElemContainer::addOnPos(int pos, std::shared_ptr<bElem>) {
    return false;
}

std::shared_ptr<bElem> bElemContainer::removeOnPos(unsigned int pos) {
    std::shared_ptr<bElem> exEl= nullptr;
    if(pos<this->eCollection.size())
    {
        exEl = this->eCollection.at(pos);
        this->eCollection.erase(this->eCollection.begin() + pos);
    }
    return exEl;
}

