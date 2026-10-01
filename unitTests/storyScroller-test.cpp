#include "storyScroller.h"
#include "gameSettings.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <set>

namespace {
/// every letter is ten pixels wide
float tenPerLetter(const std::string &text)
{
    return 10.0f * (float) text.size();
}

std::vector<goe::story> twoStories()
{
    return {{"A", "apple"}, {"", "no title"}};
}
} // namespace

TEST(StoryScroller, ShippedFilesAllHaveStories)
{
    // tests run from GoEoOL, where the game finds data/
    for (const char *file : {"data/txt/stories.json", "data/txt/stories.pl.json", "data/txt/stories.ro.json"}) {
        const auto told = goe::loadStories(file);
        EXPECT_GE(told.size(), 5u) << file;
        for (const auto &s : told) {
            EXPECT_FALSE(s.title.empty()) << file;
            EXPECT_FALSE(s.body.empty()) << file;
        }
    }
    EXPECT_EQ(gameSettings::defaultStoriesFile, std::string("data/txt/stories.json"));
}

TEST(StoryScroller, MissingOrBrokenFilesGiveNoStories)
{
    const auto dir = std::filesystem::temp_directory_path();
    EXPECT_TRUE(goe::loadStories((dir / "goe-no-such-stories.json").string()).empty());
    const auto broken = (dir / "goe-broken-stories.json").string();
    std::ofstream(broken) << "[{\"title\": \"x\", \"body\": ";
    EXPECT_TRUE(goe::loadStories(broken).empty());
    std::ofstream(broken, std::ios::trunc) << "{\"title\": \"not a list\", \"body\": \"x\"}";
    EXPECT_TRUE(goe::loadStories(broken).empty());
    std::ofstream(broken, std::ios::trunc) << "[5, {\"body\": \"kept\"}, {\"title\": \"no body\"}, {\"body\": \"\"}]";
    const auto told = goe::loadStories(broken);
    ASSERT_EQ(told.size(), 1u);
    EXPECT_EQ(told[0].body, "kept");
    EXPECT_EQ(told[0].title, "");
    std::filesystem::remove(broken);
}

TEST(StoryScroller, AStoryCrossesTheStripAndEnds)
{
    goe::storyScroller s(tenPerLetter);
    goe::rng::engine e(23);
    EXPECT_FALSE(s.chunkGenerated(e, 500)); // no stories yet
    s.setStories({{"A", "apple"}});
    ASSERT_TRUE(s.chunkGenerated(e, 500));
    ASSERT_TRUE(s.current());
    EXPECT_EQ(s.current()->body, "apple");
    EXPECT_FLOAT_EQ(s.x(), 500);
    EXPECT_FLOAT_EQ(s.width(), 80); // "A: apple"
    s.advance(1);
    EXPECT_FLOAT_EQ(s.x(), 500 - goe::storyScroller::pixelsPerSecond);
    // right edge to gone: the strip plus the line, at 115 pixels a second
    s.advance((500 + 80) / goe::storyScroller::pixelsPerSecond - 1 - 0.01f);
    EXPECT_TRUE(s.current());
    s.advance(0.02f);
    EXPECT_FALSE(s.current());
    // the next chunk tells the next story
    EXPECT_TRUE(s.chunkGenerated(e, 500));
}

TEST(StoryScroller, NewChunksWaitForTheStoryOnScreen)
{
    goe::storyScroller s(tenPerLetter);
    goe::rng::engine e(5);
    s.setStories(twoStories());
    ASSERT_TRUE(s.chunkGenerated(e, 300));
    const auto first = *s.current();
    s.advance(0.5f);
    const float x = s.x();
    for (int c = 0; c < 5; c++) // a row of five new chunks
        EXPECT_FALSE(s.chunkGenerated(e, 300));
    EXPECT_EQ(s.current()->body, first.body);
    EXPECT_FLOAT_EQ(s.x(), x);
    s.stop();
    EXPECT_FALSE(s.current());
}

TEST(StoryScroller, StoriesArePickedAtRandom)
{
    goe::storyScroller s(tenPerLetter);
    goe::rng::engine e(1);
    s.setStories(twoStories());
    std::set<std::string> seen;
    for (int c = 0; c < 55; c++) {
        ASSERT_TRUE(s.chunkGenerated(e, 100));
        seen.insert(s.current()->body);
        s.stop();
    }
    EXPECT_EQ(seen.size(), 2u);
    // a story without a title is just its body
    s.setStories({{"", "no title"}});
    s.chunkGenerated(e, 100);
    EXPECT_FLOAT_EQ(s.width(), 80);
}
