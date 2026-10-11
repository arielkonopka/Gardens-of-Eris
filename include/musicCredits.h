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


#ifndef MUSICCREDITS_H
#define MUSICCREDITS_H

#include <string>
#include <vector>

namespace goe::credits {

/// who made the songs in one music folder ("MusicCredits" in skins.json)
struct musicCredit
{
    std::string folder; ///< the songs whose file lies in this folder, relative to the game ("data/music/bacalao/")
    std::string artist;
    std::string album;
    std::string licence;
    std::string source; ///< where the songs can be found
};

/// one song of the music list, as the credits need it
struct song
{
    std::string file; ///< the file as skins.json names it, with or without the base folder in front
    std::string name; ///< "Artist - Title"
};

/// the credit for a song's file; nullptr when no credit covers its folder
const musicCredit *creditFor(const std::string &file, const std::vector<musicCredit> &credits);

/// the song's title: its name without the artist in front ("Bacalao - Protron" -> "Protron")
std::string titleOf(const song &s, const std::string &artist);

/// the credits screen's lines: for each credit in order, its artist, album and licence, its source and
/// the titles of its songs (each once, in music list order); songs no credit covers come last under
/// "Other music", so a song is never left out of the credits
std::vector<std::string> lines(const std::vector<song> &music, const std::vector<musicCredit> &credits);

} // namespace goe::credits

#endif // MUSICCREDITS_H
