#ifndef STACKEDELEMENT_H
#define STACKEDELEMENT_H

#include <movableElements.h>
#include <vector>
#include <memory>

/* element Factory must create all instances of the stacked element */
class stackedElement : public movableElements
{
    public:
        stackedElement();
        virtual bool stepOnElement(std::shared_ptr<bElem> step);
        virtual std::shared_ptr<bElem> removeElement();
        stackedElement(std::shared_ptr<chamber> board);
        virtual ~stackedElement();
        void linkAnElement(std::shared_ptr<stackedElement> newBottom);
        void setController(std::shared_ptr<stackedElement> controller);
        std::shared_ptr<stackedElement> getController();
    protected:

    private:
        std::shared_ptr<stackedElement> controlUnit=nullptr;
        std::vector<std::shared_ptr<stackedElement>> topDownConstruct;
};

#endif // STACKEDELEMENT_H
