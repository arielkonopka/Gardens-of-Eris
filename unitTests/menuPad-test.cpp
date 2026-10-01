/*
 * The menus with a game controller: pad buttons, the stick and the d-pad as menu commands,
 * and a hall of fame name entered with the pad.
 */
#include "menuPad.h"
#include "controlBindings.h"
#include <gtest/gtest.h>

using goe::controls::bindings;
using goe::controls::menuCommand;
using goe::controls::menuPad;

TEST(MenuPadTests, InteractConfirmsAndDragOrSaveAndExitGoBack)
{
    bindings b; // interact on pad button 0, drag on 1, save and exit on 7
    menuPad pad;
    EXPECT_EQ(pad.buttonDown(0, b), menuCommand::confirm);
    EXPECT_EQ(pad.buttonDown(1, b), menuCommand::back);
    EXPECT_EQ(pad.buttonDown(7, b), menuCommand::back);
    EXPECT_EQ(pad.buttonDown(3, b), menuCommand::none);
}

TEST(MenuPadTests, RebindingInControlsMovesTheMenuButtons)
{
    bindings b;
    b.bindPadButton(goe::controls::action::interact, 2);
    b.bindPadButton(goe::controls::action::drag, 3);
    menuPad pad;
    EXPECT_EQ(pad.buttonDown(2, b), menuCommand::confirm);
    EXPECT_EQ(pad.buttonDown(3, b), menuCommand::back);
    EXPECT_EQ(pad.buttonDown(0, b), menuCommand::none);
}

TEST(MenuPadTests, UnboundButtonsStillLeaveAWayToConfirmAndGoBack)
{
    bindings b;
    b.clear(goe::controls::action::interact);
    b.clear(goe::controls::action::drag);
    EXPECT_EQ(menuPad::confirmButton(b), 0);
    EXPECT_EQ(menuPad::backButton(b), 1);
}

TEST(MenuPadTests, StickMovesOncePastTheDeadzoneAndNotWhileWobbling)
{
    menuPad pad;
    EXPECT_EQ(pad.axis(0, 1, 0.3f), menuCommand::none); // inside the deadzone
    EXPECT_EQ(pad.axis(0, 1, 0.9f), menuCommand::down);
    EXPECT_EQ(pad.axis(0, 1, 0.35f), menuCommand::none); // still held: no second move
    EXPECT_EQ(pad.axis(0, 1, 0.5f), menuCommand::none);
    EXPECT_EQ(pad.axis(0, 1, 0.1f), menuCommand::none); // let go
    EXPECT_EQ(pad.axis(0, 1, -0.8f), menuCommand::up);
    EXPECT_EQ(pad.axis(0, 1, 0.0f), menuCommand::none);
    EXPECT_EQ(pad.axis(0, 0, -0.8f), menuCommand::left);
    EXPECT_EQ(pad.axis(0, 0, 0.0f), menuCommand::none);
    EXPECT_EQ(pad.axis(0, 0, 0.8f), menuCommand::right);
}

TEST(MenuPadTests, HeldDirectionRepeatsAfterADelay)
{
    menuPad pad;
    ASSERT_EQ(pad.axis(0, 1, 1.0f), menuCommand::down);
    double t = 0;
    int moves = 0;
    const double tick = 1.0 / 30;
    for (; t < menuPad::repeatDelay - tick; t += tick)
        moves += pad.wait(tick) == menuCommand::down;
    EXPECT_EQ(moves, 0);
    for (int c = 0; c < 30; c++) // one more second
        moves += pad.wait(tick) == menuCommand::down;
    EXPECT_GE(moves, 6);
    EXPECT_LE(moves, 11);
    pad.axis(0, 1, 0.0f);
    EXPECT_EQ(pad.wait(1.0), menuCommand::none);
}

TEST(MenuPadTests, DpadButtonsMoveAndRepeatUntilLetGo)
{
    EXPECT_EQ(menuPad::directionOf("UP DPAD"), menuCommand::up);
    EXPECT_EQ(menuPad::directionOf("DPad Left"), menuCommand::left);
    EXPECT_EQ(menuPad::directionOf("Hat Down"), menuCommand::down);
    EXPECT_EQ(menuPad::directionOf("A"), menuCommand::none);
    EXPECT_EQ(menuPad::directionOf("START"), menuCommand::none);

    bindings b;
    menuPad pad;
    EXPECT_EQ(pad.buttonDown(13, b, menuCommand::up), menuCommand::up);
    EXPECT_EQ(pad.wait(menuPad::repeatDelay), menuCommand::up);
    pad.buttonUp(13);
    EXPECT_EQ(pad.wait(1.0), menuCommand::none);
}

TEST(MenuPadTests, LeftStickAndDpadMoveMenusButRightStickAndTriggersDoNot)
{
    EXPECT_TRUE(menuPad::menuStick(0, false, "Left Thumbstick"));
    EXPECT_TRUE(menuPad::menuStick(3, true, "Hat 0"));
    EXPECT_TRUE(menuPad::menuStick(4, false, "DPad"));
    EXPECT_FALSE(menuPad::menuStick(1, false, "Right Thumbstick"));
    EXPECT_FALSE(menuPad::menuStick(2, false, "Left Trigger"));
}

TEST(MenuPadTests, ReleaseForgetsAHeldStick)
{
    menuPad pad;
    pad.axis(0, 0, 1.0f);
    pad.release();
    EXPECT_EQ(pad.wait(5.0), menuCommand::none);
    EXPECT_EQ(pad.axis(0, 0, 1.0f), menuCommand::right); // the next push counts again
}

TEST(MenuPadTests, NameEnteredWithThePad)
{
    std::string name;
    EXPECT_TRUE(goe::controls::editName(name, menuCommand::up, 16));
    EXPECT_EQ(name, "A");
    goe::controls::editName(name, menuCommand::up, 16);
    EXPECT_EQ(name, "B");
    goe::controls::editName(name, menuCommand::down, 16);
    goe::controls::editName(name, menuCommand::down, 16);
    EXPECT_EQ(name, "_"); // wraps around to the last letter
    goe::controls::editName(name, menuCommand::right, 16);
    EXPECT_EQ(name, "_A");
    goe::controls::editName(name, menuCommand::left, 16);
    EXPECT_EQ(name, "_");
    goe::controls::editName(name, menuCommand::left, 16);
    EXPECT_EQ(name, "");
    EXPECT_FALSE(goe::controls::editName(name, menuCommand::left, 16));
}

TEST(MenuPadTests, NameKeepsItsLengthAndMixesWithTypedLetters)
{
    std::string name = "Żaba"; // typed on a keyboard, with a letter the pad does not offer
    EXPECT_EQ(goe::controls::letterCount(name), 4u);
    EXPECT_FALSE(goe::controls::editName(name, menuCommand::right, 4));
    goe::controls::editName(name, menuCommand::left, 4);
    goe::controls::editName(name, menuCommand::left, 4);
    goe::controls::editName(name, menuCommand::left, 4);
    EXPECT_EQ(name, "Ż");
    goe::controls::editName(name, menuCommand::up, 4); // not in the list: starts at the first letter
    EXPECT_EQ(name, "A");
}
