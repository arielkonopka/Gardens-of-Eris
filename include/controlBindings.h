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
#ifndef CONTROLBINDINGS_H
#define CONTROLBINDINGS_H

#include "commons.h"
#include <allegro5/keycodes.h>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace goe::controls {

/// what the player can do; each one is bound to keys and a pad button
enum class action { up, down, left, right, shoot, interact, drag, nextItem, nextGun, use, drop, giveUp, saveAndExit, count };
constexpr int actionCount = (int) action::count;
/// pad buttons we keep track of
constexpr int padButtons = 32;

/// the keys and pad button that trigger one action
struct binding
{
    std::vector<int> keys; ///< Allegro key codes, the main one first
    int padButton = -1;    ///< -1 when no pad button is bound

    bool operator==(const binding &) const = default;
};

/// what is held down right now, on the keyboard and on the pad
struct inputState
{
    std::array<bool, ALLEGRO_KEY_MAX> keys{};
    std::array<bool, padButtons> pad{};
    /// the pad's stick, in the order up, down, left, right
    std::array<bool, 4> stick{};
};

/**
 * @brief Which keys and pad buttons do what. The Config menu edits it and settings.json keeps it.
 *
 * Directions also follow the pad's stick; that is not configurable. A key belongs to one action
 * only, so binding it takes it away from any other action.
 */
class bindings
{
public:
    static constexpr int maxKeys = 2;

    bindings(); ///< the default layout (arrows and W A S D, Shift, Ctrl, Alt, X, Z, Space, R, Backspace, Esc)

    const binding &of(action a) const;
    /// the key becomes the action's main key; the old main key stays as the spare one
    void bindKey(action a, int keycode);
    void bindPadButton(action a, int button);
    /// the action gets no keys and no pad button
    void clear(action a);

    /// what the held keys and buttons ask the player to do (see controlItem)
    controlItem translate(const inputState &in) const;

    /// "Walk up", for menus
    static std::string label(action a);
    /// "up", for settings.json
    static std::string id(action a);
    static std::optional<action> fromId(const std::string &id);
    /// "Up", "W", "Left Shift", ...
    static std::string keyName(int keycode);
    /// "Up, W, pad 3"
    std::string describe(action a) const;

    bool operator==(const bindings &) const = default;

private:
    bool held(const inputState &in, action a) const;
    std::array<binding, actionCount> table;
};

} // namespace goe::controls

#endif // CONTROLBINDINGS_H
