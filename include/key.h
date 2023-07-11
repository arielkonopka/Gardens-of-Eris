#ifndef KEY_H
#define KEY_H

#include "collectible.h"
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

class key : public collectible, public nonSteppable
{

public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType() const;
    explicit key(std::shared_ptr<chamber> board);
    explicit key(std::shared_ptr<chamber> board,int x,int y);
    explicit key(std::shared_ptr<chamber> board,int subtype);
    bool additionalProvisioning(int subtype,std::shared_ptr<key> key);
    bool additionalProvisioning(int subtype,int typeId);
    bool additionalProvisioning();

    key();


};

#endif // KEY_H
