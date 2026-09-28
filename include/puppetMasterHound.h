/*
 * Copyright (c) 2026, Ariel Konopka
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
#ifndef PUPPETMASTERHOUND_H
#define PUPPETMASTERHOUND_H

#include "puppetMasterFR.h"

/**
 * @brief The Hound: sent after a player who stays too long in one area.
 *
 * watch() runs once per tick. When the active player has stayed in the same area
 * (difficulty::areaOf) for longer than difficulty::houndPatience, a drone driven by a hound
 * appears a short way off. The hound chases the player around walls and bites when it reaches
 * them, for as long as they stay in that area; once they leave it, the drone dies. While one hound is out no other is sent, and the
 * patience starts over after each one, so camping keeps being punished.
 */
class puppetMasterHound : public puppetMasterFR
{
    friend class gameSerializer;

public:
    /// how far around itself the hound searches for a way to the player
    static constexpr int searchRadius = 55;
    /// how far from the player a hound appears
    static constexpr int spawnDistance = 15;
    /// how much a bite hurts
    static constexpr int biteDamage = 5;

    bool drive(std::shared_ptr<bElem> body) override;
    /// checks whether the active player is camping, and sends a hound when they are
    static void watch();

private:
    /// sends a hound after the player; returns the drone, or nullptr when there was no room
    static std::shared_ptr<bElem> release(const std::shared_ptr<bElem> &prey);
    /// the area the hound guards; taken from the player when the hound first moves
    coords home = NOCOORDS;
};

#endif // PUPPETMASTERHOUND_H
