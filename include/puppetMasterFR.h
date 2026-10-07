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
#ifndef PUPPETMASTERFR_H
#define PUPPETMASTERFR_H

#include <bElem.h>
#include "roam.h"

/**
 * @brief A controller: once handed to a patrolling drone, it decides how the drone moves.
 *
 * Every kind of controller is a subclass that overrides drive(). The kind is the element's
 * subtype, so the level generator, the save file and the sprite config all keep using the
 * single puppet master element type. To add a behaviour: subclass puppetMasterFR, override
 * drive() (and onAttach() if needed), add a kind below, and add it to create().
 *
 * The controlled element calls drive() from its own mechanics, so a controller runs wherever
 * its body is, and does not need to be registered as a live element itself.
 */
class puppetMasterFR : public bElem
{
    friend class gameSerializer;

public:
    /// kinds below looseKinds are placed in levels for the player to find; a guardian only
    /// ever comes with a security camera, and a hound is only sent after a player who lingers
    enum kind { patrol = 0, collector = 1, hunter = 2, wallFollower = 3, looseKinds = 4, guardian = 4, hound = 5, kindCount = 6 };

    /// creates the controller class that matches the subtype (the kind)
    static std::shared_ptr<puppetMasterFR> create(std::shared_ptr<chamber> board, int subtype);

    puppetMasterFR() = default;
    ~puppetMasterFR() override = default;
    int getType() const override;
    /// a drone never picks up a loose controller: controllers are only handed over by the player
    /// (patrollingDrone::interact), so to a drone a loose one is in the way like a wall
    bool collectibleBy(const bElem &who) const override;

    /// called once, when the controller is handed to its body
    virtual void onAttach(std::shared_ptr<bElem> body);
    /// moves the body one step; called by the body when it is free to act
    virtual bool drive(std::shared_ptr<bElem> body);
    /// where this controller last saw the player; NOCOORDS when it has no trail to follow
    coords getLastSeen() const { return this->lastSeen; }

protected:
    /// when the prey is right next to the body: turns to it, hurts it and rests; false otherwise
    bool bite(std::shared_ptr<bElem> body, std::shared_ptr<bElem> prey, int damage);
    /**
     * Chasers never cheat: they only know where the player is by seeing them. lookout() returns the
     * active player when they are within range of the body with nothing opaque in between
     * (goe::sight), and remembers that cell as lastSeen; nullptr otherwise.
     */
    std::shared_ptr<bElem> lookout(std::shared_ptr<bElem> body, int range);
    /**
     * walks towards lastSeen, around walls, never leaving the circle of radius around centre.
     * Next to that cell: faces the player when they are still in sight, otherwise they are not
     * where expected, and the trail goes cold. Returns false when there is no trail to follow
     * (cold, or no walk leads there), so the caller roams instead (goe::roam).
     */
    bool followTrail(std::shared_ptr<bElem> body, bool preyInSight, coords centre, int radius);
    /// where the player was last seen; NOCOORDS when the trail is cold
    coords lastSeen = NOCOORDS;
    /// the one of the four directions that points most directly from one cell to another
    static dir::direction towards(coords from, coords to);
    /**
     * first step of a shortest walk from the body to goal, over steppable cells, never leaving
     * the circle of radius around centre; NODIRECTION when there is no such walk.
     * Reaching a cell next to goal counts, so goal itself may be solid (a player, a wall).
     */
    static dir::direction pathTowards(std::shared_ptr<bElem> body, coords goal, coords centre, int radius);
    static int distance2(coords a, coords b) { return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y); }
};

#endif // PUPPETMASTERFR_H
