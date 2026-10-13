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


#include "model3d.h"
#include "rig.h"
#include <cmath>
#include <gtest/gtest.h>

namespace {
const std::string knight = "data/models/kaykit-adventurers/Knight.glb";
const std::string minion = "data/models/kaykit-skeletons/Skeleton_Minion.glb";

/// how far apart two poses of the same mesh are, at the corner that moved most
float furthest(const goe::models::mesh &a, const goe::models::mesh &b)
{
    float most = 0;
    for (std::size_t i = 0; i < a.triangles.size() && i < b.triangles.size(); i++)
        most = std::max(most, std::hypot(a.triangles[i].x - b.triangles[i].x, a.triangles[i].y - b.triangles[i].y,
                                         a.triangles[i].z - b.triangles[i].z));
    return most;
}
} // namespace

TEST(Rig, ReadsACharacterWithItsClips)
{
    const auto r = goe::models::rig::load(knight);
    ASSERT_TRUE(r.has_value());
    EXPECT_GT(r->clips(), 10u);
    for (const char *name : {"Idle", "Walking_A", "Death_A"}) {
        EXPECT_GE(r->clip(name), 0) << name;
        EXPECT_GT(r->length(r->clip(name)), 0.0f) << name;
    }
    EXPECT_EQ(r->clip("Moonwalk"), -1);
    EXPECT_EQ(r->length(-1), 0.0f);
}

TEST(Rig, TheBoundPoseIsWhereTheStillModelStands)
{
    // the joints stand where the mesh was bound to them, so nothing moves
    const std::vector<std::string> body{"Knight_Body", "Knight_Head", "Knight_ArmLeft", "Knight_ArmRight"};
    const auto r = goe::models::rig::load(knight, body);
    const auto still = goe::models::load(knight, body);
    ASSERT_TRUE(r && still);
    const auto bound = r->pose(-1, 0);
    ASSERT_EQ(bound.triangles.size(), still->triangles.size());
    EXPECT_LT(furthest(bound, *still), 0.01f);
    EXPECT_FALSE(r->image().empty()) << "painted with its texture";
}

TEST(Rig, WalkingMovesTheLegsAndStandsUp)
{
    const auto r = goe::models::rig::load(minion);
    ASSERT_TRUE(r.has_value());
    const int walk = r->clip("Walking_A");
    ASSERT_GE(walk, 0);
    const auto a = r->pose(walk, 0), b = r->pose(walk, r->length(walk) / 2);
    EXPECT_GT(furthest(a, b), 0.05f) << "half a stride later the legs have swapped";
    // upright in the board's axes: z up, taller than wide
    const auto box = a.bounds();
    EXPECT_GT(box.size(2), box.size(0));
    EXPECT_GT(box.size(2), box.size(1));
    // held at the ends, never extrapolated
    EXPECT_LT(furthest(r->pose(walk, -5), r->pose(walk, 0)), 1e-5f);
    EXPECT_LT(furthest(r->pose(walk, 99), r->pose(walk, r->length(walk))), 1e-5f);
}

TEST(Rig, ASwordInTheHandFollowsTheHand)
{
    // the sword is no skinned mesh, it hangs on the hand's node
    const std::vector<std::string> sword{"1H_Sword"};
    const auto r = goe::models::rig::load(knight, sword);
    ASSERT_TRUE(r.has_value());
    const int chop = r->clip("1H_Melee_Attack_Chop");
    ASSERT_GE(chop, 0);
    EXPECT_GT(furthest(r->pose(chop, 0), r->pose(chop, r->length(chop) / 2)), 0.1f);
}

TEST(Rig, DyingEndsOnTheFloor)
{
    const auto r = goe::models::rig::load(minion);
    ASSERT_TRUE(r.has_value());
    const int idle = r->clip("Idle"), death = r->clip("Death_A");
    const float standing = r->pose(idle, 0).bounds().size(2);
    const float lying = r->pose(death, r->length(death)).bounds().max[2];
    EXPECT_LT(lying, standing / 2);
}
