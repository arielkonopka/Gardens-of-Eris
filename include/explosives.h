
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
#ifndef EXPLOSIVES_H
#define EXPLOSIVES_H
#include <bElem.h>
#include "chamber.h"


class explosives : virtual public bElem
{
public:
    using bElem::additionalProvisioning;

    explosives()=default;
    virtual ~explosives()=default;
/**
 * @brief Handles the explosion logic for the explosive object.
 *
 * This function sets the explosion radius, checks preconditions,
 * and processes the explosion effect on the game board using
 * a cellular automata-based approach to determine the affected areas.
 *
 * @param radius The explosion radius.
 * @return True if the explosion logic was executed successfully, false otherwise.
 */
    bool explode(float radius);
    virtual float getViewRadius() const;
    virtual bool additionalProvisioning(int subtype);
private:
/**
 * @brief Recursively traverses the game board to resolve the shape of the explosion.
 *
 * The function uses cellular automata to determine which elements
 * are affected by the explosion based on their destroyability, position,
 * and distance from the explosion's center.
 *
 * @param center The center point of the explosion.
 * @param point The current point being evaluated during traversal.
 * @param radius The maximum allowable radius for the explosion's effect.
 * @return True if traversal is successful, false if conditions for stopping are met.
 */
    bool traverser(myUtility::Coords center, myUtility::Coords point, float radius,int plen,dir::direction noGo);
    std::shared_ptr<chamber> brd;
    int bx,by;
    float radius=1.5;

};

#endif // EXPLOSIVES_H
