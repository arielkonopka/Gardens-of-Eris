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

#ifndef VIDEODRIVER_H
#define VIDEODRIVER_H
#include <allegro5/allegro5.h>
#include <vector>
#include <unordered_map>
#include "commons.h"
#include <memory>
#include <mutex>
#include "videoElementDef.h"
#include "configManager.h"
#include <allegro5/allegro_image.h>


class videoDriver;
using vDriver = std::shared_ptr<videoDriver>;
using vElement = std::shared_ptr<videoElement::videoElementDef>;

class videoDriver : public std::enable_shared_from_this<videoDriver>
{

public:
    static videoDriver* getInstance();
    virtual ~videoDriver();
    vElement getVideoElement(int typeId);

private:

    static videoDriver* myInstance;
    videoDriver();
    std::unordered_map<int,vElement> sprites;
    int lastSpriteT=-1;
    vElement lSprite;
    static std::once_flag _onceFlag;

};

#endif // VIDEODRIVER_H
