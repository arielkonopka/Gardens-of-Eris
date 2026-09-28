#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TitleMenu
#include "gameSettings.h"
#include "titleMenu.h"
#include <allegro5/keycodes.h>
#include <boost/test/unit_test.hpp>
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

BOOST_AUTO_TEST_SUITE(TitleMenuTests)

BOOST_AUTO_TEST_CASE(MainMenuOffersStartConfigExit)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    BOOST_CHECK(m.getScreen() == titleMenu::screen::MAIN);
    auto lines = m.getLines();
    BOOST_REQUIRE_EQUAL(lines.size(), 3u);
    BOOST_CHECK_EQUAL(lines[0], "Start game");
    BOOST_CHECK_EQUAL(lines[1], "Config");
    BOOST_CHECK_EQUAL(lines[2], "Exit");
    BOOST_CHECK_EQUAL(m.getSelected(), 0);
    BOOST_CHECK(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::START);
}

BOOST_AUTO_TEST_CASE(SelectionWrapsAround)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_UP);
    BOOST_CHECK_EQUAL(m.getSelected(), 2);
    BOOST_CHECK(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::EXIT);
    m.keyDown(ALLEGRO_KEY_DOWN);
    BOOST_CHECK_EQUAL(m.getSelected(), 0);
}

BOOST_AUTO_TEST_CASE(ConfigShowsSaveLocationAndGoesBack)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_DOWN);
    BOOST_CHECK(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::NONE);
    BOOST_REQUIRE(m.getScreen() == titleMenu::screen::CONFIG);
    auto lines = m.getLines();
    BOOST_REQUIRE_EQUAL(lines.size(), 2u);
    BOOST_CHECK_EQUAL(lines[0], "Save location: .");
    BOOST_CHECK_EQUAL(lines[1], "Back");
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    BOOST_CHECK(m.getScreen() == titleMenu::screen::MAIN);
    BOOST_CHECK_EQUAL(m.getSelected(), 1);
    // "Back" works too
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    BOOST_CHECK(m.getScreen() == titleMenu::screen::MAIN);
}

BOOST_AUTO_TEST_CASE(EditingTheSaveLocationStoresIt)
{
    scratch s;
    auto target = (s.dir / "saves" / "deeper").string();
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER); // config
    m.keyDown(ALLEGRO_KEY_ENTER); // edit the save location
    BOOST_REQUIRE(m.getScreen() == titleMenu::screen::EDITING);
    BOOST_CHECK_EQUAL(m.getEditBuffer(), ".");
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    BOOST_CHECK_EQUAL(m.getEditBuffer(), "");
    type(m, target);
    BOOST_CHECK_EQUAL(m.getLines()[0], "Save location: " + target + "_");
    m.keyDown(ALLEGRO_KEY_ENTER);
    BOOST_CHECK(m.getScreen() == titleMenu::screen::CONFIG);
    BOOST_CHECK_EQUAL(m.getMessage(), "Saved");
    BOOST_CHECK(fs::is_directory(target)); // missing folders are created
    BOOST_CHECK_EQUAL(gameSettings::getInstance().getSaveDirectory(), target);
    BOOST_CHECK_EQUAL(gameSettings::getInstance().getSaveFile(),
                      (fs::path(target) / gameSettings::saveFileName).string());
    // and it survives a restart
    gameSettings::getInstance().resetToDefaults();
    BOOST_REQUIRE(gameSettings::getInstance().load(s.settingsFile()));
    BOOST_CHECK_EQUAL(gameSettings::getInstance().getSaveDirectory(), target);
}

BOOST_AUTO_TEST_CASE(UnusableSaveLocationIsRejected)
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
    BOOST_CHECK(m.getScreen() == titleMenu::screen::EDITING); // still there, to fix it
    BOOST_CHECK(m.getMessage().rfind("Cannot use", 0) == 0);
    BOOST_CHECK_EQUAL(gameSettings::getInstance().getSaveDirectory(), ".");
    BOOST_CHECK(!fs::exists(s.settingsFile()));
    // an empty path is refused as well, and Esc leaves the old value
    while (!m.getEditBuffer().empty())
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    m.keyDown(ALLEGRO_KEY_ENTER);
    BOOST_CHECK(m.getScreen() == titleMenu::screen::EDITING);
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    BOOST_CHECK(m.getScreen() == titleMenu::screen::CONFIG);
    BOOST_CHECK_EQUAL(m.getLines()[0], "Save location: .");
}

BOOST_AUTO_TEST_CASE(TypingOnlyCountsInTheEditor)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    type(m, "abc");
    BOOST_CHECK_EQUAL(m.getEditBuffer(), "");
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.typed(0x0D); // control characters are ignored
    m.typed(0x17C); // "ż" is kept as UTF-8
    BOOST_CHECK_EQUAL(m.getEditBuffer(), ".\xC5\xBC");
    m.keyDown(ALLEGRO_KEY_BACKSPACE); // removes the whole character
    BOOST_CHECK_EQUAL(m.getEditBuffer(), ".");
}

BOOST_AUTO_TEST_CASE(BrokenSettingsFileKeepsDefaults)
{
    scratch s;
    BOOST_CHECK(!gameSettings::getInstance().load((s.dir / "missing.json").string()));
    std::ofstream(s.settingsFile()) << "{ not json";
    BOOST_CHECK(!gameSettings::getInstance().load(s.settingsFile()));
    BOOST_CHECK_EQUAL(gameSettings::getInstance().getSaveDirectory(), ".");
}

BOOST_AUTO_TEST_SUITE_END()
