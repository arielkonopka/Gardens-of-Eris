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
#include "musicSongs.h"
#include "musicVocabulary.h"
#include <algorithm>
#include <cmath>

namespace goe::musician {

songbook::songbook(std::uint64_t seed)
    : choices(seed, 4)
{
}

song songbook::make(const performerPersonality &who, float variety, int slot, genre style)
{
    auto &r = this->choices;
    song s;
    s.id = ++this->nextId;
    s.slot = slot;
    if (style == genre::mixed)
        style = (genre) (1 + r.below(genreCount - 1)); // a mixed set: each song a style of its own
    s.style = style;
    const genreRules &rules = rulesOf(style);
    // the further the variety, the further from home: a related key, another mode, another tempo
    static constexpr int keys[] = {5, -5, 7, -2, 3, -3, 2};
    s.keyShift = this->started > 0 && r.chance(0.9f * variety) ? keys[r.below(7)] : 0;
    static constexpr mode modes[] = {mode::aeolian, mode::ionian, mode::dorian, mode::mixolydian};
    s.scale = this->started > 0 && r.chance(0.6f * variety) ? modes[r.below(4)] : who.homeMode;
    if (rules.modeCount > 0) // the style's modes, its first the likeliest when the variety is low
        s.scale = rules.modes[(std::size_t) (r.chance(0.4f + 0.6f * variety) ? r.below(rules.modeCount) : 0)];
    // a new tempo that is heard as one: away from the song before, unless the variety is low
    const float before = this->count > 0 ? this->current().tempo : who.baseTempo;
    for (int tries = 0; tries < 5; tries++) {
        if (rules.maxTempo > 0.0f) {
            // the style's range: near its middle at low variety, anywhere in it at high
            const float middle = 0.5f * (rules.minTempo + rules.maxTempo), half = 0.5f * (rules.maxTempo - rules.minTempo);
            s.tempo = middle + half * (0.3f + 0.7f * variety) * r.between(-1.0f, 1.0f);
        } else {
            s.tempo = who.baseTempo * (1.0f + tuning::songTempoSpread * variety * r.between(-1.0f, 1.0f));
        }
        if (std::fabs(s.tempo - before) >= 0.1f * variety * before)
            break;
    }
    // calm performers lean to the plain grooves, energetic ones to the busy ones; a style has its own
    const int grooves = vocabulary::freeGrooves;
    const float lean = std::clamp(0.6f * who.traits.energy + 0.4f * r.unit(), 0.0f, 0.999f);
    s.groove = r.chance(0.5f + 0.5f * variety) ? r.below(grooves) : std::min((int) (lean * (float) grooves), grooves - 1);
    const int styleGrooves = (int) std::count_if(rules.grooves.begin(), rules.grooves.end(), [](int g) { return g >= 0; });
    if (styleGrooves > 0)
        s.groove = rules.grooves[(std::size_t) r.below(styleGrooves)];
    const auto &g = vocabulary::grooves[(std::size_t) s.groove];
    s.swing = g.swing > 0.0f ? g.swing : (r.chance(0.3f * variety) ? r.between(0.1f, 0.2f) : 0.0f);
    s.swing = std::min(s.swing, tuning::maxSwing);
    if (g.name == "half time" && rules.maxTempo <= 0.0f)
        s.tempo *= 1.15f; // half time feels slow, so it is counted a little faster
    s.tempo = std::clamp(s.tempo, tuning::minTempo, tuning::maxTempo);
    const float phrases = (float) tuning::longestSong + ((float) tuning::shortestSong - (float) tuning::longestSong) * variety;
    s.length = std::max(3, (int) std::lround(phrases * rules.lengthScale) + r.below(3) - 1);
    s.energyLean = 0.25f * variety * r.between(-1.0f, 1.0f);
    s.articulationLean = 0.3f * variety * r.between(-1.0f, 1.0f);
    if (rules.progressions[0] >= 0) {
        // three of the style's progressions (they may repeat: a style may have few)
        for (auto &p : s.progressions)
            p = (std::uint8_t) rules.progressions[(std::size_t) r.below((int) rules.progressions.size())];
        return s;
    }
    // three favourite progressions, all different
    const int n = vocabulary::freeProgressions;
    s.progressions[0] = (std::uint8_t) r.below(n);
    s.progressions[1] = (std::uint8_t) ((s.progressions[0] + 1 + r.below(n - 1)) % n);
    do
        s.progressions[2] = (std::uint8_t) r.below(n);
    while (s.progressions[2] == s.progressions[0] || s.progressions[2] == s.progressions[1]);
    return s;
}

section songbook::sectionOf(int k, const song &s) const
{
    if (k <= 0)
        return this->cameBack ? section::chorus : section::intro; // a returning song comes straight in
    if (k >= s.length - 1)
        return section::outro;
    if (s.length >= 7 && k == (int) std::lround(0.6f * (float) s.length))
        return section::breakdown;
    static constexpr section cycle[] = {section::verse, section::verse, section::chorus, section::chorus};
    return cycle[(k - 1) % 4];
}

phrasePlan songbook::next(const performerPersonality &who, const musicalState &tension, situation now, float variety,
                          genre style)
{
    auto &r = this->choices;
    variety = std::clamp(std::isfinite(variety) ? variety : 0.5f, 0.0f, 1.0f);
    if ((int) style >= genreCount)
        style = genre::mixed;
    phrasePlan plan;
    // a song of another style than the one chosen ends here: the chosen style starts at once
    const bool wrongStyle = this->count > 0 && style != genre::mixed && this->current().style != style;
    if (this->count == 0 || this->phrase >= this->current().length || wrongStyle) {
        // an earlier song may come back: any but the one just played, of the chosen style
        std::array<int, tuning::songMemory> candidates{};
        int found = 0;
        for (int c = 0; c < this->count; c++)
            if (c != this->playing && (style == genre::mixed || this->songs[(std::size_t) c].style == style))
                candidates[(std::size_t) found++] = c;
        const float back = tuning::returnAtLowVariety + (tuning::returnAtHighVariety - tuning::returnAtLowVariety) * variety;
        this->cameBack = found > 0 && r.chance(back);
        if (this->cameBack) {
            this->playing = candidates[(std::size_t) r.below(found)];
            this->returned++;
            this->freshNext = false;
        } else {
            // a new song, in the slot of the oldest one when the memory is full
            const int slot = this->oldest;
            this->oldest = (this->oldest + 1) % tuning::songMemory;
            this->songs[(std::size_t) slot] = this->make(who, variety, slot, style);
            this->count = std::min(this->count + 1, tuning::songMemory);
            this->playing = slot;
            this->freshNext = true;
        }
        this->started++;
        this->phrase = 0;
        plan.songStart = true;
    }
    const song &s = this->current();
    plan.songId = s.id;
    plan.slot = s.slot;
    plan.freshSlot = this->freshNext;
    this->freshNext = false;
    plan.phraseInSong = this->phrase;
    plan.part = this->sectionOf(this->phrase, s);
    plan.groove = s.groove;
    plan.swing = s.swing;
    plan.progressions = s.progressions;
    plan.style = s.style;
    const genreRules &rules = rulesOf(s.style);

    // the intensity: the tension, then the section, then how much danger the player is in
    static constexpr float sectionLift[] = {-0.1f, 0.0f, 0.15f, -0.2f, 0.0f};
    const float situationLift = now == situation::danger ? tuning::dangerIntensity
                                : now == situation::alert ? tuning::alertIntensity
                                                          : 0.0f;
    plan.intensity = std::clamp(0.6f * tension.density + 0.4f * tension.rhythm + sectionLift[(int) plan.part]
                                    + situationLift,
                                0.0f, 1.0f);
    static constexpr int sectionDrums[] = {1, 1, 2, 0, 1};
    plan.drums = sectionDrums[(int) plan.part] + (plan.intensity > 0.35f ? 1 : 0) + (plan.intensity > 0.6f ? 1 : 0);
    plan.densityLift = plan.part == section::chorus ? 0.1f : 0.0f;
    switch (plan.part) {
    case section::intro:
        plan.lead = r.chance(0.5f); // often the band plays the groove first and the melody joins later
        break;
    case section::verse:
        plan.lead = r.chance(rules.leadChance); // some styles leave whole verses to the groove
        break;
    case section::breakdown:
        // the drums drop out under the melody, or the melody drops out over the drums and bass
        if (r.chance(0.5f)) {
            plan.drums = 0;
        } else {
            plan.lead = false;
            plan.chords = false;
            plan.drums = std::max(plan.drums, 2);
        }
        break;
    default:
        break;
    }
    if (plan.drums > 0 || plan.part != section::breakdown) // the style's drums, breakdowns aside
        plan.drums = std::clamp(plan.drums, rules.drumFloor, rules.drumCeiling);
    if (now == situation::danger)
        plan.drums = std::max(plan.drums, 2); // under danger the drums never leave
    plan.drums = std::clamp(plan.drums, 0, 3);
    // a fill leads into a section that differs, and into the next song
    plan.fill = plan.drums > 0 && (this->phrase + 1 >= s.length || this->sectionOf(this->phrase + 1, s) != plan.part);
    this->phrase++;
    return plan;
}

performerPersonality songbook::dressed(const performerPersonality &who) const
{
    performerPersonality p = who;
    if (this->count == 0)
        return p;
    const song &s = this->current();
    p.keyRoot = who.keyRoot + s.keyShift;
    // the chords' register stays between A2 and E3 whatever the key
    while (p.keyRoot > 52)
        p.keyRoot -= 12;
    while (p.keyRoot < 41)
        p.keyRoot += 12;
    p.homeMode = s.scale;
    p.baseTempo = s.tempo;
    p.articulation = std::clamp(who.articulation + s.articulationLean, 0.0f, 1.0f);
    // more energy only while the busy traits stay within budget
    const float room = tuning::busynessBudget - (who.busyness() - who.energy);
    p.energy = std::clamp(who.energy + s.energyLean, 0.0f, std::max(0.0f, std::min(1.0f, room)));
    return p;
}

} // namespace goe::musician
