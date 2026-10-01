#ifndef GOE_AGENT_FEATURES_H
#define GOE_AGENT_FEATURES_H

#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class bElem;

/**
 * The numbers an agent can read about an element. The same table describes a cell of the vision
 * grid (its top element), an item in the inventory and the player itself, so a name means the
 * same thing wherever it is used.
 *
 * Every value is a float. Yes/no qualities are 0 or 1, timed states (moving, dying...) are the
 * ticks left (0 when not in that state), directions are dir::direction (0 up, 1 left, 2 down,
 * 3 right, 4 none).
 */
namespace goe::agent {

/// one number read from an element
struct feature
{
    std::string_view name;
    std::string_view about;
    float (*read)(bElem &);
    /// the value where there is no element (an empty inventory slot, a cell not built yet)
    float absent = 0.0f;
};

/// what any element has: type, subtype, qualities (bElemAttr) and states (bElemStats)
std::span<const feature> elementFeatures();
/// what only the player has: position, score, view radius, difficulty...
std::span<const feature> playerFeatures();

/// the names of a table, in order
std::vector<std::string> namesOf(std::span<const feature> table);

/// the features with these names, looked up in the tables in turn; throws std::invalid_argument
/// naming the first unknown one
std::vector<feature> select(const std::vector<std::string> &names,
                            std::initializer_list<std::span<const feature>> tables);

} // namespace goe::agent

#endif // GOE_AGENT_FEATURES_H
