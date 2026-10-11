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


#include "musicCredits.h"
#include <algorithm>

namespace goe::credits {

const musicCredit *creditFor(const std::string &file, const std::vector<musicCredit> &credits)
{
    auto covers = [&file](const musicCredit &c) {
        // the folder at the start of the file, or after the base folder ("./data/music/...")
        if (c.folder.empty())
            return false;
        const auto at = file.find(c.folder);
        return at != std::string::npos && (at == 0 || file[at - 1] == '/');
    };
    const auto it = std::ranges::find_if(credits, covers);
    return it == credits.end() ? nullptr : &*it;
}

std::string titleOf(const song &s, const std::string &artist)
{
    const std::string prefix = artist + " - ";
    return !artist.empty() && s.name.starts_with(prefix) ? s.name.substr(prefix.size()) : s.name;
}

std::vector<std::string> lines(const std::vector<song> &music, const std::vector<musicCredit> &credits)
{
    std::vector<std::string> out;
    auto addTitles = [&out](std::vector<std::string> titles) {
        for (auto &t : titles)
            out.push_back(std::move(t));
    };
    // the titles of the songs one credit covers (nullptr: the songs none covers), each once, in list order
    auto titlesOf = [&](const musicCredit *credit) {
        std::vector<std::string> titles;
        for (const auto &s : music) {
            if (creditFor(s.file, credits) != credit)
                continue;
            auto t = titleOf(s, credit ? credit->artist : std::string());
            if (std::ranges::find(titles, t) == titles.end())
                titles.push_back(std::move(t));
        }
        return titles;
    };
    for (const auto &c : credits) {
        auto titles = titlesOf(&c);
        if (titles.empty())
            continue;
        if (!out.empty())
            out.emplace_back();
        out.push_back("Music by " + c.artist);
        out.push_back(c.album.empty() ? c.licence : c.album + ", " + c.licence);
        if (!c.source.empty())
            out.push_back(c.source);
        addTitles(std::move(titles));
    }
    if (auto others = titlesOf(nullptr); !others.empty()) {
        if (!out.empty())
            out.emplace_back();
        out.push_back("Other music");
        addTitles(std::move(others));
    }
    return out;
}

} // namespace goe::credits
