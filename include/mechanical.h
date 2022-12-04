#ifndef MECHANICAL_H
#define MECHANICAL_H

#include <movableElements.h>
#include "commons.h"


class mechanical : virtual public bElem
{
    public:
        mechanical(chamber *board);
        mechanical(chamber *board,bool registerEl);
        virtual ~mechanical();



};

#endif // MECHANICAL_H
