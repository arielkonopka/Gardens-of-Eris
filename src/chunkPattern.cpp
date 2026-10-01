#include "chunkPattern.h"
#include "chamber.h"
#include <algorithm>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <sstream>
#include <stdexcept>

namespace goe {
namespace {
std::invalid_argument bad(const std::string &why)
{
    return std::invalid_argument("chunk pattern: " + why);
}

/// a legend entry: null, a type (number or name), or [type, subtype]
patternCell cellOf(const rapidjson::Value &v, const std::string &key)
{
    auto typeOf = [&key](const rapidjson::Value &t) {
        if (t.IsInt())
            return t.GetInt();
        if (t.IsString()) {
            if (const auto found = chunkPattern::typeByName(t.GetString()))
                return *found;
            throw bad("unknown type name \"" + std::string(t.GetString()) + "\" for '" + key + "'");
        }
        throw bad("the type for '" + key + "' is neither a number nor a name");
    };
    if (v.IsNull())
        return {};
    if (!v.IsArray())
        return {typeOf(v), 0};
    if (v.Empty() || v.Size() > 2)
        throw bad("'" + key + "' must be [type] or [type, subtype]");
    if (v.Size() == 2 && !v[1].IsInt())
        throw bad("the subtype for '" + key + "' is not a whole number");
    return {typeOf(v[0]), v.Size() == 2 ? v[1].GetInt() : 0};
}
} // namespace

chunkPattern::chunkPattern(int width, int height, std::vector<patternCell> cells)
    : w(width)
    , h(height)
    , grid(std::move(cells))
{
    if (width < 1 || height < 1 || width > chamber::chunkSize || height > chamber::chunkSize)
        throw bad("each side must be 1 to " + std::to_string(chamber::chunkSize) + " cells, not "
                  + std::to_string(width) + " x " + std::to_string(height));
    if (this->grid.size() != (std::size_t) width * height)
        throw bad(std::to_string(this->grid.size()) + " cells given for " + std::to_string(width) + " x "
                  + std::to_string(height));
    for (const auto &c : this->grid)
        if (c.type != bElemTypes::_belemType && !placeable(c.type))
            throw bad("type " + std::to_string(c.type) + " cannot be placed");
}

chunkPattern chunkPattern::fromRows(const std::vector<std::string> &rows, const std::map<char, patternCell> &legend)
{
    if (rows.empty())
        throw bad("no rows");
    const std::size_t width = rows.front().size();
    std::vector<patternCell> cells;
    for (const auto &row : rows) {
        if (row.size() != width)
            throw bad("every row must be " + std::to_string(width) + " characters long");
        for (char c : row) {
            const auto it = legend.find(c);
            if (it == legend.end())
                throw bad(std::string("'") + c + "' is not in the legend");
            cells.push_back(it->second);
        }
    }
    return chunkPattern((int) width, (int) rows.size(), std::move(cells));
}

chunkPattern chunkPattern::fromJson(std::string_view text)
{
    rapidjson::Document doc;
    doc.Parse(text.data(), text.size());
    if (doc.HasParseError())
        throw bad(std::string("not JSON: ") + rapidjson::GetParseError_En(doc.GetParseError()));
    if (!doc.IsObject() || !doc.HasMember("legend") || !doc["legend"].IsObject() || !doc.HasMember("rows")
        || !doc["rows"].IsArray())
        throw bad("expected an object with \"legend\" (an object) and \"rows\" (a list of strings)");
    std::map<char, patternCell> legend;
    for (const auto &m : doc["legend"].GetObject()) {
        const std::string key(m.name.GetString(), m.name.GetStringLength());
        if (key.size() != 1)
            throw bad("legend keys are single characters, not \"" + key + "\"");
        legend[key.front()] = cellOf(m.value, key);
    }
    std::vector<std::string> rows;
    for (const auto &r : doc["rows"].GetArray()) {
        if (!r.IsString())
            throw bad("every row is a string");
        rows.emplace_back(r.GetString(), r.GetStringLength());
    }
    return fromRows(rows, legend);
}

chunkPattern chunkPattern::load(const std::filesystem::path &file)
{
    std::ifstream in(file, std::ios::binary);
    if (!in)
        throw bad("cannot read " + file.string());
    std::ostringstream text;
    text << in.rdbuf();
    return fromJson(text.str());
}

const patternCell &chunkPattern::at(int x, int y) const
{
    const auto wrap = [](int v, int n) { return ((v % n) + n) % n; };
    return this->grid[(std::size_t) wrap(y, this->h) * this->w + wrap(x, this->w)];
}

bool chunkPattern::places(int type) const
{
    return std::ranges::any_of(this->grid, [type](const patternCell &c) { return c.type == type; });
}

const std::vector<std::pair<std::string, int>> &chunkPattern::typeNames()
{
    static const std::vector<std::pair<std::string, int>> names = {
        {"floor", bElemTypes::_floorType},
        {"rubbish", bElemTypes::_rubishType},
        {"wall", bElemTypes::_wallType},
        {"monster", bElemTypes::_monster},
        {"drone", bElemTypes::_patrollingDrone},
        {"brick_cluster", bElemTypes::_brickClusterType},
        {"key", bElemTypes::_key},
        {"door", bElemTypes::_door},
        {"puppet_master", bElemTypes::_puppetMasterType},
        {"camera", bElemTypes::_securityCamera},
        {"player", bElemTypes::_player},
        {"gun", bElemTypes::_plainGun},
        {"bazooka", bElemTypes::_bazookaType},
        {"bunker", bElemTypes::_bunker},
        {"teleporter", bElemTypes::_teleporter},
        {"bouba", bElemTypes::_boubaType},
        {"kiki", bElemTypes::_kikiType},
        {"bomb", bElemTypes::_simpleBombType},
        {"landmine", bElemTypes::_landmineType},
        {"golden_apple", bElemTypes::_goldenAppleType},
    };
    return names;
}

std::optional<int> chunkPattern::typeByName(std::string_view name)
{
    for (const auto &[n, t] : typeNames())
        if (n == name)
            return t;
    return std::nullopt;
}

bool chunkPattern::placeable(int type)
{
    return std::ranges::any_of(typeNames(), [type](const auto &n) { return n.second == type; });
}

} // namespace goe
