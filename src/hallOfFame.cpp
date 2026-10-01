#include "hallOfFame.h"
#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>

namespace goe {
namespace {
/// cut to at most n characters, never in the middle of a UTF-8 character
std::string cut(std::string s, std::size_t n)
{
    std::size_t chars = 0, at = 0;
    while (at < s.size()) {
        if (chars == n)
            return s.substr(0, at);
        at++;
        while (at < s.size() && ((unsigned char) s[at] & 0xC0) == 0x80)
            at++;
        chars++;
    }
    return s;
}

std::string trimmed(const std::string &s)
{
    const auto first = s.find_first_not_of(" \t");
    if (first == std::string::npos)
        return {};
    return s.substr(first, s.find_last_not_of(" \t") - first + 1);
}
} // namespace

hallOfFame hallOfFame::load(const std::filesystem::path &file)
{
    hallOfFame h;
    std::ifstream in(file, std::ios::binary);
    if (!in)
        return h;
    std::stringstream text;
    text << in.rdbuf();
    rapidjson::Document doc;
    doc.Parse(text.str().c_str());
    if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("entries") || !doc["entries"].IsArray())
        return h;
    for (const auto &v : doc["entries"].GetArray()) {
        if (!v.IsObject() || !v.HasMember("score") || !v["score"].IsInt())
            continue;
        entry e;
        e.score = v["score"].GetInt();
        if (v.HasMember("name") && v["name"].IsString())
            e.name = v["name"].GetString();
        if (v.HasMember("date") && v["date"].IsString())
            e.date = v["date"].GetString();
        h.add(std::move(e));
    }
    return h;
}

bool hallOfFame::save(const std::filesystem::path &file) const
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("entries");
    w.StartArray();
    for (const auto &e : this->list) {
        w.StartObject();
        w.Key("name");
        w.String(e.name.c_str());
        w.Key("score");
        w.Int(e.score);
        w.Key("date");
        w.String(e.date.c_str());
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    const auto tmp = std::filesystem::path(file.string() + ".tmp");
    {
        std::ofstream out(tmp, std::ios::trunc | std::ios::binary);
        if (!out)
            return false;
        out << sb.GetString() << "\n";
        if (!out)
            return false;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, file, ec);
    if (ec)
        std::filesystem::remove(tmp, ec);
    return !ec;
}

bool hallOfFame::qualifies(int score) const
{
    return score > 0 && (this->list.size() < places || score > this->list.back().score);
}

std::optional<std::size_t> hallOfFame::add(entry e)
{
    if (!this->qualifies(e.score))
        return std::nullopt;
    e.name = cut(trimmed(e.name), nameLength);
    if (e.name.empty())
        e.name = nobody;
    const auto at = std::upper_bound(this->list.begin(), this->list.end(), e.score,
                                     [](int score, const entry &x) { return score > x.score; });
    const auto place = (std::size_t) (at - this->list.begin());
    this->list.insert(at, std::move(e));
    if (this->list.size() > places)
        this->list.resize(places);
    return place;
}

std::string today()
{
    const auto day = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    return std::format("{:%Y-%m-%d}", std::chrono::year_month_day{day});
}

} // namespace goe
