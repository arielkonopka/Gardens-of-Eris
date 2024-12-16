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

#include "chamber.h"
#include "floorElement.h"
#include "player.h"
int chamber::lastid = 0;

std::shared_ptr<chamber> chamber::makeNewChamber(coords csize)
{
#ifdef _VerbousMode_
    std::cout << "generate chamber" << csize.x << "," << csize.y << "\n";
#endif
    std::shared_ptr<chamber> c = makeNewChamber(myUtility::Coords (csize.x,csize.y));
#ifdef _VerbousMode_
    std::cout << "generated object\n";
#endif
    c->createFloor();
    return c;
}
std::shared_ptr<chamber> chamber::makeNewChamber(myUtility::Coords csize)
{
#ifdef _VerbousMode_
    std::cout << "generate chamber" << csize.getX() << "," << csize.getY() << "\n";
#endif
    std::shared_ptr<chamber> c = std::make_shared<chamber>(csize.getX(), csize.getY());
#ifdef _VerbousMode_
    std::cout << "generated object\n";
#endif
    c->createFloor();
    return c;
}




void chamber::createFloor()
{
#ifdef _VerbousMode_
    std::cout << "Create floor instance [" << this->getStats()->getInstanceId() << "]\n";
    std::cout << " cfsize [";
#endif
    for (int c = 0; c < this->width; c++)
    {

        std::vector<int> v2(this->height,555);
        this->visitedElements.push_back(v2);

        std::vector<std::shared_ptr<bElemContainer>> v;
        for (int d = 0; d < this->height; d++)
        {

            int subtype=0;
            if (bElem::randomNumberGenerator() % 10 == 0)
                subtype=1;
            if (bElem::randomNumberGenerator() % 100 == 0)
                subtype=2;

#ifdef _VerbousMode_
            std::cout << "Create an object to place\n";
#endif
            auto bec=std::make_shared<bElemContainer>();
            bec->element=elementFactory::generateAnElement<floorElement>(shared_from_this(),subtype);
            bec->element->setBoard(shared_from_this());
            bec->element->getStats()->setMyPosition(coords(c,d));
            bec->element->getAttrs()->setSubtype(subtype);
#ifdef _VerbousMode_
            std::cout << "created id " << b->getStats()->getInstanceId() << "\n";
#endif

#ifdef _VerbousMode_
            std::cout << "Push object into column vector id " << b->getStats()->getInstanceId() << "\n";
#endif
            v.push_back(bec);
        }
        this->chamberArray.push_back(v);
#ifdef _VerbousMode_
        std::cout << this->chamberArray.size() << " " << v.size() << " ";
#endif
    }
#ifdef _VerbousMode_
    std::cout << "\n CFsize " << this->chamberArray.size() << " " << this->chamberArray[0].size() << "\n";
#endif
}

coords chamber::getSizeOfChamber()
{
    return coords((int)this->chamberArray.size(), (this->chamberArray.size() > 0) ? (int)this->chamberArray[0].size() : -1);
}

chamber::chamber(int x, int y) : std::enable_shared_from_this<chamber>(), width(x), height(y), SEMutex(al_create_mutex_recursive()),IdMutex(al_create_mutex_recursive()),VisMutex(al_create_mutex_recursive())
{
    std::shared_ptr<randomWordGen> rwg = std::make_shared<randomWordGen>();
    this->setInstanceId(chamber::lastid++);
    this->chamberName = rwg->generateWord(3);
    this->chamberColour.a = 255;
    this->chamberColour.r = 30 + rwg->randomNumberGenerator() % 50;
    this->chamberColour.g = 30 + rwg->randomNumberGenerator() % 50;
    this->chamberColour.b = 50 + rwg->randomNumberGenerator() % 70;
    //this->createFloor();
}
chamber::chamber(coords csize) : chamber(csize.x, csize.y)
{
}

colour chamber::getChColour()
{
    //  std::lock_guard<std::mutex> guard(this->chmutex);
    return this->chamberColour;
}

chamber::~chamber()
{

}

std::string chamber::getName()
{
    return this->chamberName;
}

std::shared_ptr<bElem> chamber::getElement(coords point)
{
    return this->getElement(point.x, point.y);
}
bool chamber::visitPosition(coords point)
{
    bool res=false;
    if(point==NOCOORDS)
        return false;
    al_lock_mutex(this->VisMutex);
    const int vradius=player::getActivePlayer()->getViewRadius()/2;
    int x0=((point.x-vradius)<0)?0:((point.x-vradius>=this->width)?this->width-1:point.x-vradius);
    int y0=((point.y-vradius)<0)?0:((point.y-vradius>=this->height)?this->height-1:point.y-vradius);
    int x1=((point.x+vradius)<0)?0:((point.x+vradius>=this->width)?this->width-1:point.x+vradius);
    int y1=((point.y+vradius)<0)?0:((point.y+vradius>=this->height)?this->height-1:point.y+vradius);
    for(int x=x0; x<=x1; x++)
    {
        for(int y=y0; y<=y1; y++)
        {
            float distance=point.distance(coords(x,y));
            if (distance<=vradius && this->visitedElements[x][y]!=0)
            {
                res=true;
                this->visitedElements[x][y]=0;
            }

        }
    }
    al_unlock_mutex(this->VisMutex);
    return res;

}
void chamber::setVisible(coords point,int v)
{

    al_lock_mutex(this->VisMutex);
    if(point.x>=0 && point.y>=0 && point.x<this->width && point.y<this->height)
        this->visitedElements[point.x][point.y]=v;
    al_unlock_mutex(this->VisMutex);
}


int chamber::isVisible(int x, int y)
{
    return this->isVisible(coords(x,y));
}

int chamber::isVisible(coords point)
{
    if(point.x<this->width && point.y<this->height && point.x>=0 && point.y>=0)
        return this->visitedElements[point.x][point.y];
    return false;

}



std::shared_ptr<bElem> chamber::getElement(int x, int y)
{
    // std::lock_guard<std::mutex> guard(this->chmutex);
    if (x < 0 || y < 0 )
        return nullptr;
    if ((unsigned int)x >= this->chamberArray.size() || (unsigned int)y >= this->chamberArray[x].size())
        return nullptr;
    return this->chamberArray[x][y]->element;
}

void chamber::setElement(int x, int y, std::shared_ptr<bElem> elem)
{
    if (!elem || x < 0 || x > this->width - 1 || y < 0 || y > this->height - 1)
        return;
    al_lock_mutex(this->chamberArray[x][y]->eMutex);
    this->chamberArray[x][y]->element = elem;
    elem->setBoard(shared_from_this());
    al_unlock_mutex(this->chamberArray[x][y]->eMutex);

}

void chamber::setElement(coords point, std::shared_ptr<bElem> elem)
{
    this->setElement(point.x, point.y, elem);
}

int chamber::getInstanceId()
{
    return this->instanceid;
}

void chamber::setInstanceId(int id)
{
    al_lock_mutex(this->IdMutex);
    this->instanceid = id;
    al_unlock_mutex(this->IdMutex);
}

bool chamber::registerLiveElem(std::shared_ptr<bElem>in)
{

    auto iid=in->getStats()->getInstanceId();

    if(!in->getBoard())
        return false;
    for(unsigned int c=0; c<in->getBoard()->toDeregister.size();)
        if(in->getBoard()->toDeregister[c]==iid)
            in->getBoard()->toDeregister.erase(in->getBoard()->toDeregister.begin()+c);
        else c++;

    this->liveElems.push_back(in);
    return true;
}

bool chamber::deregisterLiveElem(std::shared_ptr<bElem>in)
{
  if(in->getStats()->hasActivatedMechanics()&& in->getBoard())
        in->getBoard()->toDeregister.push_back(in->getStats()->getInstanceId());
    in->getStats()->setActivatedMechanics(false);
    return true;
}

coords chamber::getSize() {

    return coords (this->width,this->height);
}

myUtility::Coords chamber::getSizeCrd()
{
    return myUtility::Coords(this->width,this->height);
}



