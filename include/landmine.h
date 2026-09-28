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
#ifndef LANDMINE_H
#define LANDMINE_H

#include "simpleBomb.h"

/**
 * @brief A landmine: a steppable tile that looks almost like floor.
 *
 * Whatever steps on it (the player, a monster, a drone, even a missile) sets it off, and it
 * explodes on the next tick. Like a bomb, another explosion sets it off too. The level generator
 * places more of them in deeper chambers (difficulty::landmineCopies).
 */
class landmine : public simpleBomb
{
    friend class gameSerializer;

public:
    using bElem::additionalProvisioning;

    landmine() = default;
    ~landmine() override = default;
    int getType() const override;
    bool stepOnAction(bool step, std::shared_ptr<bElem> who) override;
    /// explodes like a bomb, and also takes whatever stands on the mine with it
    bool mechanics() override;

protected:
    int fuse() const override { return 1; }
};

#endif // LANDMINE_H
