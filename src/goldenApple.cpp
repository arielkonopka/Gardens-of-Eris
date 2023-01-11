#include "goldenApple.h"

videoElement::videoElementDef* goldenApple::vd=nullptr;

unsigned int goldenApple::appleNumber=0;
std::vector<std::shared_ptr<bElem>>  goldenApple::apples;

videoElement::videoElementDef* goldenApple::getVideoElementDef()
{
    return goldenApple::vd;
}

int goldenApple::getType()
{
    return _goldenAppleType;
}

bool goldenApple::hurt(int points)
{
    this->setSubtype(1);
    return killableElements::hurt(points);
}


goldenApple::goldenApple(std::shared_ptr<chamber> board):collectible(board),nonSteppable(board),killableElements(board), explosives(board)
{
    this->setSubtype(0);
    this->setStats(std::make_shared<elemStats>(555));

}

bool goldenApple::additionalProvisioning()
{
    if(bElem::additionalProvisioning()==true)
        return true;
    goldenApple::appleNumber++;
    goldenApple::apples.push_back(shared_from_this());
    return false;
}




goldenApple::goldenApple():collectible(),nonSteppable(),killableElements(),explosives()
{

}

oState goldenApple::disposeElement()
{
 for(unsigned int cnt=0; cnt<goldenApple::apples.size();)
    {
        if(goldenApple::apples[cnt]->getInstanceid()==this->getInstanceid())
        {
            goldenApple::apples.erase(goldenApple::apples.begin()+cnt);
        }
        else cnt++;
    }
    return bElem::disposeElement();
}


goldenApple::~goldenApple()
{
    goldenApple::appleNumber--;

    std::cout<<"Removed apple\n";
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
    if(killableElements::kill())
    {

        this->explode();
        return true;
    }
    return false;
}



