// goe._goe: the agent interface (agentGame.h) for Python, with nanobind. The Python files of the
// package (goe/) wrap it into the ViZDoom-like Game and the Gymnasium-style GoeEnv.
#include "agentGame.h"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
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
    m.attr("ACTIONS") = actions;
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
        .def_rw("allow_give_up", &config::allowGiveUp);

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
