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
#include "fnordEcho.h"
#include "bElem.h"

fnordEcho::fnordEcho()
    : fnordEcho(-1,
                0,
                myUtility::NOCOORDS,
                dir::direction::NODIRECTION,
                dir::direction::NODIRECTION,
                myUtility::NOCOORDS,
                dir::direction::NODIRECTION,
                dir::direction::NODIRECTION)
{}

fnordEcho::fnordEcho(const std::shared_ptr<bElem> fnord)
{
    if (!fnord || fnord->getStats()->isDisposed()) {
        this->disposed = true;
        return;
    }
    this->fId = fnord->getStats()->getInstanceId();
    this->fType = fnord->getType();
    this->previousPos = NOCOORDS;
    this->previousDir = dir::direction::NODIRECTION;
    this->previousFace = dir::direction::NODIRECTION;
    this->currentPos = fnord->getStats()->getMyPosition();
    this->currentDir = fnord->getStats()->getMyDirection();
    this->currentFace = fnord->getStats()->getFacing();
    this->fTime = bElem::getCntr();
    this->isSteppable = fnord->getAttrs()->isSteppable();
    this->isCollectible = fnord->getAttrs()->isCollectible();
    this->isWeapon = fnord->getAttrs()->isWeapon();
}

/**
 *
 * we setup ftime here, because we assume the object is created, that is the only moment, when it is done automagically, you need to refresh it, when converting from bElem
 */
fnordEcho::fnordEcho(int fType,
                     int fId,
                     const myUtility::Coords &currentPos,
                     dir::direction currentDir,
                     dir::direction currentFace,
                     const myUtility::Coords &previousPos,
                     dir::direction previousDir,
                     dir::direction previousFace)
    : fType(fType)
    , fId(fId)
    , currentPos(currentPos)
    , currentDir(currentDir)
    , currentFace(currentFace)
    , previousPos(previousPos)
    , previousDir(previousDir)
    , previousFace(previousFace)
    , fTime(bElem::getCntr())
{}

fnordEcho::fnordEcho(const fnordEcho &other)
    : fType(other.fType)
    , fId(other.fId)
    , currentPos(other.currentPos)
    , currentDir(other.currentDir)
    , currentFace(other.currentFace)
    , previousPos(other.previousPos)
    , previousDir(other.previousDir)
    , previousFace(other.previousFace)
    , fTime(other.fTime)
{}
