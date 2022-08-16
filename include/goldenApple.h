#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H

#include <collectible.h>


class goldenApple : public collectible
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    goldenApple(chamber *board);
    goldenApple(chamber *board,int x, int y);
    virtual ~goldenApple();

protected:

private:
};

#endif // GOLDENAPPLE_H
