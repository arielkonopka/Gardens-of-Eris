#include "gameSettings.h"
#include "titleMenu.h"
#include <allegro5/keycodes.h>
#include <gtest/gtest.h>
#include "testSupport.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {
/// a scratch folder that is removed when the test ends
struct scratch
{
    fs::path dir;
    scratch()
    {
        dir = fs::temp_directory_path() / ("goe-title-test-" + std::to_string(std::rand()));
        fs::create_directories(dir);
        gameSettings::getInstance().resetToDefaults();
    }
    ~scratch()
    {
        std::error_code ec;
        fs::remove_all(dir, ec);
        gameSettings::getInstance().resetToDefaults();
    }
    std::string settingsFile() const { return (dir / "settings.json").string(); }
};

void type(titleMenu &m, const std::string &text)
{
    for (unsigned char ch : text)
        m.typed(ch);
}
} // namespace


TEST(TitleMenuTests, MainMenuOffersStartConfigExit)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::MAIN);
    auto lines = m.getLines();
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "Start game");
    EXPECT_EQ(lines[1], "Config");
    EXPECT_EQ(lines[2], "Exit");
    EXPECT_EQ(m.getSelected(), 0);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::START);
}

TEST(TitleMenuTests, SelectionWrapsAround)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_UP);
    EXPECT_EQ(m.getSelected(), 2);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::EXIT);
    m.keyDown(ALLEGRO_KEY_DOWN);
    EXPECT_EQ(m.getSelected(), 0);
}

TEST(TitleMenuTests, ConfigShowsSaveLocationAndGoesBack)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_DOWN);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::NONE);
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    auto lines = m.getLines();
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "Save location: .");
    EXPECT_EQ(lines[1], "Back");
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::MAIN);
    EXPECT_EQ(m.getSelected(), 1);
    // "Back" works too
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::MAIN);
}

TEST(TitleMenuTests, EditingTheSaveLocationStoresIt)
{
    scratch s;
    auto target = (s.dir / "saves" / "deeper").string();
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER); // config
    m.keyDown(ALLEGRO_KEY_ENTER); // edit the save location
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::EDITING);
    EXPECT_EQ(m.getEditBuffer(), ".");
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    EXPECT_EQ(m.getEditBuffer(), "");
    type(m, target);
    EXPECT_EQ(m.getLines()[0], "Save location: " + target + "_");
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_EQ(m.getMessage(), "Saved");
    EXPECT_TRUE(fs::is_directory(target)); // missing folders are created
    EXPECT_EQ(gameSettings::getInstance().getSaveDirectory(), target);
    EXPECT_EQ(gameSettings::getInstance().getSaveFile(), (fs::path(target) / gameSettings::saveFileName).string());
    // and it survives a restart
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_EQ(gameSettings::getInstance().getSaveDirectory(), target);
}

TEST(TitleMenuTests, UnusableSaveLocationIsRejected)
{
    scratch s;
    // a regular file where a folder should be
    auto blocker = s.dir / "not-a-folder";
    std::ofstream(blocker) << "x";
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, blocker.string());
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::EDITING); // still there, to fix it
    EXPECT_TRUE(m.getMessage().rfind("Cannot use", 0) == 0);
    EXPECT_EQ(gameSettings::getInstance().getSaveDirectory(), ".");
    EXPECT_TRUE(!fs::exists(s.settingsFile()));
    // an empty path is refused as well, and Esc leaves the old value
    while (!m.getEditBuffer().empty())
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::EDITING);
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_EQ(m.getLines()[0], "Save location: .");
}

TEST(TitleMenuTests, TypingOnlyCountsInTheEditor)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    type(m, "abc");
    EXPECT_EQ(m.getEditBuffer(), "");
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.typed(0x0D); // control characters are ignored
    m.typed(0x17C); // "ż" is kept as UTF-8
    EXPECT_EQ(m.getEditBuffer(), ".\xC5\xBC");
    m.keyDown(ALLEGRO_KEY_BACKSPACE); // removes the whole character
    EXPECT_EQ(m.getEditBuffer(), ".");
}

TEST(TitleMenuTests, BrokenSettingsFileKeepsDefaults)
{
    scratch s;
    EXPECT_TRUE(!gameSettings::getInstance().load((s.dir / "missing.json").string()));
    std::ofstream(s.settingsFile()) << "{ not json";
    EXPECT_TRUE(!gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_EQ(gameSettings::getInstance().getSaveDirectory(), ".");
}

