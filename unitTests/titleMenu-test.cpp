#include "gameSettings.h"
#include "titleMenu.h"
#include <allegro5/keycodes.h>
#include <gtest/gtest.h>
#include "testSupport.h"
#include "controlBindings.h"
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
    ASSERT_EQ(lines.size(), 5u);
    EXPECT_EQ(lines[0], "Save location: .");
    EXPECT_EQ(lines[1], "Music volume: 100%");
    EXPECT_EQ(lines[2], "Sound effects volume: 100%");
    EXPECT_EQ(lines[3], "Controls");
    EXPECT_EQ(lines[4], "Back");
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::MAIN);
    EXPECT_EQ(m.getSelected(), 1);
    // "Back" works too
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_UP); // wraps to the last line
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


namespace {
/// opens Config and moves to its line-th option
void openConfigAt(titleMenu &m, int line)
{
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    for (int c = 0; c < line; c++)
        m.keyDown(ALLEGRO_KEY_DOWN);
}
} // namespace

TEST(TitleMenuTests, VolumesStepByFiveAndAreKept)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 1);
    m.keyDown(ALLEGRO_KEY_RIGHT); // already at the top
    EXPECT_EQ(gameSettings::getInstance().getMusicVolume(), 100);
    for (int c = 0; c < 5; c++)
        m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(gameSettings::getInstance().getMusicVolume(), 75);
    EXPECT_EQ(m.getLines()[1], "Music volume: 75%");
    EXPECT_EQ(m.getMessage(), "Saved");

    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER); // a volume can be typed too
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::EDITING);
    EXPECT_EQ(m.getEditBuffer(), "100%");
    while (!m.getEditBuffer().empty())
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, "123");
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::EDITING); // out of range
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, "23");
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_EQ(gameSettings::getInstance().getEffectsVolume(), 23);
    for (int c = 0; c < 10; c++)
        m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(gameSettings::getInstance().getEffectsVolume(), 0);

    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_EQ(gameSettings::getInstance().getMusicVolume(), 75);
    EXPECT_EQ(gameSettings::getInstance().getEffectsVolume(), 0);
}

TEST(TitleMenuTests, ControlsCanBeRebound)
{
    using goe::controls::action;
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 3);
    m.keyDown(ALLEGRO_KEY_ENTER);
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::CONTROLS);
    EXPECT_EQ(m.getTitle(), "Controls");
    auto lines = m.getLines();
    ASSERT_EQ(lines.size(), (size_t) goe::controls::actionCount + 2);
    EXPECT_EQ(lines[0], "Walk up: Up, W, pad stick");
    EXPECT_EQ(lines[4], "Shoot (+ direction): Left Shift, Right Shift, pad button 5");
    EXPECT_EQ(lines.back(), "Back");

    // shoot gets Q; Left Shift stays as the spare key
    for (int c = 0; c < 4; c++)
        m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::BINDING);
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): press a key or pad button");
    m.keyDown(ALLEGRO_KEY_Q);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONTROLS);
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): Q, Left Shift, pad button 5");
    EXPECT_EQ(m.getMessage(), "Saved");

    // a key moves away from the action that had it
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_W);
    EXPECT_EQ(m.getLines()[0], "Walk up: Up, pad stick");
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): W, Q, pad button 5");

    // a pad button, then Esc cancels and Backspace clears
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.padButton(7);
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): W, Q, pad button 7");
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONTROLS);
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): W, Q, pad button 7");
    m.keyDown(ALLEGRO_KEY_ENTER);
    m.keyDown(ALLEGRO_KEY_BACKSPACE);
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): none");
    m.padButton(3); // only counts while waiting for a binding
    EXPECT_EQ(m.getLines()[4], "Shoot (+ direction): none");

    // the layout survives a restart
    auto kept = gameSettings::getInstance().getControls();
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_TRUE(gameSettings::getInstance().getControls() == kept);
    EXPECT_TRUE(gameSettings::getInstance().getControls().of(action::shoot).keys.empty());

    // reset, then back to Config on the Controls line
    for (int c = 4; c < goe::controls::actionCount; c++)
        m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(gameSettings::getInstance().getControls() == goe::controls::bindings());
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_EQ(m.getSelected(), 3);
}

TEST(ControlBindingsTests, DefaultLayoutPlaysAsBefore)
{
    using goe::controls::inputState;
    goe::controls::bindings b;
    auto with = [](std::initializer_list<int> keys) {
        inputState in;
        for (int k : keys)
            in.keys[k] = true;
        return in;
    };
    auto is = [&](std::initializer_list<int> keys, int type, dir::direction d) {
        auto item = b.translate(with(keys));
        return item.type == type && item.dir == d;
    };
    EXPECT_TRUE(is({}, -1, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_W}, 0, dir::direction::UP));
    EXPECT_TRUE(is({ALLEGRO_KEY_LEFT}, 0, dir::direction::LEFT));
    EXPECT_TRUE(is({ALLEGRO_KEY_UP, ALLEGRO_KEY_RIGHT}, 0, dir::direction::RIGHT));
    EXPECT_TRUE(is({ALLEGRO_KEY_RSHIFT, ALLEGRO_KEY_D}, 1, dir::direction::RIGHT));
    EXPECT_TRUE(is({ALLEGRO_KEY_LSHIFT}, -1, dir::direction::NODIRECTION)); // shooting needs a direction
    EXPECT_TRUE(is({ALLEGRO_KEY_LCTRL, ALLEGRO_KEY_S}, 2, dir::direction::DOWN));
    EXPECT_TRUE(is({ALLEGRO_KEY_X}, 3, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_ALT, ALLEGRO_KEY_A}, 4, dir::direction::LEFT));
    EXPECT_TRUE(is({ALLEGRO_KEY_Z}, 5, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_ESCAPE}, 6, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_ESCAPE, ALLEGRO_KEY_LSHIFT}, 7, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_SPACE}, 8, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_R}, 9, dir::direction::NODIRECTION));

    inputState pad;
    pad.stick[1] = true;
    pad.pad[5] = true;
    auto item = b.translate(pad);
    EXPECT_EQ(item.type, 1);
    EXPECT_TRUE(item.dir == dir::direction::DOWN);
}

TEST(ControlBindingsTests, ReboundKeysDriveTheNewAction)
{
    using goe::controls::action;
    goe::controls::bindings b;
    b.bindKey(action::use, ALLEGRO_KEY_R); // R leaves "drop"
    goe::controls::inputState in;
    in.keys[ALLEGRO_KEY_R] = true;
    EXPECT_EQ(b.translate(in).type, 8);
    EXPECT_TRUE(b.of(action::drop).keys.empty());
    b.bindKey(action::use, ALLEGRO_KEY_E);
    b.bindKey(action::use, ALLEGRO_KEY_F);
    EXPECT_EQ(b.of(action::use).keys, (std::vector<int>{ALLEGRO_KEY_F, ALLEGRO_KEY_E})); // two at most
    b.bindPadButton(action::drop, 5); // the pad button leaves "shoot"
    EXPECT_EQ(b.of(action::shoot).padButton, -1);
    EXPECT_EQ(b.of(action::drop).padButton, 5);
    EXPECT_EQ(goe::controls::bindings::fromId("nextGun"), action::nextGun);
    EXPECT_FALSE(goe::controls::bindings::fromId("fly").has_value());
}
