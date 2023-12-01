#ifndef BAZOOKAMISSILE_H
#define BAZOOKAMISSILE_H
#include "commons.h"
#include <explosives.h>
#include <movableElements.h>


class bazookaMissile : public explosives, public movableElements
{
    public:
        int getType() const;
        bazookaMissile(std::shared_ptr<chamber> board);
        bool mechanics();
        virtual ~bazookaMissile() = default;
        bool additionalProvisioning();
        bool additionalProvisioning(int subtype,int typeId);
        bool additionalProvisioning(int subtype,std::shared_ptr<bazookaMissile> sbe);
    protected:
        int steps=0;
    private:
};

#endif // BAZOOKAMISSILE_H
