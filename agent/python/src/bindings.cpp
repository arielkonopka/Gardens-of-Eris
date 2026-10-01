// goe._goe: the agent interface (agentGame.h) for Python, with nanobind. The Python files of the
// package (goe/) wrap it into the ViZDoom-like Game and the Gymnasium-style GoeEnv.
#include "agentGame.h"
#include "chamber.h"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <memory>

namespace nb = nanobind;
using namespace goe::agent;

namespace {
using array = nb::ndarray<nb::numpy, float>;

/// a numpy array that owns the numbers
array toArray(std::vector<float> values, std::initializer_list<std::size_t> shape)
{
    auto owned = std::make_unique<std::vector<float>>(std::move(values));
    float *data = owned->data();
    // the capsule frees the vector when numpy lets go of the array
    nb::capsule holder(owned.get(), [](void *p) noexcept { std::unique_ptr<std::vector<float>>((std::vector<float> *) p); });
    owned.release();
    return array(data, shape, holder);
}

nb::dict stateDict(const game &g, state s)
{
    const auto side = (std::size_t) s.visionSide;
    const std::size_t sections = g.sections().size();
    const auto slots = (std::size_t) g.getConfig().inventorySlots;
    nb::dict d;
    d["vision"] = toArray(std::move(s.vision), {g.cellFeatureNames().size(), side, side});
    d["player"] = toArray(std::move(s.player), {g.playerFeatureNames().size()});
    d["inventory"] = toArray(std::move(s.inventory), {sections, slots, g.itemFeatureNames().size()});
    d["inventory_counts"] = toArray(std::move(s.inventoryCounts), {sections});
    d["centre"] = nb::make_tuple(s.centre.x, s.centre.y);
    d["tick"] = s.tick;
    d["score"] = s.score;
    return d;
}

nb::dict eventDict(const eventCounts &counts)
{
    nb::dict d;
    for (int e = 0; e < eventCount; e++)
        d[eventName((event) e).c_str()] = counts[(std::size_t) e];
    return d;
}

/// the pattern a Python caller holds; the game keeps its own copy, so later changes do not reach it
std::shared_ptr<const goe::chunkPattern> shared(const std::optional<goe::chunkPattern> &p)
{
    return p ? std::make_shared<const goe::chunkPattern>(*p) : nullptr;
}

std::vector<std::string> describe(std::span<const feature> table)
{
    std::vector<std::string> res;
    for (const auto &f : table)
        res.push_back(std::string(f.name) + ": " + std::string(f.about));
    return res;
}
} // namespace

NB_MODULE(_goe, m)
{
    m.doc() = "Gardens of Eris without a window, for agents (see agent/include/agentGame.h)";
#ifdef GOE_DEFAULT_DATA_DIR
    m.attr("DEFAULT_DATA_DIR") = GOE_DEFAULT_DATA_DIR;
#else
    m.attr("DEFAULT_DATA_DIR") = "";
#endif

    std::vector<std::string> actions, sections;
    for (int a = 0; a < actionCount; a++)
        actions.push_back(actionName((action) a));
    for (int s = 0; s < sectionCount; s++)
        sections.push_back(sectionName((section) s));
    std::vector<std::string> events;
    for (int e = 0; e < eventCount; e++)
        events.push_back(eventName((event) e));
    m.attr("ACTIONS") = actions;
    m.attr("EVENTS") = events;
    m.attr("SECTIONS") = sections;
    m.attr("ELEMENT_FEATURES") = namesOf(elementFeatures());
    m.attr("PLAYER_FEATURES") = namesOf(playerFeatures());
    m.attr("CELL_FEATURES") = std::vector<std::string>{"exists", "in_sight"};
    m.attr("ITEM_FEATURES") = std::vector<std::string>{"selected"};
    m.def("describe_features", []() {
        auto all = describe(elementFeatures());
        for (auto &d : describe(playerFeatures()))
            all.push_back(std::move(d));
        return all;
    }, "every element and player feature, with what it means");

    std::map<std::string, int> types;
    for (const auto &[name, type] : goe::chunkPattern::typeNames())
        types[name] = type;
    m.attr("ELEMENT_TYPES") = types;
    m.attr("CHUNK_SIZE") = chamber::chunkSize;

    nb::class_<goe::chunkPattern>(m, "ChunkPattern", "a fixed layout of elements for a chunk (include/chunkPattern.h)")
        .def(
            "__init__",
            [](goe::chunkPattern *self, int width, int height, const std::vector<int> &types, const std::vector<int> &subtypes) {
                if (types.size() != subtypes.size())
                    throw nb::value_error("types and subtypes differ in length");
                std::vector<goe::patternCell> cells;
                for (std::size_t i = 0; i < types.size(); i++)
                    cells.push_back({types[i], subtypes[i]});
                new (self) goe::chunkPattern(width, height, std::move(cells));
            },
            nb::arg("width"), nb::arg("height"), nb::arg("types"), nb::arg("subtypes"))
        .def_static("from_json", [](const std::string &text) { return goe::chunkPattern::fromJson(text); }, nb::arg("text"))
        .def_static("load", &goe::chunkPattern::load, nb::arg("path"))
        .def_prop_ro("width", &goe::chunkPattern::width)
        .def_prop_ro("height", &goe::chunkPattern::height)
        .def_prop_ro("types", [](const goe::chunkPattern &p) {
            std::vector<int> res;
            for (const auto &c : p.cells())
                res.push_back(c.type);
            return res;
        })
        .def_prop_ro("subtypes", [](const goe::chunkPattern &p) {
            std::vector<int> res;
            for (const auto &c : p.cells())
                res.push_back(c.subtype);
            return res;
        })
        .def(nb::self == nb::self);

    nb::class_<config>(m, "Config", "how the game is observed and played; see goe.Game for the meaning of each field")
        .def(nb::init<>())
        .def_rw("data_dir", &config::dataDir)
        .def_rw("vision_radius", &config::visionRadius)
        .def_rw("circle", &config::circle)
        .def_rw("follow_player", &config::followPlayer)
        .def_prop_rw(
            "fixed_centre", [](const config &c) { return nb::make_tuple(c.fixedCentre.x, c.fixedCentre.y); },
            [](config &c, std::pair<int, int> p) { c.fixedCentre = coords(p.first, p.second); })
        .def_rw("cell_features", &config::cellFeatures)
        .def_rw("player_features", &config::playerFeatures)
        .def_prop_rw(
            "inventory_sections",
            [](const config &c) {
                std::vector<std::string> res;
                for (auto s : c.inventorySections)
                    res.push_back(sectionName(s));
                return res;
            },
            [](config &c, const std::vector<std::string> &names) {
                c.inventorySections.clear();
                for (const auto &n : names) {
                    int found = -1;
                    for (int s = 0; s < sectionCount; s++)
                        if (sectionName((section) s) == n)
                            found = s;
                    if (found < 0)
                        throw nb::value_error(("unknown inventory section: " + n).c_str());
                    c.inventorySections.push_back((section) found);
                }
            })
        .def_rw("inventory_slots", &config::inventorySlots)
        .def_rw("item_features", &config::itemFeatures)
        .def_rw("ticks_per_step", &config::ticksPerStep)
        .def_rw("episode_ticks", &config::episodeTicks)
        .def_rw("allow_give_up", &config::allowGiveUp)
        .def_rw("reward_weights", &config::rewardWeights)
        .def_prop_rw(
            "chunk_patterns",
            [](const config &c) {
                std::map<std::pair<int, int>, goe::chunkPattern> res;
                for (const auto &[chunk, p] : c.chunkPatterns)
                    res.emplace(chunk, *p);
                return res;
            },
            [](config &c, const std::map<std::pair<int, int>, goe::chunkPattern> &patterns) {
                c.chunkPatterns.clear();
                for (const auto &[chunk, p] : patterns)
                    c.chunkPatterns[chunk] = std::make_shared<const goe::chunkPattern>(p);
            })
        .def_prop_rw(
            "default_pattern",
            [](const config &c) { return c.defaultPattern ? std::optional(*c.defaultPattern) : std::nullopt; },
            [](config &c, const std::optional<goe::chunkPattern> &p) { c.defaultPattern = shared(p); });

    nb::class_<game>(m, "Game")
        .def(nb::init<config>(), nb::arg("config"))
        .def("new_episode", &game::newEpisode, nb::arg("seed") = nb::none(), nb::call_guard<nb::gil_scoped_release>())
        .def(
            "make_action", [](game &g, int a) { return g.makeAction((action) a); }, nb::arg("action"),
            nb::call_guard<nb::gil_scoped_release>())
        .def("get_state", [](const game &g) { return stateDict(g, g.getState()); })
        .def("is_episode_finished", &game::isEpisodeFinished)
        .def("is_player_dead", &game::isPlayerDead)
        .def("is_truncated", &game::isTruncated)
        .def_prop_ro("action_taken", &game::actionTaken)
        .def_prop_ro("episode_tick", &game::episodeTick)
        .def_prop_ro("avatars_lost", &game::avatarsLost)
        .def_prop_ro("score", &game::score)
        .def_prop_ro("seed", &game::seed)
        .def_prop_ro("step_events", [](const game &g) { return eventDict(g.stepEvents()); })
        .def_prop_ro("episode_events", [](const game &g) { return eventDict(g.episodeEvents()); })
        .def(
            "set_chunk_pattern",
            [](game &g, std::pair<int, int> chunk, const std::optional<goe::chunkPattern> &p) { g.setChunkPattern(chunk, shared(p)); },
            nb::arg("chunk"), nb::arg("pattern").none())
        .def(
            "set_default_pattern", [](game &g, const std::optional<goe::chunkPattern> &p) { g.setDefaultPattern(shared(p)); },
            nb::arg("pattern").none())
        .def("clear_chunk_patterns", &game::clearChunkPatterns)
        .def(
            "chunk_at", [](const game &g, int x, int y) { return g.chunkAt(coords(x, y)); }, nb::arg("x"), nb::arg("y"))
        .def_prop_ro("action_count", &game::actions)
        .def_prop_ro("cell_feature_names", &game::cellFeatureNames)
        .def_prop_ro("player_feature_names", &game::playerFeatureNames)
        .def_prop_ro("item_feature_names", &game::itemFeatureNames)
        .def_prop_ro("inventory_section_names", [](const game &g) {
            std::vector<std::string> res;
            for (auto s : g.sections())
                res.push_back(sectionName(s));
            return res;
        });
}
