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
#ifndef REFRESHER_H
#define REFRESHER_H

#include "explosives.h"

/**
 * @brief A refresher: what a golden apple becomes when it is shot.
 *
 * Carried, it gives its carrier 5 energy now and then while the carrier has less than 100, and
 * loses 5 itself each time; using it (interact) does the same at once. Drained, it blows up where
 * its carrier stands and takes the carrier with it. It is not an apple: it does not count towards
 * the apples left in the world.
 */
class refresher : public explosives
{
    friend class gameSerializer;

public:
    using bElem::additionalProvisioning;

    refresher() = default;
    ~refresher() override = default;
    int getType() const override;
    bool kill() final;
    bool destroy() final;
    bool interact(std::shared_ptr<bElem> who) override;
    bool mechanics() override;
    bool collectOnAction(bool collected, std::shared_ptr<bElem> who) override;

    /// energy given (and lost) at a time
    static constexpr int dose = 5;
    /// a carrier with this much energy or more is not fed
    static constexpr int fullUp = 100;
    /// ticks between two doses
    static constexpr int pause = 55;

private:
    /// gives who a dose, if there is any left
    void feed(bElem &who);
};

#endif // REFRESHER_H
