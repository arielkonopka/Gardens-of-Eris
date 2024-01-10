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
#include "commons.h"
#include "randomWordGen.h"
#include <memory>
#include <thread>
#include <mutex>
#include <allegro5/allegro5.h>
#include <bElemContainer.h>
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
public:

    static std::shared_ptr<chamber> makeNewChamber(coords csize);
    bool visitPosition(int x, int y)
    {
        return this->visitPosition((coords)
        {
            x,y
        });
    };
    bool visitPosition(coords point);
    int isVisible(int x, int y) ;
    int isVisible(coords point);
    void setVisible(coords point,int v);

    int width;
    int height;
    coords player;
    std::shared_ptr<bElem> getElement(int x, int y);
    std::shared_ptr<bElem> getElement(coords point);
    void setElement(int x, int y, std::shared_ptr<bElem> elem);
    void setElement(coords point,std::shared_ptr<bElem> elem);
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

    ALLEGRO_MUTEX *SEMutex=nullptr;
    ALLEGRO_MUTEX *IdMutex=nullptr;
    ALLEGRO_MUTEX *VisMutex=nullptr;
    std::vector<std::vector<int>> visitedElements;
    void createFloor();
    std::vector<std::vector<std::shared_ptr<bElemContainer>>> chamberArray;
    colour chamberColour;
    std::string chamberName;
    void setInstanceId(int id);
    int instanceid;
    static int lastid;
    std::mutex chmutex;

};

#endif // CHAMBER_H
