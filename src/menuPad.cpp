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

#include "menuPad.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace goe::controls {
namespace {
bool contains(std::string_view text, std::string_view word)
{
    auto lower = [](unsigned char c) { return (char) std::tolower(c); };
    std::string t(text.size(), ' '), w(word.size(), ' ');
    std::transform(text.begin(), text.end(), t.begin(), lower);
    std::transform(word.begin(), word.end(), w.begin(), lower);
    return t.find(w) != std::string::npos;
}

void popLetter(std::string &name)
{
    while (!name.empty() && (name.back() & 0xC0) == 0x80)
        name.pop_back();
    if (!name.empty())
        name.pop_back();
}
} // namespace

int menuPad::confirmButton(const bindings &b)
{
    const int bound = b.of(action::interact).padButton;
    return bound >= 0 ? bound : 0;
}

int menuPad::backButton(const bindings &b)
{
    const int bound = b.of(action::drag).padButton;
    if (bound >= 0)
        return bound;
    return confirmButton(b) == 1 ? 2 : 1;
}

bool menuPad::menuStick(int stick, bool digital, std::string_view name)
{
    return stick == 0 || digital || contains(name, "pad") || contains(name, "hat") || contains(name, "pov");
}

menuCommand menuPad::directionOf(std::string_view buttonName)
{
    if (!contains(buttonName, "pad") && !contains(buttonName, "hat") && !contains(buttonName, "pov"))
        return menuCommand::none;
    if (contains(buttonName, "up"))
        return menuCommand::up;
    if (contains(buttonName, "down"))
        return menuCommand::down;
    if (contains(buttonName, "left"))
        return menuCommand::left;
    if (contains(buttonName, "right"))
        return menuCommand::right;
    return menuCommand::none;
}

menuCommand menuPad::buttonDown(int button, const bindings &b, menuCommand direction)
{
    if (direction != menuCommand::none)
        return this->hold(buttonSource + button, direction);
    if (button == confirmButton(b))
        return menuCommand::confirm;
    if (button == backButton(b) || (button >= 0 && button == b.of(action::saveAndExit).padButton))
        return menuCommand::back;
    return menuCommand::none;
}

void menuPad::buttonUp(int button)
{
    this->letGo(buttonSource + button);
}

menuCommand menuPad::axis(int stick, int axis, float pos)
{
    if (stick < 0 || stick >= sticks || axis < 0 || axis > 1)
        return menuCommand::none;
    auto &s = this->stickStates[(std::size_t) stick];
    (axis == 0 ? s.x : s.y) = pos;
    // a pushed direction stays until the stick comes back under stickRelease along it
    auto along = [&s](menuCommand d) {
        switch (d) {
        case menuCommand::up: return -s.y;
        case menuCommand::down: return s.y;
        case menuCommand::left: return -s.x;
        case menuCommand::right: return s.x;
        default: return 0.0f;
        }
    };
    if (s.pushed != menuCommand::none && along(s.pushed) > stickRelease)
        return menuCommand::none;
    menuCommand now = menuCommand::none;
    if (std::max(std::abs(s.x), std::abs(s.y)) > stickDeadzone) {
        if (std::abs(s.y) >= std::abs(s.x))
            now = s.y < 0 ? menuCommand::up : menuCommand::down;
        else
            now = s.x < 0 ? menuCommand::left : menuCommand::right;
    }
    if (now == s.pushed)
        return menuCommand::none;
    s.pushed = now;
    if (now == menuCommand::none) {
        this->letGo(stick);
        return menuCommand::none;
    }
    return this->hold(stick, now);
}

menuCommand menuPad::hold(int source, menuCommand d)
{
    this->held = d;
    this->heldBy = source;
    this->heldFor = 0;
    this->nextRepeat = repeatDelay;
    return d;
}

void menuPad::letGo(int source)
{
    if (this->heldBy != source)
        return;
    this->held = menuCommand::none;
    this->heldBy = -1;
}

menuCommand menuPad::wait(double seconds)
{
    if (this->held == menuCommand::none)
        return menuCommand::none;
    this->heldFor += seconds;
    if (this->heldFor < this->nextRepeat)
        return menuCommand::none;
    this->nextRepeat += repeatEvery;
    return this->held;
}

void menuPad::release()
{
    this->stickStates = {};
    this->held = menuCommand::none;
    this->heldBy = -1;
}

std::size_t letterCount(const std::string &name)
{
    return (std::size_t) std::count_if(name.begin(), name.end(), [](char c) { return (c & 0xC0) != 0x80; });
}

bool editName(std::string &name, menuCommand c, std::size_t maxLetters)
{
    const int n = (int) nameLetters.size();
    switch (c) {
    case menuCommand::up:
    case menuCommand::down: {
        if (name.empty()) {
            name = nameLetters.front();
            return true;
        }
        // a letter typed on the keyboard that is not in the list starts over from the first one
        const auto at = (name.back() & 0x80) ? std::string_view::npos : nameLetters.find(name.back());
        const int i = at == std::string_view::npos ? (c == menuCommand::up ? -1 : 0) : (int) at;
        popLetter(name);
        name += nameLetters[(std::size_t) ((i + (c == menuCommand::up ? 1 : -1) + n) % n)];
        return true;
    }
    case menuCommand::right:
        if (letterCount(name) >= maxLetters)
            return false;
        name += nameLetters.front();
        return true;
    case menuCommand::left:
        if (name.empty())
            return false;
        popLetter(name);
        return true;
    default:
        return false;
    }
}

} // namespace goe::controls
