#ifndef RUBBISH_
#define RUBBISH_

#include "objectTypes.h"
#include "nonSteppable.h"
#include "collectible.h"

class rubbish:  public collectible
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        rubbish();
        explicit rubbish(std::shared_ptr<chamber> board);
        void stomp(std::shared_ptr<bElem> who) final;
        bool mechanics() final;
        constexpr int getType() final { return _rubishType; };
        constexpr bool isSteppable() final { return true; };
        constexpr bool canBeKilled() final { return false; };
        constexpr bool canBeDestroyed() final { return false; };

};

#endif // RUBBISH_
