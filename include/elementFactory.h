#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H
#include <memory>
class chamber;

class elementFactory
{
    public:
        elementFactory() = default;
        virtual ~elementFactory() = default;
            template <class T>
    static std::shared_ptr<T> generateAnElement(std::shared_ptr<chamber> board)
    {
        std::shared_ptr<T> l=std::make_shared<T>(board);
        l->additionalProvisioning();
        return l;
    }
    template <class T>
    static std::shared_ptr<T> generateAnElement(std::shared_ptr<chamber> board,int subtype)
    {
        std::shared_ptr<T> l=std::make_shared<T>(board);
        l->setSubtype(subtype);
        l->additionalProvisioning();
        return l;
    }

    template <class T>
    static std::shared_ptr<T> generateAnElement()
    {
        std::shared_ptr<T> l=std::make_shared<T>();
        l->additionalProvisioning();
        return l;

    }
    protected:

    private:
};

#endif // ELEMENTFACTORY_H
