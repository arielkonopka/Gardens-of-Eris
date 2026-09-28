/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CHAMBER_H
#define CHAMBER_H
#include "Coords.h"
#include "commons.h"
#include "randomWordGen.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <allegro5/allegro5.h>
typedef struct color
{
    int r;
    int g;
    int b;
    int a;
} colour;
class bElem;
//using boost::multi_array;



class chamber: public std::enable_shared_from_this<chamber>
{
    friend class gameSerializer;
public:
    chamber(const chamber&) = delete;
    chamber& operator=(const chamber&) = delete;

    int calculateLine(myUtility::Coords position,dir::direction Odir);
    std::shared_ptr<bElem> getLastInLine(myUtility::Coords pos,dir::direction mydir);

    /// every chamber in the world; owns them, so a chamber lives until the world is cleared
    static std::vector<std::shared_ptr<chamber>> allChambers;
    /// guards allChambers, and is held while a level is being generated or the game is saved or loaded
    static std::recursive_mutex worldMutex;
    /// set by the game thread when it wants worldMutex; the background generator pauses between levels until it is cleared
    static std::atomic<bool> worldLockWanted;
    /// false while a level generator is still filling this chamber; such chambers are not saved
    bool ready = true;
    static std::shared_ptr<chamber> makeNewChamber(coords csize);
    static std::shared_ptr<chamber> makeNewChamber(myUtility::Coords csize);
    bool visitPosition(int x, int y)
    {
        return this->visitPosition(coords(x,y));
    };
    unsigned int applesCount=0;
    bool visitPosition(coords point);
    int isVisible(int x, int y) ;
    int isVisible(coords point);
    void setVisible(coords point,int v);


 //   coords player;
    std::shared_ptr<bElem> getElement(int x, int y);
    std::shared_ptr<bElem> getElement(coords point);
    std::shared_ptr<bElem> getElement(myUtility::Coords point);
    void setElement(int x, int y, std::shared_ptr<bElem> elem);
    void setElement(coords point,std::shared_ptr<bElem> elem);
    coords getSize();
    myUtility::Coords getSizeCrd();

    explicit chamber(int x,int y);
    explicit chamber(coords csize);
    ~chamber();
    int getInstanceId();
    std::string getName();
    colour getChColour();
    coords getSizeOfChamber();

    bool registerLiveElem(std::shared_ptr<bElem> in);
    bool deregisterLiveElem(std::shared_ptr<bElem>in);
    std::vector<std::shared_ptr<bElem>> liveElems;
    std::vector<unsigned long int> toDeregister;


private:
    int width;
    int height;
    /// cells are stored column by column: index = x * height + y
    std::size_t cellIndex(int x, int y) const { return (std::size_t) x * this->height + y; }
    /// fog of war per cell; 0 once the player has seen it
    std::vector<int> visitedElements;
    void createFloor();
    /// the top element of every cell's stack
    std::vector<std::shared_ptr<bElem>> cells;
    colour chamberColour;
    std::string chamberName;
    void setInstanceId(int id);
    int instanceid;
    /// chambers are also created on the level generator thread
    static std::atomic<int> lastid;

};

#endif // CHAMBER_H
