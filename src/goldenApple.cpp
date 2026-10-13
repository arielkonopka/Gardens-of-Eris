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
#include "goldenApple.h"

unsigned int goldenApple::appleNumber = 0;
std::mutex goldenApple::applesMutex;
std::vector<std::shared_ptr<bElem>> goldenApple::apples;
std::vector<unsigned long> goldenApple::parked;

int goldenApple::getType() const
{
    return bElemTypes::_goldenAppleType;
}

/// a shot apple is an apple no more: a refresher takes its cell
bool goldenApple::hurt(int points)
{
    const auto board = this->getBoard();
    const coords at = this->getStats()->getMyPosition();
    if (this->getStats()->isCollected() || !board || at == NOCOORDS || this->getStats()->isDestroying()
        || this->getStats()->isDisposed())
        return bElem::hurt(points);
    [[maybe_unused]] const auto self = shared_from_this(); // the board may hold the last reference
    this->disposeElement();
    auto fresh = elementFactory::generateAnElement<refresher>(board, 0);
    fresh->stepOnElement(board->getElement(at));
    return true;
}

bool goldenApple::destroy()
{
    if (!this->getStats()->isDestroying())
        return this->explode(2.5);
    return false;
}

bool goldenApple::additionalProvisioning(int subtype)
{
    if (!explosives::additionalProvisioning(subtype))
        return false;
    std::lock_guard<std::mutex> lock(goldenApple::applesMutex);
    if (subtype == 0) {
        goldenApple::apples.push_back(shared_from_this());
    }
    goldenApple::recount();
    return true;
}

oState goldenApple::disposeElement()
{
    this->forget();
    return bElem::disposeElement();
}

void goldenApple::forget()
{
    std::lock_guard<std::mutex> lock(goldenApple::applesMutex);
    const auto id = this->getStats()->getInstanceId();
    std::erase_if(goldenApple::apples, [id](const std::shared_ptr<bElem> &a) {
        return !a || a->getStats()->getInstanceId() == id;
    });
    goldenApple::recount();
}

int goldenApple::getAppleNumber()
{
    std::lock_guard<std::mutex> lock(goldenApple::applesMutex);
    return goldenApple::appleNumber;
}

std::shared_ptr<bElem> goldenApple::getApple(int num)
{
    std::lock_guard<std::mutex> lock(goldenApple::applesMutex);
    // the HUD asks for the first apple even when every apple is gone
    if (num < 0 || (std::size_t) num >= goldenApple::apples.size())
        return nullptr;
    return goldenApple::apples[num];
}

bool goldenApple::kill()
{
    return this->destroy();
}
void goldenApple::recount()
{
    goldenApple::appleNumber = (unsigned int) (goldenApple::apples.size() + goldenApple::parked.size());
}

void goldenApple::park(const std::vector<std::shared_ptr<bElem>> &elements)
{
    std::lock_guard<std::mutex> lock(goldenApple::applesMutex);
    for (const auto &e : elements) {
        const auto id = e->getStats()->getInstanceId();
        if (std::erase_if(goldenApple::apples, [id](const std::shared_ptr<bElem> &a) {
                return a && a->getStats()->getInstanceId() == id;
            }) > 0)
            goldenApple::parked.push_back(id);
    }
    goldenApple::recount();
}

void goldenApple::unpark(const std::vector<std::shared_ptr<bElem>> &elements)
{
    std::lock_guard<std::mutex> lock(goldenApple::applesMutex);
    for (const auto &e : elements) {
        const auto id = e->getStats()->getInstanceId();
        if (std::erase(goldenApple::parked, id) > 0)
            goldenApple::apples.push_back(e);
    }
    goldenApple::recount();
}
