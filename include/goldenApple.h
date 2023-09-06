#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H
#include "elements.h"
#include <vector>
#include "soundManager.h"
class goldenApple : public killableElements,public explosives
{
public:
    static std::shared_ptr<bElem> getApple(int num);
    static  int getAppleNumber();
    int getType() const;
    bool kill() final;
    bool destroy() final;
    bool hurt(int points);
    explicit goldenApple(std::shared_ptr<chamber> board);

    bool mechanics();
    goldenApple();
    bool additionalProvisioning(int subtype,int typeId) final;
    bool additionalProvisioning() final;
    oState disposeElement() final;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<goldenApple> sbe);
    bool collectOnAction(bool collected, std::shared_ptr<bElem> who);

private:
    static unsigned int appleNumber;
    static std::vector<std::shared_ptr<bElem>> apples;
};

#endif // GOLDENAPPLE_H
