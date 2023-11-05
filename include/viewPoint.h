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
    void addViewPoint(std::shared_ptr<bElem> vp);
    std::shared_ptr<bElem> getOwner();
    ~viewPoint()=delete;
    viewPoint()=default;
    coords getViewPoint();
    coords getViewPointOffset();
    int calculateObscured(coords point);
    int calculateObscured(coords point,int divider);

    bool isPointVisible(coords point);

protected:

private:
    bool isElementInVector(const std::vector<std::weak_ptr<bElem>>& vec, const std::shared_ptr<bElem>& elem);
    static viewPoint* instance;
    std::weak_ptr<bElem> _owner;
    std::vector<std::weak_ptr<bElem>> viewPoints;
    static std::once_flag once;
};


#endif // VIEWPOINT_H
