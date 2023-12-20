#ifndef STACKEDELEMENT_H
#define STACKEDELEMENT_H

#include <bElem.h>
#include <vector>
#include <memory>

/* element Factory must create all instances of the stacked element */
class stackedElement : public bElem
{
public:
    stackedElement();
    virtual ~stackedElement()=default;
    virtual bool stepOnElement(std::shared_ptr<bElem> step);
    virtual std::shared_ptr<bElem> removeElement();
    stackedElement(std::shared_ptr<chamber> board);
    void linkAnElement(std::shared_ptr<stackedElement> newBottom);
    void setController(std::shared_ptr<bElem> controller);
    std::shared_ptr<bElem> getController();
    bool additionalProvisioning(int st,std::shared_ptr<stackedElement> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
protected:

private:
    std::shared_ptr<bElem> controlUnit=nullptr;
    std::vector<std::shared_ptr<stackedElement>> topDownConstruct;
};

#endif // STACKEDELEMENT_H
