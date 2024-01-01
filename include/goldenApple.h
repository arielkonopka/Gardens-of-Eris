
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
#include "soundManager.h"
class goldenApple : public explosives
{
public:
    static std::shared_ptr<bElem> getApple(int num);
    static  int getAppleNumber();
    int getType() const;
    bool kill() final;
    bool destroy() final;
    bool hurt(int points);
    explicit goldenApple(std::shared_ptr<chamber> board);
    bool interact(std::shared_ptr<bElem> who);
    bool mechanics();
    goldenApple();
    oState disposeElement() final;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<goldenApple> sbe);
    bool collectOnAction(bool collected, std::shared_ptr<bElem> who);

private:
    static unsigned int appleNumber;
    static std::vector<std::shared_ptr<bElem>> apples;
};

#endif // GOLDENAPPLE_H
