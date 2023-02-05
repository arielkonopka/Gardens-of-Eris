#ifndef MUSICELEMENT_H
#define MUSICELEMENT_H

#include "audibleElement.h"
#include "mechanical.h"


class musicElement : public audibleElement, public mechanical
{
    public:
        musicElement();
        musicElement(std::shared_ptr<chamber> board);
        virtual ~musicElement();
        bool mechanics();
    protected:

    private:
};

#endif // MUSICELEMENT_H
