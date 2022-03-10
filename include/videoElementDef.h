#ifndef VIDEOELEMENTDEF_H
#define VIDEOELEMENTDEF_H
#include <boost/multi_array.hpp>
#include <allegro5/allegro5.h>
#include <vector>
//#include <allegro5/allegro_font.h>
//#import "presenter.h"
#include "commons.h"
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
typedef boost::multi_array<::coords, 3> aphases;
typedef std::vector<::coords> coordVector;

class videoElementDef
{
    public:
        int subtypes;
        int directions;
        int animphases;
        void setValuesFor(ALLEGRO_BITMAP *sprites,int s,int d,int a,int x,int y);
        void addKill(int x, int y);
        void addTeleporting(int x, int y);
        aphases  *defArray;
        coordVector *teleporting;
        coordVector *dying;
        videoElementDef(int sts,int dirs,int anims);
        ~videoElementDef();
        videoElementDef(const videoElementDef& other);
        ALLEGRO_BITMAP *sprites;

    protected:

    private:

};


}
#endif // VIDEOELEMENTDEF_H
