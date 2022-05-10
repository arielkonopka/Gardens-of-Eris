#ifndef MECHANICAL_H
#define MECHANICAL_H

#include <nonSteppable.h>
#include "commons.h"


class mechanical : public nonSteppable
{
    public:
        mechanical(chamber *board);
        mechanical(chamber *board,int x, int y);
        virtual ~mechanical();

    protected:

    private:
};

#endif // MECHANICAL_H
