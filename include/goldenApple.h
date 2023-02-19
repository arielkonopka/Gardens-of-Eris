#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H
#include "elements.h"
#include <vector>
#include "soundManager.h"
class goldenApple : public collectible,public nonSteppable,public killableElements,public explosives
{
public:
    static videoElement::videoElementDef* vd;
    static std::shared_ptr<bElem> getApple(int num);
    virtual videoElement::videoElementDef* getVideoElementDef();
    void setCollected(std::shared_ptr<bElem> who);
    static  int getAppleNumber();
    int getType() final;
    bool kill() final;
    bool destroy() final;
    bool hurt(int points);
    explicit goldenApple(std::shared_ptr<chamber> board);
    bool mechanics();
    goldenApple();
    bool additionalProvisioning() final;
    oState disposeElement() final;
private:
    static unsigned int appleNumber;
    static std::vector<std::shared_ptr<bElem>> apples;
};

#endif // GOLDENAPPLE_H
