#ifndef KILLABLEELEMENTS_H
#define KILLABLEELEMENTS_H

#include <movableElements.h>


class killableElements : public movableElements
{
    public:
        killableElements(chamber* board,gCollect* garbageBin);
        virtual ~killableElements();
        virtual bool canBeKilled();
    protected:

    private:
};

#endif // KILLABLEELEMENTS_H
