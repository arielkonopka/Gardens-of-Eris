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

#ifndef MENUPAD_H
#define MENUPAD_H

#include "controlBindings.h"
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace goe::controls {

/// a stick counts as pushed past this, both in the game and in the menus
constexpr float stickDeadzone = 0.4f;
/// a pushed stick counts as let go only below this, so one resting near the deadzone does not flicker
constexpr float stickRelease = 0.25f;

/// what a key or pad press means in the menus
enum class menuCommand { none, up, down, left, right, confirm, back };

/**
 * @brief Turns pad buttons, the left stick and the d-pad into menu commands, without any Allegro
 * calls, so it can be tested.
 *
 * Confirm is the pad button of "Interact"; back is the button of "Drag" or of "Save and exit",
 * so rebinding them in Controls moves them in the menus too. A direction fires when it is pushed,
 * and again while it is held: after repeatDelay, then every repeatEvery.
 */
class menuPad
{
public:
    static constexpr double repeatDelay = 0.4;
    static constexpr double repeatEvery = 0.1;
    /// sticks we follow; pads have a few
    static constexpr int sticks = 8;

    /// the button that confirms (button 0 when Interact has none)
    static int confirmButton(const bindings &b);
    /// the button that goes back (button 1 when Drag has none and it is not the confirm button)
    static int backButton(const bindings &b);
    /// whether a stick moves the menus: the left stick (0) and the d-pad, which drivers report as
    /// a digital stick or name "DPad", "Hat" or "POV". The right stick and the triggers do not.
    static bool menuStick(int stick, bool digital, std::string_view name);
    /// a d-pad that a driver reports as buttons ("UP DPAD", "DPad Left", ...); none for others
    static menuCommand directionOf(std::string_view buttonName);

    /// a button went down: confirm, back or none. A d-pad button gives its direction, which repeats
    /// until buttonUp.
    menuCommand buttonDown(int button, const bindings &b, menuCommand direction = menuCommand::none);
    void buttonUp(int button);
    /// a stick axis moved (0 left and right, 1 up and down); the direction when one is newly pushed
    menuCommand axis(int stick, int axis, float pos);
    /// time passing; the held direction again when it is time to repeat it
    menuCommand wait(double seconds);
    /// forget what is held, for a screen that starts fresh
    void release();

private:
    menuCommand hold(int source, menuCommand d);
    void letGo(int source);

    struct stickState
    {
        float x = 0, y = 0;
        menuCommand pushed = menuCommand::none;
    };
    std::array<stickState, sticks> stickStates{};
    menuCommand held = menuCommand::none;
    int heldBy = -1; ///< the stick, or buttonSource + the button, holding the direction
    double heldFor = 0;
    double nextRepeat = repeatDelay;
    static constexpr int buttonSource = 100;
};

/// the letters a pad can enter, in the order Up steps through them
constexpr std::string_view nameLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .-_";

/**
 * A name entered with a pad, arcade style: Up and Down change the last letter, Right adds a letter,
 * Left takes the last one away. Letters typed on a keyboard mix in. False when nothing changed.
 */
bool editName(std::string &name, menuCommand c, std::size_t maxLetters);
/// letters (Unicode code points) in a UTF-8 name
std::size_t letterCount(const std::string &name);

} // namespace goe::controls

#endif // MENUPAD_H
