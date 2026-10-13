// Draws one of every element in the 3D view, without fog, and saves the picture: a quick look at
// the skin's 3D models ("Model" in skins.json) without playing the game to find each of them.
// Run it from the game folder (GoEoOL): goe-gallery [picture.png]
#include "chamber.h"
#include "configManager.h"
#include "elementFactory.h"
#include "elements.h"
#include "fieldCamera.h"
#include "isoFieldView.h"
#include "videoManager.h"
#include "viewPoint.h"
#include <allegro5/allegro_image.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace {
template <typename T>
std::shared_ptr<T> put(const std::shared_ptr<chamber> &board, coords at, int subtype = 0)
{
    auto e = elementFactory::generateAnElement<T>(board, subtype);
    e->stepOnElement(board->getElement(at));
    return e;
}
} // namespace

int main(int argc, char **argv)
{
    const std::string out = argc > 1 ? argv[1] : "gallery.png";
    videoManager::getInstance();
    al_init_image_addon();
    const auto cfg = configManager::getInstance()->getConfig();
    const coords tile(cfg->tileWidth, cfg->tileHeight);

    // a room with a wall round it, a corridor with a door, and everything else in rows
    const coords size(22, 13);
    auto board = chamber::makeNewChamber(size);
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            if (x == 0 || y == 0 || x == size.x - 1 || y == size.y - 1 || (x == 6 && y != 3) || (x == 8 && y != 3))
                put<wall>(board, coords(x, y));
    put<door>(board, coords(7, 3), 0);
    put<wall>(board, coords(7, 2));
    put<wall>(board, coords(7, 4));
    auto open = put<door>(board, coords(7, 6), 2);
    open->getAttrs()->setOpen(true);
    open->getStats()->setFacing(dir::direction::LEFT);
    put<wall>(board, coords(7, 5));
    put<wall>(board, coords(7, 7));

    auto avatar = put<player>(board, coords(10, 5));
    viewPoint::get_instance().setOwner(avatar);
    std::vector<std::function<void(coords)>> things{
        [&](coords c) { put<key>(board, c, 0); },
        [&](coords c) { put<key>(board, c, 3); },
        [&](coords c) { put<goldenApple>(board, c); },
        [&](coords c) { put<refresher>(board, c); },
        [&](coords c) { put<plainGun>(board, c); },
        [&](coords c) { put<bazooka>(board, c); },
        [&](coords c) { put<plainMissile>(board, c); },
        [&](coords c) { put<bazookaMissile>(board, c); },
        [&](coords c) { put<simpleBomb>(board, c); },
        [&](coords c) { put<landmine>(board, c); },
        [&](coords c) { put<bouba>(board, c); },
        [&](coords c) { put<kiki>(board, c); },
        [&](coords c) { put<rubbish>(board, c); },
        [&](coords c) { put<brickCluster>(board, c); },
        [&](coords c) { put<monster>(board, c); },
        [&](coords c) { put<patrollingDrone>(board, c, 0); },
        [&](coords c) { put<patrollingDrone>(board, c, 1); },
        [&](coords c) { put<bunker>(board, c); },
        [&](coords c) { put<securityCamera>(board, c); },
        [&](coords c) { put<puppetMasterFR>(board, c); },
        [&](coords c) { put<teleport>(board, c, 0); },
        [&](coords c) { put<teleport>(board, c, 1); },
    };
    // two cells apart, five to a row, right of the corridor
    for (std::size_t i = 0; i < things.size(); i++)
        things[i](coords(10 + 2 * (int) (i % 5) + (int) (i / 5 % 2), 2 + 2 * (int) (i / 5)));

    bool saved = false;
    {
        const coords cells(24, 14);
        const coords px(cells.x * tile.x, cells.y * tile.y);
        isoFieldView view(px, tile, cfg->spacing, cfg->fogBitmap);
        if (view.ready()) {
            view.reveal(true);
            fieldCamera camera(tile);
            // the camera glides, a step a frame; let it arrive over the room's middle
            for (int i = 0; i < 400; i++)
                camera.follow(coords(size.x / 2, size.y / 2), cells, std::nullopt);
            goe::bitmapHandle picture(al_create_bitmap(px.x, px.y));
            al_set_target_bitmap(picture.get());
            al_clear_to_color(al_map_rgb(0, 0, 0));
            view.draw({avatar, camera, cells, coords(0, 0), px});
            saved = al_save_bitmap(out.c_str(), picture.get());
            std::cout << (saved ? "Saved " : "Could not save ") << out << "\n";
        } else
            std::cerr << "No 3D here: the depth buffer or the shader could not be made\n";
    }
    // the models' buffers and textures are gone with the view; now the display can go
    videoManager::getInstance().shutdown();
    return saved ? 0 : 1;
}
