#ifndef RUBBISH_
#define RUBBISH_

#include "objectTypes.h"
#include "nonSteppable.h"
#include "collectible.h"

class rubbish: public nonSteppable, public collectible
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        rubbish();
        rubbish(std::shared_ptr<chamber> board);
        int getType();

};

#endif // RUBBISH_
