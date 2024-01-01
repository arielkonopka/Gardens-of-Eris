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

#include "plainGun.h"
#include "viewPoint.h"


plainGun::plainGun(std::shared_ptr<chamber> board): plainGun()
{
    this->setBoard(board);
}

plainGun::plainGun(std::shared_ptr<chamber>  board, int newSubtype): plainGun(board)
{
}

std::shared_ptr<bElem> plainGun::createProjectible(std::shared_ptr<bElem> who)
{
    std::shared_ptr<bElem> pm=elementFactory::generateAnElement<plainMissile>(who->getBoard(),0);
    pm->getStats()->setStatsOwner(who);
    who->lockThisObject(pm);
    pm->getStats()->setMyDirection(who->getStats()->getFacing());
    pm->getStats()->setFacing(who->getStats()->getFacing());
    pm->stepOnElement(who->getElementInDirection(who->getStats()->getFacing()));
    pm->getAttrs()->setEnergy(this->getAttrs()->getEnergy());
    if(who->getType()==_player)
        viewPoint::get_instance()->setOwner(pm);
    return pm;
}

plainGun::plainGun():bElem()
{

}
bool plainGun::additionalProvisioning(int subtype, std::shared_ptr<plainGun>sbe)
{
    if(!bElem::additionalProvisioning(subtype,sbe))
        return false;
    this->registerLiveElement(shared_from_this());
    return true;
}



int plainGun::getType() const
{
    return _plainGun;
}



bool plainGun::use(std::shared_ptr<bElem> who)
{
    std::shared_ptr<bElem> myel;
#ifdef _VerbousMode_
    if (who==nullptr)
    {
        std::cout<<"Who is nullptr for plain gun!";
        return false;
    }
  ma
#endif
   if (this->getStats()->isWaiting() || (this->getAttrs()->getAmmo()<=0 && (this->getAttrs()->getSubtype()%2)==0))
        return false;
    this->getStats()->setWaiting(_plainGunCharge);
    if(!this->getStats()->isCollected())
        who=shared_from_this();
    myel=who->getElementInDirection(who->getStats()->getFacing());
    if(myel!=nullptr)
    {
        if (this->getAttrs()->getAmmo()>0 || this->getAttrs()->getSubtype()%2==1)
        {

            coords3d c3d;
            c3d.x=who->getStats()->getMyPosition().x*32+who->getOffset().x;
            c3d.z=who->getStats()->getMyPosition().y*32+who->getOffset().y;
            c3d.y=50;
            coords3d vel= {who->getOffset().x,0,who->getOffset().y};
            soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,this->getStats()->getInstanceId(),this->getType(),this->getAttrs()->getSubtype(),"Shoot","Shoot");
            if (myel->getAttrs()->isSteppable()==true)
            {
                this->createProjectible(who);
            }
            else if (myel->getAttrs()->isKillable() )
            {
                myel->hurt(this->getAttrs()->getEnergy());
            }
            if (this->getAttrs()->getSubtype()%2==0)
            {
                this->getAttrs()->setAmmo(this->getAttrs()->getAmmo()-1);
                this->getAttrs()->setEnergy(this->getAttrs()->getEnergy()-(this->getAttrs()->getEnergy()*0.2));

            }
        }

    }
    return true;
}



bool plainGun::mechanics()
{
    bool res=bElem::mechanics();
    if(this->getAttrs()->getEnergy()<this->maxEnergy)
    {
        if (bElem::getCntr()%5==0)
            this->getAttrs()->setEnergy(this->getAttrs()->getEnergy()+1);
    }
    return res;
}
