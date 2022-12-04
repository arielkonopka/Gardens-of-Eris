#ifndef KILLABLEELEMENTS_H
#define KILLABLEELEMENTS_H

#include "mechanical.h"
#include "commons.h"



class killableElements : virtual public bElem
{
    public:
        killableElements(chamber* board);
        virtual bool hurt(int points);

};

#endif // KILLABLEELEMENTS_H
