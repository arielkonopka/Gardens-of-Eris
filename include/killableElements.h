#ifndef KILLABLEELEMENTS_H
#define KILLABLEELEMENTS_H

#include "mechanical.h"
#include "commons.h"



class killableElements : virtual public bElem
{
    public:
        explicit killableElements(std::shared_ptr<chamber> board);
        killableElements() ;
        virtual bool hurt(int points);

};

#endif // KILLABLEELEMENTS_H
