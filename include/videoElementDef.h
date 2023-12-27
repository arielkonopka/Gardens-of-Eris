#ifndef VIDEOELEMENTDEF_H
#define VIDEOELEMENTDEF_H
#include "commons.h"

#include <allegro5/allegro5.h>
#include <vector>

#include "configManager.h"



namespace videoElement
{

typedef std::vector< std::vector<std::vector<::coords>>>  aphases;
typedef std::vector<::coords> coordVector;

class videoElementDef
{
    public:
        aphases  defArray;
        coordVector teleporting;
        coordVector dying;
        coordVector destroying;
        coordVector fadingOut;
        coordVector fadingIn;
        videoElementDef(ALLEGRO_BITMAP *sprites_);
        ~videoElementDef();
        videoElementDef(const videoElementDef& other);
        ALLEGRO_BITMAP *sprites;
        static void initializeDriver();
    private:

};


}
#endif // VIDEOELEMENTDEF_H
