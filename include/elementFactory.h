// *** ADDED BY HEADER FIXUP ***
#include <istream>
// *** END ***
#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H
#include <memory>
#include <iostream>

class chamber;
class elementFactory
{
public:
    elementFactory() = default;
    virtual ~elementFactory() = default;


    template <class T>
    static std::shared_ptr<T> generateAnElement(std::shared_ptr<chamber> board,int subtype)
    {
        std::shared_ptr<T> l=std::make_shared<T>();
        l->setBoard(board);
        l->additionalProvisioning(subtype,l);
        return l;
    }


protected:

private:
};

#endif // ELEMENTFACTORY_H
