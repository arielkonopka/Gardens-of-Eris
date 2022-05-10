#ifndef KILLABLEELEMENTS_H
#define KILLABLEELEMENTS_H

#include "movableElements.h"
#include "commons.h"



class killableElements : public movableElements
{
    public:
        killableElements(chamber* board);
        killableElements(chamber* board,int x, int y);
        virtual ~killableElements();
        virtual bool canBeKilled();
        virtual bool canBeDestroyed();
        virtual bool mechanics(bool collected);
        virtual bool kill();
        virtual bool hurt(int points);
    protected:

    private:
};

#endif // KILLABLEELEMENTS_H
