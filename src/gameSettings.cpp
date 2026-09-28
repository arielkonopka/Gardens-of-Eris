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
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
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
    return true;
}

bool gameSettings::save(const std::string &file) const
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("saveDirectory");
    w.String(this->getSaveDirectory().c_str());
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
}
