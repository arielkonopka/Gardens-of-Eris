#ifndef PLAINMISSILE_H
#define PLAINMISSILE_H
#include "killableElements.h"
#include "videoElementDef.h"

class plainMissile : public killableElements
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    virtual int getSubtype();
    plainMissile(chamber *mychamber,gCollect *garbage);
    plainMissile(chamber *mychamber,gCollect *garbage,int energy);
    virtual ~plainMissile();
    virtual bool mechanics(bool collected);

protected:


    bool killing;
private:
};

#endif // PLAINMISSILE_H
