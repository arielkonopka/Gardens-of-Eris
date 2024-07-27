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

#include "randomLevelGenerator.h"

randomLevelGenerator::randomLevelGenerator(int w, int h)
{
    this->width=w;
    this->height=h;
    this->mychamber=chamber::makeNewChamber({w,h});
    std::random_device rd;
    std::array<int,4> seedData;
    std::generate_n(seedData.data(), seedData.size(), std::ref(rd));
    std::seed_seq seq(std::begin(seedData), std::end(seedData));
    this->gen.seed(seq);
    this->doorTypes=0;
}

randomLevelGenerator::~randomLevelGenerator()
{

}


int randomLevelGenerator::checkWalls(int x, int y)
{
    bool  walls[]= {false,false,false,false};
    coords chambersize=this->mychamber->getSize();
    if (x>0)
    {
        if (this->mychamber->getElement(x-1,y)->getType()==_wallType) walls[0]=true;
    }
    if (x<chambersize.x)
    {
        if (this->mychamber->getElement(x+1,y)->getType()==_wallType) walls[1]=true;

    }
    if (y>0)
    {
        if (this->mychamber->getElement(x,y-1)->getType()==_wallType) walls[2]=true;
    }
    if (y<chambersize.y)
    {
        if (this->mychamber->getElement(x,y+1)->getType()==_wallType) walls[3]=true;

    }
    if (walls[0] && walls[1] && walls[2] && !walls[3])
    {
        this->mychamber->getElement(x-1,y)->disposeElement();
        this->mychamber->setElement((coords)
        {
            x,y
        },elementFactory::generateAnElement<wall>(this->mychamber,0));
    }
    if (walls[0] && walls[1] && walls[3] && !walls[2])
    {
        this->mychamber->getElement(x+1,y)->disposeElement();
    }

    if (walls[0] && !walls[1] && walls[3] && walls[2])
    {
        this->mychamber->getElement(x,y+1)->disposeElement();
    }
    if (!walls[0] && walls[1] && walls[3] && walls[2])
    {
        this->mychamber->getElement(x,y-1)->disposeElement();
    }


    return 0;
}

/* 이것은 순환 분할 구현입니다 */

chamberArea* randomLevelGenerator::lvlGenerate(int x1, int y1, int x2, int y2,int depth,int holes)
{
    chamberArea* mychamberArea=new chamberArea(x1,y1,x2,y2);
#ifdef _VerbousMode_
    std::cout<<"create Chamber "<<x1<<","<<y1<<" "<<x2<<","<<y2<<"\n";
#endif
    std::vector<int> doorPlaces1,doorPlaces2;
    int Hmin_=((y2-y1)/2);
    int Wmin_=((x2-x1)/2);

    if(Hmin_<Hmin)
        Hmin_=Hmin;
    if(Wmin_<Wmin)
        Wmin_=Wmin;

    int c=x2,dc1=-1,dc2=-1;
    int d=y2,dd1=-1,dd2=-1;
    int dx,dy;
    dx=x2-x1;
    dc1=dx+1;
    dy=y2-y1;
    dd1=dy+1;
    c=x2;
    d=y2;

    if (depth<0 || (dx<Wmin_ && dy<Hmin_))
    {
#ifdef _VerbousMode_
        std::cout<<"Depth deplated or size too small: ("<<x1<<","<<y1<<")->("<<x2<<","<<y2<<")\n";
#endif
        return mychamberArea;
    }

    // 수평 길이는 세로 길이의 절반 이상이어야 합니다.
    if (dx>Wmin_ && dx*2>dy)
    {

        c=(this->gen() % (dx-(Wmin_)))+x1+(Wmin_/2); //find vertical divider location
        dc1=c-x1;
        dc2=x2-c+2;

    }
    if (dy>Hmin_ && dy*2>dx)
    {
        d=(this->gen()%(dy-(Hmin_)))+y1+(Hmin_/2);  // horizontal divider
        dd1=d-y1;
        dd2=y2-d+2;
    }
    // add children nodes first, then we will draw the overlay
    if (dc1>Wmin && dd1>Hmin)
    {
        mychamberArea->addChildNode(this->lvlGenerate(x1,y1,c,d,depth-1,holes));
    }
    if (dc2>Wmin && dd1>Hmin)
    {
        mychamberArea->addChildNode(this->lvlGenerate(c+2,y1,x2,d,depth-1,holes));
    }

    if (dd2>Hmin && dc1>Wmin)
    {
        mychamberArea->addChildNode(this->lvlGenerate(x1,d+2,c,y2,depth-1,holes));
    }

    if (dd2>Hmin && dc2>Wmin)
    {
        mychamberArea->addChildNode(this->lvlGenerate(c+2,d+2,x2,y2,depth-1,holes));
    }
    doorPlaces1.clear();
    doorPlaces2.clear();
    if (c!=x2) // d>0 dvider exists
    {
        //we draw vertical line
        for (int a=y1; a<=y2; a++)
        {
            if (this->mychamber->getElement(c+2,a) && a!=d && this->mychamber->getElement(c,a)->getAttrs()->isSteppable() && this->mychamber->getElement(c+2,a)->getAttrs()->isSteppable())
            {
                if (a<d+2)
                {
                    doorPlaces1.push_back(a);
                }
                else
                {
                    doorPlaces2.push_back(a);
                }
            }
            if (this->mychamber->getElement(c+1,a)->getAttrs()->isSteppable())
            {
                std::shared_ptr<bElem> newElement=elementFactory::generateAnElement<wall>(this->mychamber,0);
                newElement->stepOnElement(this->mychamber->getElement(c+1,a));
            }
            else
            {
                break;
            }
        }

//now pick the door gap randomly, and make as many holes, as requested
        for (int cnt=0; cnt<holes; cnt++)
        {
            if (doorPlaces1.size()>0)
            {
                int rnd=this->gen()%(doorPlaces1.size());
                this->mychamber->getElement(c+1,doorPlaces1[rnd])->disposeElement();
                doorPlaces1[rnd]=doorPlaces1[doorPlaces1.size()-1];
                doorPlaces1.pop_back();
            }
            if (doorPlaces2.size()>0)
            {
                int rnd=this->gen()%(doorPlaces2.size());
                this->mychamber->getElement(c+1,doorPlaces2[rnd])->disposeElement();
                doorPlaces2[rnd]=doorPlaces2[doorPlaces2.size()-1];
                doorPlaces2.pop_back();
            }
        }

    }
    //we draw horizontal line

    if (d!=y2) // divider exists
    {
        doorPlaces1.clear();
        doorPlaces2.clear();
        for (int a=x1; a<=x2; a++)
        {
            if(!this->mychamber->getElement(a,d))
            {
                std::cout<<"Nulls on board!\n";
                continue;
            }

            if (a!=c && this->mychamber->getElement(a,d)->getAttrs()->isSteppable() && this->mychamber->getElement(a,d+2)->getAttrs()->isSteppable())
            {
                if (a<c+2)
                {
                    doorPlaces1.push_back(a);
                }
                else
                {
                    doorPlaces2.push_back(a);
                }
            }
            if (mychamber->getElement(a,d+1)->getAttrs()->isSteppable())
            {
                std::shared_ptr<bElem> newElement=elementFactory::generateAnElement<wall>(this->mychamber,0);
                newElement->stepOnElement(this->mychamber->getElement(a,d+1));
            }
        }
        for (int cnt=0; cnt<holes; cnt++)
        {
            if (!doorPlaces1.empty())
            {
                int rnd=this->gen()%(doorPlaces1.size());
                this->mychamber->getElement(doorPlaces1[rnd],d+1)->disposeElement();
                doorPlaces1[rnd]=doorPlaces1[doorPlaces1.size()-1];
                doorPlaces1.pop_back();
            }
        }
        for (int cnt=0; cnt<holes; cnt++)
        {

            if (!doorPlaces2.empty())
            {
                int rnd=this->gen()%(doorPlaces2.size());
                this->mychamber->getElement(doorPlaces2[rnd],d+1)->disposeElement();
                doorPlaces2[rnd]=doorPlaces2[doorPlaces2.size()-1];
                doorPlaces2.pop_back();
            }
        }
    }

    for(unsigned int cnt=0; cnt<mychamberArea->children.size(); cnt++) // add parent to the children
    {
        mychamberArea->children[cnt]->parent=mychamberArea;
    }

    return mychamberArea;
}


bool randomLevelGenerator::placeElementCollection(chamberArea* chmbrArea,std::vector<elementToPlace>* elements)
{
#ifdef _VerbousMode_
    std::cout<<"Chosen area properties - surface: "<<chmbrArea->surface<<" x,y,x1,y1:"<<chmbrArea->upLeft.x<<" "<<chmbrArea->upLeft.y<<" "<<chmbrArea->downRight.x<<" "<<chmbrArea->downRight.y<<"\n";
#endif
    for(unsigned int cnt=0; cnt<elements->size(); cnt++)
    {

        chmbrArea->findElementsToStepOn(mychamber);
#ifdef _VerbousMode_
        std::cout<<"Found: "<<(long int)chamberArea::foundElements.size()<<" elements to step on\n";
#endif // _VerbousMode_

        for (int cnt2=0; cnt2<(*elements)[cnt].number; cnt2++) // We sometimes must create more than one element (we count from 0)
        {

            if((long int)chamberArea::foundElements.size()<=0)
                break;
            unsigned int selectedEl=this->gen()%chamberArea::foundElements.size(); //find the position in the found elements
            std::shared_ptr<bElem> newElem=createElement((*elements)[cnt]);
            newElem->stepOnElement(chamberArea::foundElements[selectedEl]); //place element on a board
            newElem->selfAlign();
            chamberArea::foundElements.erase(chamberArea::foundElements.begin()+selectedEl);
        }

    }

    return true;
}










bool randomLevelGenerator::generateLevel(int holes)
{
    int tolerance=10;
    this->headNode=this->lvlGenerate(1,1,this->width-2,this->height-2,_iterations,holes);

    this->headNode->calculateInitialSurface();

    std::vector<elementToPlace> elementCollection; // here we will store the elements to be placed on the board
    std::vector<elementToPlace> elementsToChooseFrom;
    //draw the walls around the whole chamber
    for(int c=0; c<this->width; c++)
    {
        std::shared_ptr<bElem> newElem=elementFactory::generateAnElement<wall>(this->mychamber,0);
        std::shared_ptr<bElem> newElem1=elementFactory::generateAnElement<wall>(this->mychamber,0);
        newElem->stepOnElement(this->mychamber->getElement(c,0));
        newElem1->stepOnElement(this->mychamber->getElement(c,this->height-1));

    }
    for(int c=0; c<this->height; c++)
    {

        std::shared_ptr<bElem> newElem2=elementFactory::generateAnElement<wall>(this->mychamber,0);
        std::shared_ptr<bElem> newElem3=elementFactory::generateAnElement<wall>(this->mychamber,0);
        newElem2->stepOnElement(this->mychamber->getElement(0,c));
        newElem3->stepOnElement(this->mychamber->getElement(this->width-1,c));
    }

    // build probablility table - this way we can pick random objects with different probablilities
    for(int c=1; c<(50/holes); c++)
    {
        // dangerous elements here, the more holes, the less of them in the gamefield
        elementsToChooseFrom.push_back({_monster,0,1,0,3});
        elementsToChooseFrom.push_back({_bunker,0,1,0,3});

    }
    for(int c=0; c<holes*15; c++)
    {
        elementsToChooseFrom.push_back({_bunker,0,1,0,3});

        elementsToChooseFrom.push_back({_goldenAppleType,0,1,0,3});
        elementsToChooseFrom.push_back({_simpleBombType,0,1,0,3});
        elementsToChooseFrom.push_back({_puppetMasterType,0,1,0,3});
        elementsToChooseFrom.push_back({_patrollingDrone,0,1,0,3});


    }
    for(int cnt=0; cnt<holes*5; cnt++)
    {
        elementsToChooseFrom.push_back({_key,1,1,0,3});
        elementsToChooseFrom.push_back({_key,3,1,0,3});
        elementsToChooseFrom.push_back({_bazookaType,0,1,0,3});
        elementsToChooseFrom.push_back({_plainGun,0,1,0,3});



    }

    for(int c=0; c<50; c++)
    {
        elementsToChooseFrom.push_back({_brickClusterType,0,1,0,3});
        elementsToChooseFrom.push_back({_key,0,1,0,3});
        elementsToChooseFrom.push_back({_key,2,1,0,3});
        elementsToChooseFrom.push_back({_key,4,1,0,3});
        elementsToChooseFrom.push_back({_teleporter,this->mychamber->getInstanceId()+1,1,0,3});

    }
    //  elementsToChooseFrom.push_back({_teleporter,0,1,0,6});
    elementsToChooseFrom.push_back({_player,0,1,0,3});

    //
    elementsToChooseFrom.push_back({_patrollingDrone,0,1,0,3});
    elementsToChooseFrom.push_back({_puppetMasterType,0,1,0,3});


    //first find area for the player and stuff for it

    elementCollection.push_back({_player,0,2,0,3});
    elementCollection.push_back({_key,1,2,0,3});
    elementCollection.push_back({_plainGun,0,2,0,3});
    elementCollection.push_back({_teleporter,0,1,0,3});

    /***************************refactor me******************************/
    int demandedSurface=0;
    for(unsigned int cnt=0; cnt<elementCollection.size(); cnt++) demandedSurface+=elementCollection[cnt].surface*(elementCollection[cnt].number);
    chamberArea::foundAreas.clear();
    this->headNode->findChambersCloseToSurface(demandedSurface,tolerance);
    if (chamberArea::foundAreas.empty())
    {
        std::cout<<"Found areas is empty!\n";
        return false;
    }
    int selectedChamberNo=0;

    selectedChamberNo=(this->gen()%chamberArea::foundAreas.size());
    this->placeElementCollection(chamberArea::foundAreas[selectedChamberNo],&elementCollection);
    this->placeDoors({_door,1,1,0,9},chamberArea::foundAreas[selectedChamberNo]);
    if (chamberArea::foundAreas[selectedChamberNo]->parent!=nullptr)
    {
        chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock=true;
    }

    delete chamberArea::foundAreas[selectedChamberNo];
    this->headNode->removeEmptyNodes();
    chamberArea::foundAreas[selectedChamberNo]=chamberArea::foundAreas[chamberArea::foundAreas.size()-1];
    chamberArea::foundAreas.pop_back();
    elementCollection.clear();

    elementCollection.push_back({_teleporter,0,1,0,5});
    selectedChamberNo=(this->gen()%chamberArea::foundAreas.size());
    this->placeElementCollection(chamberArea::foundAreas[selectedChamberNo],&elementCollection);
    this->placeDoors({_door,0,1,0,9},chamberArea::foundAreas[selectedChamberNo]);
    if (chamberArea::foundAreas[selectedChamberNo]->parent!=nullptr)
    {
        chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock=true;
    }

    delete chamberArea::foundAreas[selectedChamberNo];
    this->headNode->removeEmptyNodes();

    chamberArea::foundAreas[selectedChamberNo]=chamberArea::foundAreas[chamberArea::foundAreas.size()-1];
    chamberArea::foundAreas.pop_back();


    bool _ex=false;
    chamberArea::foundAreas.clear();
    while(!_ex)
    {
#ifdef _VerbousMode_
        std::cout<<"Surface total: "<<this->headNode->surface<<"\n";
#endif
        elementToPlace _nel;
        int demandedSurface=0;

        int cnt;
        int elementsToMake=((this->gen()%5)+1)*5;
        elementCollection.clear();
        for(cnt=0; cnt<elementsToMake; cnt++)
        {
            _nel=elementsToChooseFrom[this->gen()%elementsToChooseFrom.size()];
            elementCollection.push_back(_nel);

        }
        for(unsigned int cnt=0; cnt<elementCollection.size(); cnt++) demandedSurface+=elementCollection[cnt].surface*(elementCollection[cnt].number);
        chamberArea::foundAreas.clear();
        this->headNode->findChambersCloseToSurface(demandedSurface,tolerance);
        if(chamberArea::foundAreas.empty())
            _ex=true;
        if(!chamberArea::foundAreas.empty())
        {
            int selectedChamberNo=(this->gen()%chamberArea::foundAreas.size());
            this->placeElementCollection(chamberArea::foundAreas[selectedChamberNo],&elementCollection);
            elementCollection.clear();
            if(chamberArea::foundAreas[selectedChamberNo]->parent!=nullptr)
            {
                if(!chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock)
                {
                    int dice=this->gen()%100;
                    int keyType=this->gen()%10;
                    if(dice<(75/holes))
                    {
                        if(keyType>=5)
                            this->placeDoors({_brickClusterType,0,0,9},chamberArea::foundAreas[selectedChamberNo]);
                        else
                            this->placeDoors({_door,keyType,1,0,9},chamberArea::foundAreas[selectedChamberNo]);
                        elementCollection.push_back({_key,keyType,1,0,9}); // place key for the door and store it somewhere - warning, the door placed at the end might never receive the key
                        chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock=true;

                    }

                }
                this->headNode->removeEmptyNodes();
                delete chamberArea::foundAreas[selectedChamberNo];

            }

        }
        else
        {
            _ex=true;


        }


    }
    /****************************************************/




    return true;

}





bool randomLevelGenerator::placeDoors(elementToPlace element,chamberArea* location)
{
    /*
    Place doors at the location

    */
#ifdef _VerbousMode_
    std::cout<<"door "<<element.eSubType<<"\n";
#endif
    //Ok, now we need to place the door.
    for(int c1=location->upLeft.x-1; c1<=location->downRight.x+1; c1++)
    {
        if (this->mychamber->getElement(c1,location->upLeft.y-1)->getAttrs()->isSteppable())
        {
            std::shared_ptr<bElem> neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->getElement(c1,location->upLeft.y-1));
        }
        if (this->mychamber->getElement(c1,location->downRight.y+1)->getAttrs()->isSteppable())
        {
            std::shared_ptr<bElem> neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->getElement(c1,location->downRight.y+1));
        }
    }
    for (int c2=location->upLeft.y; c2<=location->downRight.y; c2++)
    {
        if (this->mychamber->getElement(location->upLeft.x-1,c2)->getAttrs()->isSteppable())
        {
            std::shared_ptr<bElem> neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->getElement(location->upLeft.x-1,c2));
        }
        if (this->mychamber->getElement(location->downRight.x+1,c2)->getAttrs()->isSteppable())
        {
            std::shared_ptr<bElem> neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->getElement(location->downRight.x+1,c2));
        }

    }


    return true;
}


std::shared_ptr<bElem> randomLevelGenerator::createElement(elementToPlace element)
{
    switch(element.eType)
    {

    case _goldenAppleType:
        return elementFactory::generateAnElement<goldenApple>(this->mychamber,0);
    case _player:
        return elementFactory::generateAnElement<player>(this->mychamber,0);
    case _door:
        return elementFactory::generateAnElement<door>(this->mychamber,element.eSubType);
    case _key:
        return elementFactory::generateAnElement<key>(this->mychamber,element.eSubType);
    case _monster:
        return elementFactory::generateAnElement<monster>(this->mychamber,element.eSubType);
    case _plainGun:
        return elementFactory::generateAnElement<plainGun>(this->mychamber,element.eSubType);
    case _bunker:
        return elementFactory::generateAnElement<bunker>(this->mychamber,0);
    case _teleporter:
        return elementFactory::generateAnElement<teleport>(this->mychamber,element.eSubType);

    case _simpleBombType:
        return elementFactory::generateAnElement<simpleBomb>(this->mychamber,0);
    case _patrollingDrone:
        return elementFactory::generateAnElement<patrollingDrone>(this->mychamber,0);
    case _brickClusterType:
        return elementFactory::generateAnElement<brickCluster>(this->mychamber,0);
    case _puppetMasterType:
        return elementFactory::generateAnElement<puppetMasterFR>(this->mychamber,0);
    case _bazookaType:
        return elementFactory::generateAnElement<bazooka>(this->mychamber,element.eSubType);
    }
    return nullptr;
}



