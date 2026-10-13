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


#include "configManager.h"
#include "model3d.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <sstream>

namespace {
const std::string dungeon = "data/models/kaykit-dungeon/";

std::vector<goe::models::spec> specsOf(const char *json)
{
    rapidjson::Document d;
    d.Parse(json);
    return readModelSpecs(d, "base/");
}

/// one triangle, enough to test placing and turning
goe::models::mesh triangle()
{
    goe::models::mesh m;
    m.triangles = {{0, 0, 0, 0, 1, 0, 0, 0}, {2, 0, 0, 0, 1, 0, 1, 0}, {2, 4, 3, 0, 1, 0, 1, 1}};
    return m;
}
} // namespace

TEST(Model3d, ReadsAGlbIntoTheBoardsAxes)
{
    // the KayKit wall is 4 wide (x), 4 tall (glTF y) and 1 thick (glTF z)
    const auto wall = goe::models::load(dungeon + "wall.glb");
    ASSERT_TRUE(wall.has_value());
    EXPECT_EQ(wall->triangles.size() % 3, 0u);
    EXPECT_GT(wall->triangles.size(), 100u);
    const auto b = wall->bounds();
    EXPECT_NEAR(b.size(0), 4.0f, 0.01f);
    EXPECT_NEAR(b.size(1), 1.0f, 0.01f); // thickness runs north to south
    EXPECT_NEAR(b.size(2), 4.0f, 0.01f); // height is up
    EXPECT_NEAR(b.min[2], 0.0f, 0.01f);
    EXPECT_FALSE(wall->image.empty());
    EXPECT_EQ(wall->imageType, ".png");
}

TEST(Model3d, ReadsOnlyTheNodesItIsAskedFor)
{
    const auto whole = goe::models::load(dungeon + "wall_doorway.glb");
    const std::vector<std::string> frame{"wall_doorway"};
    const auto open = goe::models::load(dungeon + "wall_doorway.glb", frame);
    ASSERT_TRUE(whole && open);
    EXPECT_LT(open->triangles.size(), whole->triangles.size());
    const std::vector<std::string> nothing{"no such node"};
    EXPECT_FALSE(goe::models::load(dungeon + "wall_doorway.glb", nothing).has_value());
    EXPECT_FALSE(goe::models::load(dungeon + "missing.glb").has_value());
}

TEST(Model3d, PlacesAModelInTheMiddleOfItsCell)
{
    goe::models::spec s;
    // scaled so the footprint fills the cell: 2 x 4 becomes 0.5 x 1
    const auto fit = goe::models::placed(triangle(), s).bounds();
    EXPECT_NEAR(fit.min[0], -0.25f, 1e-5f);
    EXPECT_NEAR(fit.max[0], 0.25f, 1e-5f);
    EXPECT_NEAR(fit.min[1], -0.5f, 1e-5f);
    EXPECT_NEAR(fit.max[1], 0.5f, 1e-5f);
    EXPECT_NEAR(fit.max[2], 0.75f, 1e-5f);
    // a scale, a stretch and a lift
    s.scale = 0.5f;
    s.stretch = {1, 1, 2};
    s.lift = 0.1f;
    const auto lifted = goe::models::placed(triangle(), s).bounds();
    EXPECT_NEAR(lifted.size(0), 1.0f, 1e-5f);
    EXPECT_NEAR(lifted.min[2], 0.1f, 1e-5f);
    EXPECT_NEAR(lifted.max[2], 0.1f + 3.0f, 1e-5f);
    // a quarter turn swaps the footprint's sides
    s.turn = 90;
    const auto turnedBox = goe::models::placed(triangle(), s).bounds();
    EXPECT_NEAR(turnedBox.size(0), 2.0f, 1e-4f);
    EXPECT_NEAR(turnedBox.size(1), 1.0f, 1e-4f);
}

TEST(Model3d, AWallSegmentHasItsFrontOnTheSouthernEdge)
{
    goe::models::spec s;
    s.scale = 0.25f;
    s.place = goe::models::layout::sides;
    const auto wall = goe::models::placed(*goe::models::load(dungeon + "wall.glb"), s).bounds();
    EXPECT_NEAR(wall.max[1], 0.5f, 1e-5f);
    EXPECT_NEAR(wall.size(1), 0.25f, 0.01f);
    EXPECT_NEAR(wall.size(0), 1.0f, 0.01f);
}

TEST(Model3d, TurnsClockwiseSeenFromAbove)
{
    goe::models::mesh m;
    // a point in the south, its face looking south
    m.triangles = {{0, 1, 0, 0, 1, 0, 0, 0}};
    const auto west = goe::models::turned(m, 1).triangles[0];
    EXPECT_NEAR(west.x, -1.0f, 1e-6f);
    EXPECT_NEAR(west.y, 0.0f, 1e-6f);
    EXPECT_NEAR(west.nx, -1.0f, 1e-6f);
    const auto north = goe::models::turned(m, 2).triangles[0];
    EXPECT_NEAR(north.y, -1.0f, 1e-6f);
    const auto east = goe::models::turned(m, 3).triangles[0];
    EXPECT_NEAR(east.x, 1.0f, 1e-6f);
    EXPECT_NEAR(goe::models::turned(m, 4).triangles[0].y, 1.0f, 1e-6f);
}

TEST(Model3d, FacesWhereTheElementFaces)
{
    // a model looks south; up, left, down, right are 0 to 3
    EXPECT_EQ(goe::models::quartersToFace((int) dir::direction::DOWN), 0);
    EXPECT_EQ(goe::models::quartersToFace((int) dir::direction::LEFT), 1);
    EXPECT_EQ(goe::models::quartersToFace((int) dir::direction::UP), 2);
    EXPECT_EQ(goe::models::quartersToFace((int) dir::direction::RIGHT), 3);
    EXPECT_EQ(goe::models::quartersToFace((int) dir::direction::NODIRECTION), 0);
}

TEST(Model3d, TopsAreLitBrightestAndTheSidesTheCameraSeesNext)
{
    const float top = goe::models::lightOf(0, 0, 1);
    const float south = goe::models::lightOf(0, 1, 0);
    const float west = goe::models::lightOf(-1, 0, 0);
    const float north = goe::models::lightOf(0, -1, 0);
    const float east = goe::models::lightOf(1, 0, 0);
    EXPECT_GT(top, south);
    EXPECT_GT(south, west);
    EXPECT_GT(west, east);
    EXPECT_GT(east, north);
    EXPECT_GT(north, 0.0f);
    EXPECT_LE(top, 1.0f);
}

TEST(Model3d, ALidCoversTheWholeCellAtTheTop)
{
    goe::models::spec s;
    s.scale = 0.25f;
    s.stretch = {1, 1, 0.75f};
    s.place = goe::models::layout::sides;
    const auto wall = goe::models::placed(*goe::models::load(dungeon + "wall.glb"), s);
    const auto top = goe::models::lid(wall);
    ASSERT_TRUE(top.has_value());
    ASSERT_EQ(top->triangles.size(), 6u);
    const auto b = top->bounds();
    EXPECT_NEAR(b.size(0), 1.0f, 1e-6f);
    EXPECT_NEAR(b.size(1), 1.0f, 1e-6f);
    EXPECT_NEAR(b.max[2], wall.bounds().max[2], 0.01f);
    EXPECT_LE(b.max[2], wall.bounds().max[2]);
    // nothing looks up: no lid
    goe::models::mesh side;
    side.triangles = {{0, 0, 0, 0, 1, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}, {1, 0, 1, 0, 1, 0, 0, 0}};
    EXPECT_FALSE(goe::models::lid(side).has_value());
}

TEST(Model3d, ReadsModelEntriesFromSkins)
{
    const auto one = specsOf(R"({"File": "a.glb", "Scale": 0.5, "Lift": 0.2, "Turn": 45, "Turns": true,
                                 "Stretch": [1, 2, 3], "Tint": 0.5, "Layout": "sides", "Nodes": ["n"]})");
    ASSERT_EQ(one.size(), 1u);
    EXPECT_EQ(one[0].file, "base/a.glb");
    EXPECT_EQ(one[0].scale, 0.5f);
    EXPECT_FLOAT_EQ(one[0].lift, 0.2f);
    EXPECT_FLOAT_EQ(one[0].turn, 45.0f);
    EXPECT_TRUE(one[0].turns);
    EXPECT_EQ(one[0].stretch, (std::array<float, 3>{1, 2, 3}));
    EXPECT_EQ(one[0].tint, (std::array<float, 3>{0.5f, 0.5f, 0.5f}));
    EXPECT_EQ(one[0].place, goe::models::layout::sides);
    EXPECT_EQ(one[0].nodes, std::vector<std::string>{"n"});
    // a list, one per subtype and facing; an entry without a file is left out
    const auto list = specsOf(R"([{"File": "closed.glb", "Facing": 0, "Tints": [[1, 0, 0], [0, 1, 0]]},
                                  {"File": "open.glb", "Layout": "nowhere"}, {"Scale": 2}])");
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[1].place, goe::models::layout::cell);
    EXPECT_FALSE(list[1].scale.has_value());
    EXPECT_EQ(list[0].tintFor(1), (std::array<float, 3>{0, 1, 0}));
    EXPECT_EQ(list[0].tintFor(3), (std::array<float, 3>{0, 1, 0}));
    EXPECT_EQ(list[1].tintFor(3), (std::array<float, 3>{1, 1, 1}));
    EXPECT_EQ(goe::models::pick(list, 4, 0), &list[0]);
    EXPECT_EQ(goe::models::pick(list, 4, 1), &list[1]);
    EXPECT_EQ(goe::models::pick({}, 0, 0), nullptr);
    EXPECT_TRUE(specsOf("12").empty());
}

TEST(Model3d, TipsAStandingModelOverAndSetsItOnTheFloor)
{
    // a stick standing up from the floor, 2 high
    goe::models::mesh stick;
    stick.triangles = {{0, 0, 0, 0, 0, 1, 0, 0}, {0.1f, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 2, 0, 0, 1, 0, 0}};
    goe::models::spec s;
    s.scale = 0.5f; // stays 1 long
    s.tip = -90;
    const auto south = goe::models::placed(stick, s).bounds();
    EXPECT_NEAR(south.min[2], 0.0f, 1e-5f);
    EXPECT_NEAR(south.max[2], 0.0f, 1e-5f); // lying flat
    EXPECT_NEAR(south.max[1] - south.min[1], 1.0f, 1e-5f);
    // its tip points south (y grows to the south): the top went to the larger y
    const auto &lying = goe::models::placed(stick, s).triangles;
    EXPECT_GT(lying[2].y, lying[0].y);
    EXPECT_NEAR(lying[2].nz, 0.0f, 1e-5f); // the normal tipped with it
    s.tip = 90;
    const auto &north = goe::models::placed(stick, s).triangles;
    EXPECT_LT(north[2].y, north[0].y);
}

TEST(Model3d, BuiltinShapesStandOnTheFloorInsideTheirCell)
{
    for (const auto *name : {"builtin:blob", "builtin:star"}) {
        const auto m = goe::models::load(name, {});
        ASSERT_TRUE(m.has_value()) << name;
        EXPECT_FALSE(m->triangles.empty());
        EXPECT_TRUE(m->image.empty());
        const auto b = m->bounds();
        EXPECT_NEAR(b.min[2], 0.0f, 1e-4f) << name;
        for (int axis = 0; axis < 2; axis++) {
            EXPECT_GE(b.min[axis], -0.5f) << name;
            EXPECT_LE(b.max[axis], 0.5f) << name;
        }
        for (const auto &c : m->triangles)
            EXPECT_NEAR(std::hypot(c.nx, c.ny, c.nz), 1.0f, 1e-3f) << name;
    }
    EXPECT_FALSE(goe::models::load("builtin:nothing", {}).has_value());
    // the skin's folder is not put in front of a builtin name
    EXPECT_EQ(specsOf(R"({"File": "builtin:blob", "Tip": 30})")[0].file, "builtin:blob");
    EXPECT_FLOAT_EQ(specsOf(R"({"File": "builtin:blob", "Tip": 30})")[0].tip, 30.0f);
}

TEST(Model3d, EveryModelTheSkinNamesLoadsAndIsCredited)
{
    // CC0 needs no credit, but each pack keeps its licence file and a line in CREDITS.md
    std::ifstream in("../CREDITS.md");
    ASSERT_TRUE(in.good());
    std::stringstream creditsMd;
    creditsMd << in.rdbuf();
    int models = 0;
    for (const auto &sprite : configManager::getInstance()->getConfig()->sprites)
        for (const auto &s : sprite.models) {
            models++;
            const auto m = goe::models::load(s.file, s.nodes);
            ASSERT_TRUE(m.has_value()) << s.file << " for " << sprite.name;
            EXPECT_GT(goe::models::placed(*m, s).bounds().max[2], 0.0f) << s.file;
            if (s.file.starts_with(goe::models::builtinPrefix))
                continue; // made in code, nothing to credit
            const std::filesystem::path folder = std::filesystem::path(s.file).parent_path();
            EXPECT_TRUE(std::filesystem::exists(folder / "LICENSE.txt")) << folder;
            EXPECT_NE(creditsMd.str().find("| " + folder.filename().string() + " |"), std::string::npos)
                << folder << " is missing from CREDITS.md";
        }
    EXPECT_GT(models, 0);
}
