//
// Created by c on 23.09.24.
//
#include "hudManager.h"

std::once_flag hudManager::initFlag;

hudManager::hudManager()
{
    coords size=videoManager::getInstance().getHudSize();
    // we need extra an extra tile size for the huds, we could theoretically get away with that, but we want to see, how the huds would work out for us
    this->upperCorner=coords(configManager::getInstance()->getConfig()->tileWidth*3,int(configManager::getInstance()->getConfig()->tileHeight*3));
    size=size+(upperCorner*2); // we need space from both ends
    this->HUDGamefield= al_create_bitmap(size.x,size.y);
    this->HUDGlobal= al_create_bitmap(videoManager::getInstance().getHudSize().x,videoManager::getInstance().getHudSize().y);
    if (!this->HUDGamefield || !this->HUDGlobal) {
        std::cerr << "Failed to create HUD bitmaps!" << std::endl;
        return; // lub inne działanie w przypadku błędu
    }
}

hudManager::~hudManager() {
    if(this->HUDGlobal)
        al_destroy_bitmap(this->HUDGlobal);
    if(this->HUDGamefield)
        al_destroy_bitmap(this->HUDGamefield);

}

bool hudManager::renderHUDforElement(std::shared_ptr<bElem> element, coords upLeft)
{
    coords offset=element->getOffset();
    //coords positionInBitmap=upLeft+offset;
    // TBD think through, how should it work, like should we have multiple huds for elements? like for example dialog lines?
    return false;
}

/**
 * @brief this will render a global HUD bitmap
 * It should be rendered to a separate bitmap, and then
 * @return
 */
bool hudManager::renderHUD() {

    return false;
}

hudManager& hudManager::getInstance() {
    static hudManager myHudManager;
    std::call_once(initFlag, []() {
        // TBD...
    });
    return myHudManager;
}

