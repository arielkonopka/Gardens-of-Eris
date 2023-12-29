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

#ifndef BAZOOKA_H
#define BAZOOKA_H
#include "commons.h"
#include <plainGun.h>
#include <bazookaMissile.h>
#include <viewPoint.h>

class bazooka : public plainGun
{
    public:
        bazooka();
        virtual ~bazooka()=default;
        virtual int getType() const;
        virtual std::shared_ptr<bElem> createProjectible(std::shared_ptr<bElem> who);
        virtual bool additionalProvisioning() final;
        virtual bool additionalProvisioning(int subtype,int typeId) final ;
        virtual bool additionalProvisioning(int subtype,std::shared_ptr<bazooka> sbe) final;
    protected:

    private:
};

#endif // BAZOOKA_H
