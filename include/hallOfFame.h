#ifndef HALLOFFAME_H
#define HALLOFFAME_H
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace goe {

/**
 * The best games: a name, a score and a date for each, best first, kept in a small JSON file in
 * the save folder. A lost game whose best score earns a place asks for a name; the title screen
 * shows the list between demos. The demo never enters it.
 */
class hallOfFame
{
public:
    static constexpr std::size_t places = 10;
    /// names longer than this are cut
    static constexpr std::size_t nameLength = 16;
    static constexpr const char *fileName = "halloffame.json";
    /// the name of a player who gave none
    static constexpr const char *nobody = "Anonymous";

    struct entry
    {
        std::string name;
        int score = 0;
        std::string date;
    };

    /// the list in the file; empty when the file is missing or cannot be read
    static hallOfFame load(const std::filesystem::path &file);
    /// writes the list (to a temporary file first, so a failed write keeps the old list)
    bool save(const std::filesystem::path &file) const;
    /// whether a game with this score gets a place: more than 0, and better than the last place
    /// while the list is full
    bool qualifies(int score) const;
    /// puts the entry in its place (after the entries with the same score, which came first) and
    /// drops what falls off the end; the entry's place counted from 0, none when it got none.
    /// A blank name becomes nobody; a long one is cut.
    std::optional<std::size_t> add(entry e);
    const std::vector<entry> &entries() const { return this->list; }

private:
    std::vector<entry> list;
};

/// today's date, YYYY-MM-DD
std::string today();

} // namespace goe

#endif // HALLOFFAME_H
