#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H
#include "elements.h"
#include <vector>

class goldenApple : public collectible,public nonSteppable,public killableElements,public explosives
{
public:
    static videoElement::videoElementDef* vd;
    static std::shared_ptr<bElem> getApple(int num);
    virtual videoElement::videoElementDef* getVideoElementDef();
    static  int getAppleNumber();
    virtual int getType();
    virtual bool kill();
    goldenApple(std::shared_ptr<chamber> board);
    goldenApple();
    virtual ~goldenApple();
    virtual bool additionalProvisioning();
private:
    static unsigned int appleNumber;
    static std::vector<std::shared_ptr<bElem>> apples;
};

#endif // GOLDENAPPLE_H
