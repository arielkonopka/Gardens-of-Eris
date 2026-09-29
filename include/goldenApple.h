
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


#ifndef GOLDENAPPLE_H
#define GOLDENAPPLE_H
#include "elements.h"
#include <vector>
#include <mutex>
#include "soundManager.h"
class goldenApple : public explosives
{
    friend class gameSerializer;
public:
    using bElem::additionalProvisioning;

    static std::shared_ptr<bElem> getApple(int num);
    static  int getAppleNumber();
    int getType() const override;
    bool kill() final;
    bool destroy() final;
    bool hurt(int points) override;
    bool interact(std::shared_ptr<bElem> who) override;
    bool mechanics() override;
    goldenApple()=default;
    oState disposeElement() final;
    bool additionalProvisioning(int subtype) final;
    bool collectOnAction(bool collected, std::shared_ptr<bElem> who) override;

private:
    /// drops this apple from the list of apples still out in the world
    void forget();
    static unsigned int appleNumber;
    static std::vector<std::shared_ptr<bElem>> apples;
    /// levels built in the background add apples while the game thread removes them
    static std::mutex applesMutex;
};

#endif // GOLDENAPPLE_H
