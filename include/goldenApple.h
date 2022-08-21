#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H

#include <collectible.h>
#include <vector>

class goldenApple : public collectible
{
public:
    static videoElement::videoElementDef* vd;
    static goldenApple* getApple(int num);
    virtual videoElement::videoElementDef* getVideoElementDef();
    static  int getAppleNumber();
    virtual int getType();
    goldenApple(chamber *board);
    goldenApple(chamber *board,int x, int y);
    virtual ~goldenApple();

protected:

private:
    static unsigned int appleNumber;
    static std::vector<goldenApple*> apples;
};

#endif // GOLDENAPPLE_H
