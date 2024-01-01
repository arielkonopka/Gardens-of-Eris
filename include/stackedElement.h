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

#ifndef STACKEDELEMENT_H
#define STACKEDELEMENT_H

#include <bElem.h>
#include <vector>
#include <memory>

/* element Factory must create all instances of the stacked element */
class stackedElement : public bElem
{
public:
    stackedElement();
    virtual ~stackedElement()=default;
    virtual bool stepOnElement(std::shared_ptr<bElem> step);
    virtual std::shared_ptr<bElem> removeElement();
    stackedElement(std::shared_ptr<chamber> board);
    void linkAnElement(std::shared_ptr<stackedElement> newBottom);
    void setController(std::shared_ptr<bElem> controller);
    std::shared_ptr<bElem> getController();
    bool additionalProvisioning(int st,std::shared_ptr<stackedElement> sbe);

protected:

private:
    std::shared_ptr<bElem> controlUnit=nullptr;
    std::vector<std::shared_ptr<stackedElement>> topDownConstruct;
};

#endif // STACKEDELEMENT_H
