/*
 * The hall of fame: the best games, kept in the save folder.
 */
#include "hallOfFame.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <random>

namespace fs = std::filesystem;

namespace {
fs::path scratchFile()
{
    return fs::temp_directory_path() / ("goe-fame-test-" + std::to_string(std::random_device{}()) + ".json");
}
} // namespace

TEST(HallOfFameTests, BestScoresComeFirstAndTheListStaysShort)
{
    goe::hallOfFame fame;
    EXPECT_FALSE(fame.qualifies(0));
    EXPECT_TRUE(fame.qualifies(1));
    for (int s = 1; s <= 12; s++)
        fame.add({"p" + std::to_string(s), s * 10, "2026-10-01"});
    ASSERT_EQ(fame.entries().size(), goe::hallOfFame::places);
    EXPECT_EQ(fame.entries().front().score, 120);
    EXPECT_EQ(fame.entries().back().score, 30);
    EXPECT_FALSE(fame.qualifies(30));
    EXPECT_TRUE(fame.qualifies(31));
    // the same score goes after the ones that were there first
    EXPECT_EQ(fame.add({"late", 100, ""}), std::optional<std::size_t>(3));
    EXPECT_EQ(fame.entries()[2].name, "p10");
    EXPECT_EQ(fame.add({"too low", 5, ""}), std::nullopt);
}

TEST(HallOfFameTests, NamesAreTidied)
{
    goe::hallOfFame fame;
    fame.add({"   ", 10, ""});
    EXPECT_EQ(fame.entries()[0].name, goe::hallOfFame::nobody);
    fame.add({"  Malaclypse the Younger, K.S.C.  ", 20, ""});
    EXPECT_EQ(fame.entries()[0].name, "Malaclypse the Y");
    // cut by characters, not bytes
    fame.add({"Żółć Żółć Żółć Żółć", 30, ""});
    EXPECT_EQ(fame.entries()[0].name, "Żółć Żółć Żółć Ż");
}

TEST(HallOfFameTests, TheListIsSavedAndRead)
{
    const auto file = scratchFile();
    goe::hallOfFame fame;
    fame.add({"Eris", 523, "2026-10-01"});
    fame.add({"Hagbard \"H\"", 23, "2026-09-30"});
    ASSERT_TRUE(fame.save(file));
    const auto back = goe::hallOfFame::load(file);
    ASSERT_EQ(back.entries().size(), 2u);
    EXPECT_EQ(back.entries()[0].name, "Eris");
    EXPECT_EQ(back.entries()[0].score, 523);
    EXPECT_EQ(back.entries()[1].name, "Hagbard \"H\"");
    EXPECT_EQ(back.entries()[1].date, "2026-09-30");
    fs::remove(file);
    // missing or broken files give an empty list
    EXPECT_TRUE(goe::hallOfFame::load(file).entries().empty());
    std::ofstream(file) << "{ not json";
    EXPECT_TRUE(goe::hallOfFame::load(file).entries().empty());
    fs::remove(file);
}

TEST(HallOfFameTests, TodayIsADate)
{
    const auto d = goe::today();
    ASSERT_EQ(d.size(), 10u);
    EXPECT_EQ(d[4], '-');
    EXPECT_EQ(d[7], '-');
}
