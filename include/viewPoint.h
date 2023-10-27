#ifndef VIEWPOINT_H
#define VIEWPOINT_H
#include "commons.h"
#include "elements.h"
#include <mutex>
#include <memory>



class viewPoint
{
public:
    static viewPoint* get_instance();
    void setOwner(std::shared_ptr<bElem> owner);
    std::shared_ptr<bElem> getOwner();
    ~viewPoint()=delete;
    viewPoint()=default;
    coords getViewPoint();
    coords getViewPointOffset();

protected:

private:
    static viewPoint* instance;
    std::weak_ptr<bElem> _owner;

    static std::once_flag once;
};


#endif // VIEWPOINT_H
