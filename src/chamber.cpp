#include "chamber.h"


int chamber::lastid=0;

chamber::chamber(int x,int y):width(x),height(y)
{
    randomWordGen *rwg=new randomWordGen();
    this->chamberArray=new bElem**[x];
    for(int c=0;c<x;c++)
        this->chamberArray[c]=new bElem*[y];
    this->garbageBin=gCollect::getInstance();
    this->setInstanceId(chamber::lastid++);
    for(int cX=0; cX<x; cX++)
    {
        for (int cY=0; cY<y; cY++)
        {
            bElem* neEl=new bElem(this); // With chambers it is different. The stepOnElement cannot work on nullptrs.
            this->setElement(cX,cY,neEl);
            if(bElem::randomNumberGenerator()%10==0)
                neEl->setSubtype(1);
            if(bElem::randomNumberGenerator()%100==0)
                neEl->setSubtype(2);
            neEl->setCoords(cX,cY);

        }
    }
    this->chamberName=rwg->generateWord(3);
    this->chamberColour.a=255;
    this->chamberColour.r=30+rwg->randomNumberGenerator()%50;
    this->chamberColour.g=30+rwg->randomNumberGenerator()%50;
    this->chamberColour.b=50+rwg->randomNumberGenerator()%70;
    delete rwg;



}

colour chamber::getChColour()
{
    return this->chamberColour;
}


chamber::~chamber()
{
    for (int cX=0; cX<this->width; cX++)
    {
        for (int cY=0; cY<this->height; cY++)
        {
            while(this->chamberArray[cX][cY]->disposeElementUnsafe()==DISPOSED);
        }
        this->garbageBin->purgeGarbage();

    }
}


std::string chamber::getName()
{
    return this->chamberName;
}


bElem* chamber::getElement(coords point)
{
    return this->getElement(point.x,point.y);
}


bElem* chamber::getElement(int x, int y)
{
    if (x<0 || x>this->width-1 || y<0 || y>this->height-1)
        return nullptr;
    return this->chamberArray[x][y];
}


void chamber::setElement(int x, int y, bElem* elem)
{
    if (x<0 || x>this->width-1 || y<0 || y>this->height-1)
        return;
    this->chamberArray[x][y]=elem;
}


void chamber::setElement(coords point, bElem* elem)
{
    this->setElement(point.x,point.y,elem);
}


int chamber::getInstanceId()
{
    return this->instanceid;
}

void chamber::setInstanceId(int id)
{
    this->instanceid=id;
}



