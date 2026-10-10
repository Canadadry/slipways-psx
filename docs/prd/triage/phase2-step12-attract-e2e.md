---
title: "The attract e2e"
description: "The attract mode is proven only by unit tests and make demo-check, so a change that breaks the demo start, the return or the swallowed press, or that touches the player's save, would not turn make e2e red."
status: needs-triage
---

## Problem Statement

Plan 18, phase 2, step 12, verbatim:

> 12. **The attract e2e.** Add `tests/e2e/attract.json` (under 180 frames, `"attract_after": 3`, an `initial_save` so the `resume` row is checked) with its `.txt`, `.sha256` and end state, covering section 2's `make e2e` line. Verified by `make e2e`, `tools/e2e_limits.sh` passing without an exception, and the `.txt` naming the frames that show the demo start, the first caption, the return and the swallowed press. `make e2e` not slower than step 1's timing beyond this file's own frames.

Section 2's `make e2e` line: one new replay, under 180 frames, with `"attract_after": 3`: the demo starts (record loaded, first caption, `demo` label), plays about 150 frames, a press returns to the title, the press is swallowed, the `resume` row is there and the disk save is unchanged.

After steps 10 and 11, the attract mode runs in the core and draws its captions and tag, but `make e2e`, the source of truth (`CLAUDE.md`), has no replay that reaches it. No existing e2e idles 1200 frames on the title, and none declares `"attract_after"`. So nothing in `make test-all` catches:

- a demo that no longer starts, starts on the wrong frame, or loads the wrong record;
- captions or a `demo` tag that disappear, move or change colour;
- a return that leaves the band on screen, keeps the demo's world, or loses the `resume` row;
- an interrupting press that leaks through to the title and moves or picks a row;
- a start or a return that writes or erases the player's save.

One more gap, found while preparing this step: `tests/test_live_slots.c` (run by `make sanitize-net`) replays every `tests/e2e/*.json` but ignores the `"attract_after"` member. On the new file it would replay a different game, without the demo, and pass silently.

## Solution

From the maintainer's side:

1. `make e2e` replays one more file, `tests/e2e/attract.json`, of **176 frames**, starting on the title with a player's save on disk and `"attract_after": 3`.
2. The title sits idle for 3 frames. On the 4th frame (f3) the demo starts: the compiled record is resumed (year 3412, 16 colonies, 18 slipways, 2 techs), the first caption "build a trading empire / across the stars" and the red `demo` tag are on screen.
3. The demo plays 150 frames (demo frames 0–149, all inside the opening caption).
4. At f153, X goes down and stays down for 10 frames. The title comes back on f153, with its `resume` row, and the held X is swallowed: the focus stays on "forgiving" and no game starts.
5. X is released at f163. Up at f164 moves the focus onto "resume", which proves the row is there and that the core listens again after the swallow. X at f166 resumes the player's own game (year 3407), untouched by the demo.
6. The capture's hash and the end state (the player's game, resumed) are committed. Any change to the start, the overlay, the return, the swallow or the save cut changes one or both.
7. `tests/test_live_slots.c` replays every e2e the way `replay` does, `"attract_after"` included, so `make sanitize-net` covers the demo frames of the new file too.

## User Stories

1. As a maintainer, I want a replay in `make e2e` that reaches the attract mode, so that the demo is guarded by the source of truth and not only by unit tests.
2. As a maintainer, I want the replay to use a short threshold from its own header, so that it reaches the demo in 3 frames instead of 1200.
3. As a maintainer, I want the threshold written exactly as the session recorder writes it (`"attract_after": N`), so that the e2e file is a normal recording format and `replay` reads it like any other.
4. As a maintainer, I want the replay under 180 frames, so that it needs no long-replay exception and `make e2e` stays fast.
5. As a maintainer, I want the replay to start on the title with a player's save on disk, so that the `resume` row is part of what is checked.
6. As a maintainer, I want that save cut from an existing game, never typed, so that it is a real record that decodes.
7. As a maintainer, I want the demo to start on a known frame (f3), so that a change to the idle count shows up as a hash change on that frame.
8. As a maintainer, I want the frame that starts the demo to show the demo record's galaxy, so that loading the wrong record changes the hash.
9. As a maintainer, I want the first caption and the `demo` tag in the capture from the first demo frame, so that losing or moving the overlay changes the hash.
10. As a maintainer, I want about 150 demo frames played, so that the overlay is captured over many frames of a live game, not just one.
11. As a maintainer, I want the demo interrupted by a button held several frames, so that the swallow covers a real press, not a one-frame tap.
12. As a maintainer, I want the interrupting button to be X, so that a press that leaked through would pick "forgiving" and start a new game, a change the end state shows plainly.
13. As a maintainer, I want the title after the return to show its `resume` row, so that a return that forgot to re-read the disk save changes the hash.
14. As a maintainer, I want an Up press after the release to move the focus onto "resume", so that the file proves both that the row exists and that the swallow ends at the release.
15. As a maintainer, I want X on "resume" to resume the player's game, so that the end state shows the player's year, credits and planets, which a demo that overwrote the save would change.
16. As a maintainer, I want the disk save checked byte for byte at the end of the demo, after the return and at the end of the file, so that "the disk save is unchanged" is measured, not assumed.
17. As a maintainer, I want the `.txt` to name the frame of the demo start, the first caption, the return and the swallowed press, so that a red hash can be traced to the part that changed.
18. As a maintainer, I want the `.txt` to give the readings behind each frame (state lines, pixel rows), so that a future re-baseline is checked against facts, not trusted.
19. As a maintainer, I want the `.txt` to record a negative run (the same file without the header member), so that the reader sees what a broken attract mode would look like.
20. As a maintainer, I want the `.sha256` produced for this file alone, so that no other reference is touched.
21. As a maintainer, I want the end state written only where it is missing, so that no existing `.end-state` can be rewritten by accident.
22. As a maintainer, I want every existing `*.sha256`, `.end-state` and `worst-case/` file unchanged, so that the step is shown to add a check and change nothing else.
23. As a maintainer, I want `make e2e` not slower than step 1's timing beyond the new file's own replay time, so that the loop stays fast (`CLAUDE.md`).
24. As a maintainer, I want `make test` untouched, so that the fast loop does not pay for this step at all.
25. As a maintainer, I want `test_live_slots` to replay each e2e with its header's threshold, as `replay` does, so that its invariants run on the frames the hash covers.
26. As a maintainer, I want `test_live_slots` to fail if no e2e reaches the demo, so that the attract replay cannot be lost or silently replayed without its demo.
27. As a maintainer, I want an idle window of `test_live_slots` never to span a demo start or a return, so that a game zone flip is not mistaken for a generation that grows.
28. As a maintainer, I want the idle windows inside the demo still checked, so that a demo frame that leaks game zone bytes is caught.
29. As a maintainer, I want the new file replayed under ASan and UBSan by `make sanitize-net`, so that the start and the return are memory-checked on real frames.
30. As a maintainer, I want `make state` to accept the new file without moving a peak, so that the zone budget report stays meaningful.
31. As the step 13 implementer, I want this file to cover the short interrupted cycle, so that step 13 only adds the full cycle with the default threshold.
32. As a reviewer, I want the frame plan, the save's origin and the baselining commands written down, so that the file can be rebuilt and judged without guessing.

## Implementation Decisions

### The file: hand-written frames, no tool

- **`tests/e2e/attract.json` is hand-written**, like most e2e files (`pad_tech_costs.json` is not generated either). Its frames are four runs of empty frames and three button runs, too simple to need a generator.
- **`pad_author` is not used and not changed.** Its input step is initialised on the default threshold (1200), and it has no command for a title-start `initial_save` (only `from-record`, which writes `"start": "resume"`). Teaching it both would be new tool surface beyond this step. The plan's reuse rule ("a tool that must write `"attract_after"` shares one header-member writer") binds tools, and no tool writes the member here.
- The implementer may produce the file with a throwaway script in the scratchpad. The script is not committed. The `.txt` lists the runs, so anyone can rebuild the file.
- **Header, in this order**, the session recorder's order (`"input"`, `"attract_after"`, then `"initial_save"`, then `"frames"`):
  - `"input": "pad"`
  - `"attract_after": 3`, spelled byte for byte as `replay_source_format_attract_after` writes it. The member must come before `"frames"`, which is where `replay_source` reads it.
  - `"initial_save": "<hex>"`
- Frame lines in the layout every e2e uses: `    { "pad": [] },`, button numbers in the default bindings (X = 5, Up = 1).
- Name: `attract.json`, as the plan writes it. Not `pad_`-prefixed. `test_live_slots`'s "at least 11 pad replays" count is unaffected.

### The player's save

- **The `initial_save` is `tests/e2e/pad_tech_costs.json`'s, copied byte for byte.** It is a record cut from a real game, never typed (year 3407 month 1, credits 100, science 6, 90 nodes; `pad_tech_costs.txt` gives the cut command). It decodes with the current code, since `pad_tech_costs` is green.
- It is a different galaxy from the demo record (`pico8/demo_record.hex`, year 3412). So the title's resume brings back something visibly different from the demo, and a save overwritten by the demo would change the end state.
- Not `demo_record.hex` itself: a demo that wrote its own record over the save would then go unnoticed.
- `replay` and `test_live_slots` both seed storage from a title-start file's `initial_save` (`replay_source_seed_storage`), so the boot title shows the `resume` row from f0.

### Frame plan (176 frames)

Measured on HEAD 83cd5b8 (step 10 done, step 11 not yet) with `tmp/replay` built from a clean export, on the exact file below. Only the overlay pixels are hypotheses (see Hypotheses).

| Frames | Raw pad | What the core does |
|---|---|---|
| f0–2 | empty ×3 | Title, idle count 1, 2, 3. State: no game (`clock year - month -`). |
| f3 | empty | **Demo start** (idle count reached 3): the compiled record is resumed, and demo frame 0 is played. **First caption** and `demo` tag drawn (H-11-2). State after f3: year 3412 month 1, 16 colonies, 18 slipways, 2 techs. |
| f4–152 | empty ×149 | Demo frames 1–149, all empty in `pico8/demo.json`, all under caption 0 (demo frames 0–299). State after f152: the same counts. |
| f153 | X (5) | **Return**: any button down while playing. `cart_return_to_title` runs at the top of the frame, and the core plays an empty frame. State after f153: no game. |
| f153–162 | X held ×10 | **Swallowed press**: phase "swallowing", the core sees empty frames. The title rows slide in. After f162: "resume" row (y42–50) colour 13, "forgiving" (y53–61) colour 12, read at x=108. The focus has not moved and no game started. |
| f163 | empty | Release: the swallow ends, idle count 1. |
| f164 | Up (1) | Focus onto "resume": after f164 the "resume" rows read colour 12, "forgiving" 13. The idle count resets. |
| f165 | empty | Release, idle count 1 (the demo would restart only after 3 idle title frames). |
| f166 | X (5) | X on "resume": the player's game is resumed during this frame's update. The title is gone, so the attract mode stays idle from here. |
| f167–175 | empty ×9 | Idle in the resumed game. |

- **The disk save is unchanged at every cut measured**: the stored record (`make replay … SAVE_HEX=`) equals the `initial_save` byte for byte after f2, f3, f152, f153, f162, f164, f165, f166 and f175.
- **End state** (after f175): year 3407 month 1, difficulty reasonable, credits 100, science 6, income 0, 90 planets, all unknown, 0 colonies, slipways, structures and techs, 90 nodes, 0 cheat activations: the player's record.
- **Negative run** (measured, never committed): the same file without the `"attract_after"` member (threshold 1200) never starts the demo. X at f153 picks "forgiving" and the end state reads year 3401, difficulty forgiving. This is what a press that leaked through, or a lost header member, looks like.
- **Margins**: 176 ≤ 180. If step 11, or a measurement at implementation time, needs a frame moved, shorten the X hold (not below 8 frames, so the title rows have settled before Up) or the demo segment (not below 140 demo frames). Never exceed 180 frames and never add an exception line.
- **Why X for the interruption**: a leak is then a new game, not just a moved cursor. The hash, the end state and the save all change at once.
- **Why the Up, then X on "resume"**: the plan asks for "a few title frames showing the resume row". The Up press turns that row's colour, so the row is checked by pixels and by behaviour (from "forgiving", Up moves only if the row exists, as `pad_restart_resume.txt` notes). X then puts the player's save into the end state, where `make end-state` checks it. With threshold 3, an idle title restarts the demo after 3 idle frames, so the title frames after the release must hold presses. Up and X are those presses.

### The `.txt`

`tests/e2e/attract.txt`, in the style of `pad_restart_resume.txt` and `pad_tech_costs.txt`:

- **First paragraph**: what the file does, ending with exactly one "176 frames." (`tools/e2e_limits.sh` reads the first paragraph only, and refuses zero or two matches). No other "<N> frames." in that paragraph.
- **The four frames the plan names**:
  - demo start: f3;
  - first caption: f3, shown through f152;
  - return: f153;
  - swallowed press: f153–162, release at f163.
- The readings behind each frame:
  - the state lines after f2, f3, f152, f153 and f175 (from `make state` on the file cut after that frame);
  - the title rows' colours at x=108 after f162 and f164;
  - the band and tag pixels at f3, quoted from step 11's note;
  - the `SAVE_HEX` comparison.
- The save's origin (copied from `pad_tech_costs.json`, cut as its `.txt` says) and the runs of frames, so the file can be rebuilt.
- The negative run above, as `pad_restart_resume.txt` records its own.
- What the file does **not** prove, with where it is proven:
  - The save cut while actions play: demo frames 0–149 hold no action, and without the cut the first save write lands at demo frame 540 (step 10's note). Proven by step 10's unit tests and step 13.
  - Frame substitution beyond the start: demo frames 0–149 are empty. Proven by `make demo-check`.
  - The full cycle with the default threshold: step 13.
- "No long-replay exception: 176 frames, within 180." in a later paragraph, and the replay's cost (measured ≈100 ms with the `-O0` binary, against ≈320–480 ms for `pad_tech_costs`).

### Baselining a new e2e (only this file's references)

- **Check the content before trusting the hash**:
  - every reading of the frame plan holds;
  - step 11's overlay is in the f3 dump (band rows and tag box as step 11's note gives them);
  - the f153 dump shows neither.
- **`.sha256`**: build this file's capture alone, `make tmp/frames/tests/e2e/attract.json.sha256`, and copy that hash to `tests/e2e/attract.json.sha256`. **Never `make e2e-baseline`**: it resets every reference from the current code.
- **Determinism**: a second capture, after deleting `tmp/frames/tests/e2e/attract.json.*`, gives the same hash.
- **`.end-state`**: `make end-state-baseline`, which writes only missing references and never overwrites one, or `tools/end_state.sh normalize` on this file's `make state` output. Then `make end-state` (all files) passes.
- **Check**: `git status tests/e2e` shows only the four new files (`attract.json`, `.txt`, `.json.sha256`, `.json.end-state`). `git diff` shows no change to any existing reference.

### `test_live_slots` follows the header

`tests/test_live_slots.c` is the only test that replays every `tests/e2e/*.json` (in `make sanitize-net`, outside `make test`). Measured with the new file:

- **As it is**: it ignores `"attract_after"`. It replays the file without the demo (X at f153 starts a forgiving game) and passes. That is a silent divergence from `replay`, against the test's own rule "an e2e plays … as the replay tool plays it".
- **With the threshold honoured**: the invariant pass is clean over the demo. The idle-window case then fails: `attract f1-152: game 7616 -> 93880`, "the generation grows during an idle window". The window runs from the title through the demo start, and the start flips the game zone.

Decisions:

- **`load_e2e` reads the header's threshold** with `replay_source_attract_after` while the source is open, and hands it to the input step after `input_step_init` with `input_step_set_attract_after`, as `replay` does.
- **The invariant pass counts the e2e that reach the demo** (the attract phase is "playing" after some frame), prints the count beside the e2e and pad counts, and asserts at least one ("tests/e2e/ lost its attract replay"). It is red before `load_e2e` honours the header and green after.
- **An idle window never spans a frame whose attract phase differs from the previous frame's** (the start, the return, the end of the swallow). Windows inside the demo stay checked. Measured on a scratch build: with this rule every window passes, the demo's f5–152 window included (174 windows over 20 e2e, against 173 when demo frames are left out). The rule only needs the phase after each frame, through `input_step_attract_phase`. How the window's end is detected one frame ahead (a first pass that records phases, or closing the window when the next frame breaks it) is the implementer's choice. It must keep every existing window checked exactly as today.
- No other test reads `tests/e2e/` as a whole (checked with `git grep` at HEAD), and `tests/test_e2e_limits.c` runs on fixtures.

### Unchanged

- No file under `src/` changes: the attract mode, the overlay, `replay`, `pad_author`, the recorder and the Makefile stay as steps 10 and 11 leave them. `E2E_EVENTS` is a wildcard, so `make e2e`, `make state`, `make end-state`, `make sanitize-net` and `make coverage` pick the file up with no edit.
- `tools/e2e_long_exceptions.txt` is not touched.

### Decisions taken without a human (guideline, `.claude/skills/guideline/SKILL.md`)

- **Hand-written frames, not `pad_author`**: smaller change; the plan's reuse rule binds only a tool that writes the member.
- **`pad_tech_costs.json`'s `initial_save`, copied byte for byte**: a cut record already in the suite, a galaxy different from the demo's, no new cut needed ("cut, never typed").
- **Interrupt with X held 10 frames**: a leaked press then shows as a new game. 10 frames lets the title rows settle before the Up.
- **Up, then X on "resume", inside the file**: proves the row and the end of the swallow by behaviour, and puts the untouched save into the end state. It still fits in 176 frames.
- **`test_live_slots` honours the header and skips phase changes in idle windows**: required for `make sanitize-net` to replay the file as `replay` does, and a strengthening, not a skip.
- **No `make demo-check`, `full-game`, `pad-author` or `raylib` run required**: the step changes no core, demo, `pad_*` or gameplay file (plan section 2's triggers). `make end-state`, `make state` and `make sanitize-net` are run, because the new file joins them.

## Implementation Chunks

1. **The e2e.** Write `attract.json` and check every reading of the frame plan, including step 11's overlay at f3 and its absence from f153. Write `attract.txt`, then produce `attract.json.sha256` and `attract.json.end-state` for this file alone. Done when `make test-all` passes (20 e2e), `make end-state` and `make state` pass, and no existing reference changed.
2. **`test_live_slots` follows the header.** Red: the new "reached the demo" assertion fails (0 e2e). Green: `load_e2e` honours `"attract_after"`. Red again: the idle-window failure above. Green: the phase-change rule. Done when `make sanitize-net` passes, with `test_live_slots` printing one e2e that reaches the demo.

## Testing Decisions

- A good test here is the replay itself. The capture's hash pins every pixel, and the end state pins the game the file ends in. The `.txt` readings are the facts checked before the hash is trusted. Nothing reads the attract module's fields: the readings come from `make state`, the framebuffer dump and the stored record, and `test_live_slots` reads the phase only through `input_step_attract_phase`.
- **Modules tested**:
  - the attract mode, the overlay, the return and the save cut, end to end through `replay` (`make e2e`, `make end-state`, `make sanitize-net`);
  - `test_live_slots`'s replay harness (chunk 2).
- **Prior art**:
  - `tests/e2e/pad_restart_resume.{json,txt}`: title rows read at x=108, the `resume` row, a negative run, resume checks through cut files;
  - `tests/e2e/pad_tech_costs.{json,txt}`: a title-start `initial_save` and its cut command;
  - `tests/test_cart.c`'s step 10 attract cases: threshold 3 over a player's save, the same sequence in-process.
- **Guardrail runs** (plan section 2):
  - `make test-all`;
  - `make end-state` and `make state` (measured on the scratch build: `make state` passes with the new file, which holds no peak in `tmp/state.txt`);
  - `make sanitize-net` (measured: `replay_sanitize` clean on the file);
  - the `git status` / `git diff` check on `tests/e2e/` and `worst-case/`.
- **Timing**:
  - `make test`: no change, as no file it builds or runs changes. One check run against 8.2 s.
  - `make e2e`: after `rm -rf tmp/frames`, three runs, step 1's method. Compared with 1.39 s, plus at most the new file's own replay time (≈0.1 s measured; it is far from the longest replay, so the wall clock should barely move).
  - The step note records both.
- **The step note records**: the frame plan as committed, the four named frames, the save's origin, the readings, the negative run, the commands that produced the `.sha256` and `.end-state`, the `test_live_slots` counts (windows, e2e reaching the demo), the timings and any deviation.

## Out of Scope

- The full cycle with the default threshold, the whole demo, a second start and its hash (step 13).
- The README and the Makefile legend (step 14). The legend's `make e2e` text already covers any `tests/e2e/*.json`.
- Teaching `pad_author` to write `"attract_after"` or a title-start `initial_save`.
- A caption change or a command beat inside the e2e: the first command is at demo frame 450 and caption 1 starts at demo frame 300, beyond 180 frames.
- Any change to the attract mode, the overlay, the save cut, the drivers or `tools/e2e_limits.sh`.
- Re-baselining any existing reference.

## Further Notes

- Readings in this PRD were taken on a clean export of HEAD 83cd5b8 in the scratchpad (not the working tree), with `tmp/replay`, `make state`, `make sanitize-run-tests/e2e/attract.json` and a patched copy of `test_live_slots`. The file that was measured is exactly the frame plan above.
- With `"attract_after": N`, frame N of a title-start file is demo frame 0: the idle count is 1…N after frames 0…N−1, and the start fires when it reaches N.
- After the return the title rebuilds through `cart_init`, so its rows slide in again. After f162 they are settled, at the y ranges `pad_restart_resume.txt` gives (resume 42–50, forgiving 53–61, reasonable 64–72, challenging 75–83, tough 86–94).

### Hypotheses

Step 11 (`docs/prd/triage/phase2-step11-captions-and-label.md`) is being implemented while this PRD is written. This PRD reads the code at HEAD 83cd5b8. Below is what it assumes about step 11's outcome. If step 11's note says otherwise, the note wins, and only the named part of this PRD follows it.

- **H-11-1 (done first):** step 11 is committed before step 12 starts, so the capture this step baselines includes the overlay. If `cart_draw` draws no overlay at f3, stop: never baseline a demo without its captions.
- **H-11-2 (which frames):** the overlay is drawn exactly on the frames that play a demo frame: f3–152 here (demo frames 0–149). It is not drawn on f0–2, on the return frame f153, on the swallowed frames f153–162, or after. The published demo frame is reset by `cart_return_to_title`.
- **H-11-3 (pixels):** the band covers rows 96–108, full width, colour 1. Caption 0 is drawn in colour 7: "build a trading empire" at (36, 97) and "across the stars" at (48, 103). The tag is a colour-8 box at x 143–159, y 11–17, with `demo` in colour 7 at (144, 12). The `.txt` quotes step 11's note for these, not this list.
- **H-11-4 (one caption):** caption 0 covers demo frames 0–299, so every demo frame of this file shows it, and the file has no caption change.
- **H-11-5 (no behaviour change):** step 11 changes only what is drawn. The attract timings (start on the frame the idle count reaches the threshold, return on any button, swallow until release), the game state, the stored record and `replay`'s header reading are as measured on HEAD 83cd5b8. So every state line, save comparison and row colour of the frame plan still holds after step 11.
- **H-11-6 (the title unchanged):** the title drawn after the return has no band or tag pixel left. Its rows read as measured (the row colours at x=108 are outside the band and the tag).
- **H-11-7 (no overlap):** step 11 adds no file to `tests/e2e/`, does not touch `tests/test_live_slots.c`, and changes no existing reference, so chunk 2's measurements (173/174 windows, the f1–152 failure) carry over.
