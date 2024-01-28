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

#ifndef VIEWPOINT_H
#define VIEWPOINT_H
#include "commons.h"
#include "elements.h"
#include <mutex>
#include <memory>
#include <ranges>
class vpPoint
{
public:
    vpPoint()=default;
    ~vpPoint()=default;
    int x;
    int y;
    float radius;

};

class viewPoint
{
public:
    static viewPoint* get_instance();
    void setOwner(std::shared_ptr<bElem> owner);
    void addViewPoint(std::shared_ptr<bElem> vp);
    std::shared_ptr<bElem> getOwner();
    ~viewPoint()=delete;
    viewPoint()=default;
    coords getViewPoint();
    coords getViewPointOffset();
    int calculateObscured(coords point);
    int calculateObscured(coords point,int divider);
    std::vector<vpPoint> getViewPoints(coords start, coords end);
    bool isPointVisible(coords point);

protected:

private:
    bool isElementInVector(const std::vector<std::weak_ptr<bElem>>& vec, const std::shared_ptr<bElem>& elem);
    static viewPoint* instance;
    std::weak_ptr<bElem> _owner;
    std::vector<std::weak_ptr<bElem>> viewPoints;
    static std::once_flag once;
};


#endif // VIEWPOINT_H
