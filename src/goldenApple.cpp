#include "goldenApple.h"

videoElement::videoElementDef *goldenApple::vd = nullptr;

unsigned int goldenApple::appleNumber = 0;
std::vector<std::shared_ptr<bElem>> goldenApple::apples;

videoElement::videoElementDef *goldenApple::getVideoElementDef()
{
    return goldenApple::vd;
}

int goldenApple::getType() const
{
    return _goldenAppleType;
}

bool goldenApple::hurt(int points)
{
    if (this->getAttrs()->getSubtype() != 1)
    {
        this->getAttrs()->setSubtype(1);
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
    bool res = killableElements::destroy();
    if (res)
        this->explode(2.5);
    return res;
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
    if (killableElements::kill())
    {

        this->explode(2.5);
        return true;
    }
    return false;
}
bool goldenApple::mechanics()
{
    bool r = explosives::mechanics();

    if (this->getAttrs()->getSubtype() == 0 || this->getStats()->getCollector().expired() || this->getStats()->isWaiting())
    {
        return r;
    }
    int e = this->getStats()->getCollector().lock()->getAttrs()->getEnergy();
    if (e < 100)
    {
        this->getStats()->setWaiting(25);
        this->getStats()->getCollector().lock()->getAttrs()->setEnergy(e + 1);
        this->hurt(1);
    }

    return true;
}
bool goldenApple::collectOnAction(bool collected, std::shared_ptr<bElem>who)
{
    bool r=bElem::collectOnAction(collected,who);

    if (this->getStats()->hasActivatedMechanics() == false && this->getAttrs()->getSubtype() != 0)
    {
        this->registerLiveElement(shared_from_this());
    }
    return r;
}

/*
void goldenApple::setCollected(std::shared_ptr<bElem> who)
{

}*/
