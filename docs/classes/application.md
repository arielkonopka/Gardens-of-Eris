# The application: game loop, menus, input, settings

What turns the world into a program a person plays: the title screen, the game loop, input and
settings. The diagrams are [03-application-classes](../diagrams/03-application-classes.svg),
[05-app-lifecycle-activity](../diagrams/05-app-lifecycle-activity.svg),
[06-game-tick-activity](../diagrams/06-game-tick-activity.svg) and
[11-title-menu-states](../diagrams/11-title-menu-states.svg).

## `main.cpp`

Reads `--load` and `--seed`, loads `settings.json`, installs the crash log, makes the presenter
and runs the title loop: the title screen, a game, the demo (`playDemo`, in a chunk 23 to 63
chunks out, `farChunk`) and the Game over screen with the hall of fame (`showEnd`).

## `presenter::presenter` (`presenter.h`)

The game loop and the renderer.

| Member | What it does |
|---|---|
| `presentEverything(demo, demoSeconds)` | Runs the game, 50 ticks a second, until it ends; returns a `gameEnd`. Each tick: the demo's checks or F5 / F9 / Esc, the world grows or shrinks around the player (`worldBuilder`), the music follows D, `bElem::runLiveElements`, the story scroller, then the game field, fog and HUD are drawn. In a demo the autopilot plays and nothing is saved. |
| `getLastScore()` | The score shown last; after a lost game the best of its avatars. |
| `initializeDisplay()`, `loadCofiguredData()`, `showSplash()` | Set-up. |
| `showGameField()`, `showObjectTile(...)`, `showText(...)`, `prepareStatsThing()` | Drawing the board, tiles, text and the HUD. |

`gameEnd` is `QUIT` (window closed), `LOST` (last avatar gone), `SAVED` / `SAVE_FAILED` (Esc),
`DEMO_OVER` (a press during the demo), `DEMO_DONE` (the demo's time is up).

## `titleScreen` (`titleScreen.h`)

Draws `titleMenu` and runs it with Allegro events.

| Member | What it does |
|---|---|
| `run()` | Shows the main menu until Continue, Start, Exit (or the window closes), or the menu's demo wait passes (`DEMO`). |
| `showBusy(text)` | One line while the world is built. |
| `showMessage(headline, lines)` | Until Enter, Space, Esc or the pad's confirm or back button. |
| `showHallOfFame(fame, seconds, highlight)` | For the given time or until a press; `screenEnd` says which. |
| `askName(headline, lines, name)` | A name typed, or entered with the pad (`goe::controls::editName`). |

Pad events become menu commands through its `goe::controls::menuPad`; help lines name the pad's
buttons.

## `titleMenu` (`titleMenu.h`)

What the title screen shows and how it reacts, without any drawing, so it can be tested.

- `screen`: `MAIN`, `CONFIG`, `EDITING` (typing a value), `CONTROLS`, `BINDING` (waiting for a key
  or pad button). `action`: `NONE`, `CONTINUE`, `START`, `EXIT`, `DEMO`.
- `option`: one Config line with its label, value, `apply` for typed values and `adjust` for Left
  and Right.
- Input: `keyDown(keycode)`, `typed(codepoint)`, `padButton(button)` (on `BINDING`),
  `command(menuCommand)` (the stick and d-pad move like the arrows, confirm is Enter, back is Esc).
- The demo: `setDemoAfter(seconds)`, `wait(seconds)` returns `DEMO` once the main menu waited long
  enough, `pressed()` starts the wait over.
- `refresh()` asks again whether there is a save, so Continue and New game show.
- Reading it: `getScreen()`, `getSelected()`, `getLines()`, `getTitle()`, `getEditBuffer()`,
  `getMessage()`.

## `goe::controls::menuPad` and `editName` (`menuPad.h`)

Turn pad buttons, the left stick and the d-pad into menu commands (`menuCommand`: `none`, `up`,
`down`, `left`, `right`, `confirm`, `back`) without Allegro calls.

- `confirmButton(bindings)`: the pad button of Interact (button 0 if none).
  `backButton(bindings)`: the button of Drag or Save and exit (button 1 if none).
- `buttonDown`, `buttonUp`, `axis(stick, axis, pos)`, `wait(seconds)`, `release()`: a direction
  fires when pushed and repeats while held (after `repeatDelay` 0.4 s, every `repeatEvery` 0.1 s).
  The stick counts past `stickDeadzone` (0.4) and lets go below `stickRelease` (0.25).
- `menuStick(stick, digital, name)`: the left stick and the d-pad move the menus; the right stick
  and triggers do not. `directionOf(buttonName)`: a d-pad reported as buttons.
- `editName(name, command, maxLetters)`: arcade-style name entry (Up and Down change the last
  letter from `nameLetters`, Right adds one, Left removes one). `letterCount(name)` counts UTF-8
  letters.

## `goe::controls::bindings` (`controlBindings.h`)

Which keys and pad buttons do what; the Controls screen edits it and `settings.json` keeps it.

- `action`: `up`, `down`, `left`, `right`, `shoot`, `interact`, `drag`, `nextItem`, `nextGun`,
  `use`, `drop`, `giveUp`, `saveAndExit`. `binding`: up to `maxKeys` (2) keys and one pad button.
- `bindKey`, `bindPadButton`, `clear`, `of(action)`: a key belongs to one action only.
- `translate(inputState)`: what the held keys, buttons and stick ask for, as a `controlItem`.
- `label`, `id`, `fromId`, `keyName`, `describe`: names for menus and the settings file.

## `inputManager` (`inputManager.h`)

A singleton with its own input thread reading the keyboard and the pad.

- `getCtrlItem()`, `setControlItem(item)`: the control item the player reads (the autopilot and the
  agent library set it directly).
- `takeExitRequest()`: whether Save and exit went down since the last call.
- `activity()`: counts presses (keys, pad buttons, stick pushes); the demo ends when it changes.
- `getInstance(true)`: test mode, with no keyboard, joystick or thread (unit tests and the agent).

## `gameSettings` (`gameSettings.h`)

Player-editable options in `settings.json`: save folder (`getSaveFile()` is `savegame.goe` in it),
music and effects volume, the music source (`getMusicSource`: `samples`, the skins.json songs, or
`performer`, the [adaptive musician](../adaptive-musician.md); `"music"` in the file), the
performer's sound (`getPerformerSound`, a `goe::musician::chipStyle`; `"performerSound"`), the music
style (`getMusicStyle`, a `goe::musician::genre`; `"musicStyle"`, Mixed by default), music
variety (0..100%, `"musicVariety"`) and music tempo (50..150%, `"musicTempo"`), story scroller on or off and its file, the demo times (`demoWait`,
`demoLength`, `hallOfFameLength`, 5 to 3600 s) and the control bindings. `load`, `save`,
`resetToDefaults`. A new option gets a field here, a line in `load` and `save`, and a line in
`titleMenu`'s Config screen.

## `goe::autopilot` (`autopilot.h`)

Plays the active player for the title screen's demo, as fairly as the chasers: it only reads the
cells next to the player and the lines it could shoot along. Each tick `decide(player)` picks one
control: shoot a monster or drone within `shootingRange` (5), collect an item next to it, try a
closed door (each once), or walk to the neighbour it has stood on least. `reset()` forgets where it
has been.

## `goe::hallOfFame` (`hallOfFame.h`)

The ten best games (`places`), name, score and date, in `halloffame.json` in the save folder.
`load(file)`, `save(file)` (through a temporary file), `qualifies(score)`, `add(entry)` (returns
the place). Names are cut to 16 letters; a blank one becomes "Anonymous". `today()` gives the date.

## `goe::storyScroller` (`storyScroller.h`)

The line of text scrolling over the game field, a random Discordian story each time the maze grows
by a chunk. It keeps only the numbers; the presenter draws it. `loadStories(file)` reads
`[{"title", "body"}]`; `chunkGenerated(engine, width)` starts a story unless one is scrolling;
`advance(seconds)` moves it at 115 pixels a second; `current()`, `x()`, `width()`, `stop()`.

## `goe::crashLog` (namespace, `crashLog.h`)

A crash report for players without a debugger. `install(folder)` catches bad memory access, abort,
`std::terminate` and on Windows unhandled exceptions, and writes `crash-<date>-<time>.log` into the
save folder: the reason, the details set with `setDetail` (the world seed), a stack trace and the
last printed lines (`note`, `lastLines`). `report(reason)` and `writeReport(reason)` make one.
