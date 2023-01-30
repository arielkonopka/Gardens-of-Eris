#include "goldenApple.h"

videoElement::videoElementDef *goldenApple::vd = nullptr;

unsigned int goldenApple::appleNumber = 0;
std::vector<std::shared_ptr<bElem>> goldenApple::apples;

videoElement::videoElementDef *goldenApple::getVideoElementDef()
{
    return goldenApple::vd;
}

int goldenApple::getType()
{
    return _goldenAppleType;
}

bool goldenApple::hurt(int points)
{
    if (this->getSubtype() != 1)
    {
        this->setSubtype(1);
        for (unsigned int cnt = 0; cnt < goldenApple::apples.size();)
        {
            if (goldenApple::apples[cnt]->getInstanceid() == this->getInstanceid())
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

goldenApple::goldenApple(std::shared_ptr<chamber> board) : collectible(board), nonSteppable(board), killableElements(board), explosives(board)
{
    this->setSubtype(0);
    this->setStats(std::make_shared<elemStats>(555));
}

bool goldenApple::additionalProvisioning()
{
    if (bElem::additionalProvisioning() == true)
        return true;
    goldenApple::appleNumber++;
    goldenApple::apples.push_back(shared_from_this());
    return false;
}

goldenApple::goldenApple() : collectible(), nonSteppable(), killableElements(), explosives()
{
}

oState goldenApple::disposeElement()
{
    for (unsigned int cnt = 0; cnt < goldenApple::apples.size();)
    {
        if (goldenApple::apples[cnt]->getInstanceid() == this->getInstanceid())
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

        this->explode();
        return true;
    }
    return false;
}
bool goldenApple::mechanics()
{
    bool r = explosives::mechanics();

    if (this->getSubtype() == 0 || this->getCollector().get() == nullptr || this->isWaiting())
    {
        return r;
    }
    int e = this->getCollector()->getEnergy();
    if (e < 100)
    {
        this->setWait(25);
        this->getCollector()->setEnergy(e + 1);
        this->hurt(1);
    }

    return true;
}
void goldenApple::setCollected(std::shared_ptr<bElem> who)
{
    if(this->isLiveElement()==false)
    {
        this->registerLiveElement(shared_from_this());
    }
 /*   if(who.get()!=nullptr && who.get()!=this->getCollector().get() && who->getBoard().get()!=nullptr)
    {
        coords3d c3d;
        c3d.x=who->getCoords().x*32+who->getOffset().x;
        c3d.z=who->getCoords().y*32+who->getOffset().y;
        c3d.y=50;
        coords3d vel= {who->getOffset().x,0,who->getOffset().y};
        soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,this->getInstanceid(),this->getType(),this->getSubtype(),"Found","Collect");
    }
    */
    explosives::setCollected(who);

}
