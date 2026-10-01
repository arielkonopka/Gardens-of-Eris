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

#include "storyScroller.h"
#include <fstream>
#include <rapidjson/document.h>
#include <sstream>

namespace goe {
std::vector<story> loadStories(const std::string &file)
{
    std::vector<story> found;
    std::ifstream in(file, std::ios::binary);
    if (!in)
        return found;
    std::stringstream buffer;
    buffer << in.rdbuf();
    rapidjson::Document doc;
    doc.Parse(buffer.str().c_str());
    if (doc.HasParseError() || !doc.IsArray())
        return found;
    for (const auto &entry : doc.GetArray()) {
        if (!entry.IsObject() || !entry.HasMember("body") || !entry["body"].IsString())
            continue;
        story s;
        s.body = entry["body"].GetString();
        if (s.body.empty())
            continue;
        if (entry.HasMember("title") && entry["title"].IsString())
            s.title = entry["title"].GetString();
        found.push_back(std::move(s));
    }
    return found;
}

storyScroller::storyScroller(measure width)
    : widthOf(std::move(width))
{}

void storyScroller::setStories(std::vector<story> told)
{
    this->stories = std::move(told);
}

bool storyScroller::chunkGenerated(rng::engine &e, float stripWidth)
{
    if (this->shown || this->stories.empty())
        return false;
    this->shown = rng::pick(e, this->stories);
    const std::string &t = this->shown->title;
    this->lineWidth = this->widthOf(t.empty() ? this->shown->body : t + separator + this->shown->body);
    this->left = stripWidth;
    return true;
}

void storyScroller::advance(float seconds)
{
    if (!this->shown)
        return;
    this->left -= seconds * pixelsPerSecond;
    if (this->left + this->lineWidth < 0)
        this->stop();
}

void storyScroller::stop()
{
    this->shown.reset();
}
} // namespace goe
