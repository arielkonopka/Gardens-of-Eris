#include "goldenApple.h"

videoElement::videoElementDef* goldenApple::vd=NULL;

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

goldenApple::goldenApple(chamber* board):collectible(board)
{
    this->setSubtype(0);
    goldenApple::appleNumber++;
    goldenApple::apples.push_back(this);
}

goldenApple::goldenApple(chamber* board, int x, int y):collectible(board,x,y)
{
    this->setSubtype(0);
    goldenApple::appleNumber++;
    goldenApple::apples.push_back(this);
}

goldenApple::~goldenApple()
{
    goldenApple::appleNumber--;
    for(int cnt=0;cnt<goldenApple::apples.size();)
    {
        if(goldenApple::apples[cnt]==this)
        {
            goldenApple::apples.erase(goldenApple::apples.begin()+cnt);

        } else cnt++;

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

