#include "goldenApple.h"

videoElement::videoElementDef* goldenApple::vd=NULL;


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

}

goldenApple::goldenApple(chamber* board, int x, int y):collectible(board,x,y)
{
}

goldenApple::~goldenApple()
{

}
