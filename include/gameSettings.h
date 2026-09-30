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

#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include "controlBindings.h"
#include <atomic>
#include <mutex>
#include <string>

/**
 * @brief Player-editable options, kept in a small JSON file next to the game.
 *
 * New options get a field, a getter and a setter here, a line in load() and save(),
 * and an entry in titleMenu's config screen.
 */
class gameSettings
{
public:
    static gameSettings &getInstance();

    static constexpr const char *settingsFile = "settings.json";
    static constexpr const char *saveFileName = "savegame.goe";

    std::string getSaveDirectory() const;
    /// creates the folder when it is missing; returns false, and keeps the old one, when it cannot be used
    bool setSaveDirectory(const std::string &dir);
    /// the full path of the save file inside the save folder
    std::string getSaveFile() const;

    /// volumes in percent, 0 (silent) to 100 (as loud as the files are); the sound thread reads them
    static constexpr int volumeStep = 5;
    int getMusicVolume() const { return this->musicVolume; }
    int getEffectsVolume() const { return this->effectsVolume; }
    /// out of range values are clamped
    void setMusicVolume(int percent);
    void setEffectsVolume(int percent);

    /// the story line over the game field (storyScroller.h): on or off, and the file it tells from
    static constexpr const char *defaultStoriesFile = "data/txt/stories.json";
    bool getStoriesShown() const { return this->storiesShown; }
    void setStoriesShown(bool shown) { this->storiesShown = shown; }
    std::string getStoriesFile() const;
    /// false, keeping the old file, when the file has no stories
    bool setStoriesFile(const std::string &file);

    /// the key and pad layout; the input thread copies it on every event
    goe::controls::bindings getControls() const;
    void setControls(const goe::controls::bindings &b);

    /// missing or unreadable files leave the defaults in place
    bool load(const std::string &file = settingsFile);
    bool save(const std::string &file = settingsFile) const;
    void resetToDefaults();

private:
    mutable std::mutex m;
    std::string saveDirectory = ".";
    std::atomic<int> musicVolume = 100;
    std::atomic<int> effectsVolume = 100;
    std::atomic<bool> storiesShown = true;
    std::string storiesFile = defaultStoriesFile;
    goe::controls::bindings controls;
};

#endif // GAMESETTINGS_H
