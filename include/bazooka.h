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
        virtual int getType() const;
        virtual std::shared_ptr<bElem> createProjectible(std::shared_ptr<bElem> who);
        virtual bool additionalProvisioning() final;
        virtual bool additionalProvisioning(int subtype,int typeId) final ;
        virtual bool additionalProvisioning(int subtype,std::shared_ptr<bazooka> sbe) final;
    protected:

    private:
};

#endif // BAZOOKA_H
