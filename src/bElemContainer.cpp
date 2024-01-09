#include "bElemContainer.h"


bElemContainer::~bElemContainer()
{
    al_destroy_mutex(this->eMutex);
}



bElemContainer::bElemContainer(): eMutex(al_create_mutex_recursive())
{

}

