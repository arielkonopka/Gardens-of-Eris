#include "randomLevelGenerator.h"

randomLevelGenerator::randomLevelGenerator(int w, int h,gCollect *gbin)
{
    this->width=w;
    this->height=h;
    this->mychamber=new chamber(w,h,gbin);
    this->garbageCollector=gbin;
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
    //ctor
}

randomLevelGenerator::~randomLevelGenerator()
{
    //dtor
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
        this->mychamber->chamberArray[x][y]=new wall(this->mychamber,this->garbageCollector);
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

/*
algorytm rozsiewania:
do lvlGenerate doklejamy stringa, na początku może mieć znak "B" - begin
    potem kolejno doklejamy znaki:
    a,b,
    c,d - odpowiednio dla ćwiartek w których chcemy umieścić przedmiot
    w ten sposób uzyskujemy współrzędne komory, na której kończy się rekurencja.
    np:
    Baaaabac - oznacza początek, ćwiartkę lewą górną, LG, LG, LG, PG, LG, LD
    przy każdym zakońćzeniu, gdy chcemy przestać wywoływać kolejne rekurencję, szukamy sobie ile jest takich elementów, które mają współrzędne takie, jak
    zaczynają się nasze współrzędne, np. w naszym przypadku: collectible, Baaa, oznacza, że przedmiot będzie potencjalnym kandydatem do umieszczenia.
    liczymy ile takich przedmiotów jest w tabeli do umieszczenia.
    następnie szukamy takich, których współrzędne są najdłuższe.
    Obliczamy różnicę rzeczywistych współrzędnych i tych z listy
    w naszym przypadku:
    len(Baaa)=4
    len(Baaaabac)=9
    czyli różnica jest 5, ćwiartek mamy 4, więc 5 *4 daje 20 (n) komnat w których można umieścić przedmiot.
    obliczamy prawdopodobieństwo:
    (ilosc_przedmiotów/n)
    jeśli prawdopodobieństwo jest większe>1 to
    dodajemy p przedmiotów do komnaty (usuwamy z kolejki i dodajemy do komnaty) i jeśli zostaje ułamek, robimy dalej normalnie, tj. prawdopodobieństwo mnożymy *100
    losujemy liczbę z przedziału 0..100, jeśli jest >niż 100*p dodajemy przedmiot do planszy w komnacie
    przeliczamy prawdopodobienstwo


*/


int randomLevelGenerator::lvlGenerate(int x1, int y1, int x2, int y2,int depth,int holes,std::string loc)
{

    std::vector<int> doorPlaces1,doorPlaces2;
    int Hmin_=((y2-y1)/(1+loc.size()));
    int Wmin_=((x2-x1)/(1+loc.size()));
    int blind=4; // this->gen()%4;
    if(Hmin_<Hmin)
        Hmin_=Hmin;
    if(Wmin_<Wmin)
        Wmin_=Wmin;

    int c=-1,dc1=-1,dc2=-1;
    int d=-1,dd1=-1,dd2=-1;
    //  bElem *belem;
    int dx,dy;
    if (depth<0)
        return 0;
    this->endChambers.push_back({x1,y1,x2,y2,loc});
#ifdef _debugRandomGenerator
    std::cout<<"x1,y1 "<<x1<<","<<y1<<" x2,y2: "<<x2<<","<<y2<<" loc:"<<loc<<"\n";
#endif
    dx=x2-x1;
    dy=y2-y1;
    if (dx>Wmin_) // tutaj trzeba zrobić ograniczenie
        // Im krótsza ścieżka, tym wmin bliższe 1/3 planszy
    {

        c=(this->gen() % (dx-Wmin_)) +x1+(Wmin_/2);
        dc1=c-x1;
        dc2=x2-c+2;

    }
    //else return 0;
    if (dy>Hmin_)
    {
        d=(this->gen()%(dy-Hmin_)) +y1+(Hmin_/2);
        dd1=d-y1;
        dd2=y2-d+2;
    }
    //else return 0;
    if (dc1>Wmin && dd1>Hmin)
    {
        this->lvlGenerate(x1,y1,c,d,depth-1,holes,loc+"a");
    }


    if (dc2>Wmin && dd1>Hmin)
    {
        this->lvlGenerate(c+2,y1,x2,d,depth-1,holes,loc+"b");
    }

    if (dd2>Hmin && dc1>Wmin)
    {
        this->lvlGenerate(x1,d+2,c,y2,depth-1,holes,loc+"d");
    }

    if (dd2>Hmin && dc2>Wmin)
    {
        this->lvlGenerate(c+2,d+2,x2,y2,depth-1,holes,loc+"c");
    }


    doorPlaces1.clear();
    doorPlaces2.clear();
    if (c>x1 && c<x2 && d>0)
    {

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

                bElem *newElement=new wall(this->mychamber,this->garbageCollector);
                newElement->stepOnElement(this->mychamber->chamberArray[c+1][a]);
            }
            else
            {
                break;
            }
        }
#ifdef _debugRandomGenerator
        std::cout<<" * "<<doorPlaces1.size()<<" "<<doorPlaces2.size()<<" "<<loc<<" "<<"c,d: "<<c<<","<<d<<"\n";
        int hits=0,hits1=0;
#endif
        for (int cnt=0; cnt<holes; cnt++)
        {
            if (blind!=0 && doorPlaces1.size()>0)
            {
                int rnd=this->gen()%(doorPlaces1.size());
                this->mychamber->chamberArray[c+1][doorPlaces1[rnd]]->disposeElement();
                doorPlaces1[rnd]=doorPlaces1[doorPlaces1.size()-1];
                doorPlaces1.pop_back();
#ifdef _debugRandomGenerator
                hits++;
#endif
            }

            if (blind!=1 && doorPlaces2.size()>0)
            {

                int rnd=this->gen()%(doorPlaces2.size());
                this->mychamber->chamberArray[c+1][doorPlaces2[rnd]]->disposeElement();
                //if(rnd<doorPlaces2.size()-1)
                doorPlaces2[rnd]=doorPlaces2[doorPlaces2.size()-1];
                doorPlaces2.pop_back();
#ifdef _debugRandomGenerator
                hits1++;
#endif
            }
        }
#ifdef _debugRandomGenerator
        if(hits==0 || hits1==0)
        {
            std::cout<<"Zero hits! "<<hits<<" "<<hits1<<" "<<loc<<"\n";
        }
#endif

    }


    if (d>y1 && d<y2 && c>=0)
    {
#ifdef _debugRandomGenerator
        int hits=0,hits1=0;
#endif
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
                bElem *newElement=new wall(this->mychamber,this->garbageCollector);
                newElement->stepOnElement(this->mychamber->chamberArray[a][d+1]);
            }
        }
        for (int cnt=0; cnt<holes; cnt++)
        {
            if (blind!=2 && doorPlaces1.size()>0)
            {
                int rnd=this->gen()%(doorPlaces1.size());
                this->mychamber->chamberArray[doorPlaces1[rnd]][d+1]->disposeElement();
                doorPlaces1[rnd]=doorPlaces1[doorPlaces1.size()-1];
                doorPlaces1.pop_back();
#ifdef _debugRandomGenerator
                hits++;
#endif

            }
        }
        for (int cnt=0; cnt<holes; cnt++)
        {

            if (blind!=3 && doorPlaces2.size()>0)
            {
                int rnd=this->gen()%(doorPlaces2.size());
                this->mychamber->chamberArray[doorPlaces2[rnd]][d+1]->disposeElement();
                doorPlaces2[rnd]=doorPlaces2[doorPlaces2.size()-1];
                doorPlaces2.pop_back();
#ifdef _debugRandomGenerator
                hits1++;
#endif

            }
        }
#ifdef _debugRandomGenerator
        if(hits==0 || hits1==0)
        {
            std::cout<<"Low hits! "<<hits<<" "<<hits1<<" "<<loc<<"\n";
        }
#endif
    }
    return 0;
}



void randomLevelGenerator::addElementToPlace(elementToPlace element)
{
    this->elementsToPlace.push_back(element);
}

bool randomLevelGenerator::qualifies(std::string itemLoc, std::string chamLoc)
{
    if (chamLoc.size()<itemLoc.size())
        return false;
    for(int c=0; c<itemLoc.size(); c++)
    {
        if (itemLoc.at(c)!=chamLoc.at(c))
            return false;
    }
    return true;
}




bool randomLevelGenerator::generateLevel(int holes)
{
    this->lvlGenerate(1,1,this->width-2,this->height-2,_iterations,holes,"B");
    for(int c=0; c<this->width; c++)
    {
        bElem *newElem=new wall(this->mychamber,this->garbageCollector);
        bElem *newElem1=new wall(this->mychamber,this->garbageCollector);
        newElem->stepOnElement(this->mychamber->chamberArray[c][0]);
        newElem1->stepOnElement(this->mychamber->chamberArray[c][this->height-1]);

    }
    for(int c=0; c<this->height; c++)
    {

        bElem *newElem2=new wall(this->mychamber,this->garbageCollector);
        bElem *newElem3=new wall(this->mychamber,this->garbageCollector);
        newElem2->stepOnElement(this->mychamber->chamberArray[0][c]);
        newElem3->stepOnElement(this->mychamber->chamberArray[this->width-1][c]);
    }
    for (int c=0; c<this->elementsToPlace.size(); c++)
    {

        if (this->elementsToPlace[c].eType==_door)
            this->placeDoors(this->elementsToPlace[c]);
    }
    for(int c=0; c<this->elementsToPlace.size(); c++)
    {
        if(this->elementsToPlace[c].eType!=_door)
            this->placeElement(this->elementsToPlace[c]);
    }


    return true;

}
int randomLevelGenerator::findSpotsToChoose(elementToPlace element)
{
    this->spotsToChoose.clear();
    int surface=0;

    for(int c=0; c<this->endChambers.size(); c++)
    {
//        std::cout<<" T### "<<this->endChambers[c].location<<"\n";
        if (element.location==this->endChambers[c].location)
        {

//            std::cout<<" Q### "<<this->endChambers[c].location<<"\n";
            for(int x=this->endChambers[c].x0; x<=this->endChambers[c].x1; x++)
            {
                for (int y=this->endChambers[c].y0; y<=this->endChambers[c].y1; y++)
                {
                    if (this->mychamber->chamberArray[x][y]->isSteppable())
                    {
                        this->spotsToChoose.push_back({x,y});
                        surface++;
                    }
                }
            }
        }
    }
    return surface;
}
bool randomLevelGenerator::placeDoors(elementToPlace element)
{
    coords p0=NOCOORDS,p1=NOCOORDS;
    for(int c=0; c<this->endChambers.size(); c++)
    {
        if (element.location==this->endChambers[c].location)
        {
            //Ok, now we need to place the door.
            for(int c1=this->endChambers[c].x0-1; c1<=this->endChambers[c].x1+1; c1++)
            {
                if (this->mychamber->chamberArray[c1][this->endChambers[c].y0-1]->isSteppable())
                {
                    bElem* neEl=this->createElement(element);
                    neEl->stepOnElement(this->mychamber->chamberArray[c1][this->endChambers[c].y0-1]);
                }
                if (this->mychamber->chamberArray[c1][this->endChambers[c].y1+1]->isSteppable())
                {
                    bElem* neEl=this->createElement(element);
                    neEl->stepOnElement(this->mychamber->chamberArray[c1][this->endChambers[c].y1+1]);
                }
            }
            for (int c2=this->endChambers[c].y0; c2<=this->endChambers[c].y1; c2++)
            {
                if (this->mychamber->chamberArray[this->endChambers[c].x0-1][c2]->isSteppable())
                {
                    bElem* neEl=this->createElement(element);
                    neEl->stepOnElement(this->mychamber->chamberArray[this->endChambers[c].x0-1][c2]);
                }
                if (this->mychamber->chamberArray[this->endChambers[c].x1+1][c2]->isSteppable())
                {
                    bElem* neEl=this->createElement(element);
                    neEl->stepOnElement(this->mychamber->chamberArray[this->endChambers[c].x1+1][c2]);
                }

            }

            break;
        }
    }
}

bool randomLevelGenerator::placeElement(elementToPlace element)
{
    int rnd=0;
    int surface=0;
    std::vector<rectangle> candidates;

    surface=this->findSpotsToChoose(element);

    if (element.eType==_door)
    {
        return this->placeDoors(element);
    }

    for(; element.number>0; element.number--)
    {

        if(this->spotsToChoose.size()<=0)
        {
            break; // no more spots to place anything
        }


        rnd=this->gen()%this->spotsToChoose.size();
        coords tmp;
        bElem *newElem=this->createElement(element);
        if (newElem==NULL)
        {
            return false; // we failed to place elements
        }
        newElem->stepOnElement(this->mychamber->chamberArray[this->spotsToChoose[rnd].x][this->spotsToChoose[rnd].y]);
        if (newElem->selfAlign()==true)
        {
            surface=this->findSpotsToChoose(element);
        }
        else
        {
            spotsToChoose[rnd]=spotsToChoose[spotsToChoose.size()-1];
            spotsToChoose.pop_back();
        }
    }

    if(element.number>0)
        return false; // we failed to place all the elements

    return true;
}
bElem* randomLevelGenerator::createElement(elementToPlace element)
{
    switch(element.eType)
    {
    case _collectible:
        return new collectible(this->mychamber,this->garbageCollector);
        break;
    case _player:
        return new player(this->mychamber,this->garbageCollector);
        break;
    case _door:
        return new door(this->mychamber,this->garbageCollector,element.eSubType);
        break;
    case _key:
        return new key(this->mychamber,this->garbageCollector,element.eSubType);
        break;
    }
    return NULL;
}



