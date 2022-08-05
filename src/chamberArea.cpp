#include "chamberArea.h"


std::vector<chamberArea*> chamberArea::foundAreas;
std::vector<bElem*> chamberArea::foundElements;

chamberArea::~chamberArea() // if we remove the node, we remove all its children
{
    chamberArea* _par=NULL;
    chamberArea* _originalParent=this->parent;
//    std::cout<<"parent "<<this->parent<<"\n";
    if(this->parent!=NULL)
    {
        /*
        the node has a parent? deal with it,
         remove this node from the children list,
         recalculate surface in the parent node and its parent nodes -
         this way we would not have to recalculate the surface.
         We have to remember, we remove the whole nodes. Still this will not deliver any dexterity in
         placing the objects
         */

        for(int cnt=0; cnt<this->parent->children.size();)  //find and delete me from children list
        {
            if (this->parent->children[cnt]==this)
            {
                this->parent->children.erase(this->parent->children.begin()+cnt);
            }
            else
            {
                cnt++;
            }

        }
        _par=this->parent; //iterate over all parents and readjust the surface
        while(_par!=NULL)
        {
            int _surf=0;
            for(int c=0; c<_par->children.size(); c++)
            {
                _surf+=_par->children[c]->surface;
            }
            _par->surface=_surf;
            _par=_par->parent;
        }
        //  this->parent=NULL; // this will not get in a way
        if (_originalParent->children.size()==0) //this was a last child? Remove parent node, as it is full.
            delete _originalParent;
    }

    for(int cnt=0; cnt<this->children.size(); cnt++)
    {
        this->children[cnt]->parent=NULL;
        delete this->children[cnt];
    }
    //this->children.clear();
}
chamberArea::chamberArea(int xu, int yu, int xd, int yd)
{
    this->upLeft.x=xu;
    this->upLeft.y=yu;
    this->downRight.x=xd;
    this->downRight.y=yd;
    this->parent=NULL;
    this->childrenLock=false;

}

bool chamberArea::addChildNode(chamberArea* child)
{
    this->children.push_back(child);
    return true;
}



// We calculate the nodes area, if it has no children, we assume it is the chamber without walls;
int chamberArea::calculateInitialSurface()
{
    int s=0;
    if(this->children.size()>0)
    {
        for(int cnt=0; cnt<this->children.size(); cnt++)
        {
            s=s+this->children[cnt]->calculateInitialSurface();
        }


    }
    else
    {
        int dx=this->downRight.x-this->upLeft.x;
        int dy=this->downRight.y-this->upLeft.y;
        s=dx*dy;
    }
    this->surface=s;
    return s;
}

//calculate surfaces in the children nodes
int chamberArea::calculateSurface(chamber* mychamber)
{
    int surface_=0;
    if(this->children.size()==0)
    {
        for(int x=this->upLeft.x; x<=this->downRight.x; x++)
        {
            for (int y=this->upLeft.y; y<=this->downRight.y; y++)
            {
                if (mychamber->getElement(x,y)->isSteppable() && this->checkIfElementIsFree(x,y,mychamber))
                {
                    surface_++;
                }
            }
        }
    }
    else
    {
        for(int cnt=0; cnt<this->children.size(); cnt++)
        {
            surface_=surface_+this->children[cnt]->calculateSurface(mychamber);
        }

    }
    this->surface=surface_;
    return surface_;
}

void chamberArea::findElementsRec(chamber* mychamber)
{
    if (this->children.size()==0)
    {
        for(int x=this->upLeft.x; x<=this->downRight.x; x++)
        {
            for (int y=this->upLeft.y; y<=this->downRight.y; y++)
            {
                if (mychamber->getElement(x,y)->isSteppable() && this->checkIfElementIsFree(x,y,mychamber))
                {
                    chamberArea::foundElements.push_back(mychamber->getElement(x,y));
                }
            }
        }



    }
    else
    {
        for(int cnt=0; cnt<this->children.size(); cnt++)
        {
            this->children[cnt]->findElementsRec(mychamber);
        }
    }

}

bool chamberArea::findElementsToStepOn(chamber* myChamber)
{
    chamberArea::foundElements.clear();
    this->findElementsRec(myChamber);
    return chamberArea::foundElements.size()>0;

}



void chamberArea::findChambersCloseToSurface(int s)
{
    bool last=this->children.size()==0;
    std::cout<<"ch size "<<this->children.size()<<"\n";
    if (this->surface>s && this->children.size()>0)
    {
        last=true;
        for(int cnt=0; cnt<this->children.size(); cnt++)
        {
      //      std::cout<<s<<": Demanded surface "<<"Examining child: "<<cnt<<" size "<<this->children[cnt]->children.size()<<" child surface "<<this->children[cnt]->surface<<" mine:"<<this->surface<<"\n";
            if (this->children[cnt]->surface>=s)
            {
        //        std::cout<<"reach next level child: "<<cnt<<"\n";
                this->children[cnt]->findChambersCloseToSurface(s);
                last=false;
            }
        }
    }
    if (last==true)
    {
        if(this->surface<s*5)
        {
          //  std::cout<<"Adding area\n";
            chamberArea::foundAreas.push_back(this);
        }
    }
    return;
}

// We are checking the neighboorhood. We are doing it in kind of naive way, we assume, that if there are sequences steppable/not steppable of length 1 or 4, then we say, it is impossible to put an object there.
bool chamberArea::checkIfElementIsFree(int x, int y, chamber* mychamber)
{
    sNeighboorhood neigh=mychamber->getElement(x,y)->getSteppableNeighboorhood();
    int unstep=0;
    int lastStep=neigh.nTypes[7]; //last element
    int gap=0;
    for (int c=0; c<8; c++)
    {
        if(lastStep!=neigh.nTypes[c])
        {
            if(gap==1)
            {
                return false;
            }
            gap=0;
        }
        else
        {
            gap++;
        }
    }
    if(gap==1 || gap==4)
    {
        return false;
    }

    return true;


}


