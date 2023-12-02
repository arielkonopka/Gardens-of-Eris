#ifndef BAZOOKA_H
#define BAZOOKA_H
#include "commons.h"
#include <plainGun.h>
#include <bazookaMissile.h>
#include <viewPoint.h>

class bazooka : public plainGun
{
    public:
        bazooka(std::shared_ptr<chamber> board);
        virtual ~bazooka()=default;
        int getType() const;
        std::shared_ptr<bElem> createProjectible(std::shared_ptr<bElem> who);
        bool additionalProvisioning();
        bool additionalProvisioning(int subtype,int typeId);
        bool additionalProvisioning(int subtype,std::shared_ptr<bazooka> sbe);
    protected:

    private:
};

#endif // BAZOOKA_H
