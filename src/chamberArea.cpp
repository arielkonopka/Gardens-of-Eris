#include "chamberArea.h"


std::vector<chamberArea*> chamberArea::foundAreas;
std::vector<bElem*> chamberArea::foundElements;

chamberArea::~chamberArea() // if we remove the node, we remove all its children
{
    chamberArea* _par=NULL;
    chamberArea* _originalParent=this->parent;
    this->parent=NULL;
    for(long int cnt=0; cnt<(long int)this->children.size(); cnt++)
    {
#ifdef _VerbousMode_
    std::cout<<"Destroying child"<<cnt<<"\n";
#endif // _VerbousMode_
        chamberArea* node=this->children[cnt];
        node->parent=NULL;
        delete node;
    }
    this->children.clear();
    if(_originalParent!=NULL)
    {
        /*
        the node has a parent? deal with it,
         remove this node from the children list,
         recalculate surface in the parent node and its parent nodes -
         this way we would not have to recalculate the surface.
         We have to remember, we remove the whole nodes. Still this will not deliver any dexterity in
         placing the objects
         */

        for(long int cnt=0; cnt<(long int)_originalParent->children.size();)  //find and delete me from children list
        {
            if (_originalParent->children[cnt]->upLeft==this->upLeft && _originalParent->children[cnt]->downRight==this->downRight)
            {
#ifdef _VerbousMode_
                   std::cout<<"Removed from the children list"<<cnt<<"\n";
#endif
                _originalParent->children.erase(_originalParent->children.begin()+cnt);

            }
            else
            {
                cnt++;
            }

        }
        _par=_originalParent; //iterate over all parents and readjust the surface
        while(_par!=NULL)
        {
            int _surf=0;
            for(unsigned int c=0; c<_par->children.size(); c++)
            {
                _surf+=_par->children[c]->surface;
            }
            _par->surface=_surf;
            _par=_par->parent;
        }
        //    if (_originalParent->children.size()==0) //this was a last child? Remove parent node, as it is full.
        //        delete _originalParent;
    }
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
    child->parent=this;
    return true;
}



// We calculate the nodes area, if it has no children, we assume it is the chamber without walls;
int chamberArea::calculateInitialSurface()
{
    int s=0;
    if(this->children.size()>0)
    {
        for(unsigned int cnt=0; cnt<this->children.size(); cnt++)
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
        for(unsigned int cnt=0; cnt<this->children.size(); cnt++)
        {
            surface_+=this->children[cnt]->calculateSurface(mychamber);
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
                    //       std::cout<<".";
                }
            }
        }



    }
    else
    {
        for(unsigned int cnt=0; cnt<this->children.size(); cnt++)
        {
            this->children[cnt]->findElementsRec(mychamber);
        }
    }

}

bool chamberArea::findElementsToStepOn(chamber* myChamber)
{
//   std::cout<<"FindElementsToStepOn\n[";
    chamberArea::foundElements.clear();
    this->findElementsRec(myChamber);
    // std::cout<<"]\n-----------------\n";
    return chamberArea::foundElements.size()>0;

}



void chamberArea::findChambersCloseToSurface(int s,int tolerance)
{
    bool last=true;
    if(this->surface>=s && this->children.size()>0)
        for(unsigned int cnt=0; cnt<this->children.size(); cnt++)
        {
            if (this->children[cnt]->surface>s)
            {
                this->children[cnt]->findChambersCloseToSurface(s,tolerance);
                last=false;
            }
        }

    if (last==true)
    {
        if(this->surface>=s && (this->surface<=s+((s*tolerance)/100)))
        {
            chamberArea::foundAreas.push_back(this);
        }
    }
    return;
}

// We are checking the neighboorhood. We are doing it in kind of naive way, we assume,
//that if there are sequences steppable/not steppable of length 1, then we say, it is impossible to put an object there.
bool chamberArea::checkIfElementIsFree(int x, int y, chamber* mychamber)
{
    sNeighboorhood neigh=mychamber->getElement(x,y)->getSteppableNeighboorhood();
    if(neigh.nTypes[7]==_door)
        neigh.steppable[7]=true;
    bool lastStep=neigh.steppable[7]; //last element

    int gap=0;
    for (int c=0; c<8; c++)
    {
        if(neigh.nTypes[c%8]==_door)
            neigh.steppable[c%8]=true;
        if(lastStep!=neigh.steppable[c%8])
        {
            if(gap<2)
            {
                return false;
            }
            lastStep=neigh.steppable[c%8];
            gap=0;
        }
        else
        {
            gap++;
        }
    }
    return true;
}

// This should be run as a correct after deleting a node from the tree, we do not need nodes without a surface
void chamberArea::removeEmptyNodes()
{
    for(long int c=0; c<(long int)this->children.size(); c++)
    {
        this->children[c]->removeEmptyNodes();
    }
    for(int c=0; c<(int)this->children.size(); )
    {
        chamberArea* node=this->children[c];
        if(node->surface==0)
        {
            this->children.erase(this->children.begin()+c);
            delete node;
        } else
        {
            c++;
        }
    }


}

