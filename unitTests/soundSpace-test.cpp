#include <gtest/gtest.h>
#include "soundSpace.h"

// OpenAL's default listener faces -z with +y up, so +x is the right ear and -z is in front.

TEST(SoundSpace, RightOfThePlayerIsTheRightEar)
{
    auto [x, y, z] = soundSpace::relative({13, 7, 0}, {10, 7, 0});
    EXPECT_FLOAT_EQ(x, 3.0f);
    EXPECT_FLOAT_EQ(z, 0.0f);
}

TEST(SoundSpace, LeftOfThePlayerIsTheLeftEar)
{
    auto [x, y, z] = soundSpace::relative({5, 7, 0}, {10, 7, 0});
    EXPECT_FLOAT_EQ(x, -5.0f);
}

TEST(SoundSpace, UpOnTheScreenIsInFrontAndDownIsBehind)
{
    EXPECT_LT(soundSpace::relative({10, 2, 0}, {10, 7, 0})[2], 0.0f);
    EXPECT_GT(soundSpace::relative({10, 9, 0}, {10, 7, 0})[2], 0.0f);
}

TEST(SoundSpace, CellsAreTheSameSizeInBothDirections)
{
    // a wide board must not squeeze one axis: 4 cells right and 4 cells down are as far
    auto right = soundSpace::relative({14, 7, 0}, {10, 7, 0});
    auto down = soundSpace::relative({10, 11, 0}, {10, 7, 0});
    EXPECT_FLOAT_EQ(right[0], down[2]);
}

TEST(SoundSpace, OffsetFollowsTheListener)
{
    // the same sound moves to the other ear when the player walks past it
    EXPECT_GT(soundSpace::relative({20, 5, 0}, {18, 5, 0})[0], 0.0f);
    EXPECT_LT(soundSpace::relative({20, 5, 0}, {22, 5, 0})[0], 0.0f);
}

TEST(SoundSpace, SoundOnThePlayersCellIsBelowTheEars)
{
    auto [x, y, z] = soundSpace::relative({10, 7, 0}, {10, 7, 5});
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, -soundSpace::earHeight);
    EXPECT_FLOAT_EQ(z, 0.0f);
}
