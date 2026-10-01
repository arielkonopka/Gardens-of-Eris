#include "gameSettings.h"
#include "titleMenu.h"
#include "musicChips.h"
#include "musicGenres.h"
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

TEST(TitleMenuTests, ContinueShowsOnlyWithAReadableSave)
{
    scratch s;
    bool readable = true;
    titleMenu m(gameSettings::getInstance(), s.settingsFile(), [&readable] { return readable; });
    auto lines = m.getLines();
    ASSERT_EQ(lines.size(), 4u);
    EXPECT_EQ(lines[0], "Continue");
    // with a save, starting over reads "New game"
    EXPECT_EQ(lines[1], "New game");
    EXPECT_EQ(m.getSelected(), 0);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::CONTINUE);
    m.keyDown(ALLEGRO_KEY_DOWN);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::START);
    m.keyDown(ALLEGRO_KEY_UP);
    m.keyDown(ALLEGRO_KEY_UP);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::EXIT);

    // Config, then back, lands on Config again
    m.keyDown(ALLEGRO_KEY_UP);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::NONE);
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    EXPECT_EQ(m.getLines()[m.getSelected()], "Config");

    // the save went away: refresh drops Continue and starts from the top
    readable = false;
    m.refresh();
    lines = m.getLines();
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "Start game");
    EXPECT_EQ(m.getSelected(), 0);
    EXPECT_TRUE(m.keyDown(ALLEGRO_KEY_ENTER) == titleMenu::action::START);
}

TEST(TitleMenuTests, OldSettingsGiveEscToSaveAndExit)
{
    // settings.json from before "save and exit" had Esc for giving up
    using goe::controls::action;
    scratch s;
    {
        std::ofstream out(s.settingsFile());
        out << R"({"controls": {"giveUp": {"keys": [)" << ALLEGRO_KEY_ESCAPE << R"(], "pad": 9}}})";
    }
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    const auto b = gameSettings::getInstance().getControls();
    EXPECT_EQ(b.of(action::giveUp).keys, std::vector<int>{ALLEGRO_KEY_BACKSPACE});
    EXPECT_EQ(b.of(action::giveUp).padButton, 9);
    EXPECT_EQ(b.of(action::saveAndExit).keys, (std::vector<int>{ALLEGRO_KEY_ESCAPE, ALLEGRO_KEY_F10}));

    // a player who chose Esc for giving up on purpose keeps it once the file knows both actions
    auto mine = b;
    mine.bindKey(action::giveUp, ALLEGRO_KEY_ESCAPE);
    gameSettings::getInstance().setControls(mine);
    ASSERT_TRUE(gameSettings::getInstance().save(s.settingsFile()));
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_TRUE(gameSettings::getInstance().getControls() == mine);
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
    ASSERT_EQ(lines.size(), 15u);
    EXPECT_EQ(lines[0], "Save location: .");
    EXPECT_EQ(lines[1], "Music volume: 100%");
    EXPECT_EQ(lines[2], "Sound effects volume: 100%");
    EXPECT_EQ(lines[3], "Music: Skin samples");
    EXPECT_EQ(lines[4], "Performer sound: AdLib");
    EXPECT_EQ(lines[5], "Music style: Mixed");
    EXPECT_EQ(lines[6], "Music variety: 60%");
    EXPECT_EQ(lines[7], "Music tempo: 100%");
    EXPECT_EQ(lines[8], "Story scroller: On");
    EXPECT_EQ(lines[9], "Stories file: data/txt/stories.json");
    EXPECT_EQ(lines[10], "Demo after: 60 s");
    EXPECT_EQ(lines[11], "Demo length: 30 s");
    EXPECT_EQ(lines[12], "Hall of fame shown: 10 s");
    EXPECT_EQ(lines[13], "Controls");
    EXPECT_EQ(lines[14], "Back");
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

TEST(TitleMenuTests, StoryScrollerSwitchesAndIsKept)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 8);
    m.keyDown(ALLEGRO_KEY_ENTER); // a switch: Enter flips it, no editor
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_FALSE(gameSettings::getInstance().getStoriesShown());
    EXPECT_EQ(m.getLines()[8], "Story scroller: Off");
    EXPECT_EQ(m.getMessage(), "Saved");
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_FALSE(gameSettings::getInstance().getStoriesShown());
    m.keyDown(ALLEGRO_KEY_LEFT); // Left and Right flip it too
    EXPECT_TRUE(gameSettings::getInstance().getStoriesShown());
}

TEST(TitleMenuTests, MusicSwitchesBetweenSkinSamplesAndThePerformer)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 3);
    m.keyDown(ALLEGRO_KEY_ENTER); // a switch
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_TRUE(gameSettings::getInstance().getMusicSource() == gameSettings::musicSource::performer);
    EXPECT_EQ(m.getLines()[3], "Music: Performer");
    gameSettings::getInstance().resetToDefaults();
    EXPECT_TRUE(gameSettings::getInstance().getMusicSource() == gameSettings::musicSource::samples);
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_TRUE(gameSettings::getInstance().getMusicSource() == gameSettings::musicSource::performer);
    m.keyDown(ALLEGRO_KEY_RIGHT); // Left and Right flip it too
    EXPECT_EQ(m.getLines()[3], "Music: Skin samples");
}

TEST(TitleMenuTests, PerformerSoundVarietyAndTempoAreChosenAndKept)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 4);
    // the four chips, in both directions
    m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(m.getLines()[4], "Performer sound: SID");
    m.keyDown(ALLEGRO_KEY_ENTER); // Enter steps forward
    EXPECT_EQ(m.getLines()[4], "Performer sound: POKEY");
    m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(m.getLines()[4], "Performer sound: Game Boy");
    m.keyDown(ALLEGRO_KEY_RIGHT); // wraps around
    EXPECT_EQ(m.getLines()[4], "Performer sound: AdLib");
    m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(m.getLines()[4], "Performer sound: Game Boy");
    // variety in steps of five, never past 100
    m.keyDown(ALLEGRO_KEY_DOWN); // past the music style
    m.keyDown(ALLEGRO_KEY_DOWN);
    for (int c = 0; c < 10; c++)
        m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(m.getLines()[6], "Music variety: 100%");
    m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(m.getLines()[6], "Music variety: 95%");
    // tempo from 50% to 150%, typed or stepped
    m.keyDown(ALLEGRO_KEY_DOWN);
    for (int c = 0; c < 20; c++)
        m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(m.getLines()[7], "Music tempo: 50%");
    m.keyDown(ALLEGRO_KEY_ENTER);
    for (int c = 0; c < 4; c++)
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, "200");
    m.keyDown(ALLEGRO_KEY_ENTER); // refused: out of range, the editor stays open
    EXPECT_EQ(m.getMessage(), "Cannot use \"200\"");
    EXPECT_EQ(gameSettings::getInstance().getMusicTempo(), 50);
    for (int c = 0; c < 3; c++)
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, "120%");
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_EQ(m.getLines()[7], "Music tempo: 120%");
    // kept in the settings file
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_TRUE(gameSettings::getInstance().getPerformerSound() == goe::musician::chipStyle::gameboy);
    EXPECT_EQ(gameSettings::getInstance().getMusicVariety(), 95);
    EXPECT_EQ(gameSettings::getInstance().getMusicTempo(), 120);
}

TEST(TitleMenuTests, MusicStyleCyclesAndIsKept)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 5);
    m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(m.getLines()[5], "Music style: Free");
    m.keyDown(ALLEGRO_KEY_ENTER); // Enter steps forward
    EXPECT_EQ(m.getLines()[5], "Music style: Rave");
    m.keyDown(ALLEGRO_KEY_LEFT);
    m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(m.getLines()[5], "Music style: Mixed");
    m.keyDown(ALLEGRO_KEY_LEFT); // wraps around
    EXPECT_EQ(m.getLines()[5], "Music style: Rock");
    m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(m.getLines()[5], "Music style: Jazz");
    // the performer's sound is left alone
    EXPECT_TRUE(gameSettings::getInstance().getPerformerSound() == goe::musician::chipStyle::adlib);
    gameSettings::getInstance().resetToDefaults();
    EXPECT_TRUE(gameSettings::getInstance().getMusicStyle() == goe::musician::genre::mixed);
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_TRUE(gameSettings::getInstance().getMusicStyle() == goe::musician::genre::jazz);
    std::ifstream in(s.settingsFile());
    const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_NE(text.find("\"musicStyle\""), std::string::npos);
}

TEST(TitleMenuTests, StoriesFileStepsThroughTheLanguagesAndRefusesEmptyFiles)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 9);
    // the files next to stories.json, by name: stories.json, stories.pl.json, stories.ro.json
    m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(gameSettings::getInstance().getStoriesFile(), "data/txt/stories.pl.json");
    m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(gameSettings::getInstance().getStoriesFile(), "data/txt/stories.ro.json");
    m.keyDown(ALLEGRO_KEY_RIGHT); // wraps around
    EXPECT_EQ(gameSettings::getInstance().getStoriesFile(), "data/txt/stories.json");
    m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(gameSettings::getInstance().getStoriesFile(), "data/txt/stories.ro.json");

    // a typed file with no stories is refused, one with stories is taken
    const auto empty = (s.dir / "none.json").string();
    std::ofstream(empty) << "[{\"title\": \"no body\"}]";
    m.keyDown(ALLEGRO_KEY_ENTER);
    while (!m.getEditBuffer().empty())
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, empty);
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::EDITING);
    EXPECT_EQ(gameSettings::getInstance().getStoriesFile(), "data/txt/stories.ro.json");
    const auto mine = (s.dir / "mine.json").string();
    std::ofstream(mine) << "[{\"title\": \"Hail\", \"body\": \"Eris\"}]";
    while (!m.getEditBuffer().empty())
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, mine);
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_EQ(gameSettings::getInstance().getStoriesFile(), mine);
}

TEST(TitleMenuTests, ControlsCanBeRebound)
{
    using goe::controls::action;
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    openConfigAt(m, 13);
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
    EXPECT_EQ(m.getSelected(), 13);
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
    EXPECT_TRUE(is({ALLEGRO_KEY_BACKSPACE}, 6, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_BACKSPACE, ALLEGRO_KEY_LSHIFT}, 7, dir::direction::NODIRECTION));
    // save and exit wins over everything held with it, so leaving never costs an avatar
    EXPECT_TRUE(is({ALLEGRO_KEY_ESCAPE}, 10, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_F10}, 10, dir::direction::NODIRECTION));
    EXPECT_TRUE(is({ALLEGRO_KEY_ESCAPE, ALLEGRO_KEY_BACKSPACE, ALLEGRO_KEY_W}, 10, dir::direction::UP));
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

TEST(TitleMenuTests, TheDemoStartsAfterTheMainMenuWaited)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    // without a wait set, never
    EXPECT_TRUE(m.wait(1000.0) == titleMenu::action::NONE);
    m.setDemoAfter(60.0);
    EXPECT_TRUE(m.wait(59.0) == titleMenu::action::NONE);
    // any press starts the wait over, also one the menu does nothing with
    m.pressed();
    EXPECT_TRUE(m.wait(59.0) == titleMenu::action::NONE);
    m.keyDown(ALLEGRO_KEY_DOWN);
    EXPECT_TRUE(m.wait(59.0) == titleMenu::action::NONE);
    EXPECT_TRUE(m.wait(1.0) == titleMenu::action::DEMO);
}

TEST(TitleMenuTests, TheDemoWaitsWhileASettingIsOpen)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.setDemoAfter(60.0);
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER); // Config
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_TRUE(m.wait(1000.0) == titleMenu::action::NONE);
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::MAIN);
    // back on the main menu, the whole wait again
    EXPECT_TRUE(m.wait(59.0) == titleMenu::action::NONE);
    EXPECT_TRUE(m.wait(1.0) == titleMenu::action::DEMO);
}

TEST(TitleMenuTests, TheDemoTimesCanBeSetAndAreKept)
{
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER); // Config
    for (int i = 0; i < 11; i++)
        m.keyDown(ALLEGRO_KEY_DOWN); // Demo length
    EXPECT_EQ(m.getLines()[m.getSelected()], "Demo length: 30 s");
    m.keyDown(ALLEGRO_KEY_RIGHT);
    EXPECT_EQ(m.getLines()[11], "Demo length: 35 s");
    // typed, with or without the unit
    m.keyDown(ALLEGRO_KEY_DOWN);
    m.keyDown(ALLEGRO_KEY_ENTER);
    for (int i = 0; i < 4; i++)
        m.keyDown(ALLEGRO_KEY_BACKSPACE);
    type(m, "20 s");
    m.keyDown(ALLEGRO_KEY_ENTER);
    EXPECT_EQ(m.getLines()[12], "Hall of fame shown: 20 s");
    // never under five seconds
    m.keyDown(ALLEGRO_KEY_UP);
    m.keyDown(ALLEGRO_KEY_UP);
    for (int i = 0; i < 20; i++)
        m.keyDown(ALLEGRO_KEY_LEFT);
    EXPECT_EQ(m.getLines()[10], "Demo after: 5 s");
    m.keyDown(ALLEGRO_KEY_ESCAPE);
    // kept in the settings file
    gameSettings::getInstance().resetToDefaults();
    ASSERT_TRUE(gameSettings::getInstance().load(s.settingsFile()));
    EXPECT_EQ(gameSettings::getInstance().getDemoWait(), 5);
    EXPECT_EQ(gameSettings::getInstance().getDemoLength(), 35);
    EXPECT_EQ(gameSettings::getInstance().getHallOfFameLength(), 20);
}

TEST(TitleMenuTests, PadWalksTheMenusAndComesBack)
{
    using goe::controls::menuCommand;
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    EXPECT_TRUE(m.command(menuCommand::down) == titleMenu::action::NONE);
    EXPECT_EQ(m.getSelected(), 1); // Config
    m.command(menuCommand::confirm);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    m.command(menuCommand::back);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::MAIN);
    EXPECT_EQ(m.getSelected(), 1);
    m.command(menuCommand::up);
    EXPECT_TRUE(m.command(menuCommand::confirm) == titleMenu::action::START);
    m.command(menuCommand::up); // wraps to Exit
    EXPECT_TRUE(m.command(menuCommand::confirm) == titleMenu::action::EXIT);
}

TEST(TitleMenuTests, PadChangesValuesWithoutOpeningTheTextEditor)
{
    using goe::controls::menuCommand;
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.command(menuCommand::down);
    m.command(menuCommand::confirm); // Config
    m.command(menuCommand::down);    // Music volume
    const int before = gameSettings::getInstance().getMusicVolume();
    m.command(menuCommand::left);
    EXPECT_EQ(gameSettings::getInstance().getMusicVolume(), before - gameSettings::volumeStep);
    m.command(menuCommand::confirm); // a pad cannot type, so no editor
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
    EXPECT_FALSE(m.getMessage().empty());
    for (int c = 0; c < 7; c++)
        m.command(menuCommand::down); // Story scroller, a switch
    const bool shown = gameSettings::getInstance().getStoriesShown();
    m.command(menuCommand::confirm);
    EXPECT_NE(gameSettings::getInstance().getStoriesShown(), shown);
}

TEST(TitleMenuTests, PadBindsButtonsOnTheControlsScreen)
{
    using goe::controls::menuCommand;
    scratch s;
    titleMenu m(gameSettings::getInstance(), s.settingsFile());
    m.command(menuCommand::down);
    m.command(menuCommand::confirm); // Config
    m.command(menuCommand::up);      // Back
    m.command(menuCommand::up);      // Controls
    m.command(menuCommand::confirm);
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::CONTROLS);
    m.command(menuCommand::confirm); // Walk up
    ASSERT_TRUE(m.getScreen() == titleMenu::screen::BINDING);
    // while waiting for a button, confirm and back are only buttons to bind
    m.command(menuCommand::back);
    m.command(menuCommand::down);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::BINDING);
    EXPECT_EQ(m.getSelected(), 0);
    m.padButton(6);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONTROLS);
    EXPECT_EQ(gameSettings::getInstance().getControls().of(goe::controls::action::up).padButton, 6);
    m.command(menuCommand::back);
    EXPECT_TRUE(m.getScreen() == titleMenu::screen::CONFIG);
}
