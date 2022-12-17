#ifndef VIDEOELEMENTDEF_H
#define VIDEOELEMENTDEF_H
#include <allegro5/allegro5.h>
#include <vector>
//#include <allegro5/allegro_font.h>
//#import "presenter.h"
#include "commons.h"
#include "configManager.h"

/*
    Pointer to an object of this class, will be attached to every element in the game. This way, we can either share some animation definitions among
    elements in the game, but we also will be able to give different animation definitions to the same type of objects.


*/

namespace videoElement
{
/*
by zmieścić wszystkie możliwe typy ikonek dla obietków zdefiniujemy następującą strukturę:

  subtype[ direction[ animPhase[ ] ] ]
  zrealizujemy multiarray

*/
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
        videoElementDef(ALLEGRO_BITMAP *sprites_);
        ~videoElementDef();
        videoElementDef(const videoElementDef& other);
        ALLEGRO_BITMAP *sprites;
        static void initializeDriver();
    private:
        static bool initialized;

};


}
#endif // VIDEOELEMENTDEF_H
