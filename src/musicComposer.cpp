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
#include "musicComposer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace goe::musician {

using vocabulary::stepsPerBar;

namespace {
/// a scale degree inside one octave (0..6), also for negative degrees
int wrapDegree(int d)
{
    return ((d % 7) + 7) % 7;
}

int floorDiv7(int d)
{
    return d >= 0 ? d / 7 : -((6 - d) / 7);
}

/// is this sixteenth of the bar a note in the pattern
bool plays(std::string_view pattern, int step)
{
    return step >= 0 && step < (int) pattern.size() && pattern[(std::size_t) step] == 'x';
}
} // namespace

int motif::notes() const
{
    return std::min(vocabulary::onsets(vocabulary::leadRhythms[this->rhythm[0]])
                        + vocabulary::onsets(vocabulary::leadRhythms[this->rhythm[1]]),
                    tuning::motifNotes);
}

bool phraseBuffer::add(const noteEvent &e)
{
    if (this->count >= (int) this->events.size())
        return false;
    noteEvent placed = e;
    placed.order = (std::uint32_t) this->count; // keeps the written order of events at the same sample
    this->events[(std::size_t) this->count++] = placed;
    return true;
}

void phraseBuffer::clear()
{
    this->count = 0;
    this->length = this->barLength = 0;
    this->bars = 0;
}

composer::composer(std::uint64_t seed)
    : phraseChoices(seed, 1)
    , eventChoices(seed, 2)
{
}

int composer::pitchOf(int degree, const performerPersonality &who, const vocabulary::theme &th, int octave) const
{
    const int *scale = scaleOf(th.scale);
    return who.keyRoot + th.transpose + 12 * (octave + floorDiv7(degree)) + scale[wrapDegree(degree)];
}

bool composer::inScale(int pitch, const performerPersonality &who, const vocabulary::theme &th) const
{
    const int *scale = scaleOf(th.scale);
    const int pc = ((pitch - who.keyRoot - th.transpose) % 12 + 12) % 12;
    return std::find(scale, scale + 7, pc) != scale + 7;
}

int composer::pickRhythm(float notesPerBar, float offBeat)
{
    // the three patterns closest to the wanted density and syncopation; the closest is likeliest
    std::array<std::pair<float, int>, vocabulary::leadRhythms.size()> scored{};
    for (int c = 0; c < (int) vocabulary::leadRhythms.size(); c++) {
        const auto p = vocabulary::leadRhythms[(std::size_t) c];
        scored[(std::size_t) c] = {std::fabs((float) vocabulary::onsets(p) - notesPerBar) / 4.0f
                                       + 2.0f * std::fabs(vocabulary::syncopation(p) - offBeat),
                                   c};
    }
    std::partial_sort(scored.begin(), scored.begin() + 3, scored.end());
    const float u = this->phraseChoices.unit();
    return scored[u < 0.6f ? 0 : (u < 0.9f ? 1 : 2)].second;
}

motif composer::fresh(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th)
{
    auto &r = this->phraseChoices;
    const float perBeat = std::clamp(0.6f + 1.1f * who.energy + 0.9f * tension.density + 2.0f * th.density, 0.25f,
                                     tuning::maxNoteDensity);
    const float offBeat = std::clamp(0.6f * who.syncopation + 0.35f * tension.rhythm * (0.5f + who.rhythmicComplexity),
                                     0.0f, 0.8f);
    motif m;
    m.id = m.family = ++this->nextMotif;
    m.rhythm[0] = (std::uint8_t) this->pickRhythm(perBeat * 4.0f, offBeat);
    m.rhythm[1] = r.chance(0.5f + 0.3f * who.repetition) ? m.rhythm[0]
                                                         : (std::uint8_t) this->pickRhythm(perBeat * 4.0f, offBeat);
    m.start = (std::int8_t) (r.below(5) - 2);
    // a contour: 0 arch, 1 falling, 2 rising, 3 wave, 4 around one note
    const int contour = r.below(5);
    const int n = m.notes();
    const float leapChance = 0.1f + 0.3f * who.melodicRange;
    int lastStep = 0;
    for (int k = 1; k < n; k++) {
        const float at = (float) k / (float) n;
        int dir;
        switch (contour) {
        case 0: dir = at < 0.5f ? 1 : -1; break;
        case 1: dir = -1; break;
        case 2: dir = 1; break;
        case 3: dir = std::sin(at * 6.283f) >= 0.0f ? 1 : -1; break;
        default: dir = k % 2 == 0 ? 1 : -1; break;
        }
        if (r.chance(0.25f))
            dir = -dir; // a contour is a tendency, not a rule
        int size = 1;
        if (std::abs(lastStep) >= 2 && r.chance(0.7f)) {
            dir = lastStep > 0 ? -1 : 1; // a leap is followed by a step back into the gap
        } else if (r.chance(leapChance)) {
            size = 2 + r.below(who.melodicRange > 0.6f ? 3 : 2);
        } else if (r.chance(0.15f)) {
            size = 0;
        }
        lastStep = dir * size;
        m.steps[(std::size_t) k] = (std::int8_t) lastStep;
    }
    return m;
}

motif composer::vary(const motif &m, int changes, const performerPersonality &who, const musicalState &tension,
                     const vocabulary::theme &th)
{
    auto &r = this->phraseChoices;
    motif v = m;
    v.id = ++this->nextMotif;
    for (int c = 0; c < changes; c++) {
        const int n = v.notes();
        switch (r.below(5)) {
        case 0: { // one bar takes a neighbouring rhythm
            const int bar = r.below(2);
            const int was = vocabulary::onsets(vocabulary::leadRhythms[v.rhythm[(std::size_t) bar]]);
            const float offBeat = std::clamp(0.6f * who.syncopation + 0.35f * tension.rhythm, 0.0f, 0.8f);
            const float wanted = (float) (was + r.below(3) - 1) + 4.0f * th.density;
            v.rhythm[(std::size_t) bar] = (std::uint8_t) this->pickRhythm(wanted, offBeat);
            for (int k = n; k < v.notes(); k++) // new notes step gently
                v.steps[(std::size_t) k] = (std::int8_t) (r.below(3) - 1);
            break;
        }
        case 1: { // one or two steps bend by one
            for (int t = 0, k; t < 1 + r.below(2) && n > 1; t++) {
                k = 1 + r.below(n - 1);
                v.steps[(std::size_t) k] = (std::int8_t) std::clamp(v.steps[(std::size_t) k] + (r.chance(0.5f) ? 1 : -1), -4, 4);
            }
            break;
        }
        case 2: // the whole idea moves up or down
            v.start = (std::int8_t) std::clamp(v.start + (r.chance(0.5f) ? 1 : -1) * (1 + r.below(2)), -4, 4);
            break;
        case 3: // a different ending
            if (n > 2) {
                v.steps[(std::size_t) n - 1] = (std::int8_t) (r.below(5) - 2);
                v.steps[(std::size_t) n - 2] = (std::int8_t) (r.below(3) - 1);
            }
            break;
        default: // the second half turns upside down, or a note is left out or put back
            if (r.chance(0.5f)) {
                for (int k = n / 2; k < n; k++)
                    v.steps[(std::size_t) k] = (std::int8_t) -v.steps[(std::size_t) k];
            } else if (n > 1) {
                v.rests ^= 1u << (1 + r.below(n - 1));
            }
            break;
        }
    }
    return v;
}

std::int64_t composer::timeOf(int sixteenth, double step) const
{
    const bool offBeat = (sixteenth % 2 + 2) % 2 == 1;
    return std::llround(((double) sixteenth + (offBeat ? this->swing : 0.0)) * step);
}

void composer::remember(situation s, const motif &m)
{
    auto &bank = this->banks[(std::size_t) this->slot][(int) s];
    for (int c = 0; c < bank.count; c++)
        if (bank.motifs[(std::size_t) c].id == m.id)
            return;
    bank.motifs[(std::size_t) bank.next] = m;
    bank.next = (bank.next + 1) % tuning::phraseMemory;
    bank.count = std::min(bank.count + 1, tuning::phraseMemory);
    if (!bank.hasHome) {
        bank.home = m;
        bank.hasHome = true;
    }
}

motif composer::chooseMotif(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                            phraseReport::relation &made)
{
    auto &r = this->phraseChoices;
    const auto &song = this->banks[(std::size_t) this->slot];
    auto &bank = song[(int) th.when];
    if (bank.count == 0) {
        // a theme's first idea grows from the theme before it, so the band still sounds like itself
        const auto &calm = song[(int) situation::calm];
        const auto &alert = song[(int) situation::alert];
        this->repeats = this->unrelated = 0;
        if (th.when == situation::alert && calm.hasHome) {
            motif m = calm.home; // the main theme upside down
            m.id = m.family = ++this->nextMotif;
            for (auto &s : m.steps)
                s = (std::int8_t) -s;
            made = phraseReport::relation::variation;
            return m;
        }
        if (th.when == situation::danger && (alert.hasHome || calm.hasHome)) {
            motif m = alert.hasHome ? alert.home : calm.home; // its first bar, twice
            m.id = m.family = ++this->nextMotif;
            const int first = vocabulary::onsets(vocabulary::leadRhythms[m.rhythm[0]]);
            m.rhythm[1] = m.rhythm[0];
            for (int k = 0; k < first && first + k < tuning::motifNotes; k++)
                m.steps[(std::size_t) (first + k)] = m.steps[(std::size_t) k];
            made = phraseReport::relation::variation;
            return m;
        }
        made = phraseReport::relation::fresh;
        this->unrelated = 1;
        return this->fresh(who, tension, th);
    }
    const bool mustReturn = this->unrelated >= tuning::maxConsecutiveUnrelated;
    const float returnChance = 0.55f + 0.35f * who.repetition - 0.1f * tension.phrase;
    if (!mustReturn && !r.chance(returnChance)) {
        this->unrelated++;
        this->repeats = 0;
        made = phraseReport::relation::fresh;
        return this->fresh(who, tension, th);
    }
    this->unrelated = 0;
    const motif &source = (r.chance(0.5f) || bank.count == 1) ? bank.home
                                                              : bank.motifs[(std::size_t) r.below(bank.count)];
    float amount = who.phraseVariation * (0.4f + 0.6f * tension.phrase) + 0.2f * r.unit();
    if (this->repeats >= tuning::maxConsecutiveRepetitions)
        amount = std::max(amount, 0.25f);
    const int changes = (int) std::lround(amount * 4.0f);
    if (changes == 0) {
        this->repeats++;
        made = phraseReport::relation::repeat;
        return source;
    }
    this->repeats = 0;
    made = phraseReport::relation::variation;
    return this->vary(source, changes, who, tension, th);
}

int composer::writeChords(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                          const std::array<int, 4> &roots, const phrasePlan &plan, double step, phraseBuffer &out)
{
    auto &r = this->phraseChoices;
    const genreRules &rules = rulesOf(plan.style);
    const float colourChance = rules.shape == chordShape::power
                                   ? 0.0f // a power chord has no third to colour
                                   : std::min(tuning::maxDissonance,
                                              tension.harmony * tuning::maxDissonance * (0.4f + who.dissonance) * th.colour
                                                  + 0.05f * who.harmonicAdventurousness);
    const float velocity = 0.45f + 0.2f * who.energy;
    int low = who.keyRoot + th.transpose + 10; // chords sit in one octave from here: smooth voice leading
    while (low >= 62)
        low -= 12;
    while (low < 50)
        low += 12;
    auto place = [low](int pitch) {
        while (pitch < low)
            pitch += 12;
        while (pitch >= low + 12)
            pitch -= 12;
        return pitch;
    };
    int coloured = 0;
    for (int bar = 0; bar < this->last.bars; bar++) {
        const int root = roots[(std::size_t) bar];
        std::array<int, 4> tones{root, root + 2, root + 4, root + 6};
        int count = rules.shape == chordShape::seventh ? 4 : 3;
        if (rules.shape == chordShape::power)
            tones = {root, root + 4, root + 7, root + 7};
        int suspended = -1; // the tone a suspension resolves to, mid-bar
        bool borrowed = false;
        if (r.chance(colourChance)) {
            coloured++;
            switch (r.below(tension.harmony > 0.4f ? 4 : 3)) {
            case 0: tones[3] = root + 6; count = 4; break;            // seventh (on a seventh chord: the same)
            case 1: suspended = tones[1]; tones[1] = root + 3; break;  // suspended fourth, resolving
            case 2: tones[3] = root + 8; count = 4; break;            // added ninth
            default: borrowed = true; break;                          // the flat sixth's major chord
            }
        }
        auto pitchAt = [&](int t) {
            static constexpr int major[] = {8, 12, 15, 15};
            if (borrowed)
                return place(who.keyRoot + th.transpose + major[t]);
            if (rules.shape == chordShape::power && t == 2)
                return place(this->pitchOf(root, who, th, 0)) + 12; // the octave above the root
            return place(this->pitchOf(tones[(std::size_t) t], who, th, 0));
        };
        // one chord from `from` to `to`: on a chip one voice runs through its tones, lowest first
        auto play = [&](std::int64_t from, std::int64_t to, float vel) {
            std::array<int, 4> pitches{};
            for (int t = 0; t < count; t++)
                pitches[(std::size_t) t] = pitchAt(t);
            if (!plan.arpeggioChords) {
                for (int t = 0; t < count; t++) {
                    const std::uint32_t id = this->note();
                    out.add({from, 0, noteEvent::kind::on, part::pad, id, (float) pitches[(std::size_t) t], vel});
                    out.add({to, 0, noteEvent::kind::off, part::pad, id, 0, 0});
                }
                return;
            }
            for (int i = 1; i < count; i++) // at most four tones: put them in order by hand
                for (int j = i; j > 0 && pitches[(std::size_t) j] < pitches[(std::size_t) j - 1]; j--)
                    std::swap(pitches[(std::size_t) j], pitches[(std::size_t) j - 1]);
            noteEvent e{from, 0, noteEvent::kind::on, part::pad, this->note(), (float) pitches[0], vel};
            for (int t = 1; t < count; t++)
                e.arp[(std::size_t) t - 1] = (std::int8_t) (pitches[(std::size_t) t] - pitches[0]);
            e.arpCount = (std::uint8_t) (count - 1);
            out.add(e);
            out.add({to, 0, noteEvent::kind::off, part::pad, e.note, 0, 0});
        };
        if (!rules.chordRhythm.empty()) {
            // the style's stabs: short chords on its sixteenths
            for (int s = 0; s < stepsPerBar; s++) {
                if (!plays(rules.chordRhythm, s))
                    continue;
                const std::int64_t from = this->timeOf(bar * stepsPerBar + s, step);
                play(from, from + std::llround(1.5 * step), velocity * (s % 4 == 0 ? 1.0f : 0.9f));
            }
            continue;
        }
        const std::int64_t start = (std::int64_t) std::llround(bar * stepsPerBar * step);
        const std::int64_t end = (std::int64_t) std::llround((bar + 1) * stepsPerBar * step) - 1;
        const std::int64_t middle = (std::int64_t) std::llround((bar * stepsPerBar + 8) * step);
        if (suspended >= 0) {
            play(start, middle - 1, velocity);
            tones[1] = suspended; // the fourth resolves to the third half way through the bar
            play(middle, end, velocity * 0.9f);
        } else {
            play(start, end, velocity);
        }
    }
    return coloured;
}

void composer::writeBass(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                         const std::array<int, 4> &roots, bool pedal, const phrasePlan &plan, double step, float rate,
                         phraseBuffer &out)
{
    auto &e = this->eventChoices;
    const bassLine line = rulesOf(plan.style).bass;
    int pattern = std::clamp((int) std::lround((who.energy * 0.6f + tension.density * 0.5f) * 4.0f), 0, 4);
    if (th.drive > 0.0f)
        pattern = std::max(pattern, (int) std::lround(th.drive * 5.0f));
    if (this->last.silent)
        pattern = std::min(pattern, 1); // the band leaves the space open with the lead
    // a style plays its own line; the performer's own follows the energy and the tension
    std::string_view rhythm = vocabulary::bassRhythms[(std::size_t) pattern];
    float hold = 0.85f; // of the gap to the next note
    switch (line) {
    case bassLine::offbeat: rhythm = "..x...x...x...x."; break;
    case bassLine::rolling: rhythm = ".xxx.xxx.xxx.xxx"; hold = 0.6f; break;
    case bassLine::octave: rhythm = "x.x.x.x.x.x.x.x."; hold = 0.7f; break;
    case bassLine::chug: rhythm = "x.xxx.x.x.xxx.xx"; hold = 0.55f; break;
    case bassLine::walking: rhythm = "x...x...x...x..."; hold = 0.9f; break;
    case bassLine::eighths: rhythm = "x.x.x.x.x.x.x.x."; hold = 0.75f; break;
    default: break;
    }
    int low = who.keyRoot + th.transpose - 14; // the bass's octave starts between G1 and F#2
    while (low >= 43)
        low -= 12;
    while (low < 31)
        low += 12;
    auto inRange = [low](int pitch) {
        while (pitch < low)
            pitch += 12;
        while (pitch >= low + 12)
            pitch -= 12;
        return pitch;
    };
    for (int bar = 0; bar < this->last.bars; bar++) {
        const int root = pedal ? 0 : roots[(std::size_t) bar];
        const int nextRoot = pedal ? 0 : roots[(std::size_t) (bar + 1 < this->last.bars ? bar + 1 : 0)];
        for (int s = 0; s < stepsPerBar; s++) {
            if (!plays(rhythm, s))
                continue;
            int gap = 1;
            while (s + gap < stepsPerBar && !plays(rhythm, s + gap))
                gap++;
            int degree = root;
            if (line == bassLine::walking) {
                // the root, a step or the third, the fifth, then a half step into the next chord
                static constexpr int walk[] = {0, 2, 4};
                degree = s < 12 ? root + (s == 4 && e.chance(0.4f) ? 1 : walk[s / 4]) : nextRoot;
            } else if ((line == bassLine::follow ? pattern >= 3 : line == bassLine::eighths) && !pedal && s % 8 != 0
                       && e.chance(0.3f)) {
                degree = root + 4; // the fifth on a weak beat
            }
            int pitch = inRange(this->pitchOf(degree, who, th, -1));
            if (line == bassLine::walking && s == 12)
                pitch += e.chance(0.5f) ? -1 : 1; // the approach note, outside the scale on purpose
            if (line == bassLine::octave && s % 4 == 2)
                pitch += 12;
            const float vel = std::clamp(0.55f + 0.25f * who.energy + (s == 0 ? 0.1f : 0.0f)
                                             + 0.05f * who.dynamics * e.between(-1.0f, 1.0f),
                                         0.1f, 1.0f);
            const double jitter = this->drift * 0.3 * rate / 1000.0;
            const std::int64_t on = std::max<std::int64_t>(0, this->timeOf(bar * stepsPerBar + s, step) + std::llround(jitter));
            const std::int64_t off = on + std::max<std::int64_t>((std::int64_t) (0.06f * rate), std::llround(gap * step * hold));
            const std::uint32_t id = this->note();
            out.add({on, 0, noteEvent::kind::on, part::bass, id, (float) pitch, vel});
            out.add({off, 0, noteEvent::kind::off, part::bass, id, 0, 0});
        }
    }
}

void composer::writeLead(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                         const motif &m, const std::array<int, 4> &roots, double step, float rate, phraseBuffer &out)
{
    auto &e = this->eventChoices;
    auto &rep = this->last;
    // the second half of a four bar phrase answers the first: the same idea, a little changed
    const motif answer = rep.bars > 2 ? this->vary(m, 1, who, tension, th) : m;
    const int range = 3 + (int) std::lround(who.melodicRange * 4.0f);
    const int centre = (int) std::lround((who.registerBias - 0.5f) * 4.0f + tension.phrase * 1.5f);
    const float omit = who.silence * 0.08f * (1.0f + tension.rhythm);
    const float anticipate = who.anticipation * (0.3f + 0.7f * tension.rhythm);
    // the melody's tonic sits between middle C and the C above, whatever the key and theme
    int octave = 2;
    while (who.keyRoot + th.transpose + 12 * octave >= 72)
        octave--;
    while (who.keyRoot + th.transpose + 12 * octave < 60)
        octave++;
    int n = 0;
    for (int bar = 0; bar < rep.bars; bar++) {
        const motif &half = bar < 2 ? m : answer;
        const auto rhythm = vocabulary::leadRhythms[half.rhythm[(std::size_t) (bar % 2)]];
        int k = bar % 2 == 0 ? 0 : vocabulary::onsets(vocabulary::leadRhythms[half.rhythm[0]]);
        // the degree of the note before this bar's first one
        int degree = centre + half.start;
        for (int c = 1; c < k && c < tuning::motifNotes; c++)
            degree += half.steps[(std::size_t) c];
        const int chord = roots[(std::size_t) bar];
        for (int s = 0; s < stepsPerBar; s++) {
            if (!plays(rhythm, s))
                continue;
            if (k > 0 && k < tuning::motifNotes)
                degree += half.steps[(std::size_t) k];
            // keep inside the performer's register, turning back at its edges
            if (degree > centre + range)
                degree -= 2 * (degree - centre - range);
            if (degree < centre - range)
                degree += 2 * (centre - range - degree);
            int played = degree;
            if (s % 4 == 0) {
                // on a beat the melody lands on a tone of the chord
                int best = played, bestDist = 99;
                for (int t : {0, 2, 4})
                    for (int o = -1; o <= 1; o++) {
                        const int cand = chord + t + 7 * (floorDiv7(played) + o);
                        if (std::abs(cand - played) < bestDist) {
                            bestDist = std::abs(cand - played);
                            best = cand;
                        }
                    }
                played = best;
            }
            const bool rested = (k < 32 && (half.rests >> k) & 1u) || (n > 0 && e.chance(omit));
            k++;
            if (rested || n >= (int) this->leadNotes.size())
                continue;
            int pitch = this->pitchOf(played, who, th, octave);
            const int previous = n > 0 ? this->leadNotes[(std::size_t) n - 1].pitch : this->lastLead;
            while (previous >= 0 && std::abs(pitch - previous) > tuning::maxRegisterJump)
                pitch += pitch > previous ? -12 : 12;
            int at = bar * stepsPerBar + s;
            if (s == 0 && bar > 0 && n > 0 && this->leadNotes[(std::size_t) n - 1].step < at - 1 && e.chance(anticipate)) {
                at -= 1; // played a sixteenth early, leaning into the bar
                rep.anticipated++;
            }
            auto &ln = this->leadNotes[(std::size_t) n++];
            ln.step = at;
            ln.pitch = pitch;
        }
    }
    // chromatic colour: passing and neighbour notes on weak positions, never more than the limit
    const float chromaChance = std::min(tuning::maxChromaticRate,
                                        (0.02f + 0.10f * tension.harmony) * (0.5f + who.dissonance) * th.colour);
    const int cap = (int) std::floor(tuning::maxChromaticRate * (float) n);
    for (int i = 1; i + 1 < n; i++) {
        auto &cur = this->leadNotes[(std::size_t) i];
        if (cur.step % 4 == 0 || rep.chromatic >= cap || !e.chance(chromaChance))
            continue;
        const int prev = this->leadNotes[(std::size_t) i - 1].pitch, next = this->leadNotes[(std::size_t) i + 1].pitch;
        int cand = -1;
        if (std::abs(next - prev) == 2)
            cand = (prev + next) / 2;                 // a chromatic passing note
        else if (this->leadNotes[(std::size_t) i + 1].step % 4 == 0)
            cand = next - 1;                          // a lower neighbour leaning on the beat
        if (cand < 0 || this->inScale(cand, who, th) || std::abs(cand - prev) > tuning::maxRegisterJump)
            continue;
        cur.pitch = cand;
        rep.chromatic++;
    }
    // lengths, loudness and timing
    const float maxDrift = tuning::maxTimingJitterMs * who.timingDrift * (0.6f + 0.4f * tension.rhythm);
    const float base = 0.5f + 0.25f * who.energy + 0.1f * tension.density;
    const int total = rep.bars * stepsPerBar;
    for (int i = 0; i < n; i++) {
        auto &ln = this->leadNotes[(std::size_t) i];
        const int nextStep = i + 1 < n ? this->leadNotes[(std::size_t) i + 1].step : total;
        const double gap = (double) (nextStep - ln.step) * step / rate;
        const float hold = (0.45f + 0.5f * who.articulation) * (1.0f + 0.1f * e.between(-1.0f, 1.0f));
        ln.seconds = (float) std::clamp(gap * hold, 0.06, 1.2);
        const float arc = std::sin(3.14159f * (float) ln.step / (float) std::max(total, 1));
        ln.velocity = std::clamp(base + (ln.step % 4 == 0 ? 0.1f : -0.05f) * who.dynamics + 0.08f * who.dynamics * arc
                                     + 0.06f * who.dynamics * e.between(-1.0f, 1.0f),
                                 0.15f, 1.0f);
        // the drift wanders slowly and is pulled in at the phrase's start, so the pulse stays
        const float dt = (float) (gap > 0 ? gap : 0.1);
        this->drift += -this->drift * std::min(1.0f, dt / 0.8f) + 0.5f * maxDrift * std::sqrt(dt) * e.gauss() / 3.0f;
        this->drift = std::clamp(this->drift, -maxDrift, maxDrift);
        const float offset = ln.step <= 0 ? this->drift * 0.25f : this->drift;
        rep.maxJitterMs = std::max(rep.maxJitterMs, std::fabs(offset));
        const std::int64_t on = std::max<std::int64_t>(0, this->timeOf(ln.step, step) + std::llround(offset * rate / 1000.0f));
        const std::int64_t off = on + std::llround(ln.seconds * rate);
        const std::uint32_t id = this->note();
        out.add({on, 0, noteEvent::kind::on, part::lead, id, (float) ln.pitch, ln.velocity});
        out.add({off, 0, noteEvent::kind::off, part::lead, id, 0, 0});
        rep.leadNotes++;
        if (ln.step % 4 != 0)
            rep.syncopated++;
        if (i > 0)
            rep.maxLeap = std::max(rep.maxLeap, std::abs(ln.pitch - this->leadNotes[(std::size_t) i - 1].pitch));
    }
    if (n > 0)
        this->lastLead = this->leadNotes[(std::size_t) n - 1].pitch;
}

void composer::writeDrums(const performerPersonality &who, const phrasePlan &plan, double step, float rate,
                          phraseBuffer &out)
{
    auto &e = this->eventChoices;
    auto &rep = this->last;
    const auto &g = vocabulary::grooves[(std::size_t) std::clamp(plan.groove, 0, (int) vocabulary::grooves.size() - 1)];
    const auto fill = vocabulary::fills[(std::size_t) e.below((int) vocabulary::fills.size())];
    const float loud = 0.75f + 0.25f * plan.intensity;
    // one drum channel (a SID voice, the Game Boy's noise) cannot play a hat over a kick or a snare
    const bool crowded = plan.drumChannels <= 1;
    auto hit = [&](int sixteenth, drum d, float velocity) {
        const float vel = std::clamp(velocity * loud + 0.05f * who.dynamics * e.between(-1.0f, 1.0f), 0.1f, 1.0f);
        const std::int64_t on = std::max<std::int64_t>(0, this->timeOf(sixteenth, step));
        const float ring = d == drum::openHat || d == drum::tom ? 0.3f : 0.15f;
        const std::uint32_t id = this->note();
        out.add({on, 0, noteEvent::kind::on, part::drums, id, (float) d, vel});
        out.add({on + std::llround(ring * rate), 0, noteEvent::kind::off, part::drums, id, 0, 0});
        rep.drumHits++;
    };
    for (int bar = 0; bar < rep.bars; bar++) {
        const bool filling = plan.fill && bar == rep.bars - 1;
        for (int s = 0; s < stepsPerBar; s++) {
            const int at = bar * stepsPerBar + s;
            if (filling && s >= 8) {
                // the fill: snares and toms getting louder into the next section
                const char f = fill[(std::size_t) (s - 8)];
                const float rise = 0.6f + 0.05f * (float) (s - 8);
                if (f == 's')
                    hit(at, drum::snare, rise);
                else if (f == 't')
                    hit(at, drum::tom, rise);
                continue;
            }
            bool kick = plays(plan.drums >= 3 && !g.busyKick.empty() ? g.busyKick : g.kick, s);
            bool snare = plays(g.snare, s);
            bool hat = plays(plan.drums >= 3 ? g.busyHats : g.hats, s);
            if (plan.drums == 1) {
                // light: the kick on the strong beats, the backbeat, the hats on the beat
                kick = kick && s % 8 == 0;
                hat = hat && s % 4 == 0;
            }
            const bool crash = plan.drums >= 3 && bar == 0 && s == 0 && plan.phraseInSong > 0;
            const bool ghost = !snare && plan.drums >= 3 && s % 4 == 3
                               && e.chance(0.15f + 0.2f * who.rhythmicComplexity);
            // open hats where the groove has them, or one at the end of every second bar
            const bool open = plan.drums >= 2 && (g.openHats.empty() ? bar % 2 == 1 && s == 14 : plays(g.openHats, s));
            hat = hat || open;
            // on one channel only the first of these sounds: kick, snare, crash, ghost, hat
            int played = 0;
            auto play = [&](bool wanted, drum d, float velocity) {
                if (wanted && !(crowded && played > 0)) {
                    hit(at, d, velocity);
                    played++;
                }
            };
            play(kick, drum::kick, s % 8 == 0 ? 0.95f : 0.8f);
            play(snare, drum::snare, 0.85f);
            play(crash, drum::openHat, 0.8f); // a crash at the top of a driving phrase
            play(ghost, drum::snare, 0.3f);
            play(hat && !crash, open ? drum::openHat : drum::hat, s % 4 == 0 ? 0.55f : 0.4f);
        }
    }
}

void composer::compose(const performerPersonality &who, const musicalState &tension, situation now, float sampleRate,
                       const phrasePlan &plan, phraseBuffer &out)
{
    auto &r = this->phraseChoices;
    out.clear();
    if (plan.slot >= 0 && plan.slot < tuning::songMemory) {
        this->slot = plan.slot;
        if (plan.freshSlot)
            this->banks[(std::size_t) this->slot] = {};
    }
    if (plan.songStart) {
        // a new song is a new start: no old count of repeats, no cadence carried over
        this->repeats = this->unrelated = 0;
        this->deceptive = false;
        this->lastSilent = false;
    }
    this->swing = std::clamp((double) plan.swing, 0.0, (double) tuning::maxSwing);
    const genreRules &rules = rulesOf(plan.style);
    vocabulary::theme th = vocabulary::themeFor(now, who);
    th.density += plan.densityLift + rules.leadDensity;
    th.colour *= rules.colour;
    if (plan.part == section::chorus)
        th.drive = std::max(th.drive, 0.4f);
    phraseReport &rep = this->last;
    rep = {};
    rep.theme = now;
    rep.transpose = th.transpose;
    rep.song = plan.songId;
    rep.style = plan.style;
    rep.part = plan.part;
    rep.drums = plan.drums;
    rep.fill = plan.fill;

    const float tempo = std::clamp(who.baseTempo * (1.0f + tuning::tempoTensionLift * tension.tempo) * th.tempoLift,
                                   tuning::minTempo, tuning::maxTempo);
    rep.tempo = tempo;
    const double step = (double) sampleRate * 60.0 / tempo / 4.0; // samples per sixteenth
    rep.bars = r.chance(0.15f + 0.25f * tension.phrase * who.phraseVariation) ? 2 : 4;
    out.bars = rep.bars;
    out.barLength = (std::int64_t) std::llround(stepsPerBar * step);
    out.length = (std::int64_t) std::llround(rep.bars * stepsPerBar * step);

    // harmony: one of the song's progressions; a two bar phrase keeps its first and last chord
    const int chosen = plan.progressions[(std::size_t) r.below((int) plan.progressions.size())];
    const auto &prog = vocabulary::progressions[(std::size_t) std::clamp(chosen, 0, (int) vocabulary::progressions.size() - 1)];
    std::array<int, 4> roots = prog;
    if (rep.bars == 2)
        roots = {prog[0], prog[3], prog[3], prog[3]};
    if (this->deceptive)
        roots[0] = 5; // the cadence before went somewhere unexpected
    const float colour = std::min(tuning::maxDissonance, tension.harmony * tuning::maxDissonance * (0.4f + who.dissonance) * th.colour);
    const bool pedalPoint = th.pedal || r.chance(colour * 0.3f);
    this->deceptive = roots[(std::size_t) rep.bars - 1] == 4 && r.chance(colour * 0.5f);

    // the lead: rest for a whole phrase now and then, never twice running
    rep.silent = !plan.lead
                 || (!this->lastSilent && this->remembered(now) > 0 && r.chance(who.silence * (0.10f + 0.20f * tension.phrase)));
    this->lastSilent = rep.silent;
    if (!rep.silent) {
        phraseReport::relation made;
        const motif m = this->chooseMotif(who, tension, th, made);
        rep.made = made;
        rep.motifId = m.id;
        rep.family = m.family;
        this->remember(now, m);
        this->writeLead(who, tension, th, m, roots, step, sampleRate, out);
    }
    if (plan.chords) {
        rep.chords = rep.bars;
        rep.colouredChords = this->writeChords(who, tension, th, roots, plan, step, out)
                             + (pedalPoint && !th.pedal ? 1 : 0);
    }
    if (plan.bass)
        this->writeBass(who, tension, th, roots, pedalPoint, plan, step, sampleRate, out);
    if (plan.drums > 0)
        this->writeDrums(who, plan, step, sampleRate, out);

    std::sort(out.events.begin(), out.events.begin() + out.count, [](const noteEvent &a, const noteEvent &b) {
        return a.at != b.at ? a.at < b.at : a.order < b.order;
    });
    int sounding = 0;
    for (int c = 0; c < out.count; c++) {
        if (out.events[(std::size_t) c].who == part::drums)
            continue;
        sounding += out.events[(std::size_t) c].what == noteEvent::kind::on ? 1 : -1;
        rep.maxSimultaneous = std::max(rep.maxSimultaneous, sounding);
    }
}

} // namespace goe::musician
