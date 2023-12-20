#ifndef BAZOOKAMISSILE_H
#define BAZOOKAMISSILE_H
#include "commons.h"
#include <explosives.h>
#include <movableElements.h>


class bazookaMissile : public explosives, public movableElements
{
    public:
        int getType() const;
        explicit bazookaMissile();
        bool mechanics();
        virtual ~bazookaMissile() = default;
        virtual bool additionalProvisioning();
        virtual bool additionalProvisioning(int subtype,int typeId);
        virtual bool additionalProvisioning(int subtype,std::shared_ptr<bazookaMissile> sbe);
    protected:
        int steps=0;
    private:
};

#endif // BAZOOKAMISSILE_H
