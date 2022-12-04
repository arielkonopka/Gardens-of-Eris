#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H
#include "elements.h"
#include <vector>

class goldenApple : public collectible,public nonSteppable,public killableElements,public explosives
{
public:
    static videoElement::videoElementDef* vd;
    static goldenApple* getApple(int num);
    virtual videoElement::videoElementDef* getVideoElementDef();
    static  int getAppleNumber();
    virtual int getType();
    virtual bool kill();
    goldenApple(chamber *board);
    virtual ~goldenApple();
protected:

private:
    static unsigned int appleNumber;
    static std::vector<goldenApple*> apples;
};

#endif // GOLDENAPPLE_H
