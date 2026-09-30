#include <gtest/gtest.h>
#include "fogMask.h"
#include "viewPoint.h"
#include <vector>

namespace {
vpPoint point(int x, int y, float radius)
{
    vpPoint p;
    p.x = x;
    p.y = y;
    p.radius = radius;
    return p;
}
constexpr int cell = fogMask::cellPx;
} // namespace

TEST(FogMask, CoversTheWholeSceneInCells)
{
    fogMask m(100, 41);
    EXPECT_EQ(m.width(), (100 + cell - 1) / cell);
    EXPECT_EQ(m.height(), (41 + cell - 1) / cell);
    EXPECT_EQ(m.texels().size(), static_cast<std::size_t>(m.width() * m.height()));
}

TEST(FogMask, NoViewPointsMeansAllHidden)
{
    fogMask m(64, 64);
    EXPECT_FALSE(m.paint({}, {0, 0}));
    for (auto v : m.texels())
        EXPECT_EQ(v, 0);
}

TEST(FogMask, SeenAtThePointAndHiddenBeyondItsRadius)
{
    fogMask m(320, 320);
    std::vector<vpPoint> pts{point(160 - cell / 2, 160 - cell / 2, 64.0f)};
    EXPECT_TRUE(m.paint(pts, {0, 0}));
    EXPECT_GT(m.at(160 / cell - 1, 160 / cell - 1), 250);
    EXPECT_EQ(m.at(0, 0), 0);
    EXPECT_EQ(m.at(160 / cell + 64 / cell + 1, 160 / cell), 0);
}

TEST(FogMask, FadesWithDistance)
{
    fogMask m(320, 320);
    std::vector<vpPoint> pts{point(0, 160, 200.0f)};
    m.paint(pts, {0, 0});
    const int row = 160 / cell;
    for (int cx = 1; cx < m.width(); cx++)
        EXPECT_LE(m.at(cx, row), m.at(cx - 1, row));
}

TEST(FogMask, CentreShiftMovesThePointToTheMiddleOfItsTile)
{
    fogMask a(320, 320), b(320, 320);
    std::vector<vpPoint> corner{point(64, 64, 64.0f)};
    std::vector<vpPoint> middle{point(96, 96, 64.0f)};
    a.paint(corner, {32, 32});
    b.paint(middle, {0, 0});
    EXPECT_TRUE(std::ranges::equal(a.texels(), b.texels()));
}

TEST(FogMask, OverlappingPointsTakeTheBetterView)
{
    fogMask one(320, 320), two(320, 320);
    std::vector<vpPoint> single{point(100, 100, 80.0f)};
    std::vector<vpPoint> both{point(100, 100, 80.0f), point(140, 100, 40.0f)};
    one.paint(single, {0, 0});
    two.paint(both, {0, 0});
    for (std::size_t i = 0; i < one.texels().size(); i++)
        EXPECT_GE(two.texels()[i], one.texels()[i]);
}

TEST(FogMask, PointsWithoutSightAndOffScreenAreSafe)
{
    fogMask m(64, 64);
    std::vector<vpPoint> pts{point(10, 10, -1.0f), point(-5000, 9000, 300.0f), point(32, 32, 0.0f)};
    EXPECT_FALSE(m.paint(pts, {0, 0}));
}

TEST(FogMask, RepaintingTheSameViewReportsNoChange)
{
    fogMask m(200, 200);
    std::vector<vpPoint> pts{point(50, 50, 64.0f)};
    EXPECT_TRUE(m.paint(pts, {0, 0}));
    EXPECT_FALSE(m.paint(pts, {0, 0}));
    pts[0].x += 8;
    EXPECT_TRUE(m.paint(pts, {0, 0}));
}
