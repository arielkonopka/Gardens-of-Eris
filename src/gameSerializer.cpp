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

#include "gameSerializer.h"
#include "elements.h"
#include "viewPoint.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace {
constexpr char saveMagic[8] = {'G', 'O', 'E', 'S', 'A', 'V', 'E', '\0'};
constexpr uint8_t cellCompact = 0;
constexpr uint8_t cellRecord = 1;
} // namespace

/*
 * Low level binary IO. Values are written in host byte order (little endian on every platform the
 * game builds for); the file starts with a magic and a version, so a format change can be detected.
 */
class gameSerializer::writer
{
public:
    explicit writer(const std::string &fileName)
        : out(fileName, std::ios::binary | std::ios::trunc)
    {}
    bool good() const { return out.good(); }

    template<typename T>
    void pod(T v)
    {
        out.write(reinterpret_cast<const char *>(&v), sizeof(T));
    }
    void raw(const char *data, size_t n) { out.write(data, (std::streamsize) n); }
    void u8(uint8_t v) { pod(v); }
    void u32(uint32_t v) { pod(v); }
    void i32(int32_t v) { pod(v); }
    void u64(uint64_t v) { pod(v); }
    void f32(float v) { pod(v); }
    void str(const std::string &s)
    {
        u32((uint32_t) s.size());
        out.write(s.data(), (std::streamsize) s.size());
    }

    /// writes a reference as an instance id (0 = none) and queues the element to be written as a record
    void ref(const std::shared_ptr<bElem> &e)
    {
        if (!e) {
            u64(0);
            return;
        }
        auto id = e->getStats()->getInstanceId();
        if (compactIds.count(id)) {
            // plain floors and walls are not addressable; nothing should point at them
            u64(0);
            return;
        }
        u64(id);
        if (seen.insert(id).second)
            queue.push_back(e);
    }
    void ref(const std::weak_ptr<bElem> &e) { ref(e.lock()); }
    void refs(const std::vector<std::shared_ptr<bElem>> &v)
    {
        u32((uint32_t) v.size());
        for (const auto &e : v)
            ref(e);
    }

    std::unordered_set<unsigned long> seen;
    std::unordered_set<unsigned long> compactIds;
    std::deque<std::shared_ptr<bElem>> queue;

private:
    std::ofstream out;
};

class gameSerializer::reader
{
public:
    explicit reader(const std::string &fileName)
        : in(fileName, std::ios::binary)
    {}
    bool good() const { return in.good(); }

    template<typename T>
    T pod()
    {
        T v{};
        in.read(reinterpret_cast<char *>(&v), sizeof(T));
        if (!in)
            throw std::runtime_error("save file is truncated");
        return v;
    }
    uint8_t u8() { return pod<uint8_t>(); }
    uint32_t u32() { return pod<uint32_t>(); }
    int32_t i32() { return pod<int32_t>(); }
    uint64_t u64() { return pod<uint64_t>(); }
    float f32() { return pod<float>(); }
    std::string str()
    {
        auto n = u32();
        if (n > (1u << 20))
            throw std::runtime_error("save file string is too long");
        std::string s(n, '\0');
        in.read(s.data(), n);
        if (!in)
            throw std::runtime_error("save file is truncated");
        return s;
    }
    std::vector<uint64_t> ids()
    {
        auto n = u32();
        std::vector<uint64_t> v;
        v.reserve(n);
        for (uint32_t c = 0; c < n; c++)
            v.push_back(u64());
        return v;
    }

private:
    std::ifstream in;
};

/// what the loader needs to resolve references once every element exists
struct gameSerializer::loadContext
{
    std::unordered_map<uint64_t, std::shared_ptr<bElem>> byId;
    std::unordered_map<int, std::shared_ptr<chamber>> chambersById;
    std::vector<std::function<void()>> fixups;

    std::shared_ptr<bElem> get(uint64_t id)
    {
        if (id == 0)
            return nullptr;
        auto it = byId.find(id);
        if (it == byId.end())
            throw std::runtime_error("save file references a missing element");
        return it->second;
    }
    /// resolve reference `id` once all records are read, and hand it to `set`
    void later(uint64_t id, std::function<void(std::shared_ptr<bElem>)> set)
    {
        fixups.push_back([this, id, set]() { set(get(id)); });
    }
    std::vector<std::shared_ptr<bElem>> getAll(const std::vector<uint64_t> &ids)
    {
        std::vector<std::shared_ptr<bElem>> v;
        for (auto id : ids)
            if (auto e = get(id))
                v.push_back(e);
        return v;
    }
};

std::shared_ptr<bElem> gameSerializer::createByType(int type, int subtype)
{
    std::shared_ptr<chamber> none = nullptr;
    switch (type) {
    case bElemTypes::_floorType:
        return elementFactory::generateAnElement<floorElement>(none, subtype);
    case bElemTypes::_wallType:
        return elementFactory::generateAnElement<wall>(none, subtype);
    case bElemTypes::_rubishType:
        return elementFactory::generateAnElement<rubbish>(none, subtype);
    case bElemTypes::_monster:
        return elementFactory::generateAnElement<monster>(none, subtype);
    case bElemTypes::_patrollingDrone:
        return elementFactory::generateAnElement<patrollingDrone>(none, subtype);
    case bElemTypes::_securityCamera:
        return elementFactory::generateAnElement<securityCamera>(none, subtype);
    case bElemTypes::_puppetMasterType:
        return puppetMasterFR::create(none, subtype);
    case bElemTypes::_brickClusterType:
        return elementFactory::generateAnElement<brickCluster>(none, subtype);
    case bElemTypes::_player:
        return elementFactory::generateAnElement<player>(none, subtype);
    case bElemTypes::_key:
        return elementFactory::generateAnElement<key>(none, subtype);
    case bElemTypes::_door:
        return elementFactory::generateAnElement<door>(none, subtype);
    case bElemTypes::_plainMissile:
        return elementFactory::generateAnElement<plainMissile>(none, subtype);
    case bElemTypes::_plainGun:
        return elementFactory::generateAnElement<plainGun>(none, subtype);
    case bElemTypes::_bazookaMissileType:
        return elementFactory::generateAnElement<bazookaMissile>(none, subtype);
    case bElemTypes::_bazookaType:
        return elementFactory::generateAnElement<bazooka>(none, subtype);
    case bElemTypes::_bunker:
        return elementFactory::generateAnElement<bunker>(none, subtype);
    case bElemTypes::_teleporter:
        return elementFactory::generateAnElement<teleport>(none, subtype);
    case bElemTypes::_goldenAppleType:
        return elementFactory::generateAnElement<goldenApple>(none, subtype);
    case bElemTypes::_simpleBombType:
        return elementFactory::generateAnElement<simpleBomb>(none, subtype);
    case bElemTypes::_landmineType:
        return elementFactory::generateAnElement<landmine>(none, subtype);
    case bElemTypes::_boubaType:
        return elementFactory::generateAnElement<bouba>(none, subtype);
    case bElemTypes::_kikiType:
        return elementFactory::generateAnElement<kiki>(none, subtype);
    case bElemTypes::_belemType:
        return elementFactory::generateAnElement<bElem>(none, subtype);
    }
    throw std::runtime_error("save file contains an unknown element type " + std::to_string(type));
}

/*
 * Plain floor and wall tiles are most of every chamber. When one carries no state beyond its type,
 * subtype and orientation, it is written in four fields instead of a full record.
 */
bool gameSerializer::isCompact(const std::shared_ptr<bElem> &e)
{
    int type = e->getType();
    if (type != bElemTypes::_floorType && type != bElemTypes::_wallType)
        return false;
    const auto &s = *e->getStats();
    if (s.disposed || s.active || s.marked || s.activatedMechanics || s.collected
        || s.telTimeReq || s.telReqTime || s.killTimeReq || s.killTimeBeg || s.destTimeReq
        || s.destTimeBeg || s.telInProgress || s.interacted || !s.statistics.empty()
        || s.movingTotalTime != -1 || s.fadingOut != -1 || s.fadingIn != -1 || s.fadingInReq
        || s.fadingOutReq || s.waiting != -1 || s.moved != -1 || s.destroyed != -1
        || s.animPhase != 0 || s.ammo != 0 || s.killed != -1 || !s.collector.expired()
        || !s.statsOwner.expired() || !e->lockers.empty())
        return false;
    const auto &a = *e->getAttrs();
    if (a.inv)
        return false;
    // attributes must be exactly what a freshly provisioned tile of this subtype gets
    static thread_local std::map<std::pair<int, int>, bElemAttr> defaults;
    auto key = std::make_pair(type, a.subType);
    auto it = defaults.find(key);
    if (it == defaults.end()) {
        bElemAttr probe(nullptr, type, a.subType);
        probe.getDefaultValues(type, a.subType);
        it = defaults.emplace(key, probe).first;
    }
    const auto &fresh = it->second;
    return fresh.killable == a.killable && fresh.destroyable == a.destroyable
           && fresh.steppable == a.steppable && fresh.movable == a.movable
           && fresh.interactive == a.interactive && fresh.collectible == a.collectible
           && fresh.push == a.push && fresh.pushed == a.pushed && fresh.collect == a.collect
           && fresh.weapon == a.weapon && fresh.open == a.open && fresh.locked == a.locked
           && fresh.energy == a.energy && fresh.maxEnergy == a.maxEnergy && fresh.ammo == a.ammo
           && fresh.maxAmmo == a.maxAmmo;
}

void gameSerializer::writeElement(writer &w, const std::shared_ptr<bElem> &e)
{
    const auto &s = *e->getStats();
    const auto &a = *e->getAttrs();
    auto board = e->attachedBoard.lock();

    w.i32(e->getType());
    w.u64(s.instanceId);
    w.i32(board ? board->getInstanceId() : -1);

    // stats; stacking links and grid positions are rebuilt from the chamber cells
    w.u8(s.disposed);
    w.u8(s.active);
    w.u8(s.marked);
    w.u8(s.activatedMechanics);
    w.u8(s.collected);
    w.u32(s.telTimeReq);
    w.u32(s.telReqTime);
    w.u32(s.killTimeReq);
    w.u32(s.killTimeBeg);
    w.u32(s.destTimeReq);
    w.u32(s.destTimeBeg);
    w.u32(s.telInProgress);
    w.u32(s.interacted);
    std::map<int, int> stats(s.statistics.begin(), s.statistics.end()); // sorted, so saves are stable
    w.u32((uint32_t) stats.size());
    for (const auto &[k, v] : stats) {
        w.i32(k);
        w.i32(v);
    }
    w.i32(s.movingTotalTime);
    w.i32(s.fadingOut);
    w.i32(s.fadingIn);
    w.u32(s.fadingInReq);
    w.u32(s.fadingOutReq);
    w.i32(s.waiting);
    w.i32(s.moved);
    w.i32(s.destroyed);
    w.i32(s.animPhase);
    w.i32(s.taterCounter);
    w.i32(s.ammo);
    w.i32(s.killed);
    bool noPos = s.myPosition == myUtility::NOCOORDS;
    w.u8(noPos);
    w.i32(s.myPosition.getX());
    w.i32(s.myPosition.getY());
    w.u8((uint8_t) s.myDirection);
    w.u8((uint8_t) s.facing);
    w.ref(s.collector);
    w.ref(s.statsOwner);

    // attributes
    w.u8(a.provisioned);
    w.i32(a.bElemType);
    w.i32(a.subType);
    w.u8(a.killable);
    w.u8(a.destroyable);
    w.u8(a.steppable);
    w.u8(a.movable);
    w.u8(a.interactive);
    w.u8(a.collectible);
    w.u8(a.push);
    w.u8(a.pushed);
    w.u8(a.collect);
    w.u8(a.weapon);
    w.u8(a.open);
    w.u8(a.locked);
    w.i32(a.energy);
    w.i32(a.maxEnergy);
    w.i32(a.ammo);
    w.i32(a.maxAmmo);

    // inventory
    w.u8(a.inv != nullptr);
    if (a.inv) {
        const auto &inv = *a.inv;
        w.refs(inv.weapons);
        w.refs(inv.mods);
        w.refs(inv.tokens);
        w.refs(inv.usables);
        w.refs(inv.keys);
        w.u32((uint32_t) inv.tokenNumbers.size());
        for (const auto &[t, n] : inv.tokenNumbers) {
            w.i32(t.tokenType);
            w.i32(t.tokenSubtype);
            w.i32(n);
        }
        w.i32(inv.wPos);
        w.i32(inv.uPos);
    }
    w.refs(e->lockers);

    // state that only some element types carry
    if (auto p = std::dynamic_pointer_cast<player>(e)) {
        w.f32(p->vRadius);
        w.i32(p->animPh);
        w.u8(p->activated);
        w.u8(p->provisioned);
    }
    if (auto m = std::dynamic_pointer_cast<monster>(e)) {
        w.ref(std::static_pointer_cast<bElem>(m->weapon));
        w.u8(m->inited);
        w.i32(m->rotA);
        w.i32(m->rotB);
    }
    if (auto d = std::dynamic_pointer_cast<patrollingDrone>(e)) {
        w.u8(d->brained);
        w.ref(d->brainModule);
    }
    if (auto b = std::dynamic_pointer_cast<bunker>(e)) {
        w.ref(b->activatedBy);
        w.i32(b->help);
        w.ref(b->myGun);
    }
    if (auto x = std::dynamic_pointer_cast<explosives>(e)) {
        w.i32(x->brd ? x->brd->getInstanceId() : -1);
        w.i32(x->bx);
        w.i32(x->by);
        w.f32(x->radius);
    }
    if (auto sb = std::dynamic_pointer_cast<simpleBomb>(e))
        w.u8(sb->triggered);
    if (auto bm = std::dynamic_pointer_cast<bazookaMissile>(e))
        w.i32(bm->steps);
    if (auto bo = std::dynamic_pointer_cast<bouba>(e))
        w.i32(bo->timer);
    if (auto g = std::dynamic_pointer_cast<plainGun>(e))
        w.u32(g->shot);
    if (auto t = std::dynamic_pointer_cast<teleport>(e))
        w.ref(std::static_pointer_cast<bElem>(t->theOtherEnd.lock()));
    if (auto k = std::dynamic_pointer_cast<kiki>(e))
        w.u8((uint8_t) k->direction);
    if (auto c = std::dynamic_pointer_cast<securityCamera>(e)) {
        w.u8(c->guardiansSpawned);
        w.i32(c->alertAt.x);
        w.i32(c->alertAt.y);
        w.u32(c->alertNumber);
    }
    if (auto g = std::dynamic_pointer_cast<puppetMasterGuardian>(e)) {
        w.ref(std::static_pointer_cast<bElem>(g->camera.lock()));
        w.ref(g->gun);
        w.i32(g->home.x);
        w.i32(g->home.y);
        w.i32(g->target.x);
        w.i32(g->target.y);
        w.u32(g->handledAlert);
    }
}

std::shared_ptr<bElem> gameSerializer::readElement(reader &r, loadContext &ctx)
{
    int type = r.i32();
    uint64_t id = r.u64();
    int boardId = r.i32();

    // read everything first; the element is created once its subtype is known
    struct rawStats
    {
        bool disposed, active, marked, activatedMechanics, collected;
        uint32_t telTimeReq, telReqTime, killTimeReq, killTimeBeg, destTimeReq, destTimeBeg,
            telInProgress, interacted;
        std::vector<std::pair<int, int>> statistics;
        int movingTotalTime, fadingOut, fadingIn;
        uint32_t fadingInReq, fadingOutReq;
        int waiting, moved, destroyed, animPhase, taterCounter, ammo, killed;
        bool noPos;
        int x, y;
        uint8_t dir, facing;
        uint64_t collector, statsOwner;
    } rs;
    rs.disposed = r.u8();
    rs.active = r.u8();
    rs.marked = r.u8();
    rs.activatedMechanics = r.u8();
    rs.collected = r.u8();
    rs.telTimeReq = r.u32();
    rs.telReqTime = r.u32();
    rs.killTimeReq = r.u32();
    rs.killTimeBeg = r.u32();
    rs.destTimeReq = r.u32();
    rs.destTimeBeg = r.u32();
    rs.telInProgress = r.u32();
    rs.interacted = r.u32();
    for (uint32_t c = r.u32(); c > 0; c--) {
        int k = r.i32();
        rs.statistics.emplace_back(k, r.i32());
    }
    rs.movingTotalTime = r.i32();
    rs.fadingOut = r.i32();
    rs.fadingIn = r.i32();
    rs.fadingInReq = r.u32();
    rs.fadingOutReq = r.u32();
    rs.waiting = r.i32();
    rs.moved = r.i32();
    rs.destroyed = r.i32();
    rs.animPhase = r.i32();
    rs.taterCounter = r.i32();
    rs.ammo = r.i32();
    rs.killed = r.i32();
    rs.noPos = r.u8();
    rs.x = r.i32();
    rs.y = r.i32();
    rs.dir = r.u8();
    rs.facing = r.u8();
    rs.collector = r.u64();
    rs.statsOwner = r.u64();

    bool provisioned = r.u8();
    int attrType = r.i32();
    int subType = r.i32();

    auto e = createByType(type, subType);
    auto &s = *e->getStats();
    auto &a = *e->getAttrs();
    s.instanceId = id;
    s.disposed = rs.disposed;
    s.active = rs.active;
    s.marked = rs.marked;
    s.activatedMechanics = rs.activatedMechanics;
    s.collected = rs.collected;
    s.telTimeReq = rs.telTimeReq;
    s.telReqTime = rs.telReqTime;
    s.killTimeReq = rs.killTimeReq;
    s.killTimeBeg = rs.killTimeBeg;
    s.destTimeReq = rs.destTimeReq;
    s.destTimeBeg = rs.destTimeBeg;
    s.telInProgress = rs.telInProgress;
    s.interacted = rs.interacted;
    s.statistics.clear();
    for (const auto &[k, v] : rs.statistics)
        s.statistics[(pointsType) k] = v;
    s.movingTotalTime = rs.movingTotalTime;
    s.fadingOut = rs.fadingOut;
    s.fadingIn = rs.fadingIn;
    s.fadingInReq = rs.fadingInReq;
    s.fadingOutReq = rs.fadingOutReq;
    s.waiting = rs.waiting;
    s.moved = rs.moved;
    s.destroyed = rs.destroyed;
    s.animPhase = rs.animPhase;
    s.taterCounter = rs.taterCounter;
    s.ammo = rs.ammo;
    s.killed = rs.killed;
    s.myPosition = rs.noPos ? myUtility::NOCOORDS : myUtility::Coords(rs.x, rs.y);
    s.myDirection = (dir::direction) rs.dir;
    s.facing = (dir::direction) rs.facing;
    s.steppingOn = nullptr;
    s.standingOn.reset();
    s.parent = false;
    s.collector.reset();
    s.statsOwner.reset();
    // the collected flag is part of the saved state; don't let the setters recompute it
    ctx.later(rs.collector, [e](std::shared_ptr<bElem> c) { e->getStats()->collector = c; });
    ctx.later(rs.statsOwner, [e](std::shared_ptr<bElem> o) { e->getStats()->statsOwner = o; });

    a.provisioned = provisioned;
    a.bElemType = attrType;
    a.subType = subType;
    a.owner = e;
    a.killable = r.u8();
    a.destroyable = r.u8();
    a.steppable = r.u8();
    a.movable = r.u8();
    a.interactive = r.u8();
    a.collectible = r.u8();
    a.push = r.u8();
    a.pushed = r.u8();
    a.collect = r.u8();
    a.weapon = r.u8();
    a.open = r.u8();
    a.locked = r.u8();
    a.energy = r.i32();
    a.maxEnergy = r.i32();
    a.ammo = r.i32();
    a.maxAmmo = r.i32();

    a.inv = nullptr;
    if (r.u8()) {
        auto inv = std::make_shared<inventory>();
        inv->owner = e;
        a.inv = inv;
        auto weapons = r.ids(), mods = r.ids(), tokens = r.ids(), usables = r.ids(), keys = r.ids();
        ctx.fixups.push_back([inv, weapons, mods, tokens, usables, keys, &ctx]() {
            inv->weapons = ctx.getAll(weapons);
            inv->mods = ctx.getAll(mods);
            inv->tokens = ctx.getAll(tokens);
            inv->usables = ctx.getAll(usables);
            inv->keys = ctx.getAll(keys);
        });
        for (uint32_t c = r.u32(); c > 0; c--) {
            tType t;
            t.tokenType = r.i32();
            t.tokenSubtype = r.i32();
            inv->tokenNumbers[t] = r.i32();
        }
        inv->wPos = r.i32();
        inv->uPos = r.i32();
    }
    auto lockers = r.ids();
    ctx.fixups.push_back([e, lockers, &ctx]() { e->lockers = ctx.getAll(lockers); });

    if (auto p = std::dynamic_pointer_cast<player>(e)) {
        p->vRadius = r.f32();
        p->animPh = r.i32();
        p->activated = r.u8();
        p->provisioned = r.u8();
    }
    if (auto m = std::dynamic_pointer_cast<monster>(e)) {
        ctx.later(r.u64(), [m](std::shared_ptr<bElem> g) {
            m->weapon = std::dynamic_pointer_cast<plainGun>(g);
        });
        m->inited = r.u8();
        m->rotA = r.i32();
        m->rotB = r.i32();
    }
    if (auto d = std::dynamic_pointer_cast<patrollingDrone>(e)) {
        d->brained = r.u8();
        ctx.later(r.u64(), [d](std::shared_ptr<bElem> b) { d->brainModule = b; });
    }
    if (auto b = std::dynamic_pointer_cast<bunker>(e)) {
        ctx.later(r.u64(), [b](std::shared_ptr<bElem> by) { b->activatedBy = by; });
        b->help = r.i32();
        ctx.later(r.u64(), [b](std::shared_ptr<bElem> g) { b->myGun = g; });
    }
    if (auto x = std::dynamic_pointer_cast<explosives>(e)) {
        int brdId = r.i32();
        ctx.fixups.push_back([x, brdId, &ctx]() {
            auto it = ctx.chambersById.find(brdId);
            x->brd = (it != ctx.chambersById.end()) ? it->second : nullptr;
        });
        x->bx = r.i32();
        x->by = r.i32();
        x->radius = r.f32();
    }
    if (auto sb = std::dynamic_pointer_cast<simpleBomb>(e))
        sb->triggered = r.u8();
    if (auto bm = std::dynamic_pointer_cast<bazookaMissile>(e))
        bm->steps = r.i32();
    if (auto bo = std::dynamic_pointer_cast<bouba>(e))
        bo->timer = r.i32();
    if (auto g = std::dynamic_pointer_cast<plainGun>(e))
        g->shot = r.u32();
    if (auto t = std::dynamic_pointer_cast<teleport>(e))
        ctx.later(r.u64(), [t](std::shared_ptr<bElem> o) {
            t->theOtherEnd = std::dynamic_pointer_cast<teleport>(o);
        });
    if (auto k = std::dynamic_pointer_cast<kiki>(e))
        k->direction = (dir::direction) r.u8();
    if (auto c = std::dynamic_pointer_cast<securityCamera>(e)) {
        c->guardiansSpawned = r.u8();
        int x = r.i32();
        c->alertAt = coords(x, r.i32());
        c->alertNumber = r.u32();
    }
    if (auto g = std::dynamic_pointer_cast<puppetMasterGuardian>(e)) {
        ctx.later(r.u64(), [g](std::shared_ptr<bElem> c) {
            g->camera = std::dynamic_pointer_cast<securityCamera>(c);
        });
        ctx.later(r.u64(), [g](std::shared_ptr<bElem> gun) { g->gun = gun; });
        int x = r.i32();
        g->home = coords(x, r.i32());
        x = r.i32();
        g->target = coords(x, r.i32());
        g->handledAlert = r.u32();
    }

    ctx.fixups.push_back([e, boardId, &ctx]() {
        auto it = ctx.chambersById.find(boardId);
        e->attachedBoard = (it != ctx.chambersById.end()) ? it->second : nullptr;
    });
    if (!ctx.byId.emplace(id, e).second)
        throw std::runtime_error("save file contains a duplicate element id");
    return e;
}

void gameSerializer::clearWorld()
{
    {
        std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
        teleport::allTeleporters.clear();
        teleport::pendingTeleporters.clear();
    }
    player::activePlayer = nullptr;
    player::visitedPlayers.clear();
    goldenApple::apples.clear();
    goldenApple::appleNumber = 0;
    bElem::toDispose.clear();
    viewPoint::get_instance().viewPoints.clear();
    viewPoint::get_instance()._owner.reset();
    chamber::allChambers.clear();
}

bool gameSerializer::saveGame(const std::string &fileName)
{
    std::lock_guard<std::recursive_mutex> worldLock(chamber::worldMutex);
    std::string tmpName = fileName + ".tmp";
    {
        writer w(tmpName);
        if (!w.good()) {
            std::cout << "Cannot write save file " << tmpName << "\n";
            return false;
        }
        // header and global state
        w.raw(saveMagic, sizeof(saveMagic));
        w.u32(formatVersion);
        w.u32(gameClock::ticks);
        w.u64(bElemStats::currentInstance);
        w.i32(chamber::lastid);
        std::ostringstream rng;
        rng << goe::rng::saved();
        w.str(rng.str());
        w.u8(teleport::firstReceiverRemoved);
        w.u32(goldenApple::appleNumber);

        // static registries
        w.ref(player::activePlayer);
        w.refs(player::visitedPlayers);
        w.refs(goldenApple::apples);
        {
            std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
            std::vector<std::shared_ptr<bElem>> tps;
            for (const auto &t : teleport::allTeleporters)
                if (auto sp = t.lock())
                    tps.push_back(sp);
            w.refs(tps);
        }
        w.refs(bElem::toDispose);
        auto &vp = viewPoint::get_instance();
        w.ref(vp._owner);
        std::vector<std::shared_ptr<bElem>> vps;
        for (const auto &p : vp.viewPoints)
            if (auto sp = p.lock())
                vps.push_back(sp);
        w.refs(vps);

        // chambers
        std::vector<std::shared_ptr<chamber>> chambers;
        for (const auto &c : chamber::allChambers)
            if (c->ready)
                chambers.push_back(c);
        w.u32((uint32_t) chambers.size());
        for (const auto &c : chambers) {
            w.i32(c->instanceid);
            w.str(c->chamberName);
            w.i32(c->chamberColour.r);
            w.i32(c->chamberColour.g);
            w.i32(c->chamberColour.b);
            w.i32(c->chamberColour.a);
            w.i32(c->width);
            w.i32(c->height);
            w.u32(c->applesCount);
            w.i32(c->depth);
            w.i32(c->origin.x);
            w.i32(c->origin.y);
            // fog of war, run-length encoded
            {
                std::vector<std::pair<int32_t, uint32_t>> runs;
                for (int x = 0; x < c->width; x++)
                    for (int y = 0; y < c->height; y++) {
                        int v = c->visitedElements[c->cellIndex(x, y)];
                        if (!runs.empty() && runs.back().first == v)
                            runs.back().second++;
                        else
                            runs.emplace_back(v, 1);
                    }
                w.u32((uint32_t) runs.size());
                for (const auto &[v, n] : runs) {
                    w.i32(v);
                    w.u32(n);
                }
            }
            // cells: each stack, bottom first
            std::vector<std::shared_ptr<bElem>> stack;
            for (int x = 0; x < c->width; x++)
                for (int y = 0; y < c->height; y++) {
                    stack.clear();
                    for (auto e = c->cells[c->cellIndex(x, y)]; e && stack.size() < 255;
                         e = e->getStats()->getSteppingOn())
                        stack.push_back(e);
                    w.u8((uint8_t) stack.size());
                    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
                        const auto &e = *it;
                        auto id = e->getStats()->getInstanceId();
                        if (!w.seen.count(id) && isCompact(e)) {
                            w.compactIds.insert(id);
                            w.u8(cellCompact);
                            w.i32(e->getType());
                            w.i32(e->getAttrs()->getSubtype());
                            w.u8((uint8_t) e->getStats()->myDirection);
                            w.u8((uint8_t) e->getStats()->facing);
                        } else {
                            w.u8(cellRecord);
                            w.ref(e);
                        }
                    }
                }
            w.refs(c->liveElems);
            w.u32((uint32_t) c->toDeregister.size());
            for (auto id : c->toDeregister)
                w.u64(id);
        }

        // every element referenced so far, and whatever those reference in turn
        while (!w.queue.empty()) {
            auto e = w.queue.front();
            w.queue.pop_front();
            w.u8(1);
            writeElement(w, e);
        }
        w.u8(0);
        if (!w.good()) {
            std::cout << "Writing save file " << tmpName << " failed\n";
            return false;
        }
    }
    // replace the old save only once the new one is complete; std::rename would refuse to
    // overwrite an existing save on Windows, std::filesystem::rename replaces it everywhere
    std::error_code ec;
    std::filesystem::rename(tmpName, fileName, ec);
    if (ec) {
        std::cout << "Cannot move " << tmpName << " to " << fileName << "\n";
        return false;
    }
    return true;
}

bool gameSerializer::loadGame(const std::string &fileName)
{
    std::lock_guard<std::recursive_mutex> worldLock(chamber::worldMutex);
    reader r(fileName);
    if (!r.good()) {
        std::cout << "Cannot open save file " << fileName << "\n";
        return false;
    }
    // creating elements registers teleporters and apples; undo that if the file turns out bad
    std::vector<std::weak_ptr<teleport>> teleportersBefore;
    {
        std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
        teleportersBefore = teleport::allTeleporters;
    }
    auto applesBefore = goldenApple::apples;
    auto appleNumberBefore = goldenApple::appleNumber;
    try {
        auto magic = r.pod<std::array<char, 8>>();
        if (std::memcmp(magic.data(), saveMagic, sizeof(saveMagic)) != 0)
            throw std::runtime_error("not a Gardens of Eris save file");
        const uint32_t version = r.u32();
        if (version < 1 || version > formatVersion)
            throw std::runtime_error("unsupported save file version");

        // read everything before touching the running world, so a bad file leaves it intact
        auto taterCounter = r.u32();
        auto instanceCounter = r.u64();
        auto lastChamberId = r.i32();
        auto rngState = r.str();
        bool firstReceiverRemoved = r.u8();
        auto appleNumber = r.u32();
        auto activePlayerId = r.u64();
        auto visitedPlayerIds = r.ids();
        auto appleIds = r.ids();
        auto teleporterIds = r.ids();
        auto toDisposeIds = r.ids();
        auto viewOwnerId = r.u64();
        auto viewPointIds = r.ids();

        // new elements get ids above every saved one; saved records then take their own ids back
        if (bElemStats::currentInstance < instanceCounter)
            bElemStats::currentInstance = instanceCounter;

        loadContext ctx;
        struct cellEntry
        {
            std::shared_ptr<bElem> compact;
            uint64_t id = 0;
        };
        struct chamberData
        {
            std::shared_ptr<chamber> c;
            std::vector<std::vector<cellEntry>> cells; // x-major, one stack per cell
            std::vector<uint64_t> liveIds;
            std::vector<uint64_t> toDeregister;
        };
        std::vector<chamberData> chambers;
        for (uint32_t n = r.u32(); n > 0; n--) {
            chamberData cd;
            int id = r.i32();
            std::string name = r.str();
            colour col;
            col.r = r.i32();
            col.g = r.i32();
            col.b = r.i32();
            col.a = r.i32();
            int w = r.i32(), h = r.i32();
            if (w <= 0 || h <= 0 || (int64_t) w * h > (1 << 26))
                throw std::runtime_error("save file has an invalid chamber size");
            auto c = std::make_shared<chamber>(w, h);
            c->instanceid = id;
            c->chamberName = name;
            c->chamberColour = col;
            c->applesCount = r.u32();
            // version 1 saves have no difficulty data: their chambers load as depth 0, no origin
            if (version >= 2) {
                c->depth = r.i32();
                c->origin.x = r.i32();
                c->origin.y = r.i32();
            }
            c->visitedElements.assign((size_t) w * h, 0);
            {
                int64_t pos = 0, total = (int64_t) w * h;
                for (uint32_t runs = r.u32(); runs > 0; runs--) {
                    int v = r.i32();
                    uint32_t cnt = r.u32();
                    for (uint32_t k = 0; k < cnt && pos < total; k++, pos++)
                        c->visitedElements[pos] = v;
                }
            }
            c->cells.resize((size_t) w * h);
            cd.cells.resize((size_t) w * h);
            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    auto &stack = cd.cells[(size_t) x * h + y];
                    for (uint8_t k = r.u8(); k > 0; k--) {
                        cellEntry ce;
                        if (r.u8() == cellCompact) {
                            int type = r.i32();
                            int subtype = r.i32();
                            ce.compact = createByType(type, subtype);
                            ce.compact->getAttrs()->setSubtype(subtype);
                            ce.compact->getStats()->myDirection = (dir::direction) r.u8();
                            ce.compact->getStats()->facing = (dir::direction) r.u8();
                        } else {
                            ce.id = r.u64();
                        }
                        stack.push_back(ce);
                    }
                }
            }
            cd.liveIds = r.ids();
            for (uint32_t k = r.u32(); k > 0; k--)
                cd.toDeregister.push_back(r.u64());
            cd.c = c;
            ctx.chambersById[id] = c;
            chambers.push_back(std::move(cd));
        }

        while (r.u8() == 1)
            readElement(r, ctx);
        for (auto &f : ctx.fixups)
            f();

        // rebuild stacks: bottom element first, the top one sits in the grid
        for (auto &cd : chambers) {
            auto &c = cd.c;
            for (int x = 0; x < c->width; x++)
                for (int y = 0; y < c->height; y++) {
                    std::shared_ptr<bElem> below;
                    for (auto &ce : cd.cells[(size_t) x * c->height + y]) {
                        auto e = ce.compact ? ce.compact : ctx.get(ce.id);
                        if (!e)
                            continue;
                        auto &s = *e->getStats();
                        e->attachedBoard = c;
                        s.myPosition = myUtility::Coords(x, y);
                        s.steppingOn = below;
                        s.standingOn.reset();
                        s.parent = false;
                        if (below) {
                            below->getStats()->standingOn = e;
                            below->getStats()->parent = true;
                        }
                        below = e;
                    }
                    c->cells[c->cellIndex(x, y)] = below;
                }
            c->liveElems = ctx.getAll(cd.liveIds);
            c->toDeregister.assign(cd.toDeregister.begin(), cd.toDeregister.end());
        }

        // the file is good: swap the loaded world in
        clearWorld();
        for (auto &cd : chambers)
            chamber::allChambers.push_back(cd.c);
        chamber::lastid = lastChamberId;
        player::activePlayer = ctx.get(activePlayerId);
        player::visitedPlayers = ctx.getAll(visitedPlayerIds);
        goldenApple::apples = ctx.getAll(appleIds);
        goldenApple::appleNumber = appleNumber;
        {
            std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
            for (auto &t : ctx.getAll(teleporterIds))
                teleport::allTeleporters.push_back(std::dynamic_pointer_cast<teleport>(t));
            teleport::firstReceiverRemoved = firstReceiverRemoved;
        }
        bElem::toDispose = ctx.getAll(toDisposeIds);
        auto &vp = viewPoint::get_instance();
        vp._owner = ctx.get(viewOwnerId);
        for (auto &p : ctx.getAll(viewPointIds))
            vp.viewPoints.push_back(p);
        gameClock::ticks = taterCounter;
        std::istringstream rng(rngState);
        rng >> goe::rng::saved();

        // music of the global teleporters is attached to them when they are placed; redo that
        for (auto &[id, e] : ctx.byId) {
            auto t = std::dynamic_pointer_cast<teleport>(e);
            if (!t || t->getAttrs()->getSubtype() != 0 || !t->getBoard())
                continue;
            auto pos = t->getStats()->getMyPosition();
            soundManager::getInstance().setupSong(t->getStats()->getInstanceId(),
                                                   1,
                                                   {(float) pos.x, (float) pos.y, 0.0f},
                                                   t->getBoard()->getInstanceId(),
                                                   true);
            if (t->getStats()->getMyDirection() == dir::direction::LEFT)
                soundManager::getInstance().pauseSong(t->getStats()->getInstanceId());
        }
        if (player::activePlayer && player::activePlayer->getBoard())
            soundManager::getInstance().setListenerChamber(player::activePlayer->getBoard()->getInstanceId());
    } catch (const std::exception &ex) {
        {
            std::lock_guard<std::recursive_mutex> lock(teleport::registryMutex);
            teleport::allTeleporters = teleportersBefore;
        }
        goldenApple::apples = applesBefore;
        goldenApple::appleNumber = appleNumberBefore;
        std::cout << "Cannot load " << fileName << ": " << ex.what() << "\n";
        return false;
    }
    return true;
}
