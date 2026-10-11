#include "musicCredits.h"
#include "configManager.h"
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

using goe::credits::musicCredit;
using goe::credits::song;

namespace {
const std::vector<musicCredit> twoArtists = {
    {"data/music/a/", "Alpha", "First album", "CC BY 4.0", "alpha.example"},
    {"data/music/b/", "Beta", "", "CC0", ""},
};
} // namespace

TEST(MusicCreditsTests, SongsFindTheirCreditByFolder)
{
    EXPECT_EQ(goe::credits::creditFor("data/music/a/1.mp3", twoArtists), &twoArtists[0]);
    // with the base folder in front, as configManager keeps them
    EXPECT_EQ(goe::credits::creditFor("./data/music/b/2.mp3", twoArtists), &twoArtists[1]);
    EXPECT_EQ(goe::credits::creditFor("data/music/c/3.mp3", twoArtists), nullptr);
    // a folder only counts as a whole path part
    EXPECT_EQ(goe::credits::creditFor("olddata/music/a/1.mp3", twoArtists), nullptr);
}

TEST(MusicCreditsTests, TitlesLoseTheArtistInFront)
{
    EXPECT_EQ(goe::credits::titleOf({"x", "Alpha - Song"}, "Alpha"), "Song");
    EXPECT_EQ(goe::credits::titleOf({"x", "Song - Alpha"}, "Alpha"), "Song - Alpha");
    EXPECT_EQ(goe::credits::titleOf({"x", "Alpha - Song"}, ""), "Alpha - Song");
}

TEST(MusicCreditsTests, LinesGroupSongsByArtistAndKeepTheUncredited)
{
    const std::vector<song> music = {
        {"data/music/b/1.mp3", "Beta - One"},
        {"data/music/a/1.mp3", "Alpha - Uno"},
        {"data/music/x/1.mp3", "Somebody - Else"},
        {"data/music/a/2.mp3", "Alpha - Dos"},
        {"data/music/a/1.mp3", "Alpha - Uno"}, // listed twice, credited once
    };
    const std::vector<std::string> expected = {
        "Music by Alpha", "First album, CC BY 4.0", "alpha.example", "Uno", "Dos", "",
        "Music by Beta", "CC0", "One", "",
        "Other music", "Somebody - Else",
    };
    EXPECT_EQ(goe::credits::lines(music, twoArtists), expected);
    // an artist with no songs in the list is left out
    EXPECT_EQ(goe::credits::lines({music[0]}, twoArtists), (std::vector<std::string>{"Music by Beta", "CC0", "One"}));
    EXPECT_TRUE(goe::credits::lines({}, twoArtists).empty());
}

TEST(MusicCreditsTests, EverySongTheGameShipsIsCreditedWithItsLicence)
{
    // CC BY needs the attribution, so every song of skins.json has a credit with a licence and a source,
    // and CREDITS.md (shipped next to the game) names it too
    const auto &cfg = configManager::getInstance()->getConfig();
    ASSERT_FALSE(cfg->music.empty());
    std::ifstream in("../CREDITS.md");
    ASSERT_TRUE(in.good());
    std::stringstream creditsMd;
    creditsMd << in.rdbuf();
    for (const auto &m : cfg->music) {
        const auto *credit = goe::credits::creditFor(m.filename, cfg->musicCredits);
        ASSERT_NE(credit, nullptr) << m.filename << " has no MusicCredits entry in skins.json";
        EXPECT_FALSE(credit->licence.empty()) << credit->artist;
        EXPECT_FALSE(credit->source.empty()) << credit->artist;
        const auto title = goe::credits::titleOf({m.filename, m.name}, credit->artist);
        EXPECT_NE(title, m.name) << m.name << " should start with \"" << credit->artist << " - \"";
        EXPECT_NE(creditsMd.str().find("| " + title + " | " + credit->artist + " |"), std::string::npos)
            << title << " by " << credit->artist << " is missing from CREDITS.md";
    }
}
