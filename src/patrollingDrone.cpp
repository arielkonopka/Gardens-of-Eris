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
#include "patrollingDrone.h"


patrollingDrone::patrollingDrone(std::shared_ptr<chamber> board) : patrollingDrone()
{
    this->setBoard(board);
}


patrollingDrone::patrollingDrone() : bElem()
{

    //  this->getAttrs()->setSubtype(0);
    // this->setInventory(std::make_shared<inventory>());

}


bool patrollingDrone::additionalProvisioning(int subtype, std::shared_ptr<patrollingDrone>sbe)
{
     if(!bElem::additionalProvisioning(subtype,sbe))
        return false;
    this->getAttrs()->setCollect(true);
    this->getAttrs()->setEnergy((1024*bElem::randomNumberGenerator())%155);
    return true;
}
float patrollingDrone::getViewRadius() const
{
    return 5.0;
}


/*
    We will request brain on interaction
*/
bool patrollingDrone::interact(std::shared_ptr<bElem> who)
{
    bool res = bElem::interact(who);
    if (res && !this->brained && this->getAttrs()->getSubtype() == 0 && who->getAttrs()->canCollect())
    {
        std::shared_ptr<bElem> token = who->getAttrs()->getInventory()->requestToken(_puppetMasterType, -1,true);
        if (token)
        {
            this->playSound("Boot", "Success");
            this->brained = true;
            this->brainModule = token;
            token->getStats()->setCollector(shared_from_this());
            token->collectOnAction(true,shared_from_this()); // since we collect the object ourselves, we should also trigger the action
            token->getStats()->setWaiting(55);
            if(who->getType()==_player)
                viewPoint::get_instance()->setOwner(shared_from_this());
            return true;
        }
        this->playSound("Boot", "Failure");
    }
    return false;
}

int patrollingDrone::getType() const
{
    return _patrollingDrone;
}


