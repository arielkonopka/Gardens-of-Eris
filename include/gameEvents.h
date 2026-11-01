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


#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H
#include <functional>

class bElem;

/// What happens in the game that a watcher may want to count, such as an agent's reward. The
/// game itself does not listen: with no watcher set, report() does nothing.
namespace goe::events {
enum class kind : int {
    collect,  ///< subject was collected by actor
    use,      ///< actor used subject, the usable in its hand
    open,     ///< actor opened subject, a door (unlocked with a key, or opened)
    teleport, ///< actor was sent through subject, a teleporter
    kill,     ///< actor's missile or blast killed or destroyed subject
    activate, ///< actor woke subject, a spare avatar, so it joins the avatars to switch to
    harm,     ///< subject lost energy, or was killed or destroyed; actor is what did it (see harming)
};

/// actor may be null: nobody known did it
using observer = std::function<void(kind k, const bElem &subject, const bElem *actor)>;
/// one watcher at a time; an empty function clears it
void observe(observer o);
void report(kind k, const bElem &subject, const bElem *actor);
/// whether e is dying, being destroyed or gone
bool isDown(const bElem &e);

/// While a blame lives, kills are put down to `who` (a missile's or a blast's shooter): a weapon
/// hurting or destroying something holds one around the hit. Blames nest; null blames nobody.
class blame
{
public:
    explicit blame(const bElem *who);
    ~blame();
    blame(const blame &) = delete;
    blame &operator=(const blame &) = delete;

private:
    const bElem *before;
};
/// whom kills are put down to now; null outside any blame
const bElem *blamed();

/// While a harming lives, harm (kind::harm) is put down to `who`, the thing that does it: the
/// monster that bites, the bunker or monster whose missile hits, the bomb that blows up, the mine,
/// the beam. Unlike blame it names the hand, not only shooters; it is for watchers that count
/// what hurts the player, and changes nothing in the game. Harmings nest; null names nobody.
class harming
{
public:
    explicit harming(const bElem *who);
    ~harming();
    harming(const harming &) = delete;
    harming &operator=(const harming &) = delete;

private:
    const bElem *before;
};
/// what harm is put down to now; null outside any harming
const bElem *harmer();
} // namespace goe::events

#endif // GAMEEVENTS_H
