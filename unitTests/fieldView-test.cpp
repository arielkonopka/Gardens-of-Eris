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

#include "chamber.h"
#include "elementFactory.h"
#include "elements.h"
#include "fieldCamera.h"
#include "isoMesh.h"
#include <algorithm>
#include "spriteFrame.h"
#include <gtest/gtest.h>

namespace {
constexpr coords tile{64, 64};

/// a definition with one picture per sequence, so the animation phase does not matter
videoElement::videoElementDef oneFramePerSequence()
{
    videoElement::videoElementDef def(nullptr);
    // two subtypes, four facings each; cell (subtype, facing)
    for (int st = 0; st < 2; st++) {
        std::vector<std::vector<coords>> facings;
        for (int f = 0; f < 4; f++)
            facings.push_back({coords(st, f)});
        def.defArray.push_back(facings);
    }
    def.dying = {coords(10, 0)};
    def.destroying = {coords(11, 0)};
    def.teleporting = {coords(12, 0)};
    def.fadingOut = {coords(13, 0)};
    def.fadingIn = {coords(14, 0)};
    return def;
}
} // namespace

TEST(FieldCamera, GlidesTowardsTheCentreAStepAtATime)
{
    fieldCamera cam(tile);
    // the window of 10 x 6 cells centred on (20, 13) starts at (15, 10)
    cam.follow({20, 13}, {10, 6});
    EXPECT_EQ(cam.scroll(), coords(15 * fieldCamera::step % 64, 10 * fieldCamera::step % 64));
    for (int i = 0; i < 200; i++)
        cam.follow({20, 13}, {10, 6});
    EXPECT_EQ(cam.topLeft(), coords(15, 10));
    EXPECT_EQ(cam.scroll(), coords(0, 0));
    EXPECT_FLOAT_EQ(cam.leftCells(), 15.0f);
}

TEST(FieldCamera, SettlesOnWholeCellsWhenGoingNegative)
{
    fieldCamera cam(tile);
    for (int i = 0; i < 400; i++)
        cam.follow({-7, -3}, {10, 6});
    EXPECT_EQ(cam.topLeft(), coords(-12, -6));
    EXPECT_EQ(cam.scroll(), coords(0, 0));
}

TEST(FieldCamera, ABoundedBoardKeepsTheWindowInside)
{
    fieldCamera cam(tile);
    for (int i = 0; i < 400; i++)
        cam.follow({1, 1}, {10, 6}, coords(30, 20));
    EXPECT_EQ(cam.topLeft(), coords(0, 0));
    for (int i = 0; i < 800; i++)
        cam.follow({29, 19}, {10, 6}, coords(30, 20));
    EXPECT_EQ(cam.topLeft(), coords(20, 14));
}

TEST(SpriteFrame, SubtypeAndFacingPickThePicture)
{
    auto board = chamber::makeNewChamber(coords(10, 10));
    auto w = elementFactory::generateAnElement<wall>(board, 1);
    const auto def = oneFramePerSequence();
    w->getStats()->setFacing(dir::direction::DOWN);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(1, 2));
    // a subtype beyond the definitions wraps around
    w->getAttrs()->setSubtype(3);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(1, 2));
}

TEST(SpriteFrame, DyingDestroyingAndTeleportingShowTheirSequences)
{
    auto board = chamber::makeNewChamber(coords(10, 10));
    const auto def = oneFramePerSequence();
    auto w = elementFactory::generateAnElement<wall>(board, 0);
    w->getStats()->setTelInProgress(5);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(12, 0));
    w->getStats()->setFadingIn(5);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(14, 0));
    w->getStats()->setFadingOut(5);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(13, 0));
    w->getStats()->setDestroyed(5);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(11, 0));
    w->getStats()->setKilled(5);
    EXPECT_EQ(goe::sprites::frame(*w, def), coords(10, 0));
}

TEST(SpriteFrame, NoPictureWhenTheDefinitionIsEmpty)
{
    auto board = chamber::makeNewChamber(coords(10, 10));
    auto w = elementFactory::generateAnElement<wall>(board, 0);
    EXPECT_FALSE(goe::sprites::frame(*w, videoElement::videoElementDef(nullptr)).has_value());
}

TEST(SpriteFrame, SheetPixelsSkipTheSpacing)
{
    EXPECT_EQ(goe::sprites::pixelOf({0, 0}, tile, 4), coords(4, 4));
    EXPECT_EQ(goe::sprites::pixelOf({2, 1}, tile, 4), coords(2 * 64 + 3 * 4, 64 + 2 * 4));
}

namespace {
using goe::iso::meshBuilder;
using goe::iso::shape;
const goe::iso::picture somePicture{{4, 4}, {64, 64}};
/// the camera of the 3D view: from the south west
meshBuilder fromSouthWest() { return meshBuilder(-1.0f, 1.0f); }
} // namespace

TEST(IsoMesh, ShapesByNameAndByDefault)
{
    EXPECT_EQ(goe::iso::parseShape("block"), shape::block);
    EXPECT_EQ(goe::iso::parseShape("ground"), shape::ground);
    EXPECT_EQ(goe::iso::parseShape("standee"), shape::standee);
    EXPECT_EQ(goe::iso::parseShape("flat"), shape::flat);
    EXPECT_FALSE(goe::iso::parseShape("cube").has_value());
    EXPECT_EQ(goe::iso::defaultShape(bElemTypes::_floorType), shape::ground);
    EXPECT_EQ(goe::iso::defaultShape(bElemTypes::_wallType), shape::block);
    EXPECT_EQ(goe::iso::defaultShape(bElemTypes::_door), shape::block);
    EXPECT_EQ(goe::iso::defaultShape(bElemTypes::_landmineType), shape::flat);
    EXPECT_EQ(goe::iso::defaultShape(bElemTypes::_player), shape::standee);
}

TEST(IsoMesh, TheFloorIsOneSquareLaidOverTheBoardColour)
{
    auto m = fromSouthWest();
    m.ground(3, 4, somePicture);
    ASSERT_EQ(m.vertices().size(), 6u);
    for (const auto &v : m.vertices()) {
        EXPECT_FLOAT_EQ(v.z, 0.0f);
        EXPECT_TRUE(v.backdrop);
        EXPECT_GE(v.x, 3.0f);
        EXPECT_LE(v.x, 4.0f);
        EXPECT_GE(v.y, 4.0f);
        EXPECT_LE(v.y, 5.0f);
        EXPECT_GE(v.u, 4.0f);
        EXPECT_LE(v.u, 68.0f);
    }
}

TEST(IsoMesh, ABlockShowsItsTopAndTheTwoSidesFacingTheCamera)
{
    auto m = fromSouthWest();
    m.block(0, 0, somePicture);
    // top, south side and west side: three squares
    ASSERT_EQ(m.vertices().size(), 18u);
    int top = 0, south = 0, west = 0;
    for (const auto &v : m.vertices()) {
        EXPECT_FALSE(v.backdrop);
        if (v.light == goe::iso::light::top) {
            top++;
            EXPECT_FLOAT_EQ(v.z, meshBuilder::blockHeight);
        } else if (v.light == goe::iso::light::southFace) {
            south++;
            EXPECT_FLOAT_EQ(v.y, 1.0f);
        } else if (v.light == goe::iso::light::westFace) {
            west++;
            EXPECT_FLOAT_EQ(v.x, 0.0f);
        }
    }
    EXPECT_EQ(top, 6);
    EXPECT_EQ(south, 6);
    EXPECT_EQ(west, 6);
    // seen from the north east, the other two sides
    meshBuilder ne(1.0f, -1.0f);
    ne.block(0, 0, somePicture);
    for (const auto &v : ne.vertices())
        EXPECT_TRUE(v.light == goe::iso::light::top || v.light == goe::iso::light::northFace
                    || v.light == goe::iso::light::eastFace);
}

TEST(IsoMesh, ACutBlockIsLow)
{
    auto m = fromSouthWest();
    m.add(shape::block, 0, 0, somePicture, true);
    float highest = 0;
    for (const auto &v : m.vertices())
        highest = std::max(highest, v.z);
    EXPECT_FLOAT_EQ(highest, meshBuilder::cutHeight);
}

TEST(IsoMesh, AStandeeStandsInTheMiddleOfItsCellFacingTheCamera)
{
    auto m = fromSouthWest();
    m.standee(2, 2, somePicture);
    ASSERT_EQ(m.vertices().size(), 6u);
    float cx = 0, cy = 0, highest = 0;
    for (const auto &v : m.vertices()) {
        EXPECT_FALSE(v.backdrop);
        // across the line of sight: every corner is as far towards the camera as the cell's middle
        EXPECT_NEAR((v.x - 2.5f) * -1.0f + (v.y - 2.5f) * 1.0f, 0.0f, 1e-5f);
        cx += v.x;
        cy += v.y;
        highest = std::max(highest, v.z);
    }
    EXPECT_NEAR(cx / 6, 2.5f, 0.2f);
    EXPECT_NEAR(cy / 6, 2.5f, 0.2f);
    EXPECT_FLOAT_EQ(highest, meshBuilder::standeeSize);
    // the picture's top left is on the left as the camera sees it, the west side
    const auto &first = m.vertices()[0];
    EXPECT_FLOAT_EQ(first.u, 4.0f);
    EXPECT_LT(first.x, 2.5f);
}
