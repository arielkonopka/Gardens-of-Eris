#ifndef CHUNKPATTERN_H
#define CHUNKPATTERN_H
#include "commons.h"
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace goe {

/// one cell of a chunk pattern: what is placed on the floor there
struct patternCell
{
    /// bElemTypes; -1 (_belemType) places nothing, 0 (_floorType) gives the floor itself the subtype
    int type = bElemTypes::_belemType;
    int subtype = 0;
    bool operator==(const patternCell &) const = default;
};

/**
 * A fixed layout of elements for a chunk of the endless world, built instead of a random maze
 * (see worldBuilder::setPattern). Agents use it to train on a world they choose, the same in
 * every episode.
 *
 * A pattern is width x height cells, at most a chunk (chamber::chunkSize) on each side. A smaller
 * one is repeated to fill the chunk, starting at the chunk's top left cell. Cells are given row
 * by row: cell (x, y) is cells[y * width + x], y growing downwards as on the board.
 *
 * A patterned chunk has no maze walls and no chunk walls of its own; a wall is wherever the
 * pattern puts one. In the start chunk the first player in the pattern (row by row) is the one
 * the game starts with; when the pattern has none, the player goes on the free floor nearest the
 * chunk's middle. Players in other chunks are spare avatars.
 */
class chunkPattern
{
public:
    chunkPattern(int width, int height, std::vector<patternCell> cells);
    /// from rows of characters, all as long as the first, and what each character stands for
    static chunkPattern fromRows(const std::vector<std::string> &rows, const std::map<char, patternCell> &legend);
    /**
     * From JSON text:
     *   {"legend": {"#": "wall", "@": "player", "k": ["key", 1], "D": [52, 1], ".": null},
     *    "rows": ["#####", "#@.k#", "#.D.#"]}
     * A legend entry is null (nothing), a type (its number or its name, see typeByName), or
     * [type, subtype]. Throws std::invalid_argument when the text is not such a pattern.
     */
    static chunkPattern fromJson(std::string_view text);
    /// fromJson with the file's text
    static chunkPattern load(const std::filesystem::path &file);

    int width() const { return this->w; }
    int height() const { return this->h; }
    const std::vector<patternCell> &cells() const { return this->grid; }
    /// the cell at (x, y) of a chunk: the pattern repeated to fill it
    const patternCell &at(int x, int y) const;
    /// whether some cell places an element of the type
    bool places(int type) const;
    bool operator==(const chunkPattern &) const = default;

    /// whether a pattern may place the type: anything the game builds a world from (not missiles)
    static bool placeable(int type);
    /// the names a pattern may use for types: "wall", "player", "golden_apple", ...
    static const std::vector<std::pair<std::string, int>> &typeNames();
    static std::optional<int> typeByName(std::string_view name);

private:
    int w, h;
    std::vector<patternCell> grid;
};

} // namespace goe

#endif // CHUNKPATTERN_H
