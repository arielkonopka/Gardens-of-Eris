#ifndef MECHANICAL_H
#define MECHANICAL_H

#include <movableElements.h>
#include "commons.h"


class mechanical : public movableElements
{
    public:
        mechanical(chamber *board);
        mechanical(chamber *board,int x, int y);
        mechanical(chamber *board,bool registerEl);
        mechanical(chamber *board,int x, int y,bool registerEl);
        virtual ~mechanical();
        virtual bool kill();

    protected:

    private:

};

#endif // MECHANICAL_H
