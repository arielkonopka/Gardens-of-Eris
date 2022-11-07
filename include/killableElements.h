#ifndef KILLABLEELEMENTS_H
#define KILLABLEELEMENTS_H

#include "mechanical.h"
#include "commons.h"



class killableElements : virtual public bElem
{
    public:
        killableElements(chamber* board);
        killableElements(chamber* board,int x, int y);
        killableElements(chamber *board,bool registerEl);
        killableElements(chamber *board,int x,int y,bool registerEl);
        virtual ~killableElements();
        virtual bool hurt(int points);
    protected:

    private:
};

#endif // KILLABLEELEMENTS_H
