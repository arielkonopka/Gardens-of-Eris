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

#ifndef PLAYER_H
#define PLAYER_H
#include "commons.h"
#include "inputManager.h"
#include "objectTypes.h"
#include "videoElementDef.h"
#include "plainGun.h"
#include "soundManager.h"
#include "viewPoint.h"

class player : public bElem
{
public:
    using bElem::additionalProvisioning;

    player()=default;

    ~player() final=default;
    static unsigned int countVisitedPlayers() ;
    static std::shared_ptr<bElem> getActivePlayer();
    bool stepOnElement(std::shared_ptr<bElem> step);
    bool mechanics() final;
    bool interact(std::shared_ptr<bElem> who) final;
    int getType() const;
    int getAnimPh() const;
    bool shootGun();
    oState disposeElement() final;
    float getViewRadius() const;
    bool additionalProvisioning(int subtype,std::shared_ptr<player> sbe) ;

private:
    float vRadius=2;
    static std::shared_ptr<bElem> activePlayer;
    static std::vector<std::shared_ptr<bElem>> visitedPlayers;
    int animPh=0;
    bool activated=false;
    bool provisioned=false;
    int mysound;
};

#endif // PLAYER_H
