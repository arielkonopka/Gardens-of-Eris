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
#include "teleport.h"
#include "worldBuilder.h"
#include "elementSound.h"
bool teleport::firstReceiverRemoved = false;

std::vector<std::weak_ptr<teleport>> teleport::allTeleporters;
std::vector<teleport::parkedTeleporter> teleport::parked;
std::recursive_mutex teleport::registryMutex;
thread_local bool teleport::deferRegistration = false;
thread_local std::vector<std::weak_ptr<teleport>> teleport::pendingTeleporters;

teleport::registrationBatch::registrationBatch()
{
    teleport::deferRegistration = true;
}

teleport::registrationBatch::~registrationBatch()
{
    teleport::deferRegistration = false;
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    for (const auto &t : teleport::pendingTeleporters)
        if (auto sp = t.lock(); sp && !sp->getStats()->isDisposed())
            teleport::allTeleporters.push_back(t);
    teleport::pendingTeleporters.clear();
}

bool teleport::additionalProvisioning(int value)
{
    if (!bElem::additionalProvisioning(value))
        return false;
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    if (teleport::allTeleporters.empty() && teleport::pendingTeleporters.empty()
        && this->getAttrs()->getSubtype() == 0) {
        this->getStats()->setFacing(dir::direction::LEFT);
        this->getStats()->setMyDirection(this->getStats()->getFacing());
    }
    auto me = std::dynamic_pointer_cast<teleport>(shared_from_this());
    if (teleport::deferRegistration)
        teleport::pendingTeleporters.push_back(me);
    else
        teleport::allTeleporters.push_back(me);
    return true;
}

/* here we will try to teleport an object to the beacon connected to this teleporter. if the beacon is not yet established, randomly choose one */
bool teleport::interact(std::shared_ptr<bElem> who)
{
    bool r;
    if (this->getStats()->getMyDirection() == dir::direction::LEFT || !bElem::interact(who))
        return false;
    auto other = this->partner();
    if (!other && this->createConnectionsWithinSubtype())
        other = this->partner();
    goe::sound::play(*this, "Teleport", "Teleporting");
    if (other)
        r = other->teleportIt(who);
    else
        r = this->teleportIt(who);
    return r;
}

/***
 * @brief Find a partner for a teleport
 * this method will select a random counterpart for our teleport, and set it up. the counterpart will become a receiver.
 * @return whether a partner was found
 */
bool teleport::createConnectionsWithinSubtype()
{
    /// We do this only once, as soon as the first level is created. we can get away with this construct, because we know, that the first mirror is a receiver, and will be inactive.
    /// therefore we have to remove it from all teleporters vector.
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    if (!teleport::allTeleporters.empty() && !teleport::firstReceiverRemoved) {
        teleport::allTeleporters.erase(teleport::allTeleporters.begin());
        teleport::firstReceiverRemoved = true;
    }
    std::shared_ptr<teleport> tmpt;
    std::erase_if(teleport::allTeleporters, [&](const std::weak_ptr<teleport> &wp) {
        if (auto sp = wp.lock()) {
            return sp->getStats()->getInstanceId() == this->getStats()->getInstanceId();
        }
        return true;
    });
    candidates.clear();
    for (const auto &tel : teleport::allTeleporters) {
        if (tel.expired())
            continue;
        if (auto t = tel.lock()) {
            if (t->getAttrs()->getSubtype() == this->getAttrs()->getSubtype()
                && t->getStats()->getInstanceId() != this->getStats()->getInstanceId()) {
                candidates.push_back(t);
            }
        }
    }
    std::vector<parkedTeleporter> parkedCandidates;
    for (const auto &p : teleport::parked)
        if (p.subtype == this->getAttrs()->getSubtype())
            parkedCandidates.push_back(p);
    const std::size_t total = candidates.size() + parkedCandidates.size();
    if (total == 0)
        return false;
    // any matching teleporter can be the other end: local ones pick within their region of
    // the world, global ones (subtype 0) anywhere in it, also in chunks that are on disk
    const std::size_t pick = goe::rng::below(goe::rng::gameplay(), total);
    if (pick < candidates.size()) {
        tmpt = candidates[pick];
    } else {
        const parkedTeleporter chosen = parkedCandidates[pick - candidates.size()];
        // reading the chunk back puts its teleporters into the registry again
        if (auto board = this->getBoard())
            worldBuilder::bringIn(board, chosen.at);
        for (const auto &tel : teleport::allTeleporters)
            if (auto t = tel.lock(); t && t->getStats()->getInstanceId() == chosen.id)
                tmpt = t;
        if (!tmpt) {
            std::erase_if(teleport::parked, [&chosen](const parkedTeleporter &p) { return p.id == chosen.id; });
            this->candidates.clear();
            return false;
        }
    }
    std::erase_if(teleport::allTeleporters, [&](const std::weak_ptr<teleport> &wp) {
        if (auto sp = wp.lock()) {
            return sp->getStats()->getInstanceId() == tmpt->getStats()->getInstanceId();
        }
        return true;
    });
    tmpt->getStats()->setFacing(dir::direction::LEFT);
    tmpt->getStats()->setMyDirection(tmpt->getStats()->getFacing());
    this->linkWith(tmpt);
    this->candidates.clear();
    return true;
}

void teleport::linkWith(const std::shared_ptr<teleport> &t)
{
    auto me = std::dynamic_pointer_cast<teleport>(shared_from_this());
    this->theOtherEnd = t;
    this->otherEndId = t->getStats()->getInstanceId();
    this->otherEndAt = t->getStats()->getMyPosition();
    t->theOtherEnd = me;
    t->otherEndId = this->getStats()->getInstanceId();
    t->otherEndAt = this->getStats()->getMyPosition();
}

std::shared_ptr<teleport> teleport::partner()
{
    // a copy left behind when its chunk went to disk may still be held somewhere; it is not the other end
    if (auto t = this->theOtherEnd.lock(); t && !t->getStats()->isDisposed())
        return t;
    auto board = this->getBoard();
    if (this->otherEndId == 0 || !board)
        return nullptr;
    // the other end's chunk went to disk; read it back and find the other end in its cell
    worldBuilder::bringIn(board, this->otherEndAt);
    for (auto e = board->getElement(this->otherEndAt); e; e = e->getStats()->getSteppingOn())
        if (e->getStats()->getInstanceId() == this->otherEndId)
            if (auto t = std::dynamic_pointer_cast<teleport>(e)) {
                this->theOtherEnd = t;
                return t;
            }
    // it is gone for good
    this->otherEndId = 0;
    this->otherEndAt = NOCOORDS;
    return nullptr;
}

void teleport::park(const std::vector<std::shared_ptr<bElem>> &elements)
{
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    for (const auto &e : elements) {
        auto t = std::dynamic_pointer_cast<teleport>(e);
        if (!t)
            continue;
        // both ends learn where the other is, so the link survives either going to disk
        if (auto other = t->theOtherEnd.lock()) {
            t->otherEndId = other->getStats()->getInstanceId();
            t->otherEndAt = other->getStats()->getMyPosition();
            other->otherEndId = t->getStats()->getInstanceId();
            other->otherEndAt = t->getStats()->getMyPosition();
        }
        const auto id = t->getStats()->getInstanceId();
        const auto before = teleport::allTeleporters.size();
        std::erase_if(teleport::allTeleporters, [id](const std::weak_ptr<teleport> &wp) {
            auto sp = wp.lock();
            return !sp || sp->getStats()->getInstanceId() == id;
        });
        if (teleport::allTeleporters.size() != before)
            teleport::parked.push_back({id, t->getAttrs()->getSubtype(), t->getStats()->getMyPosition()});
    }
}

void teleport::unpark(const std::vector<std::shared_ptr<bElem>> &elements)
{
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    for (const auto &e : elements) {
        auto t = std::dynamic_pointer_cast<teleport>(e);
        if (!t)
            continue;
        const auto id = t->getStats()->getInstanceId();
        if (std::erase_if(teleport::parked, [id](const parkedTeleporter &p) { return p.id == id; }) > 0)
            teleport::allTeleporters.push_back(t);
    }
}

std::size_t teleport::parkedCount()
{
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    return teleport::parked.size();
}

int teleport::getType() const
{
    return bElemTypes::_teleporter;
}

// Teleport to this becon
bool teleport::teleportIt(std::shared_ptr<bElem> who)
{
    if (this->getStats()->isWaiting() || !this->getBoard() || !who || !who->getAttrs()->isMovable())
        return false;
    this->getStats()->setWaiting(50);
    who->getStats()->setTelInProgress(GoEConstants::_teleportationTime);
    if (who->getStats()->getSteppingOn() != nullptr)
        who->getStats()->getSteppingOn()->getStats()->setTelInProgress(
            GoEConstants::_teleportationTime);
    std::vector<std::shared_ptr<bElem>> spots;
    for (auto d : dir::allDirections) {
        if (this->isSteppableDirection(d))
            spots.push_back(this->getElementInDirection(d));
    }
    if (spots.empty()) {
        who->getStats()->getSteppingOn()->getStats()->setTelInProgress(
            GoEConstants::_teleportationTime);
        return false;
    } else {
        who->stepOnElement(goe::rng::pick(goe::rng::gameplay(), spots));
        return true;
    }
}

bool teleport::stepOnAction(bool step, std::shared_ptr<bElem> who)
{
    bElem::stepOnAction(step, who);
    goe::sound::play(*this, "Teleport", "HummingSound");
    if (this->getStats()->getMyDirection() == dir::direction::LEFT)
        return false;
    if (step && !who->getStats()->isTeleporting() && !this->getStats()->isTeleporting()) {
        this->getStats()->setWaiting(GoEConstants::_teleportStandTime);
        this->registerLiveElement(shared_from_this());
    } else {
        if (this->getStats()->hasActivatedMechanics())
            this->deregisterLiveElement(this->getStats()->getInstanceId());
    }
    return true;
}

bool teleport::mechanics()
{
    if (!bElem::mechanics())
        return false;
    if (!this->getStats()->isWaiting() && this->getStats()->hasParent()
        && this->getStats()->getMyDirection() != dir::direction::LEFT) {
        this->interact(this->getStats()->getStandingOn().lock());
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    }
    return true;
}

bool teleport::removeFromAllTeleporters()
{
    std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
    std::erase_if(teleport::pendingTeleporters, [&](const std::weak_ptr<teleport> &wp) {
        auto sp = wp.lock();
        return !sp || sp->getStats()->getInstanceId() == this->getStats()->getInstanceId();
    });
    for (unsigned int c = 0; c < teleport::allTeleporters.size();) {
        std::shared_ptr<teleport> t = teleport::allTeleporters[c].lock();
        if (teleport::allTeleporters[c].expired()
            || t->getStats()->getInstanceId() == this->getStats()->getInstanceId()) {
            teleport::allTeleporters.erase(teleport::allTeleporters.begin() + c);
        } else {
            c++;
        }
    }
    return true;
}

oState teleport::disposeElement()
{
    this->removeFromAllTeleporters();
    return bElem::disposeElement();
}

