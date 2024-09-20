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

#include "puppetMasterFR.h"
#include "viewPoint.h"





bool puppetMasterFR::collectOnAction(bool c, std::shared_ptr<bElem>who)
{
    bool r=bElem::collectOnAction(c,who);

    if(c && r && who && who->getType()==bElemTypes::_patrollingDrone )
    {
        if(who->getAttrs()->getInventory()->retrieveCollectibleFromInventory(this->getStats()->getInstanceId(),false))
        {
            return who->dropItem(this->getStats()->getInstanceId());
        } else if (this->getAttrs()->getSubtype() == 0) // if subtype not set, set one randomly
        {
            this->getAttrs()->setSubtype(this->randomNumberGenerator()%2);
            if(this->getAttrs()->getSubtype()==0)
                viewPoint::get_instance()->addViewPoint(who);
        }
        this->registerLiveElement(shared_from_this());
    } else if(this->getStats()->hasActivatedMechanics())
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    return true;
}


bool puppetMasterFR::mechanics()
{
    bool res = bElem::mechanics();

    std::shared_ptr<bElem> clc = this->getStats()->getCollector().lock();
    if (res && clc.get() != nullptr && clc->getType() == bElemTypes::_patrollingDrone && !clc->getStats()->isMoving() && !clc->getStats()->isWaiting())
    {
        switch (this->getAttrs()->getSubtype()) // here we will route all the mechanics, when we are in the monster
        {
        case 0:
            return this->mechanicsPatrollingDrone();
        case 1:
            if(!this->collectorMechanics())
                return this->mechanicsPatrollingDrone();
            return true;
        default:
            return false;
        }
    }
    return res;
}

bool puppetMasterFR::collectorMechanics()
{
    std::shared_ptr<bElem> _collector=this->getStats()->getCollector().lock();
    dir::direction _d=_collector->getStats()->getMyDirection();
    dir::direction d=_d;
    dir::direction _d1=(dir::direction)(((int)(_d)+1)%4);
    dir::direction _d2=(dir::direction)(((int)(_d)+3)%4);
    dir::direction _d3=(this->randomNumberGenerator()%2==0)?_d1:_d2;
    for(int c=0; c<4; c++)
    {
        d=(dir::direction)(((int)_d+c)%4);
        switch((int)d-(int)_d)
        {

        case 1:
        case -3:
            _d3=_d1;
            break;
        case -1:
        case 3:
            _d3=_d2;
            break;
        }
        std::shared_ptr<bElem> check=this->findObjectInDirection(d);
        if(check && check->getAttrs()->isCollectible() && check->getType()!=this->getType())
        {
            if(_collector->getStats()->getMyDirection()!=d)
            {
                _collector->getStats()->setMyDirection(_d3);
                _collector->getStats()->setFacing(_d3);
                this->getStats()->setWaiting(_mov_delay);
                return true;
            }
            return _collector->moveInDirection(d);
        }
    }

    return false;
}

std::shared_ptr<bElem> puppetMasterFR::findObjectInDirection(dir::direction dir)
{
    std::shared_ptr<bElem> b = this->getStats()->getCollector().lock();
    b=b->getElementInDirection(dir);
    while(b!=nullptr && b->getAttrs()->isSteppable())
    {
        b=b->getElementInDirection(dir);
    }
    return b;
}

bool puppetMasterFR::mechanicsPatrollingDrone()
{
    std::shared_ptr<bElem> collector = this->getStats()->getCollector().lock();
    dir::direction cdir = collector->getStats()->getMyDirection();
    dir::direction pdir1 = (dir::direction)((((int)cdir) + 1) % 4);
    dir::direction pdir2 = (dir::direction)((((int)cdir) + 3) % 4);
    bool b1 = false, b2 = false;
    if (collector->getElementInDirection(pdir1))
        b1 = collector->getElementInDirection(pdir1)->getAttrs()->isSteppable();
    if (collector->getElementInDirection(pdir2))
        b2 = collector->getElementInDirection(pdir2)->getAttrs()->isSteppable();
    int roulette=this->randomNumberGenerator() %555;
    if (b1 && roulette == 5) // same probablility for each
    {
        collector->getStats()->setMyDirection(pdir1);
        collector->getStats()->setFacing(pdir1);
        collector->getStats()->setWaiting(_mov_delay);
        this->getStats()->setWaiting(_mov_delay);
        return true;
    }
    else if (b2 && roulette==25)
    {
        collector->getStats()->setMyDirection(pdir1);
        collector->getStats()->setFacing(pdir1);
        collector->getStats()->setWaiting(_mov_delay);
        this->getStats()->setWaiting(_mov_delay);
        return true;
    }

    if(collector->moveInDirection(cdir))
    {
        collector->getStats()->setMyDirection(cdir);
        collector->getStats()->setFacing(cdir);
    }
    else
    {
        dir::direction ndir=(bElem::randomNumberGenerator()%2==0)?pdir2:pdir1;
        collector->getStats()->setMyDirection(ndir);
        collector->getStats()->setFacing(ndir);
    }
    collector->getStats()->setWaiting(_mov_delay);
    this->getStats()->setWaiting(_mov_delay);
    return true;
}

int puppetMasterFR::getType() const
{
    return bElemTypes::_puppetMasterType;
}













