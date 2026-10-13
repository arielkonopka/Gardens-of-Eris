/*
 * Copyright (c) 2026, Ariel Konopka
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

#include "spriteFrame.h"
#include "bElem.h"

namespace goe::sprites {
namespace {
std::optional<coords> phaseOf(const std::vector<coords> &sequence, int phase)
{
    if (sequence.empty())
        return std::nullopt;
    return sequence[(std::size_t) floorMod(phase, (int) sequence.size())];
}
} // namespace

std::optional<coords> frame(const bElem &elem, const videoElement::videoElementDef &def)
{
    const auto &stats = *elem.getStats();
    const int phase = elem.getAnimPh();
    // the floor never dies; everything else shows what happens to it first
    if (elem.getType() != bElemTypes::_floorType) {
        if (stats.isDying())
            return phaseOf(def.dying, phase);
        if (stats.isDestroying())
            return phaseOf(def.destroying, phase);
        if (stats.isTeleporting()) {
            if (stats.isFadingOut())
                return phaseOf(def.fadingOut, phase);
            if (stats.isFadingIn())
                return phaseOf(def.fadingIn, phase);
            return phaseOf(def.teleporting, phase);
        }
    }
    if (def.defArray.empty())
        return std::nullopt;
    const auto &kind = def.defArray[(std::size_t) floorMod(elem.getAttrs()->getSubtype(), (int) def.defArray.size())];
    if (kind.empty())
        return std::nullopt;
    // sprites are drawn for the four sides; one going diagonally looks the way it goes across
    const int facing = (int) dir::across(stats.getFacing());
    return phaseOf(kind[(std::size_t) floorMod(facing, (int) kind.size())], phase);
}
} // namespace goe::sprites
