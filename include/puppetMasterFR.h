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
    bool collectOnAction(bool c, std::shared_ptr<bElem> who) override;

    /// called once, when the controller is handed to its body
    virtual void onAttach(std::shared_ptr<bElem> body);
    /// moves the body one step; called by the body when it is free to act
    virtual bool drive(std::shared_ptr<bElem> body);

protected:
    /// the default behaviour: go straight, turn at random or when blocked
    bool wander(std::shared_ptr<bElem> body);
    /// turns the body to face d and makes it wait, without moving
    void turn(std::shared_ptr<bElem> body, dir::direction d);
    /// moves the body in d, facing that way; returns false when blocked
    bool step(std::shared_ptr<bElem> body, dir::direction d);
    /// when the prey is right next to the body: turns to it, hurts it and rests; false otherwise
    bool bite(std::shared_ptr<bElem> body, std::shared_ptr<bElem> prey, int damage);
    /// the one of the four directions that points most directly from one cell to another
    static dir::direction towards(coords from, coords to);
    /**
     * first step of a shortest walk from the body to goal, over steppable cells, never leaving
     * the circle of radius around centre; NODIRECTION when there is no such walk.
     * Reaching a cell next to goal counts, so goal itself may be solid (a player, a wall).
     */
    static dir::direction pathTowards(std::shared_ptr<bElem> body, coords goal, coords centre, int radius);
    static int distance2(coords a, coords b) { return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y); }
    static dir::direction leftOf(dir::direction d) { return (dir::direction) (((int) d + 1) % 4); }
    static dir::direction rightOf(dir::direction d) { return (dir::direction) (((int) d + 3) % 4); }
    static dir::direction behind(dir::direction d) { return (dir::direction) (((int) d + 2) % 4); }
};

#endif // PUPPETMASTERFR_H
