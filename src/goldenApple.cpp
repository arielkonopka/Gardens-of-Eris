#include "goldenApple.h"


unsigned int goldenApple::appleNumber = 0;
std::vector<std::shared_ptr<bElem>> goldenApple::apples;


int goldenApple::getType() const
{
    return _goldenAppleType;
}

bool goldenApple::hurt(int points)
{
    if (this->getAttrs()->getSubtype() != 1)
    {
        this->getAttrs()->setSubtype(1);
        this->getAttrs()->setInteractive(true);
        for (unsigned int cnt = 0; cnt < goldenApple::apples.size();)
        {
            if (goldenApple::apples[cnt]->getStats()->getInstanceId() == this->getStats()->getInstanceId())
            {
                goldenApple::apples.erase(goldenApple::apples.begin() + cnt);
                goldenApple::appleNumber--;
            }
            else
                cnt++;
        }
    }

    return killableElements::hurt(points);
}

bool goldenApple::destroy()
{

    if(!this->getStats()->isDestroying())
        return this->explode(2.5);
    return false;
}

goldenApple::goldenApple(std::shared_ptr<chamber> board) : goldenApple()
{
    this->setBoard(board);
}
goldenApple::goldenApple() : killableElements(), explosives()
{
}

bool goldenApple::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}
bool goldenApple::additionalProvisioning(int subtype, int typeId)
{
    bElem::additionalProvisioning(subtype,typeId);
    if(subtype==0)
    {
        goldenApple::appleNumber++;
        goldenApple::apples.push_back(shared_from_this());
    }
    return true;
}
bool goldenApple::additionalProvisioning(int subtype, std::shared_ptr<goldenApple>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}


oState goldenApple::disposeElement()
{
    for (unsigned int cnt = 0; cnt < goldenApple::apples.size();)
    {
        if (goldenApple::apples[cnt]->getStats()->getInstanceId() == this->getStats()->getInstanceId())
        {
            goldenApple::apples.erase(goldenApple::apples.begin() + cnt);
            goldenApple::appleNumber--;
        }
        else
            cnt++;
    }
    return bElem::disposeElement();
}

int goldenApple::getAppleNumber()
{
    return goldenApple::appleNumber;
}

std::shared_ptr<bElem> goldenApple::getApple(int num)
{
    return goldenApple::apples.at(num);
}
/*
Here we will have a lottery made with random level generator (there will be a static method to generate a random object)
Random level generator will be great for that, because it would hold information on which objects should be generated
and we will have to add that information only once.

*/
bool goldenApple::kill()
{
    return this->destroy();
}
bool goldenApple::mechanics()
{
    bool r = explosives::mechanics();
    std::shared_ptr<bElem> _owner=this->getStats()->getCollector().lock();
    if (this->getAttrs()->getSubtype() == 0 || !this->getStats()->isCollected() || this->getStats()->isWaiting() || !_owner)
    {
        return r;
    }
    int e = _owner->getAttrs()->getEnergy();
    if (e < 100)
    {
        this->getStats()->setWaiting(55);
        _owner->getAttrs()->setEnergy(e + 5);
        this->hurt(5);
    }
    return true;
}

bool goldenApple::interact(std::shared_ptr<bElem>who)
{
    if(!this->getAttrs()->isInteractive() || !who || !bElem::interact(who))
        return false;
    this->getStats()->setWaiting(55);
    this->getStats()->setInteracted(55);
    who->getAttrs()->setEnergy(who->getAttrs()->getEnergy() + 5);
    this->hurt(5);
    return true;
}



bool goldenApple::collectOnAction(bool collected, std::shared_ptr<bElem>who)
{

    if(collected && who )
    {
        if(who->getAttrs()->getInventory()->countTokens(this->getType(),this->getAttrs()->getSubtype())>1 || this->getAttrs()->getSubtype()==0)
        {
            this->deregisterLiveElement(this->getStats()->getInstanceId());
        }
        else
        {
            this->registerLiveElement(shared_from_this());
        }
    }
    else
    {
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    }
    bool r=bElem::collectOnAction(collected,who);
    return r;
}

/*
void goldenApple::setCollected(std::shared_ptr<bElem> who)
{

}*/
