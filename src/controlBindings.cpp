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

#include "controlBindings.h"
#include <algorithm>

namespace goe::controls {
namespace {
struct actionInfo
{
    const char *id;
    const char *label;
    binding defaults;
};

const std::array<actionInfo, actionCount> &infos()
{
    static const std::array<actionInfo, actionCount> all = {{
        {"up", "Walk up", {{ALLEGRO_KEY_UP, ALLEGRO_KEY_W}, -1}},
        {"down", "Walk down", {{ALLEGRO_KEY_DOWN, ALLEGRO_KEY_S}, -1}},
        {"left", "Walk left", {{ALLEGRO_KEY_LEFT, ALLEGRO_KEY_A}, -1}},
        {"right", "Walk right", {{ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_D}, -1}},
        {"shoot", "Shoot (+ direction)", {{ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_RSHIFT}, 5}},
        {"interact", "Interact, pick up (+ direction)", {{ALLEGRO_KEY_LCTRL, ALLEGRO_KEY_RCTRL}, 0}},
        {"drag", "Drag (+ direction)", {{ALLEGRO_KEY_ALT, ALLEGRO_KEY_ALTGR}, 1}},
        {"nextItem", "Next usable item", {{ALLEGRO_KEY_X}, 3}},
        {"nextGun", "Next kind of gun", {{ALLEGRO_KEY_Z}, 2}},
        {"use", "Use the item", {{ALLEGRO_KEY_SPACE}, 4}},
        {"drop", "Drop the item", {{ALLEGRO_KEY_R}, 8}},
        {"giveUp", "Give up this avatar", {{ALLEGRO_KEY_BACKSPACE}, 9}},
        {"saveAndExit", "Save and exit to the menu", {{ALLEGRO_KEY_ESCAPE, ALLEGRO_KEY_F10}, 7}},
    }};
    return all;
}
} // namespace

bindings::bindings()
{
    for (int c = 0; c < actionCount; c++)
        this->table[c] = infos()[c].defaults;
}

const binding &bindings::of(action a) const
{
    return this->table[(int) a];
}

void bindings::bindKey(action a, int keycode)
{
    if (keycode <= 0 || keycode >= ALLEGRO_KEY_MAX)
        return;
    for (auto &b : this->table)
        std::erase(b.keys, keycode);
    auto &keys = this->table[(int) a].keys;
    keys.insert(keys.begin(), keycode);
    if (keys.size() > (size_t) maxKeys)
        keys.resize(maxKeys);
}

void bindings::bindPadButton(action a, int button)
{
    if (button < 0 || button >= padButtons)
        return;
    for (auto &b : this->table)
        if (b.padButton == button)
            b.padButton = -1;
    this->table[(int) a].padButton = button;
}

void bindings::clear(action a)
{
    this->table[(int) a] = {};
}

bool bindings::held(const inputState &in, action a) const
{
    const auto &b = this->of(a);
    if (std::any_of(b.keys.begin(), b.keys.end(), [&in](int k) { return in.keys[k]; }))
        return true;
    if (b.padButton >= 0 && in.pad[b.padButton])
        return true;
    return a <= action::right && in.stick[(int) a];
}

controlItem bindings::translate(const inputState &in) const
{
    int type = -1;
    dir::direction d = dir::direction::NODIRECTION;
    // the later direction wins when several are held, as it always did
    const std::pair<action, dir::direction> walks[] = {{action::up, dir::direction::UP},
                                                       {action::down, dir::direction::DOWN},
                                                       {action::left, dir::direction::LEFT},
                                                       {action::right, dir::direction::RIGHT}};
    for (auto [a, wd] : walks)
        if (this->held(in, a)) {
            type = 0;
            d = wd;
        }
    if (this->held(in, action::shoot) && type >= 0)
        type = 1;
    if (this->held(in, action::interact) && type >= 0)
        type = 2;
    if (this->held(in, action::nextItem))
        type = 3;
    if (this->held(in, action::drag) && type >= 0)
        type = 4;
    if (this->held(in, action::nextGun))
        type = 5;
    if (this->held(in, action::giveUp))
        type = this->held(in, action::shoot) ? 7 : 6; // with the shoot key: quit the game
    if (this->held(in, action::use))
        type = 8;
    if (this->held(in, action::drop))
        type = 9;
    if (this->held(in, action::saveAndExit))
        type = 10; // wins over everything, so leaving never costs an avatar
    return controlItem(type, d);
}

std::string bindings::label(action a)
{
    return infos()[(int) a].label;
}

std::string bindings::id(action a)
{
    return infos()[(int) a].id;
}

std::optional<action> bindings::fromId(const std::string &id)
{
    for (int c = 0; c < actionCount; c++)
        if (id == infos()[c].id)
            return (action) c;
    return std::nullopt;
}

std::string bindings::keyName(int keycode)
{
    if (keycode >= ALLEGRO_KEY_A && keycode <= ALLEGRO_KEY_Z)
        return std::string(1, (char) ('A' + keycode - ALLEGRO_KEY_A));
    if (keycode >= ALLEGRO_KEY_0 && keycode <= ALLEGRO_KEY_9)
        return std::string(1, (char) ('0' + keycode - ALLEGRO_KEY_0));
    if (keycode >= ALLEGRO_KEY_PAD_0 && keycode <= ALLEGRO_KEY_PAD_9)
        return "Pad " + std::to_string(keycode - ALLEGRO_KEY_PAD_0);
    if (keycode >= ALLEGRO_KEY_F1 && keycode <= ALLEGRO_KEY_F12)
        return "F" + std::to_string(keycode - ALLEGRO_KEY_F1 + 1);
    switch (keycode) {
    case ALLEGRO_KEY_UP: return "Up";
    case ALLEGRO_KEY_DOWN: return "Down";
    case ALLEGRO_KEY_LEFT: return "Left";
    case ALLEGRO_KEY_RIGHT: return "Right";
    case ALLEGRO_KEY_LSHIFT: return "Left Shift";
    case ALLEGRO_KEY_RSHIFT: return "Right Shift";
    case ALLEGRO_KEY_LCTRL: return "Left Ctrl";
    case ALLEGRO_KEY_RCTRL: return "Right Ctrl";
    case ALLEGRO_KEY_ALT: return "Alt";
    case ALLEGRO_KEY_ALTGR: return "AltGr";
    case ALLEGRO_KEY_LWIN: return "Left Win";
    case ALLEGRO_KEY_RWIN: return "Right Win";
    case ALLEGRO_KEY_MENU: return "Menu";
    case ALLEGRO_KEY_SPACE: return "Space";
    case ALLEGRO_KEY_ENTER: return "Enter";
    case ALLEGRO_KEY_PAD_ENTER: return "Pad Enter";
    case ALLEGRO_KEY_ESCAPE: return "Esc";
    case ALLEGRO_KEY_TAB: return "Tab";
    case ALLEGRO_KEY_BACKSPACE: return "Backspace";
    case ALLEGRO_KEY_INSERT: return "Insert";
    case ALLEGRO_KEY_DELETE: return "Delete";
    case ALLEGRO_KEY_HOME: return "Home";
    case ALLEGRO_KEY_END: return "End";
    case ALLEGRO_KEY_PGUP: return "Page Up";
    case ALLEGRO_KEY_PGDN: return "Page Down";
    case ALLEGRO_KEY_MINUS: return "-";
    case ALLEGRO_KEY_EQUALS: return "=";
    case ALLEGRO_KEY_OPENBRACE: return "[";
    case ALLEGRO_KEY_CLOSEBRACE: return "]";
    case ALLEGRO_KEY_SEMICOLON: return ";";
    case ALLEGRO_KEY_QUOTE: return "'";
    case ALLEGRO_KEY_BACKSLASH: return "\\";
    case ALLEGRO_KEY_COMMA: return ",";
    case ALLEGRO_KEY_FULLSTOP: return ".";
    case ALLEGRO_KEY_SLASH: return "/";
    case ALLEGRO_KEY_TILDE: return "~";
    case ALLEGRO_KEY_PAD_SLASH: return "Pad /";
    case ALLEGRO_KEY_PAD_ASTERISK: return "Pad *";
    case ALLEGRO_KEY_PAD_MINUS: return "Pad -";
    case ALLEGRO_KEY_PAD_PLUS: return "Pad +";
    case ALLEGRO_KEY_PAD_DELETE: return "Pad Del";
    default: return "Key " + std::to_string(keycode);
    }
}

std::string bindings::describe(action a) const
{
    const auto &b = this->of(a);
    std::string s;
    for (int k : b.keys)
        s += (s.empty() ? "" : ", ") + keyName(k);
    if (b.padButton >= 0)
        s += (s.empty() ? "" : ", ") + std::string("pad button ") + std::to_string(b.padButton);
    if (a <= action::right)
        s += (s.empty() ? "" : ", ") + std::string("pad stick");
    return s.empty() ? "none" : s;
}

} // namespace goe::controls
