# Plan 15 — Pad navigation

## 1. Goal

The game is comfortable to play with a pad or a keyboard, with no pointer to steer pixel by pixel. The D-pad jumps between planets along the player's own slipway network and between items of a menu. A free cursor is kept only where the game needs a point in empty space: probes, the starting wormhole, structures.

The goals behind every rule below:

- **Playable now.** Steering the current virtual cursor (`src/engine/button_cursor.c`, 1–2 px per frame) across the map is too slow to play the game.
- **PSX is the target.** The controls are designed for the PSX controller (D-pad, ×○□△, L1/R1, Start), not for PICO-8's 6 buttons. The PC keyboard and gamepad map onto those PSX buttons.
- **The mouse will go.** This plan keeps the mouse alive only so the 26 existing mouse replays keep running. A later plan ports them to the pad and removes the mouse. So pad coverage must grow toward everything the mouse covers, and it never drops.

## 2. Rules

### Invariants

- `make test-all` passes on every commit. `make test` and `make e2e` never get slower.
- The 26 mouse replays of `tests/e2e/` (every file except `keyboard_*`) keep their `*.sha256` unchanged for the whole plan.
- Until step 4.1, the 4 `keyboard_*` replays (`"keyboard_cursor": true`) keep running through `button_cursor`, and their `*.sha256` stay unchanged too. Every step before 4.1 checks them alongside the mouse hashes.
- **Refreshing pad hashes is allowed, and only pad hashes.**
  - Any step after 1.4 whose own behaviour shows in an earlier `pad_*` replay deliberately changes how that replay looks. Examples: free mode at game start, network visuals, the build tab's focus, the colour-12 tab, modality.
  - This is the plan's explicit exception to the "every `*.sha256` stays unchanged" rule of `check-e2e` and `guideline`, and it covers `pad_*` files only.
  - Each such step's check states up front which earlier `pad_*` replays it expects to change.
  - The step's own implementation refreshes the affected `pad_*.json.sha256` one file at a time, before `check-e2e` runs. Never `make e2e-baseline` over the directory.
  - It reruns `make state` on each refreshed replay and confirms the end-state lines in its `.txt` still hold.
  - The step note lists which pad hashes changed and why.
  - `git diff --stat tests/e2e/` shows no mouse or `keyboard_*` `.sha256` changed.
- **A new pad replay proves what it claims.**
  - Its hash passes by construction, so a hash alone proves nothing.
  - Every step that adds a `pad_*.json` runs `make state EVENTS=<replay>` (and reads the dump at the key frames). It records the decisive end-state lines in the replay's `.txt`, as the `keyboard_*` notes do: slipways, techs, clock, credits.
  - When the replay covers a `keyboard_*` scenario, its note puts that end state next to the old replay's end state.
- **Coverage never drops.** Each `keyboard_*` scenario has a pad replay covering the same thing before that `keyboard_*` replay is deleted (step 4.1). Each pad-replay step's note records which `keyboard_*` scenario it covers, if any.
- Never launch the raylib app. Every check is headless: replays, dumps, `make state`.
- **`make raylib` must build, and it is checked.** `make test` and `make e2e` never compile `src/cmd/game.c` or `src/platform/`.
  - Step 1.2 starts with `make raylib-vendor-setup` and confirms that `make raylib` builds on the unchanged tree. If vendoring can't be done in the session, the step note says so, and the CI `raylib` job on the pushed commit becomes the check instead.
  - Every step whose diff touches `src/cmd/game.c` or `src/platform/` lists "`make raylib` builds" (or "the CI raylib job is green") in its check. Its step note records the result.
- Pad mode adds behavior and visuals that the cart doesn't have. That is accepted. No document tracks divergences from the cart: `docs/porting-gaps.md` was deleted by plan 14.

### Input modes

- **Two exclusive modes, fixed at startup.**
  - **Pad** is the default.
  - `--mouse` on the game's command line selects **mouse** mode.
  - The two modes never mix, in play or in a replay.
- **Mouse mode** behaves exactly as today without `button_cursor`, which means the cart's behavior: the mouse, plus the D-pad scrolling the camera. It has no tabs, no Start, no pad navigation, and it reads no pad buttons beyond what the cart and the cheat read today.
- **Pad mode** ignores the mouse completely, both position and buttons. The OS cursor stays hidden.
- **Replays:**
  - A replay declares `"input": "mouse"` or `"input": "pad"` in its header, and that header decides the mode.
  - A replay without the field is a mouse replay, so the 26 existing files don't change.
  - A pad replay stores PSX buttons per frame, and no mouse field at all. `tools/e2e_limits.sh` (`make e2e`) rejects a pad replay that contains a mouse field.
  - The session recorder writes the format of the mode being played.
- **PSX buttons in the input frame:** Up, Down, Left, Right, ×, ○, □, △, L1, R1, Start. Select, L2 and R2 are not used.
- **Cheat code** (`src/game/cheat.c`, sequence O O X X O O X X):
  - In pad mode, **× feeds the cheat's O and ○ feeds its X**, so the sequence becomes × × ○ ○ × × ○ ○.
  - The navigation side effects of those presses are accepted. For example, × on empty space in free mode launches a probe while the cheat is being typed.
  - Options rejected by the user:
    - a dedicated button (Select) with no other role;
    - no cheat in pad mode.
- **In pad mode the D-pad never reaches the cart's camera scroll** (`scroll.c`, cart `qb:ph`). The navigation layer owns the camera.

### Keymap (PC only)

- **`keymap.cfg`** at the repository root, versioned, read at startup by the raylib backend (`src/platform/`).
  - One line per PSX button, in `button = key[, key…]` form, with raylib key and gamepad names written without their `KEY_` / `GAMEPAD_BUTTON_` prefix.
  - If the file is missing, built-in defaults apply. They are identical to the versioned file.
  - An invalid line stops the game at startup with a message naming the file and the line, before the window opens. A key that silently does nothing is not acceptable.
  - Replays never read the keymap: they store PSX buttons, not keys.
- **Default mapping:**

  | PSX | Keyboard | Gamepad (raylib, pad 0) |
  |---|---|---|
  | D-pad | arrows | D-pad |
  | × | X | RIGHT_FACE_DOWN |
  | ○ | C | RIGHT_FACE_RIGHT |
  | □ | Z | RIGHT_FACE_LEFT |
  | △ | V | RIGHT_FACE_UP |
  | L1 / R1 | A / S | LEFT_TRIGGER_1 / RIGHT_TRIGGER_1 |
  | Start | Enter | MIDDLE_RIGHT |

- The replay speed keys (PageUp, PageDown, End) and the Space marker stay as they are, outside the keymap.
- **No analog stick:** the gamepad's D-pad only. The user rejected stick support.
- Option rejected: a compile-time C header for the mapping. Every change would need a rebuild.

### Pad navigation — structure

- **Tabs.**
  - L1/R1 cycle the tabs `map → build → end turn → tech → score → map`. R1 goes forward and L1 goes back.
  - Landing on a tab opens what it shows: the tech panel, the score panel, and so on.
  - The active tab's HUD button is drawn in the hover colour (12).
  - The HUD bar is no longer something the D-pad navigates into.
- **Start opens the pause menu.** Pause is outside the tab cycle.
- **× confirms. ○ goes back:**
  - ○ first closes the innermost sub-mode or panel.
  - On the `build`, `end turn`, `tech` or `score` tab with nothing to close, ○ returns to the `map` tab.
  - On the map in network mode, ○ does nothing.
- **End turn takes two actions:** land on the `end turn` tab, then press ×. △ is not an end-turn shortcut.
- **One mechanism under everything.**
  - A new module, `src/game/pad_nav.c`, turns each frame's pad buttons into a synthetic cursor position plus synthetic left presses and releases.
  - These go through the existing dispatch (`ui_update_hover`, `ui_handle_button`, `left_press` / `drag` / `accept_drop` / `drop`), so no game rule is duplicated in the navigation layer.
  - Right click has no pad equivalent: ○ replaces its cancel uses.
- **No auto-repeat.** A held direction or button acts once, on the press. Holding Left/Right on slipways or a direction in a panel moves the selection only once. The only exception is free mode, where a held D-pad moves the camera continuously (speed curve below). This is the user's decision. A held × is still a held left button for as long as it is down, so holds and drags keep working (for example the space fold, confirmed by holding).

### Map tab — network mode

The selection is a planet. Structures count as planets: they are planet-kind entities.

- **Left/Right** cycle through the slipways attached to the selected planet, clockwise on screen. The selected slipway is drawn in a brighter colour.
- **Up and Down** both move the selection to the other end of the selected slipway. The camera follows the selection.
- **△** opens the selected planet's menu (project / colonise panel).
- **□** enters free mode. The free cursor starts on the selected planet.
- **×** does nothing in network mode.
- A planet without slipways: Left/Right/Up/Down do nothing, and □ is the way out.
- **Visuals:**
  - The cursor sprite is hidden.
  - The selected planet gets a blinking circle around it.
  - Whatever mouse hover shows for a planet today (info, tooltip) shows for the selected planet.
- Option rejected: directional jumps to the nearest planet in a cone (TV-UI heuristic). Some planets can never win any direction from any neighbour, so they become unreachable. Navigating the network, plus free mode for everything else, reaches every planet.

### Map tab — free mode (also called probe mode)

- **The game starts in free mode,** to place the starting wormhole and then probe from it. There is no home planet.
- **The cursor is fixed at the centre of the screen and the D-pad moves the camera:**
  - 1 px/frame;
  - 2 px/frame after 10 frames held;
  - 4 px/frame after 40 frames held.

  The camera is not clamped (`scroll.c`), so the whole map is reachable. The cart's cursor sprite is shown, with its variants depending on what is under it.
  Option rejected: a cursor that moves on screen and scrolls the camera at the edges, as `button_cursor` does. It has edge cases and contradicts "the camera follows".
- **× on empty space** launches a probe, or a wormhole when a probe already exists, as a left click on the background does today. You stay in free mode afterwards.
- **× on a planet** creates a slipway from the starting planet (the one selected before □) to that planet, if the game accepts that drop. If there is no starting planet, or the drop is refused, × does nothing. After a slipway is created, you land on the target planet in network mode.
- **△ on a planet** opens its menu, and you land on that planet in network mode.
- **○** returns to the starting planet in network mode. Without a starting planet, at the start of the game, ○ does nothing.
- **□** does nothing in free mode.

### Build tab and structure placement

- Left/Right pick one of the 7 structure icons (`cart.c`, `structure_icon_new`). The focused icon shows its tooltip, as on hover.
- **×** spawns that structure (`structure_icon_left_press`) and switches to the map in placement mode:
  - placement mode is free mode carrying the structure;
  - × drops it where the cursor is, through the existing drop logic;
  - ○ cancels and the structure is not built.

### Panels and menus

This covers the title screen, the planet menu, the tech panel, the score panel, the pause menu, message boxes and help.

- **Generic, not per-panel code.**
  - The focusable items are the clickable elements of the top UI layer, the ones `ui_update_hover` hit-tests today.
  - The D-pad moves the focus to the nearest item in that direction.
  - The focused item is drawn as hovered, tooltip included. × clicks it, ○ closes the panel.
- A unit test per panel asserts that every item can be reached from the initial focus.

### Modality

- **L1/R1 during free mode or placement:** cancel the sub-mode, as ○ would (a structure being placed is not built), then switch tabs.
- **L1/R1 with a planet menu open:** close it, then switch tabs.
- **L1/R1 on a message box or the pause menu:** ignored, because these are modal. Close them with × or ○, or with Start for pause.
- **Start during a sub-mode:** opens pause on top. Closing pause returns to the sub-mode exactly as it was.

### Deleted

- `src/engine/button_cursor.c`, `button_cursor.h` and `tests/test_button_cursor.c`.
- The `"keyboard_cursor"` field and the fixtures `tests/fixtures/events_keyboard_cursor_*.json`.
- The `keyboard_*` rule in `tools/e2e_limits.sh` and in the `Makefile` legend.
- The 4 `tests/e2e/keyboard_*` replays.

### Out of scope

- Porting the 26 mouse replays to the pad, and removing the mouse: a later plan.
- The PSX controller driver. `docs/plan-10.md` is only updated to point at the pad navigation instead of `button_cursor`.
- Audio, always.

## 3. Phases and steps

### Phase 1 — Input foundation

1. **Pad frames in the input layer and the replay format.**
   - `p8_input_frame_t` (`src/p8/input.h`) carries the PSX button set.
   - `src/devtools/input_json.c` parses and saves `"input": "pad"` replays, where each frame lists PSX buttons. A file without `"input"` stays a mouse replay.
   - `tools/e2e_limits.sh` rejects a pad replay that contains a mouse field. It counts a pad replay's frames correctly, so the 180/600-frame limit applies to pad replays too (today it counts `"mouse_x"` keys).
   - Verified by:
     - unit tests in `tests/test_input.c` (round trip, malformed pad frame, missing field means mouse);
     - `tests/test_e2e_limits.c` cases for the mouse-field rejection and for a pad replay over 180 frames, not listed in the exceptions, being rejected;
     - every `*.sha256` unchanged.
2. **Mode selection and the replay driver.**
   - `src/cmd/game.c` takes `--mouse`, and pad is the default.
   - `src/cmd/replay.c` and `src/devtools/replay_source.c` pick the mode from the replay header.
   - In pad mode the mouse is ignored, and the D-pad is not passed to the cart's camera scroll.
   - The session recorder writes pad frames in pad mode.
   - In pad mode, × and ○ feed the cheat (× = O, ○ = X).
   - `src/cmd/game.c` and `src/cmd/replay.c` run one shared per-frame input step (mode, then pad navigation and cheat, then dispatch) instead of each wiring input on its own. That way every pad replay also covers the code the human plays.
   - Verified by:
     - unit tests in `tests/test_replay_source.c` and `tests/test_recording_continuity.c`;
     - a unit test in which the pad sequence × × ○ ○ × × ○ ○ fires the cheat;
     - a unit test of the shared input step: a pad frame recorded by the session recorder and played back gives the same input as the live frame;
     - a headless pad-mode run with a moving mouse, mouse buttons pressed and the D-pad held, which leaves the camera, the hovered entity and the game state unchanged;
     - every mouse `*.sha256` unchanged.
3. **`keymap.cfg` and the raylib backend.**
   - The keymap parser is plain C, with a name table and no raylib call, so it can be unit-tested.
   - The default `keymap.cfg` is versioned.
   - `raylib_backend.c` fills the PSX buttons from the keyboard and gamepad 0 through the keymap.
   - Verified by:
     - `tests/test_keymap.c` (defaults when the file is missing, every default line parses, an invalid line is reported with its number);
     - `make raylib` builds.
4. **`pad_nav` skeleton and panel navigation on the title screen.**
   - `src/game/pad_nav.c` emits a synthetic cursor and clicks through the existing dispatch.
   - Generic panel navigation (nearest clickable item of the top layer, focus drawn as hover, × clicks) works on the title screen.
   - Verified by:
     - reachability unit tests for every panel listed under "Panels and menus" in §2: title screen, planet menu, tech, score, pause menu, a message box, help. These panels already exist in the mouse game, so the tests open them directly, without tabs or pad navigation. If a panel fails, this step's note records it and it is settled before phase 2 starts;
     - the first pad replay, `tests/e2e/pad_title.json`: choose a difficulty with D-pad and ×.

### Phase 2 — Map modes

1. **Free mode.**
   - The game starts in it.
   - The cursor is fixed at the centre, and the D-pad moves the camera with the 1/2/4 px speed curve.
   - × launches a probe or wormhole, and ○ does nothing while there is no starting planet.
   - Verified by `pad_probe.json`: place the starting wormhole, launch a probe, then confirm a space fold by holding × (the `keyboard_slipway` fold).
2. **Planet menu and network mode.**
   - △ in free mode opens the planet menu, and generic panel navigation runs inside it.
   - Colonising lands the selection on that planet in network mode.
   - The network visuals are in place: hidden cursor, blinking circle, the planet's hover info.
   - Verified by `pad_colonise.json`.
3. **Slipways from free mode, and network travel.**
   - □ enters free mode from the selected planet.
   - × on a planet creates a slipway when the drop is accepted. A refused drop does nothing.
   - Left/Right cycle slipways clockwise, with the brighter slipway. Up/Down travel.
   - ○ returns to the starting planet.
   - Verified by:
     - `pad_slipway.json`: two colonies, a slipway, travel back and forth, a refused target, and one slipway whose starting planet is off screen when × is pressed;
     - unit tests for the clockwise order and for travel.

### Phase 3 — Tabs and modality

1. **Tabs, end turn, tech, score.**
   - L1/R1 cycle `map → build → end turn → tech → score`, and the active tab is drawn in colour 12.
   - ○ returns to the map. × on `end turn` ends the turn.
   - The tech and score panels use generic panel navigation.
   - Verified by:
     - `pad_end_turn.json`;
     - `pad_research.json`: the cheat typed as × × ○ ○ × × ○ ○, then a tech invented, covering `keyboard_research`;
     - `pad_score.json`.
2. **Build tab and placement.**
   - Pick a structure icon, place it with ×, and cancel a second one with ○.
   - Verified by `pad_build.json`.
3. **Pause and modality.**
   - Start opens pause. L1/R1 are ignored on pause and on message boxes.
   - L1/R1 cancel free mode, placement and the planet menu before switching tabs.
   - Start during a sub-mode returns to it intact.
   - Verified by:
     - `pad_pause_restart.json`: pause, restart, new game, covering `keyboard_menus`;
     - `pad_message_box.json`, which also opens help from the pause menu and navigates it;
     - unit tests for each modality rule.
4. **Game end on the pad.** Verified by `pad_game_end.json`: years skipped through the `end turn` tab, the final score panel, "again?", back to the title screen. It covers `keyboard_game_end`, and `tools/e2e_long_exceptions.txt` gets an entry only if the replay needs more than 180 frames.

### Phase 4 — Remove the old keyboard cursor

1. **Delete `button_cursor` and the `keyboard_*` replays.**
   - Before deleting anything, check the coverage mapping and the end-state pairs recorded in the earlier step notes and `.txt` files, and copy the mapping into this step's note:
     - `keyboard_slipway` → `pad_probe`, `pad_colonise`, `pad_slipway` (the fold is in `pad_probe`).
     - `keyboard_research` → `pad_research`.
     - `keyboard_menus` → `pad_pause_restart`.
     - `keyboard_game_end` → `pad_game_end`.
   - Then delete everything listed under "Deleted" in §2.
   - Verified by `make test-all`, and `git grep -n "button_cursor\|keyboard_cursor"` finding nothing outside `docs/plan-*.md`.
2. **Docs.**
   - Update the README's controls section to the pad controls and `keymap.cfg`.
   - Update `docs/plan-10.md` step "Controller" to point at the pad navigation (`src/game/pad_nav.c` and the PSX buttons of the input frame) instead of `src/engine/button_cursor`.
   - Verified by reading: no doc outside this plan mentions `button_cursor`.

## 4. Done when

- `make test-all` passes, and the 26 mouse `*.sha256` are unchanged from the start of the plan.
- The 11 pad replays exist, pass `make e2e`, and contain no mouse field: `pad_title`, `pad_probe`, `pad_colonise`, `pad_slipway`, `pad_end_turn`, `pad_research`, `pad_score`, `pad_build`, `pad_pause_restart`, `pad_message_box`, `pad_game_end`. If a step merges two scenarios into one replay, its note says so.
- `button_cursor` and the `keyboard_*` replays are gone, with the coverage mapping recorded in the step 4.1 note.
- `make raylib` builds with `keymap.cfg` support. A human then plays a game with the keyboard and with a gamepad, and confirms it is comfortable. That last check is the user's, not an agent's.

## 5. Open questions

None.


## 6. Step notes

### Step 1.1

- Done: `p8_input_frame_t` gained `int pad_buttons`, a bitmask with `P8_PAD_UP`…`P8_PAD_START` (11 bits, 1…1024). `p8_input_t` gained `p8_input_mode_t mode` (`P8_INPUT_MOUSE` = 0, `P8_INPUT_PAD`). `p8_input_save` / `p8_input_append` take the mode as their last parameter. A pad replay is `"input": "pad"` then frames `{ "pad": ["up", "cross"] }`, saved in a fixed button order. The loader rejects unknown or duplicate names, a missing `"pad"`, any mouse or `dpad` field, pad plus `keyboard_cursor`, and an unknown `"input"` value. Mouse output is unchanged byte for byte.
- `tools/e2e_limits.sh` counts a pad replay's `"pad":` keys, so the header's `"pad"` value is not counted as a frame. It rejects `mouse_x`, `mouse_y`, `mouse_buttons`, `dpad` and `keyboard_cursor` in a pad replay.
- Deviation: the PRD said positional frame initialisers would keep compiling. Under `-Wextra -Werror` they don't, so 53 test initialisers got a trailing `0`.
- Left for 1.2: `raylib_backend_poll_input` and the frame in `src/cmd/game.c` don't set `pad_buttons` yet. `make raylib` was not run, because raylib is not vendored in the session.
- Every `*.sha256` is unchanged.

### Step 1.2

- Done: `make raylib-vendor-setup` worked in the session (raylib 6.0), and `make raylib` built before and after the step.
- `replay_source_input_mode` (pad without a recording) and `replay_source_resolve_mode` (`--mouse`; refused with a pad recording). The session recorder takes the mode. A fresh session no longer declares `keyboard_cursor`, and the two tests that froze the old default were updated.
- New module `src/game/input_step.{c,h}` does mode, then `TODO step 1.4` (pad nav), then cheat, then dispatch install. Pad mode zeroes mouse and `dpad` and keeps `pad_buttons`. × / ○ feed the cheat's O / X. `src/cmd/replay.c` and `src/cmd/game.c` both run on it.
- `game [--mouse] [<recording>]` prints the mode at startup. In pad mode it says pad is not playable yet; that notice is a `TODO` to drop.
- The README has a "Running the game" section and "Controls (mouse mode)".
- Tests in `tests/test_input_step.c` (new, in `UNIT_TESTS`): per-mode played frame, pad cheat, live against recorded-and-replayed, and a forked quiet-against-noisy headless pad run (camera, hovered entity, full state text). That last test holds directions in non-opposing pairs, because opposite directions cancel in the camera. It draws once per run, to stay around 20 ms.
- Breaks pointer rule 12 on purpose: `input_step_t` holds a `p8_input_t` whose `frames` points at the step's own played frame, because `dispatch_context_set_input` keeps a `p8_input_t *`. It is re-pointed on every run, and pointer-guard's count is unchanged.
- The backend poll does not fill `pad_buttons` yet (step 1.3).
- Every `*.sha256` is unchanged.

### Step 1.3

- Done: `src/platform/keymap.{c,h}` and `keymap_names.h` hold a plain C parser with no raylib call and no allocation. The name lists are X-macros generated from raylib 6.0's `raylib.h`.
- `keymap.cfg` is versioned. A test holds it byte for byte equal to the built-in default text, and the defaults are that text run through the same parser.
- Errors carry their line number. Line 0 is about the file as a whole (unreadable, too large, a button missing). Space, PageUp, PageDown, End and Escape are reserved.
- `tests/test_keymap.c` has 21 tests.
- `raylib_backend_init` takes the keymap. The poll fills `pad_buttons` in pad mode only: keyboard plus gamepad 0, with no stick, and availability checked every frame. One C99 compile-time check per name against `KEY_*` / `GAMEPAD_BUTTON_*`; a scratch mismatch was shown to fail the build.
- `game.c` loads `keymap.cfg` before the window: a notice when it is missing, a refusal with `keymap.cfg:<line>` when it is invalid. The pad-mode startup notice was reworded and its `TODO` kept.
- `make raylib` builds (raylib vendored in the session). Every `*.sha256` is unchanged.

### Step 1.4

- Done: `src/game/pad_nav.{c,h}` runs in `input_step` (pad mode), between the mode and the cheat.
  - A panel is open when no game is running, a box is open, or a modal backdrop exists.
  - The items are the clickable candidates of the top layer that have a hit box. The D-pad moves the focus to the nearest item beyond the current box edge, scored by axis distance plus twice the sideways gap.
  - The synthetic cursor sits on the focused item's centre, so the game hovers it. × is the held left button. Presses act on their edge only.
  - With no panel open, nothing moves yet. The focus is an `entity_id_t`, and pointer-guard's count is unchanged.
- `tests/test_pad_nav.c` (about 40 ms) runs the title cases, then a reachability and hover test per panel: planet menu (world space, camera moved), tech, score during play (no item), score at game end (again? / back), pause, year box, help. **No panel failed.** Removing the camera offset was shown to fail the planet menu test.
- `tests/e2e/pad_title.json`: Down, Down, Right (nothing), Up, × on "reasonable", 30 frames. End state: year 3401 month 1, credits 100, planets 90, colonies 0, slipways 0, techs 0. That end state is the same for every difficulty, which only biases hidden planet types. So the choice is proven by the colour-12 row at f17 and by `test_pad_nav` checking `economy_score_difficulty()`. It covers no `keyboard_*` scenario.
- `test_input_step`'s pad test froze "pad mode zeroes the mouse". It now checks that the raw mouse never reaches the played frame.
- **Settled by `guideline`, before phase 2:**
  - **Help.** The mouse game has no help panel: the pause row "how to play?" is an empty handler in the cart and in the port. "Help" in this plan means the title screen, already covered by `pad_title`. No help panel is built. In step 3.3, `pad_message_box.json` shows that × on "how to play?" opens nothing and pause stays up, then ○ closes pause.
  - **Message box.** "Message box" means the buttonless year box ("5 years remain"). It has no hit test and is not modal. In 3.3 the replay shows pad input passing through while it is up. The modal rule of §2 Modality (L1/R1 ignored, close with × or ○) applies to the pause menu only. The replay keeps the name `pad_message_box`.
- `make raylib` builds. Every earlier `*.sha256` is unchanged.
- End of phase 1: `warn-review` raised 17 warnings on phase 1's code, and `plan-review` found no method change in them. The warnings:
  - pointer rules: `input_step` pointer rules 11 and 12 (on purpose); `replay_source` keeps an unread `path`;
  - duplication: the PSX name table in both `input_json.c` and `keymap.c`; the button count and P8_PAD_* index mapping restated; the dpad O/X indices redefined; bare `dpad[0..5]` indices; repeated header scanning and whitespace skipping in `input_json.c`;
  - layering: `platform/` depends on `devtools/`; `p8_` functions are implemented in `devtools/`; PSX names and `keyboard_cursor` live in the p8 runtime;
  - an unused `frame` parameter in `raylib_backend_present`.
