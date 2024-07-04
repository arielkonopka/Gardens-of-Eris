#ifndef BELEMCONTAINER_H
#define BELEMCONTAINER_H
#include <allegro5/allegro5.h>

#include <commons.h>

class bElem;
class bElemContainer

{

public:
    bElemContainer();
    ~bElemContainer();

    std::shared_ptr<bElem> element;
    ALLEGRO_MUTEX* eMutex;
    /*TODO we will introduce here a vector of elements, to keep the ones, we step on.*/
    bool addOnTop(std::shared_ptr<bElem>);
    bool removeFromTop();
    bool addOnPos(int pos,std::shared_ptr<bElem>);
    bool removeOnPos(int pos);


private:
    std::vector<std::shared_ptr<bElem>> eCollection;


};

#endif // BELEMCONTAINER_H
