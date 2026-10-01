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

#ifndef STORYSCROLLER_H
#define STORYSCROLLER_H
#include "randomStreams.h"
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace goe {
/// one entry of a stories file: [{"title": "...", "body": "..."}, ...]
struct story
{
    std::string title;
    std::string body;
};

/// the stories of a file; entries without a body are skipped, and a missing or broken file gives none
std::vector<story> loadStories(const std::string &file);

/**
 * The line of text that scrolls over the top of the game field, telling a random story each time
 * the maze grows by a chunk. It only keeps the numbers; the presenter draws it.
 *
 * Positions are in pixels from the left edge of the strip the text scrolls in. A story comes in
 * at the right edge and is done once its end has left on the left. While one is scrolling, new
 * chunks start nothing, so crossing into a row of five new chunks tells one story, not five.
 */
class storyScroller
{
public:
    /// how wide a text is drawn, in pixels
    using measure = std::function<float(const std::string &)>;
    /// 5 x 23 pixels a second: slow enough to read on the move
    static constexpr float pixelsPerSecond = 115.0f;
    /// what goes between the title and the body
    static constexpr const char *separator = ": ";

    explicit storyScroller(measure width);

    void setStories(std::vector<story> told);
    std::size_t storyCount() const { return this->stories.size(); }

    /// a new chunk was made: starts a random story at the right edge of a strip this wide,
    /// unless one is still scrolling or there are no stories; true when one started
    bool chunkGenerated(rng::engine &e, float stripWidth);
    /// moves the story left; it ends once it has left the strip
    void advance(float seconds);
    /// drops the story on screen, if any
    void stop();

    /// the story on screen, if any
    const std::optional<story> &current() const { return this->shown; }
    /// where the story's first letter is
    float x() const { return this->left; }
    /// how wide the whole line (title, separator and body) is
    float width() const { return this->lineWidth; }

private:
    measure widthOf;
    std::vector<story> stories;
    std::optional<story> shown;
    float left = 0;
    float lineWidth = 0;
};
} // namespace goe

#endif // STORYSCROLLER_H
