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

#include "gameSettings.h"
#include "musicChips.h"
#include "musicGenres.h"
#include "storyScroller.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <sstream>

gameSettings &gameSettings::getInstance()
{
    static gameSettings instance;
    return instance;
}

std::string gameSettings::getSaveDirectory() const
{
    std::lock_guard<std::mutex> lock(this->m);
    return this->saveDirectory;
}

bool gameSettings::setSaveDirectory(const std::string &dir)
{
    if (dir.empty())
        return false;
    std::error_code ec;
    std::filesystem::path p(dir);
    if (!std::filesystem::exists(p, ec))
        std::filesystem::create_directories(p, ec);
    if (!std::filesystem::is_directory(p, ec))
        return false;
    // make sure we can actually write a save there
    auto probe = p / ".goe-write-test";
    {
        std::ofstream f(probe);
        if (!f)
            return false;
    }
    std::filesystem::remove(probe, ec);
    std::lock_guard<std::mutex> lock(this->m);
    this->saveDirectory = dir;
    return true;
}

std::string gameSettings::getSaveFile() const
{
    return (std::filesystem::path(this->getSaveDirectory()) / saveFileName).string();
}

void gameSettings::setMusicVolume(int percent)
{
    this->musicVolume = std::clamp(percent, 0, 100);
}

void gameSettings::setPerformerSound(goe::musician::chipStyle s)
{
    this->performerSound = (goe::musician::chipStyle) std::clamp((int) s, 0, goe::musician::chipStyleCount - 1);
}

void gameSettings::setMusicStyle(goe::musician::genre g)
{
    this->musicStyle = (goe::musician::genre) std::clamp((int) g, 0, goe::musician::genreCount - 1);
}

void gameSettings::setMusicVariety(int percent)
{
    this->musicVariety = std::clamp(percent, 0, 100);
}

void gameSettings::setMusicTempo(int percent)
{
    this->musicTempo = std::clamp(percent, minMusicTempo, maxMusicTempo);
}

void gameSettings::setEffectsVolume(int percent)
{
    this->effectsVolume = std::clamp(percent, 0, 100);
}

std::string gameSettings::getStoriesFile() const
{
    std::lock_guard<std::mutex> lock(this->m);
    return this->storiesFile;
}

bool gameSettings::setStoriesFile(const std::string &file)
{
    if (goe::loadStories(file).empty())
        return false;
    std::lock_guard<std::mutex> lock(this->m);
    this->storiesFile = file;
    return true;
}

namespace {
int secondsIn(int s)
{
    return std::clamp(s, gameSettings::minSeconds, gameSettings::maxSeconds);
}
} // namespace

void gameSettings::setDemoWait(int seconds)
{
    this->demoWait = secondsIn(seconds);
}

void gameSettings::setDemoLength(int seconds)
{
    this->demoLength = secondsIn(seconds);
}

void gameSettings::setHallOfFameLength(int seconds)
{
    this->hallOfFameLength = secondsIn(seconds);
}

goe::controls::bindings gameSettings::getControls() const
{
    std::lock_guard<std::mutex> lock(this->m);
    return this->controls;
}

void gameSettings::setControls(const goe::controls::bindings &b)
{
    std::lock_guard<std::mutex> lock(this->m);
    this->controls = b;
}

namespace {
/// reads the "controls" object; actions it does not name keep what they had
goe::controls::bindings readControls(const rapidjson::Value &v, goe::controls::bindings b)
{
    using goe::controls::bindings;
    for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it) {
        auto a = bindings::fromId(it->name.GetString());
        if (!a || !it->value.IsObject())
            continue;
        b.clear(*a);
        if (it->value.HasMember("pad") && it->value["pad"].IsInt())
            b.bindPadButton(*a, it->value["pad"].GetInt());
        if (it->value.HasMember("keys") && it->value["keys"].IsArray()) {
            const auto &keys = it->value["keys"].GetArray();
            // bindKey puts each key first, so the main key goes in last
            for (auto k = keys.End(); k != keys.Begin();) {
                --k;
                if (k->IsInt())
                    b.bindKey(*a, k->GetInt());
            }
        }
    }
    // a file from before "save and exit" kept Esc for giving up; Esc now leaves without losing
    // the avatar, and giving up moves to its new default key
    using goe::controls::action;
    if (!v.HasMember(bindings::id(action::saveAndExit).c_str())
        && b.of(action::giveUp).keys == std::vector<int>{ALLEGRO_KEY_ESCAPE}) {
        const int pad = b.of(action::giveUp).padButton;
        const bindings fresh;
        b.clear(action::giveUp);
        for (int k : fresh.of(action::giveUp).keys)
            b.bindKey(action::giveUp, k);
        b.bindPadButton(action::giveUp, pad);
        b.bindKey(action::saveAndExit, ALLEGRO_KEY_ESCAPE);
    }
    return b;
}
} // namespace

bool gameSettings::load(const std::string &file)
{
    std::ifstream in(file);
    if (!in)
        return false;
    std::stringstream buffer;
    buffer << in.rdbuf();
    rapidjson::Document doc;
    doc.Parse(buffer.str().c_str());
    if (doc.HasParseError() || !doc.IsObject())
        return false;
    if (doc.HasMember("saveDirectory") && doc["saveDirectory"].IsString()) {
        std::lock_guard<std::mutex> lock(this->m);
        this->saveDirectory = doc["saveDirectory"].GetString();
    }
    if (doc.HasMember("musicVolume") && doc["musicVolume"].IsInt())
        this->setMusicVolume(doc["musicVolume"].GetInt());
    if (doc.HasMember("effectsVolume") && doc["effectsVolume"].IsInt())
        this->setEffectsVolume(doc["effectsVolume"].GetInt());
    if (doc.HasMember("music") && doc["music"].IsString())
        this->setMusicSource(std::string(doc["music"].GetString()) == "performer" ? musicSource::performer
                                                                                  : musicSource::samples);
    if (doc.HasMember("performerSound") && doc["performerSound"].IsString())
        this->setPerformerSound(goe::musician::styleNamed(doc["performerSound"].GetString()));
    if (doc.HasMember("musicStyle") && doc["musicStyle"].IsString())
        this->setMusicStyle(goe::musician::genreNamed(doc["musicStyle"].GetString()));
    if (doc.HasMember("musicVariety") && doc["musicVariety"].IsInt())
        this->setMusicVariety(doc["musicVariety"].GetInt());
    if (doc.HasMember("musicTempo") && doc["musicTempo"].IsInt())
        this->setMusicTempo(doc["musicTempo"].GetInt());
    if (doc.HasMember("storyScroller") && doc["storyScroller"].IsBool())
        this->setStoriesShown(doc["storyScroller"].GetBool());
    if (doc.HasMember("storiesFile") && doc["storiesFile"].IsString()) {
        // kept even when it cannot be read now, like the save folder; the game then tells no stories
        std::lock_guard<std::mutex> lock(this->m);
        this->storiesFile = doc["storiesFile"].GetString();
    }
    if (doc.HasMember("controls") && doc["controls"].IsObject())
        this->setControls(readControls(doc["controls"], this->getControls()));
    if (doc.HasMember("demoWait") && doc["demoWait"].IsInt())
        this->setDemoWait(doc["demoWait"].GetInt());
    if (doc.HasMember("demoLength") && doc["demoLength"].IsInt())
        this->setDemoLength(doc["demoLength"].GetInt());
    if (doc.HasMember("hallOfFameLength") && doc["hallOfFameLength"].IsInt())
        this->setHallOfFameLength(doc["hallOfFameLength"].GetInt());
    return true;
}

bool gameSettings::save(const std::string &file) const
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb); // people edit it by hand too
    w.StartObject();
    w.Key("saveDirectory");
    w.String(this->getSaveDirectory().c_str());
    w.Key("musicVolume");
    w.Int(this->getMusicVolume());
    w.Key("effectsVolume");
    w.Int(this->getEffectsVolume());
    w.Key("music");
    w.String(this->getMusicSource() == musicSource::performer ? "performer" : "samples");
    w.Key("performerSound");
    w.String(std::string(goe::musician::nameOf(this->getPerformerSound())).c_str());
    w.Key("musicStyle");
    w.String(std::string(goe::musician::nameOf(this->getMusicStyle())).c_str());
    w.Key("musicVariety");
    w.Int(this->getMusicVariety());
    w.Key("musicTempo");
    w.Int(this->getMusicTempo());
    w.Key("storyScroller");
    w.Bool(this->getStoriesShown());
    w.Key("storiesFile");
    w.String(this->getStoriesFile().c_str());
    w.Key("demoWait");
    w.Int(this->getDemoWait());
    w.Key("demoLength");
    w.Int(this->getDemoLength());
    w.Key("hallOfFameLength");
    w.Int(this->getHallOfFameLength());
    w.Key("controls");
    w.StartObject();
    const auto bound = this->getControls();
    for (int c = 0; c < goe::controls::actionCount; c++) {
        auto a = (goe::controls::action) c;
        w.Key(goe::controls::bindings::id(a).c_str());
        w.StartObject();
        w.Key("keys");
        w.StartArray();
        for (int k : bound.of(a).keys)
            w.Int(k);
        w.EndArray();
        w.Key("pad");
        w.Int(bound.of(a).padButton);
        w.EndObject();
    }
    w.EndObject();
    w.EndObject();
    std::ofstream out(file, std::ios::trunc);
    if (!out)
        return false;
    out << sb.GetString() << "\n";
    return (bool) out;
}

void gameSettings::resetToDefaults()
{
    std::lock_guard<std::mutex> lock(this->m);
    this->saveDirectory = ".";
    this->musicVolume = 100;
    this->effectsVolume = 100;
    this->music = musicSource::samples;
    this->performerSound = goe::musician::chipStyle::adlib;
    this->musicStyle = goe::musician::genre::mixed;
    this->musicVariety = 60;
    this->musicTempo = 100;
    this->storiesShown = true;
    this->storiesFile = defaultStoriesFile;
    this->demoWait = 60;
    this->demoLength = 30;
    this->hallOfFameLength = 10;
    this->controls = {};
}
