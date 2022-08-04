#include "randomLevelGenerator.h"

randomLevelGenerator::randomLevelGenerator(int w, int h)
{
    this->width=w;
    this->height=h;
    this->mychamber=new chamber(w,h);
    this->garbageCollector=gCollect::getInstance();
    std::random_device rd;
    std::mt19937::result_type seed = rd() ^ (
                                         (std::mt19937::result_type)
                                         std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch()
                                         ).count() +
                                         (std::mt19937::result_type)
                                         std::chrono::duration_cast<std::chrono::microseconds>(
                                                 std::chrono::high_resolution_clock::now().time_since_epoch()
                                         ).count() );
    this->gen.seed(seed);
    this->doorTypes=0;
    //ctor
}

randomLevelGenerator::~randomLevelGenerator()
{

}


int randomLevelGenerator::checkWalls(int x, int y)
{
    bool  walls[]= {false,false,false,false};
    if (x>0)
    {
        if (this->mychamber->chamberArray[x-1][y]->getType()==_wallType) walls[0]=true;
    }
    if (x<this->mychamber->width)
    {
        if (this->mychamber->chamberArray[x+1][y]->getType()==_wallType) walls[1]=true;

    }
    if (y>0)
    {
        if (this->mychamber->chamberArray[x][y-1]->getType()==_wallType) walls[2]=true;
    }
    if (y<this->mychamber->height)
    {
        if (this->mychamber->chamberArray[x][y+1]->getType()==_wallType) walls[3]=true;

    }
    if (walls[0] && walls[1] && walls[2] && !walls[3])
    {
        this->mychamber->chamberArray[x-1][y]->disposeElement();
        this->mychamber->chamberArray[x][y]=new wall(this->mychamber);
    }
    if (walls[0] && walls[1] && walls[3] && !walls[2])
    {
        this->mychamber->chamberArray[x+1][y]->disposeElement();
    }

    if (walls[0] && !walls[1] && walls[3] && walls[2])
    {
        this->mychamber->chamberArray[x][y+1]->disposeElement();
    }
    if (!walls[0] && walls[1] && walls[3] && walls[2])
    {
        this->mychamber->chamberArray[x][y-1]->disposeElement();
    }


    return 0;
}

/* this is recurrent division implementation */

chamberArea* randomLevelGenerator::lvlGenerate(int x1, int y1, int x2, int y2,int depth,int holes)
{
    chamberArea* mychamberArea=new chamberArea(x1,y1,x2,y2);
    std::cout<<"create Chamber "<<x1<<","<<y1<<"\n";
    std::vector<int> doorPlaces1,doorPlaces2;
    int Hmin_=((y2-y1)/2);
    int Wmin_=((x2-x1)/2);
    int sa=0,sb=0,sc=0,sd=0;
    if(Hmin_<Hmin)
        Hmin_=Hmin;
    if(Wmin_<Wmin)
        Wmin_=Wmin;

    int c=-1,dc1=-1,dc2=-1;
    int d=-1,dd1=-1,dd2=-1;
    //  bElem *belem;
    int dx,dy;
    if (depth<0)
    {
        std::cout<<"Depth deplated\n";
        return mychamberArea;
    }

    dx=x2-x1;
    dy=y2-y1;
    if (dx>Wmin_ ) // tutaj trzeba zrobić ograniczenie
        // Im krótsza ścieżka, tym wmin bliższe 1/3 planszy
    {

        c=(this->gen() % (dx-Wmin_)) +x1+(Wmin_/2); //find vertical divider location
        dc1=c-x1;
        dc2=x2-c+2;

    }
    //else return 0;
    if (dy>Hmin_ )
    {
        d=(this->gen()%(dy-Hmin_)) +y1+(Hmin_/2);  // horizontal divider
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
    if (c>x1 && c<x2 && d>0) // d>0 dvider exists
    {
        //we draw vertical line
        for (int a=y1; a<=y2; a++)
        {
            if (a!=d && this->mychamber->chamberArray[c][a]->isSteppable()==true && this->mychamber->chamberArray[c+2][a]->isSteppable()==true)
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
            if (this->mychamber->chamberArray[c+1][a]->isSteppable())
            {
                bElem *newElement=new wall(this->mychamber);
                newElement->stepOnElement(this->mychamber->chamberArray[c+1][a]);
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
                this->mychamber->chamberArray[c+1][doorPlaces1[rnd]]->disposeElement();
                doorPlaces1[rnd]=doorPlaces1[doorPlaces1.size()-1];
                doorPlaces1.pop_back();
            }
            if (doorPlaces2.size()>0)
            {
                int rnd=this->gen()%(doorPlaces2.size());
                this->mychamber->chamberArray[c+1][doorPlaces2[rnd]]->disposeElement();
                //if(rnd<doorPlaces2.size()-1)
                doorPlaces2[rnd]=doorPlaces2[doorPlaces2.size()-1];
                doorPlaces2.pop_back();
            }
        }

    }
    //we draw horizontal line
    if (d>y1 && d<y2 && c>=0) // divider exists
    {
        doorPlaces1.clear();
        doorPlaces2.clear();
        for (int a=x1; a<=x2; a++)
        {
            if (a!=c && this->mychamber->chamberArray[a][d]->isSteppable()==true && this->mychamber->chamberArray[a][d+2]->isSteppable()==true)
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
            if (mychamber->chamberArray[a][d+1]->isSteppable())
            {
                bElem *newElement=new wall(this->mychamber);
                newElement->stepOnElement(this->mychamber->chamberArray[a][d+1]);
            }
        }
        for (int cnt=0; cnt<holes; cnt++)
        {
            if (doorPlaces1.size()>0)
            {
                int rnd=this->gen()%(doorPlaces1.size());
                this->mychamber->chamberArray[doorPlaces1[rnd]][d+1]->disposeElement();
                doorPlaces1[rnd]=doorPlaces1[doorPlaces1.size()-1];
                doorPlaces1.pop_back();
            }
        }
        for (int cnt=0; cnt<holes; cnt++)
        {

            if (doorPlaces2.size()>0)
            {
                int rnd=this->gen()%(doorPlaces2.size());
                this->mychamber->chamberArray[doorPlaces2[rnd]][d+1]->disposeElement();
                doorPlaces2[rnd]=doorPlaces2[doorPlaces2.size()-1];
                doorPlaces2.pop_back();
            }
        }
    }

    for(int cnt=0; cnt<mychamberArea->children.size(); cnt++) // add parent to the children
    {
        mychamberArea->children[cnt]->parent=mychamberArea;
    }

    return mychamberArea;
}


bool randomLevelGenerator::placeElementCollection(chamberArea* chmbrArea,std::vector<elementToPlace>* elements)
{
//   chamberArea::foundAreas.clear();
    chmbrArea->findElementsToStepOn(mychamber);
    std::cout<<"Chosen area properties - surface: "<<chmbrArea->surface<<" x,y,x1,y1:"<<chmbrArea->upLeft.x<<" "<<chmbrArea->upLeft.y<<" "<<chmbrArea->downRight.x<<" "<<chmbrArea->downRight.y<<"\n";

    for(int cnt=0; cnt<elements->size(); cnt++)
    {
        //   std::cout<<"Type: "<<(*elements)[cnt].eType<<"\n";
        for (int cnt2=0; cnt2<(*elements)[cnt].number; cnt2++) // We sometimes must produce more than one element (we count from 0)
        {
            int selectedEl=this->gen()%chamberArea::foundElements.size(); //find the position in the found elements
            bElem* newElem=createElement((*elements)[cnt]);
            newElem->stepOnElement(chamberArea::foundElements[selectedEl]); //place element on a board
            chamberArea::foundElements[selectedEl]=chamberArea::foundElements[chamberArea::foundElements.size()-1]; //remove it from the list, so it would not be used again
            chamberArea::foundElements.pop_back();

        }

    }

    return true;
}










bool randomLevelGenerator::generateLevel(int holes)
{
    this->headNode=this->lvlGenerate(1,1,this->width-2,this->height-2,_iterations,holes);

    int availableSurface=this->headNode->calculateInitialSurface();

    std::vector<elementToPlace> elementCollection; // here we will store the elements to be placed on the board
    std::vector<elementToPlace> elementsToChooseFrom;

    //draw the walls around the whole chamber
    for(int c=0; c<this->width; c++)
    {
        bElem *newElem=new wall(this->mychamber);
        bElem *newElem1=new wall(this->mychamber);
        newElem->stepOnElement(this->mychamber->chamberArray[c][0]);
        newElem1->stepOnElement(this->mychamber->chamberArray[c][this->height-1]);

    }
    for(int c=0; c<this->height; c++)
    {

        bElem *newElem2=new wall(this->mychamber);
        bElem *newElem3=new wall(this->mychamber);
        newElem2->stepOnElement(this->mychamber->chamberArray[0][c]);
        newElem3->stepOnElement(this->mychamber->chamberArray[this->width-1][c]);
    }


    elementsToChooseFrom.push_back({_monster,0,1,0,9});
    elementsToChooseFrom.push_back({_monster,1,1,0,9});
    elementsToChooseFrom.push_back({_collectible,0,1,0,9});
    elementsToChooseFrom.push_back({_key,0,1,0,9});
    elementsToChooseFrom.push_back({_key,1,1,0,9});
    elementsToChooseFrom.push_back({_key,2,1,0,9});
    elementsToChooseFrom.push_back({_key,3,1,0,9});
    elementsToChooseFrom.push_back({_key,4,1,0,9});
    elementsToChooseFrom.push_back({_plainGun,0,1,0,9});
//    elementsToChooseFrom.push_back({_plainGun,1,1,0,9});
    //elementsToChooseFrom.push_back({_collectible,0,1,0,9});
    elementsToChooseFrom.push_back({_bunker,0,1,0,9});
    elementsToChooseFrom.push_back({_teleporter,0,1,0,9});
    elementsToChooseFrom.push_back({_teleporter,1,1,0,9});

    //first find area for the player and stuff for it
    elementCollection.push_back({_player,0,2,0,9});
    elementCollection.push_back({_key,0,2,0,9});
    elementCollection.push_back({_plainGun,0,1,0,9});
    int demandedSurface=0;
    for(int cnt=0; cnt<elementCollection.size(); cnt++) demandedSurface+=elementCollection[cnt].surface*(elementCollection[cnt].number);
    chamberArea::foundAreas.clear();
    this->headNode->findChambersCloseToSurface(demandedSurface);
    if (chamberArea::foundAreas.size()==0)
    {
        std::cout<<"Found areas is empty!\n";
        return false;
    }
    int selectedChamberNo=(this->gen()%chamberArea::foundAreas.size());
    this->placeElementCollection(chamberArea::foundAreas[selectedChamberNo],&elementCollection);
    this->placeDoors({_door,0,1,0,9},chamberArea::foundAreas[selectedChamberNo]);
    if (chamberArea::foundAreas[selectedChamberNo]->parent!=NULL)
    {
        chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock=true;
    }

    delete chamberArea::foundAreas[selectedChamberNo];
    chamberArea::foundAreas[selectedChamberNo]=chamberArea::foundAreas[chamberArea::foundAreas.size()-1];
    chamberArea::foundAreas.pop_back();

    while(this->headNode->surface>availableSurface/20)
    {
        std::cout<<"Surface total: "<<this->headNode->surface<<"\n";
        elementToPlace _nel;
        int demandedSurface=0;

        int cnt;
        elementCollection.clear();
        for(cnt=0; cnt<5; cnt++)
        {
            _nel=elementsToChooseFrom[this->gen()%elementsToChooseFrom.size()];
            elementCollection.push_back(_nel);

        }
        for(int cnt=0; cnt<elementCollection.size(); cnt++) demandedSurface+=elementCollection[cnt].surface*(elementCollection[cnt].number);
        if(chamberArea::foundAreas.size()>0)
        {
            int selectedChamberNo=(this->gen()%chamberArea::foundAreas.size());
            this->placeElementCollection(chamberArea::foundAreas[selectedChamberNo],&elementCollection);
            elementCollection.clear();

            if(chamberArea::foundAreas[selectedChamberNo]->parent!=NULL)
            {
                if(chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock==false)
                {
                    int dice=this->gen()%100;
                    int keyType=this->gen()%5;
                    if(dice<(50/holes))
                    {
                        this->placeDoors({_door,keyType,1,0,9},chamberArea::foundAreas[selectedChamberNo]);
                        elementCollection.push_back({_key,keyType,2,0,9}); // place key for the door and store it somewhere - warning, the door placed at the end might never receive the key
                        chamberArea::foundAreas[selectedChamberNo]->parent->childrenLock=true;

                    }

                }



            }

            delete chamberArea::foundAreas[selectedChamberNo];
            chamberArea::foundAreas[selectedChamberNo]=chamberArea::foundAreas[chamberArea::foundAreas.size()-1];
            chamberArea::foundAreas.pop_back();
        }
        else
        {
            this->headNode->findChambersCloseToSurface(demandedSurface);
            if(chamberArea::foundAreas.size()==0)
                break;
        }


    }





    return true;

}

/*bool randomLevelGenerator::addSpaceToCreate(spaceToCreate spc)
{
    this->spacesToCreate.push_back(spc);
}
*/




bool randomLevelGenerator::placeDoors(elementToPlace element,chamberArea* location)
{
    /*
    Place doors at the location

    */
    std::cout<<"door "<<element.eSubType<<"\n";

    //Ok, now we need to place the door.
    for(int c1=location->upLeft.x-1; c1<=location->downRight.x+1; c1++)
    {
        if (this->mychamber->chamberArray[c1][location->upLeft.y-1]->isSteppable())
        {
            bElem* neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->chamberArray[c1][location->upLeft.y-1]);
        }
        if (this->mychamber->chamberArray[c1][location->downRight.y+1]->isSteppable())
        {
            bElem* neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->chamberArray[c1][location->downRight.y+1]);
        }
    }
    for (int c2=location->upLeft.y; c2<=location->downRight.y; c2++)
    {
        if (this->mychamber->chamberArray[location->upLeft.x-1][c2]->isSteppable())
        {
            bElem* neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->chamberArray[location->upLeft.x-1][c2]);
        }
        if (this->mychamber->chamberArray[location->downRight.x+1][c2]->isSteppable())
        {
            bElem* neEl=this->createElement(element);
            neEl->stepOnElement(this->mychamber->chamberArray[location->downRight.x+1][c2]);
        }

    }


    return true;
}


bElem* randomLevelGenerator::createElement(elementToPlace element)
{
    switch(element.eType)
    {
    case _collectible:
        return new collectible(this->mychamber);
        break;
    case _player:
        return new player(this->mychamber);
        break;
    case _door:
        return new door(this->mychamber,element.eSubType);
        break;
    case _key:
        return new key(this->mychamber,element.eSubType);
        break;
    case _monster:
        return new monster(this->mychamber);
        break;
    case _plainGun:
        return new plainGun(this->mychamber);
        break;
    case _bunker:
        return new bunker(this->mychamber);
        break;
    case _teleporter:
        return new teleport(this->mychamber);
    case _movableType:
        return new movableElements(this->mychamber);
    }
    return NULL;
}



