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
#include "../include/bElem.h"
#include "elementSound.h"
#include "elements.h"
#include "floorElement.h"
#include "rubbish.h"
#include <algorithm>
#include <unordered_set>
std::vector<std::shared_ptr<bElem>> bElem::toDispose;

bElem::bElem()
    : std::enable_shared_from_this<bElem>()
{
    this->status = std::make_shared<bElemStats>();
    this->getStats()->setMyDirection(dir::direction::UP);
    this->getStats()->setFacing(this->getStats()->getMyDirection());
}

bool bElem::collectOnAction(bool collected, std::shared_ptr<bElem> who)
{
    if (collected && who && who->getType() == bElemTypes::_player) {
        goe::sound::play(*this, "Found", "Collect");
    }
    return true;
}

std::shared_ptr<chamber> bElem::getBoard() const
{
    return this->attachedBoard.lock();
}

void bElem::setBoard(std::shared_ptr<chamber> board)
{
    this->attachedBoard = board;
    if (this->getAttrs() && this->getAttrs()->canCollect()) {
        this->getAttrs()->getInventory()->updateBoard();
    }
}

bool bElem::dropItem(unsigned long int instanceId)
{
    if (!this->getAttrs()->canCollect())
        return false;

    std::shared_ptr<bElem> item
        = this->getAttrs()->getInventory()->retrieveCollectibleFromInventory(instanceId, true);
    if (!item)
        return false;
    item->collectOnAction(false, shared_from_this());
    for (auto d : dir::allDirections) {
        coords dir = dir::dirToCoords(d);

        if (this->isSteppableDirection(dir)) {
            if (this->getType() == bElemTypes::_player)
                goe::sound::play(*item, "Drop", "Item");
            item->stepOnElement(this->getElementInDirection(dir));
            return true;
        }
    }
    return this->collect(item); // re-collect, since there was no place to drop it.
}

/*
   This is the most basic method for dealing with all objects. Because all of them must be put to a place on a boart at some point,
   and this is the method that is to be used for that purpose.
   It should work on all types of configurations, except standing on an empty point
   If places on an object that already is covered by another one, the newly placed object is placed in between
   This method takes "steppable" flag into consideration
*/
bool bElem::stepOnElement(std::shared_ptr<bElem> step)
{
    auto elig = [](std::shared_ptr<bElem> step) -> bool {
        if (!step || !step->getAttrs()->isSteppable() || !step->getBoard()
            || step->getStats()->isDisposed() || step->getStats()->getMyPosition() == NOCOORDS)
            return false;
        else
            return true;
    };
    if (this->getStats()->isDisposed() || !elig(step))
        return false;
    bool chamberChange = !(step->getBoard() == this->getBoard());
    std::shared_ptr<bElem> s0;
    if (step->getAttrs()->isCollectible() && this->getAttrs()->canCollect()) {
        std::shared_ptr<bElem> s2 = step->getStats()->getSteppingOn();
        if (!elig(s2))
            return false;
        this->collect(step);
        step = s2;
        if (step) {
            return this->stepOnElement(step);
        } else {
            return false;
        }
    }
    std::shared_ptr<bElem> st = this->getStats()->getSteppingOn();
    if (chamberChange && this->getStats()->hasActivatedMechanics())
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    else
        chamberChange = false;
    this->removeElement();
    if (st)
        st->stepOnAction(false, shared_from_this());
    bool hp = step->getStats()->hasParent();
    this->setBoard(step->getBoard());
    this->getStats()->setMyPosition(step->getStats()->getMyPosition());
    this->getStats()->setSteppingOn(step);
    s0 = step->getStats()->getStandingOn().lock();
    step->getStats()->setStandingOn(shared_from_this());
    if (hp) {
        s0->getStats()->setSteppingOn(shared_from_this());
        this->getStats()->setStandingOn(s0);
    } else {
        this->getBoard()->setElement(this->getStats()->getMyPosition(), shared_from_this());
    }
    if (chamberChange)
        this->registerLiveElement(shared_from_this());
    step->stepOnAction(true, shared_from_this());
    return true;
}

oState bElem::disposeElement()
{
    std::shared_ptr<chamber> board = this->getBoard();
    const coords at = this->getStats()->getMyPosition();
    if (this->getStats()->isDisposed())
        return ERROR;
    // deregistering needs the board, so it goes before the element leaves it
    if (this->getStats()->hasActivatedMechanics())
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    this->removeElement();
    if (this->getAttrs() && this->getAttrs()->canCollect()) {
        if (this->dropsInventoryOnDeath())
            this->leaveStash(board, at);
        else
            this->getAttrs()->getInventory()->clear();
    }
    this->getStats()->setDisposed(true);
    this->setBoard(nullptr);
    this->getStats()->setMyPosition(NOCOORDS);
    soundManager::getInstance().stopSoundsByElementId(this->getStats()->getInstanceId());
    return DISPOSED;
}

bool bElem::dropsInventoryOnDeath() const
{
    return true;
}

// Whatever the element carried is left behind in a rubbish pile, on the cell it died on or, when
// that cell cannot take it, on a free neighbour. With no room anywhere the pile is burnt.
void bElem::leaveStash(const std::shared_ptr<chamber> &board, coords at)
{
    auto inv = this->getAttrs()->getInventory();
    if (!board || at == NOCOORDS || inv->isEmpty())
        return;
    auto stash = elementFactory::generateAnElement<rubbish>(board, 0);
    stash->getAttrs()->setCollect(true);
    stash->getAttrs()->getInventory()->mergeInventory(inv);
    auto here = board->getElement(at);
    if (stash->stepOnElement(here))
        return;
    for (auto d : dir::allDirections)
        if (here && here->isSteppableDirection(d)
            && stash->stepOnElement(here->getElementInDirection(d)))
            return;
    stash->disposeElement();
}

std::shared_ptr<bElem> bElem::getElementInDirection(dir::direction di)
{
    return this->getElementInDirection(dir::dirToCoords(di));
}
std::shared_ptr<bElem> bElem::getElementInDirection(coords di)
{
    // this runs for every neighbour check in the game, so the board and position are read once
    auto board = this->attachedBoard.lock();
    if (!board)
        return nullptr;
    coords pos = this->getStats()->getMyPosition();
    if (pos == NOCOORDS)
        return nullptr;
    coords crd = (pos + di).validate(board->getSize());
    if (crd == NOCOORDS)
        return nullptr;
    if (di == coords(0, 0))
        return shared_from_this();
    return board->getElement(crd);
}


bool bElem::use(std::shared_ptr<bElem> who)
{
    return false;
}

bool bElem::interact(std::shared_ptr<bElem> who)
{
    if (this->getAttrs()->isInteractive()
        && !this->getStats()->isInteracting()) /* penalty for getting into counter overflow */
    {
        this->getStats()->setInteracted(GoEConstants::_interactedTime);
        return true;
    }
    return false;
}

bool bElem::destroy()
{
    if (this->getAttrs()->isDestroyable() || this->getAttrs()->isSteppable()
        || this->getAttrs()->isKillable()) {
        if (this->getStats()->isDying()) {
            this->getStats()->setKilled(0);
            this->getStats()->setKillTimeBeg(0);
        }
        this->getStats()->setDestroyed(GoEConstants::_defaultDestroyTime);
        if (this->getAttrs()->isDestroyable() || this->getAttrs()->isKillable()) {
            bElem::toDispose.push_back(shared_from_this());
        }
        return true;
    }
    return false;
}

bool bElem::selfAlign()
{
    return false;
}

int bElem::getAnimPh() const
{
    int base = (int) bElem::getCntr();
    if (this->getStats()->isDying()) {
        base = (int) (bElem::getCntr() - this->getStats()->getKillTimeBeg());
    }
    if (this->getStats()->isDestroying()) {
        base = (int) (bElem::getCntr() - this->getStats()->getDestTimeBeg());
    }
    if (this->getStats()->isTeleporting()) {
        base = (int) (bElem::getCntr() - this->getStats()->getTelReqTime());
    }
    if (this->getStats()->isFadingIn()) {
        base = (int) (bElem::getCntr() - this->getStats()->getFadingInReq());
    }
    if (this->getStats()->isFadingOut()) {
        base = (int) (bElem::getCntr() - this->getStats()->getFadingOutReq());
    }
    return base >> 3;
}

float bElem::getViewRadius() const
{
    return 2.5;
}

bool bElem::hurt(int points)
{
    if (!this->getAttrs()->isKillable() || this->getStats()->isDying()
        || this->getStats()->isDestroying() || this->getStats()->isTeleporting()
        || this->getStats()->isDisposed())
        return false;
    this->getAttrs()->setEnergy(this->getAttrs()->getEnergy() - points);
    if (this->getAttrs()->getEnergy() <= 0)
        this->kill();
    return true;
}

bool bElem::mechanics()
{
    const bElemStats &st = *this->getStats();
    // expired() checks the board without taking a reference to it
    if ((this->attachedBoard.expired() || this->getStats()->getMyPosition() == NOCOORDS)
        && !st.isCollected())
        return false;
    return !(st.busy() || (this->getAttrs()->isInteractive() && st.isInteracting()));
}

bool bElem::isSteppableDirection(coords di) const
{
    auto board = this->attachedBoard.lock();
    if (!board)
        return false;
    coords pos = this->getStats()->getMyPosition();
    if (pos == NOCOORDS)
        return false;
    coords crd = (pos + di).validate(board->getSize());
    if (crd == NOCOORDS)
        return false;
    auto e = board->getElement(crd);
    return e && e->getAttrs()->isSteppable();
}
bool bElem::isSteppableDirection(dir::direction di) const
{
    return this->isSteppableDirection(dir::dirToCoords(di));
}

/**
 * @brief removes element from its board, or inventory
 *
 * This method check, if the instance is attached to a board or is in an inventory,
 * is so it will act accordingly, if collected, removed from the inventory.
 * If on board, removed, with respect to object stacking
 * @param none
 *
 * @note This method will create a floor element, if the last stacking element is removed from the board.
 */
std::shared_ptr<bElem> bElem::removeElement()
{
    coords _pos = this->getStats()->getMyPosition();
    std::shared_ptr<chamber> _chmbr = this->getBoard();
    if (this->getStats()->isDisposed())
        return nullptr;

    if (this->getStats()->isCollected()) {
        // the inventory may hold the last reference to us, so keep ourselves alive first
        std::shared_ptr<bElem> self = shared_from_this();
        std::shared_ptr<bElem> collector = this->getStats()->getCollector().lock();
        if (collector)
            collector->getAttrs()->getInventory()->removeCollectibleFromInventory(
                this->getStats()->getInstanceId());
        return self;
    }
    if (this->getStats()->getMyPosition() == NOCOORDS || !_chmbr) {
        return shared_from_this(); // it is not yet placed on a board.
    }

    if (this->getStats()->hasParent()) {
        std::shared_ptr<bElem> p = this->getStats()->getStandingOn().lock();
        p->getStats()->setSteppingOn(this->getStats()->getSteppingOn());
        if (this->getStats()->getSteppingOn())
            this->getStats()->getSteppingOn()->getStats()->setStandingOn(p);
    } else {
        std::shared_ptr<bElem> _Stp = this->getStats()->getSteppingOn();
        _chmbr->setElement(_pos, _Stp);
        if (_Stp) {
            _Stp->getStats()->setHasParent(false); /// this is how we do "unstomp" now.
        } else /// This rather should not happen, but we fix the situation, when we remove the last element, and a null is created, we create a new floor element.
        {
            std::shared_ptr<bElem> nf = elementFactory::generateAnElement<floorElement>(_chmbr, 555);
            nf->getStats()->setMyPosition(_pos);
            _chmbr->setElement(_pos, nf);
        }
    }
    this->setBoard(nullptr);
    this->getStats()->setMyPosition(NOCOORDS);
    return shared_from_this();
}

// Collect another element. The collectible contains location information. that way,
// we restore the element below the collectible, and the collectible will be stored in a vector structure.
// but for now it does nothing
bool bElem::collect(std::shared_ptr<bElem> collectible)
{
    std::shared_ptr<bElem> collected;
    if (collectible.get() == nullptr || !collectible->getAttrs()->isCollectible()
        || !this->getAttrs()->canCollect() || collectible->getStats()->isDying()
        || collectible->getStats()->isTeleporting() || collectible->getStats()->isDestroying()) {
        return false;
    }

    collected = collectible->removeElement();
    if (collected.get() == nullptr) // this should never happen!
    {
        std::cout << "Collecting failed, removed null?\n";
        return false;
    }
#ifdef _VerbousMode_
    std::cout << "Collect " << collected->getType()
              << " st: " << collected->getAttrs()->getSubtype() << "\n";
#endif
    collectible->getStats()->setCollector(shared_from_this());
    this->getAttrs()->getInventory()->addToInventory(collectible);
    collectible->collectOnAction(true, shared_from_this());
    this->getStats()->setPoints(COLLECTS, this->getStats()->getPoints(COLLECTS) + 1);
#ifdef _VerbousMode_
    std::cout << "Collected set? " << (collectible->getStats()->isCollected()) << "\n";
#endif

    return true;
}

bool bElem::kill()
{
    if (!this->getAttrs()->isKillable() || this->getStats()->isDying()
        || this->getStats()->isDestroying() || this->getStats()->isTeleporting()) {
        return false;
    }
    if (this->getAttrs()->isKillable()) {
        // viewPoint::get_instance().addViewPoint(shared_from_this());
        bElem::toDispose.push_back(shared_from_this());
    }
    this->getStats()->setKilled(GoEConstants::_defaultKillTime);
    return true;
}
bool bElem::additionalProvisioning(int subtype)
{
    bool r = false;
    std::call_once(this->_provOnce, [&]() {
        this->attrs = std::make_shared<bElemAttr>(shared_from_this(), this->getType(), subtype);
        r = true;
    });
    return r;
}

int bElem::getType() const
{
    return bElemTypes::_belemType;
}

/*
Here we want to avoid the duplication of boundary checking, that is why we use getABSCoords, isSteppableInDirection and getElementInDirection;
*/
sNeighboorhood bElem::getSteppableNeighborhood()
{
    sNeighboorhood myNeigh;

    for (int c = 0; c < 8; c += 2) {
        int c1 = c / 2;
        dir::direction d = (dir::direction)(c1 % 4);
        dir::direction d1 = (dir::direction)((c1 + 1) % 4);
        std::shared_ptr<bElem> e = this->getElementInDirection(d);

        if (e) {
            auto e1 = e->getElementInDirection(d1);
            myNeigh.nTypes[c] = e->getType();
            myNeigh.steppable[c] = e->getAttrs()->isSteppable();
            myNeigh.steppableClose[c / 2] = myNeigh.steppable[c];
            if (e1) {
                myNeigh.nTypes[c + 1] = e1->getType();
                myNeigh.steppable[c + 1] = e1->getAttrs()->isSteppable();
            } else {
                myNeigh.nTypes[c + 1] = -1;
                myNeigh.steppable[c + 1] = false;
            }
        } else {
            myNeigh.nTypes[c + 1] = -1;
            myNeigh.steppable[c + 1] = false;
            myNeigh.nTypes[c] = -1;
            myNeigh.steppable[c] = false;
            myNeigh.steppableClose[c / 2] = myNeigh.steppable[c];
        }
    }
    return myNeigh;
}

bool bElem::moveInDirectionSpeed(dir::direction dir, int speed)
{
    std::shared_ptr<bElem> stepOn = this->getElementInDirection(dir);
    if (stepOn.get() == nullptr || this->getStats()->isMoving() || this->getStats()->isDying()
        || this->getStats()->isTeleporting() || this->getStats()->isDestroying()
        || dir == dir::direction::NODIRECTION)
        return false;
    std::shared_ptr<bElem> stepOn2 = stepOn->getElementInDirection(dir);
    this->getStats()->setMyDirection(dir);
    if (stepOn->getAttrs()->isSteppable()) {
        this->stepOnElement(stepOn);
        this->getStats()->setMoved(speed);
        goe::sound::play(*this, "Move", "StepOn");
        return true;
    } else if (this->getAttrs()->canCollect() && stepOn->getAttrs()->isCollectible()
               && this->collect(stepOn)) {
        return true;
    } else if (this->getAttrs()->canPush() && stepOn->getAttrs()->canBePushed()
               && stepOn->getAttrs()->isMovable() && stepOn2 && stepOn2->getAttrs()->isSteppable()
               && stepOn->moveInDirectionSpeed(dir, speed + 1)) {
        this->stepOnElement(this->getElementInDirection(dir)); // move the initiating object
        this->getStats()->setMoved(speed + 1);
        goe::sound::play(*this, "Move", "StepOn");
        return true;
    } else if (this->getAttrs()->isInteractive() && stepOn->interact(shared_from_this())) {
        return true;
    }
    return false;
}
bool bElem::moveInDirection(dir::direction d)
{
    return this->moveInDirectionSpeed(d, GoEConstants::_mov_delay);
}
bool bElem::dragInDirection(dir::direction dragIntoDirection)
{
    const int speed = GoEConstants::_mov_delay * 2;
    dir::direction objFromDir = (dir::direction)((((int) dragIntoDirection) + 2) % 4);
    dir::direction d2 = dragIntoDirection;
    std::shared_ptr<bElem> draggedObj = this->getElementInDirection(objFromDir);
    if (draggedObj.get() == nullptr)
        return false;
    if (!draggedObj->getAttrs()->isMovable()) {
        d2 = (dir::direction)((((int) this->getStats()->getMyDirection()) + 2) % 4);
        draggedObj = this->getElementInDirection(d2);
        d2 = this->getStats()->getMyDirection();
        if (draggedObj.get() == nullptr || !draggedObj->getAttrs()->isMovable())
            return false;
    }

    this->moveInDirectionSpeed(dragIntoDirection, speed);
    return draggedObj->moveInDirectionSpeed(d2, speed);
}

void bElem::registerLiveElement(std::shared_ptr<bElem> who)
{
    if (who->getStats()->hasActivatedMechanics())
        return;
    who->getStats()->setActivatedMechanics(true);
    if (who->getBoard())
        who->getBoard()->registerLiveElem(who);
}

void bElem::deregisterLiveElement(unsigned int instanceId)
{
    if (this->getStats()->hasActivatedMechanics() && this->getBoard()) {
        this->getBoard()->toDeregister.push_back(instanceId);
        this->getStats()->setActivatedMechanics(false);
    }
}

void bElem::runLiveElements()
{
    bElem::tick();
    std::shared_ptr<bElem> ap = player::getActivePlayer();
    std::shared_ptr<chamber> cchmbr = ap ? ap->getBoard() : nullptr;
    /// No active player, no game
    if (!cchmbr)
        return;
    // Elements that stopped dying, being destroyed or teleporting are disposed now. They are moved
    // out of the list first, because disposing an element may queue more elements.
    std::vector<std::shared_ptr<bElem>> finished;
    std::erase_if(bElem::toDispose, [&finished](const std::shared_ptr<bElem> &e) {
        auto st = e->getStats();
        if (st->isDying() || st->isDestroying() || st->isTeleporting())
            return false;
        finished.push_back(e);
        return true;
    });
    for (const auto &e : finished)
        if (!e->getStats()->isDisposed())
            e->disposeElement();
    // drop the deregistered elements in one pass
    if (!cchmbr->toDeregister.empty()) {
        std::unordered_set<unsigned long> gone(cchmbr->toDeregister.begin(), cchmbr->toDeregister.end());
        std::erase_if(cchmbr->liveElems, [&gone](const std::shared_ptr<bElem> &e) {
            return e && gone.contains(e->getStats()->getInstanceId());
        });
        cchmbr->toDeregister.clear();
    }
    // Run every live element, compacting out the disposed ones as we go. Elements registered
    // during this loop are appended to the vector and run in this tick too, as before.
    auto &live = cchmbr->liveElems;
    size_t kept = 0;
    for (size_t r = 0; r < live.size(); r++) {
        // moved out and back in: only push_back touches the list while elements run
        std::shared_ptr<bElem> e = std::move(live[r]);
        if (e->getStats()->isDisposed() || e->getType() == bElemTypes::_player)
            continue;
        e->mechanics();
        if (e->getAttrs()->canCollect())
            e->getAttrs()->getInventory()->runLives();
        live[kept++] = std::move(e);
    }
    live.resize(kept);
    ap->mechanics();
    if (ap->getAttrs()->canCollect())
        ap->getAttrs()->getInventory()->runLives();
    puppetMasterHound::watch();
}

/**
 * @brief performs the action, when an element steps on this
 *
 * This method is just a stub, at this type does absolutely nothing
 * @param step
 * true - stepping on, false - unstepping
 * @param who
 * who steps/unsteps
 *
 * @note This should be used in stepOn methods.
 */
bool bElem::stepOnAction(bool step, std::shared_ptr<bElem> who)
{
    return false;
}

void bElem::setStatsOwner(std::shared_ptr<bElem> owner)
{
    this->getStats()->setStatsOwner(owner);
}
