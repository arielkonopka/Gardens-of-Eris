#ifndef AUDIBLEELEMENT_H
#define AUDIBLEELEMENT_H
#include <memory>
#include "bElem.h"
#include "soundManager.h"
#include "commons.h"
class audibleElement : virtual public bElem
{
    public:
        audibleElement();
        audibleElement(std::shared_ptr<chamber> board);
        virtual ~audibleElement() ;
        void playSound(std::string eventType,std::string event);
    protected:

    private:
        void ps(std::shared_ptr<bElem> who,std::string eventType,std::string event);
};

#endif // AUDIBLEELEMENT_H
