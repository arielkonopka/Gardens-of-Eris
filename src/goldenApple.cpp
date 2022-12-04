#include "goldenApple.h"

videoElement::videoElementDef* goldenApple::vd=nullptr;

unsigned int goldenApple::appleNumber=0;
std::vector<goldenApple*>  goldenApple::apples;

videoElement::videoElementDef* goldenApple::getVideoElementDef()
{
    return goldenApple::vd;
}

int goldenApple::getType()
{
    return _goldenAppleType;
}

goldenApple::goldenApple(chamber* board):collectible(board),nonSteppable(board),killableElements(board), explosives(board)
{
    this->setSubtype(0);
    this->setStats(new elemStats(55));
    goldenApple::appleNumber++;
    goldenApple::apples.push_back(this);
}

goldenApple::goldenApple(chamber* board, int x, int y):collectible(board,x,y),nonSteppable(board,x,y),killableElements(board,x,y), explosives(board,x,y)
{
    this->setSubtype(0);
    this->setStats(new elemStats(55));
    goldenApple::appleNumber++;
    goldenApple::apples.push_back(this);
}

goldenApple::~goldenApple()
{
    goldenApple::appleNumber--;
    for(unsigned int cnt=0; cnt<goldenApple::apples.size();)
    {
        if(goldenApple::apples[cnt]==this)
        {
            goldenApple::apples.erase(goldenApple::apples.begin()+cnt);

        }
        else cnt++;

    }
}

int goldenApple::getAppleNumber()
{
    return goldenApple::appleNumber;
}


goldenApple* goldenApple::getApple(int num)
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



