#ifndef RUBBISH_
#define RUBBISH_

#include "stillElem.h"
#include "gCollect.h"
#include "objectTypes.h"


class rubbish :public stillElem
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        rubbish(chamber *board);
        bool isCollectible();
        virtual ~rubbish();
        int getType();
        int getSubtype();
    protected:

    private:
};

#endif // RUBBISH_
