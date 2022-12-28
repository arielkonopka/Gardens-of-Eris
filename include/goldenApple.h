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
    int getType() final;
    bool kill() final;
    explicit goldenApple(std::shared_ptr<chamber> board);
    goldenApple();
    virtual ~goldenApple();
    bool additionalProvisioning() final;
    oState disposeElement() final;
private:
    static unsigned int appleNumber;
    static std::vector<std::shared_ptr<bElem>> apples;
};

#endif // GOLDENAPPLE_H
