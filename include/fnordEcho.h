/*
 * Copyright (c) 2025, Ariel Konopka
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
#ifndef FNORDECHO_H
#define FNORDECHO_H

#include "Coords.h"
#include "bElem.h"
#include "commons.h"
/**
     * @brief The fnordEcho class
     * Abstraction class that translates from bElem, to avoid any issues with elements being removed from the board. it contains lastSeen value
     * @param fTime - last seen time. this will be used to strip the fnordEcho from fnordMap in fnordNavigator
     */
class fnordEcho
{
public:
    fnordEcho();
    fnordEcho(const std::shared_ptr<bElem> fnord);
    fnordEcho(const fnordEcho &other);
    fnordEcho(int fType,
              int fId,
              const myUtility::Coords &currentPos,
              dir::direction currentDir,
              dir::direction currentFace,
              const myUtility::Coords &previousPos,
              dir::direction previousDir,
              dir::direction previousFace);
    int fType;
    int fSubtype;
    int fScore = -6502;
    /**
 * @brief fnordEcho::operator <=> 
 * @param other
 * We compare fnordEchos by their scoring
 */
    auto operator<=>(const fnordEcho &other) const { return fScore <=> other.fScore; }

    bool hasInventory = false;
    bool hasGun = false;
    bool isSteppable = true;
    bool isCollectible = false;
    bool isWeapon = false;
    bool disposed = false;
    bool fnordMapped = false;
    unsigned long int fId = 0;
    myUtility::Coords currentPos;
    dir::direction currentDir;
    dir::direction currentFace;
    myUtility::Coords previousPos;
    dir::direction previousDir;
    dir::direction previousFace;
    unsigned long int fTime; // When the information was written in taterCounter Time
};

#endif
