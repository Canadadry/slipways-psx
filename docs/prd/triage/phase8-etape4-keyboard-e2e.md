---
title: "Plan 8, step 4 — keyboard e2e replays"
description: "Nothing in the e2e suite proves that the menus, colonization and slipway, research and end-of-game screens can be reached with the keyboard alone, so a screen that only the mouse can reach would be found only by the human trial."
status: needs-triage
---

## Problem Statement

Plan 8 wants a full game played on PC with the keyboard alone: menus, colonization, slipways, research, end of game. Step 2 landed the button → mouse layer (`button_cursor`), the `"keyboard_cursor": true` session declaration read by the events loader, and a headless replay driver that runs a declaring file through the layer, the cheat still reading the raw buttons. So headless keyboard replays already work today.

But no e2e replay uses them. All 26 replays in `tests/e2e/` drive the game with the mouse. If a screen cannot be reached by keyboard (a button only a click outside the cursor's range can hit, a hold the layer cannot produce, a cheat sequence broken by the clicks the layer adds), the suite stays green and the human trial of step 5 is the first to find out — with a window, which agents never open. Settled decision "the keyboard e2e come before the human trial" exists to close that gap.

## Solution

Four short e2e replays, one per screen family named in the plan's goal. Each declares the keyboard cursor, keeps the mouse at one constant position with no button in every frame, and drives the game only with the six buttons (four directions, O, X):

| Replay | Screens | Frames (prototype) | What the last frames show |
|---|---|---|---|
| `keyboard_menus` | title, pause menu, title again | 107 | a new game on "challenging", JAN 3401 |
| `keyboard_research` | map, tech screen | 90 | cheat applied, "space folding" invented, 95 research |
| `keyboard_slipway` | map, probe, space fold, build menu, slipway drag | 162 | two colonies joined by a slipway |
| `keyboard_game_end` | top bar, "final score" panel, title | 160 | back on the title screen through "again?" |

Each replay has its `.txt` note and its `.json.sha256`, set by `make e2e-baseline` like any other. `make e2e` replays them with the others. `tools/e2e_limits.sh`, already run by `make e2e`, also checks that a replay whose name starts with `keyboard_` declares the keyboard cursor and never moves the mouse or presses a mouse button.

Every replay was prototyped headlessly on a clean export of HEAD (78baab6). The frame numbers, coordinates and outcomes below come from those runs.

## User Stories

1. As a developer, I want a keyboard replay that clicks a difficulty on the title screen, so that I know a game can be started without a mouse.
2. As a developer, I want a keyboard replay that opens the pause menu and picks "restart", so that I know the pause menu works by keyboard.
3. As a developer, I want that replay to start a second game on another difficulty after "restart", so that the title screen is proven reachable twice and not only at session start.
4. As a developer, I want a keyboard replay that places the probe and confirms a space fold by holding O, so that the 15-frame hold is proven to work through the layer.
5. As a developer, I want that replay to colonize two planets through their build menus, so that menu rows next to a planet can be reached by keyboard.
6. As a developer, I want that replay to drag a slipway between the two colonies with O held, so that the drag gesture (D4) is proven end to end.
7. As a developer, I want a keyboard replay that types the cheat in game, so that I know the raw-button cheat (D6) still fires while the keyboard owns the cursor.
8. As a developer, I want that replay to open the tech screen from the top-left button and invent a technology, so that research is proven reachable by keyboard.
9. As a developer, I want a keyboard replay that skips years until the "final score" panel, so that the end of game is proven reachable by keyboard.
10. As a developer, I want that replay to press "again?" and land on the title screen, so that the loop back to a new game is covered.
11. As a developer, I want each keyboard replay to hold the mouse still and press no mouse button in any frame, so that the replay proves the keyboard alone did the work.
12. As a developer, I want that property checked by `make e2e`, so that a later edit cannot slip a mouse move into a keyboard replay unnoticed.
13. As a developer, I want a keyboard replay that forgets the `"keyboard_cursor": true` declaration to fail the same check, so that a file meant for the keyboard is never silently replayed as a mouse session.
14. As a developer, I want the check to name the file and the reason, like the other `e2e_limits.sh` failures, so that the fix is obvious.
15. As a developer, I want the check unit-tested on tiny generated files, like the frame-limit check, so that its behavior is pinned without replaying anything.
16. As a developer, I want every keyboard replay to stay under the 180-frame limit with no exception, so that `make e2e` grows by four short replays only.
17. As a developer, I want every existing `*.sha256` unchanged, so that the port's source of truth is untouched.
18. As a developer, I want each `.txt` note to list the frame of each key action and the cursor position it reaches, so that a reader can follow the replay without decoding the JSON.
19. As a developer, I want the notes to say where the cursor starts (the constant mouse position) and why the first key press is at frame 1, so that a reader understands the first frames.
20. As a developer, I want the notes to state the observable outcome (`make state` line or visible panel), so that a reviewer can check the replay does what it claims.
21. As a developer, I want the movement policy used to author the files written down, so that anyone can re-author a replay after a deliberate change of speeds (step 5).
22. As a developer, I want to know that a keyboard replay plays in a different galaxy from every mouse replay, so that I do not copy planet coordinates from a mouse replay and wonder why nothing is revealed.
23. As a developer, I want to know that the cheat typed on the title screen does not survive into the game, so that I type it on the map instead.
24. As a developer, I want to know that every O and X tap feeds the cheat matcher, so that I reset it before typing the sequence.
25. As a developer, I want `docs/plan-8.md` D6 corrected on the title-screen advice, so that the step-5 trial does not follow a hint that does not work.
26. As an agent running `make state`, I want the keyboard replays to be picked up like every other e2e, so that their zone and cap peaks are measured without extra wiring.
27. As an agent running `make sanitize-net`, I want the keyboard replays to run under the sanitizers like the others, so that the layer's path is covered by ASan and UBSan too.
28. As a human reviewer, I want each note to end with "Human review in raylib: pending.", like the other notes, so that review status stays uniform.
29. As the step-5 trial, I want the keyboard e2e to have shown which screens are reachable, so that an unplayable session points at the layer's speeds or feel, not at a missing path.
30. As a developer, I want this step independent of step 3's raylib wiring, so that it can land in either order.

## Implementation Decisions

### What step 2 landed (read at HEAD 78baab6)

- `button_cursor` (engine): the mouse owns the cursor at session start, and the cursor sits at the raw mouse position. A rising edge of any of the six buttons hands the cursor to the keyboard, unless the owner holds one of its buttons. The cursor then stays where it is and moves 1 px per frame for the first 10 frames a direction is held, then 2 px. It is clamped to 0..159 × 0..119. A direction is passed on to the game only when the cursor sits on that screen edge. O held is the left mouse button and X held the right one.
- On the first frame the layer copies the raw frame into its "previous" frame, so **no button can rise on frame 0**: the first key press that takes the cursor must be at frame 1 or later.
- `p8_input_load` reads `"keyboard_cursor": true` only before `"frames"`. The headless replay driver plays a declaring file through the layer. The cheat always reads the raw frame.
- Frames are counted by `"mouse_x"` keys (`e2e_limits.sh`), so the head line changes no count. The `.sha256` is the hash of the capture (the frames drawn), so the head line's bytes do not enter the hash.

### File shape

- Same shape as the existing e2e files: `{`, then `  "keyboard_cursor": true,`, then `  "frames": [`, one frame object per line with keys in the order `mouse_x`, `mouse_y`, `mouse_buttons`, `dpad` and a 6-boolean `dpad`, then `  ]` and `}`. That is the head step 2's fixture uses and the head step 3's writer is planned to emit (see H-3-2), so a keyboard e2e looks exactly like a keyboard recording.
- Every frame has the same `mouse_x` and `mouse_y` (the replay's **start position**, where the cursor starts) and `mouse_buttons` 0.
- Frame 0 presses nothing.
- The files are committed as data. Like every existing e2e, no generator is committed (`tools/` holds checks and guards only). The implementer may author them with a throwaway script outside the repository. The movement policy below makes such a script deterministic.

### Movement policy used to author the files

Each gesture is a list of steps:

- **wait n**: n frames with nothing pressed (or with only O held, for a hold);
- **tap b**: one frame with b pressed, then one frame released. A click is a tap of O.
- **move to (x, y)**, optionally with O held: one frame at a time, until the cursor is on (x, y). On each axis not yet aligned, the direction towards the target is pressed if the distance left is at least the speed that direction would have on this frame (1 for its first 10 held frames, 2 after). Otherwise it is released for that frame, which resets its speed to 1 on the next one. The two axes are independent (D3).

Cost of one axis: d frames for d ≤ 10, then 10 + ⌈(d − 10) / 2⌉, plus at most one released frame to land exactly. The start position is chosen per replay to save travel. All four start at (76,70), which is the "reasonable" row of the title screen, so the title click is a tap of O at frame 1 with no travel.

### Traps found while prototyping

- **K-1, a different galaxy.** The galaxy depends on how many frames the title screen ran. Every mouse replay clicks at frame 0 and a keyboard replay cannot (no rise on frame 0), so every keyboard replay plays a galaxy no mouse replay has seen. Planet coordinates must come from `make state EVENTS=<file>` ("screen" columns), never from a mouse replay's note. With the title click at frame 1, the planets near the centre are p88 (67,45), p68 (87,68), p77 (112,45), p30 (44,22) and p47 (68,91).
- **K-2, a probe or fold point too close to a planet is refused.** Held within about 20 px of a planet, the fold shows a red hatched disk around that planet and is not confirmed: no $3, nothing revealed. This is why the slipway replay places the probe at (86,32) and folds at (68,20), not at the start position.
- **K-3, the cheat typed on the title screen is lost.** On the title screen it writes the placeholder economy, which starting a game replaces: in a prototype run that typed it from frame 1, `make state` showed 200 credits and 99 research at frame 17, still on the title screen, then a game starts at $100 and 0 research. This is the cart's behavior, not a gap. It contradicts D6's hint "type it on the title screen", so chunk 2 amends D6 (see below).
- **K-4, every click feeds the cheat matcher.** The title click is an O edge, so the matcher is at 1 when the game starts, and O O X X … typed next fails on its third O. One X tap first (a right click with no movement: the map is dragged by zero, D6) brings the matcher back to 0. Rule for the notes: count the O and X edges since the last reset.

### The four replays

All start at (76,70), all start with an idle frame 0 and a tap of O at frame 1 ("reasonable"), and all stay under 180 frames with no exception. The frames listed are the prototype's. A replay re-authored with the same policy reproduces them.

**`keyboard_menus`** (107 frames): title → pause menu → restart → title → new game.
- f1: tap O, "reasonable".
- f3–44: move to (2,111). f45: tap O, the pause button (`ix[4]`). The menu opens: "how to play?" is visible from f45.
- f47–49: wait. f50–57: move to (10,104). f58: tap O, "restart": back to the title screen (`make state`: clock "-", 0 planets).
- f60–62: wait. f63–100: move to (76,82). f101: tap O, "challenging". f102–106: a new game, JAN 3401, 90 planets.

**`keyboard_research`** (90 frames): cheat in game → tech screen → invention.
- f1: tap O, "reasonable".
- f3: tap X, which resets the cheat matcher (K-4). f5–f19: O O X X O O X X, one tap every two frames, cursor still at (76,70). The cheat fires at f19: $200, 99 research.
- f21–61: move to (4,4). f62: tap O, tech screen (`ix[2]`).
- f64–65: wait. f66–83: move to (30,24). f84: tap O, first row: "space folding" invented, 95 research (`make state`: `techs space folding`, 1 tech).
- f86–89: wait.

**`keyboard_slipway`** (162 frames): probe → space fold → two colonies → slipway.
- f1: tap O, "reasonable".
- f3–26: move to (86,32). f27: tap O, which places the probe.
- f29–42: move to (68,20). f43–58: O held 16 frames at (68,20), released at f59 (an idle frame): the fold is confirmed ($100 → $97) and reveals p30 (44,22, type m) and p88 (67,45, type f). p28, off screen above, is revealed too, then vanishes.
- f60–76: move to (44,22). f77: tap O on p30, which opens its build menu. f79–91: move to (48,38). f92: tap O, first row: p30 colonized.
- f94–109: move to (67,45). f110: tap O on p88. f112–124: move to (71,61). f125: tap O, first row: p88 colonized.
- f127–139: move to (67,45). f140: O pressed on p88, held while moving to (44,22), released at f159 on p30: slipway built.
- f159–161: wait. `make state`: `slipway p88 p30 normal`, 2 colonies, $80, month 9.

**`keyboard_game_end`** (160 frames): 25 years skipped → "final score" → "again?".
- f1: tap O, "reasonable".
- f3–44: move to (150,0). f45–93: 25 taps of O on the year skip (`ix[1]`), one every two frames. After the last one, year 3426 and the "final score (reasonable)" panel with its "again?" button (visible at f99).
- f95–96: wait. f97–153: move to (80,101). f154: tap O, "again?": back to the title screen (`make state`: clock "-", 0 planets).
- f156–159: wait.

The cursor's path never pushes outward against an edge (it stops at (150,0) and moves only along x there), so no keyboard replay scrolls the map. Scrolling is not one of the plan goal's screens (see Out of Scope).

### Notes (`.txt`)

The same form as the existing notes: one title line with the frame count, then one bullet per key action with its frame and coordinates, then "Human review in raylib: pending.". Each keyboard note also says:
- the start position and that the mouse never moves;
- that frame 0 is idle because a button cannot rise on it;
- the observable outcome, as a `make state` line or a panel visible in the dump;
- for `keyboard_research`, why an X tap comes before the cheat (K-4);
- for `keyboard_slipway`, that its planets are not the mouse replays' (K-1).

### The "no mouse" guard

- `tools/e2e_limits.sh` gains one rule, for each events file whose base name starts with `keyboard_`:
  - it contains a `"keyboard_cursor": true` member;
  - all its `"mouse_x"` values are equal, all its `"mouse_y"` values are equal, and every `"mouse_buttons"` value is 0.
  A failure prints `e2e: <events>: ...` with the reason, like the script's other failures, and every file is examined before the script concludes.
- The rule keys on the name, not on the declaration. A step-5 recording or a future e2e that tests D2's mouse takeover may legitimately declare the keyboard cursor and move the mouse: it just does not take the `keyboard_` prefix. The prefix states the intent "keyboard only", and the guard holds the file to it.
- Values are read with the tolerant spacing the script already relies on (`"mouse_x":` followed by optional spaces). The guard reads four small files with `grep`, `sort -u` and `wc`: no measurable cost.
- The script's header comment and the `make e2e` line of the Makefile legend gain the rule, a few words each.

### D6 amendment

Chunk 2 changes D6's hint in `docs/plan-8.md` "Settled decisions" by one sentence: the cheat's grant typed on the title screen is lost when the game starts (K-3), so it is typed on the map, after an X tap that resets the matcher if a click came before. Its four left and four right clicks land where the cursor stands. The rest of D6 (raw buttons, sequence, reset rule, the side effects of clicks on the map) stays.

### Hypotheses on step 3 (not landed)

This step does not depend on step 3: the files are authored as data, played by the headless driver step 2 landed, and checked by a script step 3 does not touch.

- **H-3-1.** Step 3 leaves `button_cursor`'s rules and speeds, the loader's reading of the declaration, and the headless replay driver's order (layer, then cheat on the raw frame) as step 2 landed them. Its PRD lists them as untouched. If it changes any of them, the four replays' hashes change. Re-author with the policy above and re-baseline.
- **H-3-2.** Step 3's writers emit the head `{`, `  "keyboard_cursor": true,`, `  "frames": [` (its PRD's stated shape). If the spelling differs, aligning the keyboard e2e heads with it is optional and moves no hash, since the hash covers the capture only.
- **H-3-3.** The keyboard e2e are not produced by recording in the window (agents never open one), so `p8_input_save`'s and `p8_input_append`'s signatures, whatever step 3 gives them, do not matter here.

Sections to re-check once step 3 lands: "What step 2 landed", "File shape", "Movement policy used to author the files", "The four replays" (frame numbers), "Testing Decisions" (the guard's fixture shape if the frame line changes).

## Implementation Chunks

Each chunk leaves `make test-all` green, with every existing `*.sha256` unchanged.

1. **The guard.** The `keyboard_` rule in `tools/e2e_limits.sh`, its tests in `test_e2e_limits`, the script's header and the Makefile legend. Red first: a `keyboard_` file with a moved mouse passes before the rule exists.
2. **`keyboard_menus` and `keyboard_research`.** The two replays with notes and `.sha256` (`make e2e-baseline` sets only the files that have no reference yet, and the other references stay byte-identical, which is checked with `git status`). The D6 amendment goes here, since `keyboard_research` is the replay that shows it.
3. **`keyboard_slipway`.** Replay, note, `.sha256`.
4. **`keyboard_game_end`.** Replay, note, `.sha256`.

Chunks 2–4 depend on chunk 1 only for the guard to cover them as they land. They are independent of each other.

## Testing Decisions

- **What a good test is here**: the replay's capture is the test (its hash), and its note names the observable outcome. The guard is tested by what the script answers on tiny generated files (exit status, message naming the file), never by its internals.
- **`test_e2e_limits`**, extended, with the `write_e2e` helper it already has, given a mouse position per frame and a declaration flag:
  - a `keyboard_` file, declared, mouse constant, buttons 0, passes;
  - a `keyboard_` file whose `mouse_x` (then `mouse_y`) differs in one frame fails, and the log names the file;
  - a `keyboard_` file with `mouse_buttons` 1 in one frame fails, and the log names the file;
  - a `keyboard_` file without the declaration fails, and the log names the file;
  - a file not named `keyboard_` that declares and moves the mouse passes (the rule keys on the name);
  - the existing frame-limit and companion tests pass unchanged.
- **The replays**: `make e2e` compares each new capture with its `.sha256`. Before baselining, the implementer checks each outcome with `make state EVENTS=tests/e2e/keyboard_<name>.json` (and with cut-down copies of the file in `tmp/` for intermediate frames, as in the prototype), then writes it in the note.
- **Prior art**: `test_e2e_limits` for the guard. `cheat_code`, `pause_restart`, `slipway_build`, `tech_xenofoods_recipes` and `game_end_final_score` are the mouse replays these mirror, and their notes are the model.
- **Speed**: about 519 frames over four replays. A prototype replay took 86–128 ms single-threaded (`trade_rich_mine`, 152 frames: 96 ms), and `make e2e` runs replays in parallel on all cores, so the suite grows by roughly 0.1 s of wall time. The guard adds a few `grep`s. `make test` gains a handful of asserts in an existing binary.
- **Checks of the step**: `make test-all` passes. Every pre-existing `*.sha256` is byte-identical. `make state` and `make sanitize-net` pass with the four new replays included (they pick up every e2e automatically). The raylib app is never launched.

## Out of Scope

- Step 3 (raylib wiring, writers, README controls) and step 5 (human trial, speed changes, adding a recording to `worst-case/`).
- Any change to `button_cursor`, the loader, the replay driver, the cheat or the game.
- A keyboard replay for map scrolling (D5 edge push, X right-drag). It is not one of the goal's screens and is unit-tested in `button_cursor`. It is a candidate if step 5 finds scrolling hard to reach.
- Replays of every screen variant (bankruptcy, help screen, structures carried from their icon, the toggles of the pause menu). One replay per screen family is the plan's ask.
- A replay that mixes devices (D2 takeover). It would not take the `keyboard_` prefix.
- A committed generator for e2e files.
- Audio.

## Further Notes

- **Unattended run.** No human was available. The to-prd check-ins were resolved with the `guideline` skill (simplicity, surgical changes, surfaced assumptions, verifiable goals):
  - *Do the modules match expectations?* No new module. Four data files with notes, one rule in an existing guard script, and one sentence in the plan. A committed generator was rejected: no e2e has one, `tools/` is for checks, and the movement policy written here is enough to re-author a file.
  - *Which modules get tests?* The guard (unit test on tiny files). The replays are their own tests.
  - *How should "no mouse movement" be proven?* With a cheap check in the script `make e2e` already runs, keyed on the `keyboard_` prefix. A check on every declaring file was rejected because it would forbid a future mixed-device e2e. A check inside the replay driver was rejected because it would change a binary for a property of data.
  - *Does it need cutting?* Yes: one chunk for the guard and one chunk per replay (menus and research share one, with the D6 amendment).
  - *Frame budget.* Under 180 with no exception, and the prototype's largest is 162. A start position per replay was considered to shorten travel. (76,70) serves all four because the title click is the one gesture every replay shares.
- **Prototype.** Built from `git archive HEAD` in a scratch directory with `make tmp/replay` (the working tree holds step 3 in progress and was not read). The four files were produced by a throwaway script that applies the movement policy above. Each was checked with `STATE=` dumps at intermediate frames, and the dumps were rendered to images to see the pause menu, the fold's red refusal disk (K-2) and the "final score (reasonable)" panel. None of it is committed.
- The prototype's in-game cheat taps O four times at (76,70) on empty space with no probe out. The last frame was identical to a run without the cheat, and the start position is about 11 px from p68 (inside the refusal radius of K-2), but the implementer should confirm with `make state` and the dump whether a probe appeared, and say so in the note.
