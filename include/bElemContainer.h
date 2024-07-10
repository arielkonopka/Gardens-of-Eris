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

    ALLEGRO_MUTEX* eMutex;

    std::shared_ptr<bElem> element;
    bool addOnTop(std::shared_ptr<bElem> element);
    std::shared_ptr<bElem> removeFromTop();
    bool addOnPos(int pos,std::shared_ptr<bElem> element);
    std::shared_ptr<bElem>  removeOnPos(unsigned int pos);


private:

    std::vector<std::shared_ptr<bElem>> eCollection;


};

#endif // BELEMCONTAINER_H
