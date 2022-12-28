#ifndef MONSTER_H
#define MONSTER_H
#include "objectTypes.h"
#include "chamber.h"
#include "videoElementDef.h"
#include "killableElements.h"
#include "plainGun.h"

class monster : public killableElements, public nonSteppable, public mechanical, public movableElements
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        monster();
        explicit monster(std::shared_ptr<chamber> board);
        explicit monster(std::shared_ptr<chamber> board,int newSubtype);
        ~monster();
        int getType();
        virtual bool mechanics();
        virtual bool checkNeigh();
        virtual bool steppableNeigh();


    private:
        plainGun* weapon=nullptr;
        bool inited=false;
        int rotA=3;
        int rotB=1;

};

#endif // MONSTER_H
