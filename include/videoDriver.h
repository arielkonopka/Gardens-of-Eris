#ifndef VIDEODRIVER_H
#define VIDEODRIVER_H
#include <allegro5/allegro5.h>
#include <vector>
#include <unordered_map>
#include "commons.h"
#include <memory>
#include <mutex>
#include "videoElementDef.h"
#include "configManager.h"
#include <allegro5/allegro_image.h>


class videoDriver;
using vDriver = std::shared_ptr<videoDriver>;
using vElement = std::shared_ptr<videoElement::videoElementDef>;

class videoDriver : public std::enable_shared_from_this<videoDriver>
{

public:
    static videoDriver* getInstance();
    virtual ~videoDriver();
    vElement getVideoElement(int typeId);

private:

    static videoDriver* myInstance;
    videoDriver();
    std::unordered_map<int,vElement> sprites;
    int lastSpriteT=-1;
    vElement lSprite;
    static std::once_flag _onceFlag;

};

#endif // VIDEODRIVER_H
