---
title: "The demo script"
description: "The attract mode has no demo to play: no captioned script, no committed demo.json to compile in, and no check that the demo still shows what it claims after a gameplay change."
status: needs-triage
---

## Problem Statement

Plan 18, phase 1, step 7: **The demo script.**

> 7. **The demo script.** Write `pico8/demo.pad` from the record with the beats and captions of section 2, and generate the committed `pico8/demo.json`, with a `pico8/demo.txt` describing each beat's frames, as e2e `.txt` files do, and the command that cut the record. If the demo cannot meet the beats, the pacing or the 3600-frame limit, stop and ask the user rather than relax a number or a condition. Verified with step 5's tool at the end of `demo.json`: one more colony, one more slipway, one more year, one more invented tech than the record, no cheat frame; total frames at most 3600; captions in section 2's order and text. The step also creates `make demo-check`, outside `make test-all`, with two checks so the plan-pilot rule can run from step 8 on: `pad_author` regenerates `pico8/demo.json` byte for byte, and playing `demo.json` with step 5's tool reaches those counts.

The attract mode (steps 8 to 13) plays a recorded pad stream with captions, from the record frozen in step 2. Everything it needs exists except the demo itself:

- `pico8/demo_record.hex` is the starting record (year 3412, 16 colonies, 18 slipways, 2 techs).
- `pad_author` starts a script from a record (`from-record`, step 5) and, once step 6 lands, attaches captions with frame ranges it computes (`caption`).
- `tools/pad_author_proofs/demo_beats.record.pad` (step 5) proves the five beats play from the record, but it is a proof, at the minimum pacing sizes (1085 frames, 18 s), and it does not produce the file the attract mode compiles in.

What is missing:

- **No demo.** There is no `pico8/demo.pad`, so no `pico8/demo.json` for step 8's generator to compile, and nothing a human could later watch and accept.
- **No pleasant length.** Section 2 asks for about 2700 frames (45 s), at most 3600. The beats at the minimum pacing take 1085: the demo would flash past.
- **No guard.** Nothing says, after a gameplay change, whether the committed demo still regenerates the same, still shows one more colony, slipway, year and tech than the record, and does it honestly (no cheat). Section 2's goal "the demo never lies" and "a gameplay change that breaks it is caught by a check" have no check behind them yet.
- **No explanation.** A reviewer has no `.txt` that says which frames show which beat and where the record came from.

## Solution

1. **`pico8/demo.pad`**, a `pad_author` script that starts with `from-record demo_record.hex` (resolved relative to the script, as step 5's proof does), and plays section 2's seven beats with its seven captions, in order, with fixed idle sizes chosen for a pleasant length:

   | Part | Caption (`caption …` line) | Frames |
   |---|---|---|
   | Opening card | `build a trading empire/across the stars` | idle 300 |
   | Probe | `probe the dark/to find new planets` | idle 150, `fold 118 307`, idle 270 |
   | Colonize | `colonize planets:/each one needs and makes goods` | idle 150, `colonise p32`, idle 270 |
   | Slipway | `link colonies with slipways/so goods can flow` | idle 150, `slipway p32 p55`, idle 270 |
   | End of year | `end the year:/trade earns money and science` | idle 150, `year`, idle 270 |
   | Tech | `spend science on new tech` | idle 150, `research xenofoods`, idle 270 |
   | Closing card | `slipways/press any button` | idle 180 |

   With step 5's measured command lengths (fold 65, colonise 20, slipway 10, year 45, research 16: 156 frames), the demo is **300 + 5 × 420 + 156 + 180 = 2736 frames**, 45.6 s, 864 frames under the 3600 cap.

2. **`pad_author` turns it into `pico8/demo.json`**, committed: the record as `initial_save`, `"start": "resume"`, the seven captions with the frame ranges `pad_author` computed, and the frames. `make pad-author` regenerates it next to the `worst-case/` games, so "`make pad-author` with no diff in `worst-case/` or `pico8/demo.json`" (plan guardrail, section 4) is one command.

3. **`pico8/demo.txt`** explains the file, as the e2e `.txt` files do: what the demo shows, the record and the exact commands that cut it, the command that regenerates the JSON, then each beat's caption range, press frames and readings, and the end state.

4. **`make demo-check`**, outside `make test-all`, with the plan's two checks:
   - **Regeneration:** `pad_author` regenerates `pico8/demo.json` into `tmp/`, byte for byte equal to the committed file.
   - **Content:** `replay`, step 5's tool, plays `pico8/demo.json` from the resume twice (`REPLAY_STARTS=2`), and a small script compares each end state with the record's state: exactly one more year, colony, slipway and invented tech, zero cheat activations. The same script also checks what the step's "Verified" line lists and `pad_author` does not: at most 3600 frames, the seven captions in section 2's order and text, the closing card exactly 180 frames to the last frame.

## User Stories

1. As a player left on the title, I want the demo to show the game's core loop (probe, colonize, link, end a year, buy a tech), so that I understand what the game is about without reading a manual.
2. As a player, I want each caption on screen before the action it explains and long enough to read, so that the demo teaches instead of flashing text.
3. As a player, I want the demo to last about 45 seconds, so that it is long enough to follow and short enough to come back to the title soon.
4. As a player, I want to watch each action's outcome (the planets revealed, the new colony, the goods flowing along the slipway, the year's earnings, the tech) for a few seconds before the next caption, so that I see what the action did.
5. As a player, I want the demo to end on a closing card that says the game's name and invites me to press a button, so that I know how to start.
6. As a player, I want the demo never to cheat, so that what it shows is what I can do.
7. As the step-8 implementer, I want a committed `pico8/demo.json` with the record, the frames and the captions, so that my generator has its input.
8. As the step-10 implementer, I want the counts the demo reaches written down (one more of each than the record), so that my test of the whole compiled demo through the attract module has a target.
9. As the step-12 and step-13 implementers, I want `pico8/demo.txt` to name the first caption's frames and each beat's frames, so that my e2e and full-cycle notes can point at them.
10. As plan-pilot, I want `make demo-check` to exist from this step, so that the "run it in every step that touches the demo, `src/game/pad_*`, `src/devtools/pad_*` or gameplay" rule can run from step 8 on.
11. As a maintainer, I want `make demo-check` to fail when `pad_author` no longer regenerates the committed demo byte for byte, so that a gameplay change that moves the demo is seen at once.
12. As a maintainer, I want `make demo-check` to fail when the demo no longer gains exactly one colony, one slipway, one year and one tech over the record, so that a gameplay change that breaks a beat is seen even if I regenerated the JSON.
13. As a maintainer, I want `make demo-check` to fail when a cheat activates during the demo, so that "the demo never lies" stays true.
14. As a maintainer, I want `make demo-check` to fail when the demo grows past 3600 frames, so that a gameplay change that lengthens a command cannot silently break the length rule.
15. As a maintainer, I want `make demo-check` to fail when a caption is missing, misspelt, out of order, or the closing card is not 180 frames to the end, so that a script edit cannot drift from section 2's table.
16. As a maintainer, I want the content check to compare the demo's end with the record's own state, not with numbers typed into a script, so that the check stays right if the record is ever re-cut.
17. As a maintainer, I want the content check to play the demo twice in one process (`REPLAY_STARTS=2`), so that the second attract start is checked as well as the first.
18. As a maintainer, I want `make demo-check` outside `make test-all`, so that `make test` and `make e2e` never get slower.
19. As a maintainer, I want the content check's script tested on hand-written state files, so that a broken comparison cannot pass a broken demo.
20. As a maintainer, I want that test to run in `make demo-check` and not in `make test`, so that `make test` gains nothing.
21. As a maintainer, I want `make pad-author` to regenerate `pico8/demo.json` along with `worst-case/`, so that one command shows any diff in either.
22. As a maintainer, I want `demo.pad`'s regeneration exempt from the aim budget, so that a rule written for the realism of the worst-case games does not refuse the demo, whose idles have no business being counted against it.
23. As a maintainer, I want one label per beat in `demo.pad`, so that `make pad-author SCRIPT=pico8/demo.pad STOP_AT=<beat>` inspects the state at any beat and the content check reads the record's state at the first one.
24. As a reviewer, I want `pico8/demo.txt` to give the commands that cut the record, so that I can reproduce `pico8/demo_record.hex` byte for byte.
25. As a reviewer, I want `pico8/demo.txt` to say why each idle has its size, so that I can judge the pacing without watching the demo.
26. As a reviewer, I want the captions near the top of `pico8/demo.json`, so that I read the demo's words without scrolling past thousands of frames.
27. As a human accepting the demo at the plan's end, I want `pico8/demo.txt` to tell me what I should see and when, so that I can tell a wrong demo from a right one while watching `game`.
28. As the step-7 implementer, I want the idle sizes fixed in this PRD, so that I do not tune the demo by taste.
29. As the step-7 implementer, I want a clear rule for when I may change an idle size (only when `pad_author` refuses or a beat fails) and when I must stop and ask, so that I neither relax a rule nor stall on a fixable script.
30. As a future author regenerating the demo after a gameplay change, I want the captions placed by `caption` lines, so that their ranges follow the beats without retyping frame numbers.

## Implementation Decisions

### The script, `pico8/demo.pad`

- **Start.** `from-record demo_record.hex` (resolved relative to the script, as `include` paths are; the step 5 proof writes `../../pico8/demo_record.hex` from `tools/pad_author_proofs/`). Frame 0 is the first frame after the resume (step 5's contract).
- **Commands.** The beat commands are those step 2 tried from the record and step 5's proof plays: `fold 118 307`, `colonise p32`, `slipway p32 p55`, `year`, `research xenofoods`. The probe beat is a `fold` (step 2's decision: a probe is already live in the record and `pad_author` refuses a second). No `tap` or `hold`: every beat has a high-level command (section 2: low-level commands only where none fits).
- **Captions.** Section 2's seven captions, written with `/` as the line break (H-6-1). Each `caption` line sits immediately before the idle that opens its part, so the caption appears 150 frames before the beat's first press. The opening caption is the script's first `caption`, before the first frame. No `caption -`: each caption is replaced by the next, and the closing caption runs to the last frame (H-6-2). So the captions tile the whole demo with no gap.
- **Labels.** One clock-free label per part, before its `caption` line: `probe`, `colonize`, `slipway`, `year`, `tech`, `closing`. The `probe` label marks the end of the opening card: the state there is the record's state (only empty frames have run), and the content check reads it from there.
- **Comments.** A header of at most a few lines saying what the file is and pointing to `pico8/demo.txt`; the explanation lives in the `.txt` (CLAUDE.md § Comments).

### Pacing and length (decision)

Section 2's floor: each caption 90 frames before the next press and at least 150 frames on screen; the closing card 180 frames; at most 3600 frames; aim for about 2700. Step 5's proof meets the floor at 1085 frames. The demo reaches its length with **longer idles at both sides of each action**, never with extra actions:

- **Opening card: idle 300 (5 s).** The record's galaxy, 16 colonies and 18 slipways with goods already flowing, under the two-line pitch. It is the longest reading moment of the demo and the first impression.
- **Lead: idle 150 (2.5 s) before each beat's command.** Section 2 asks 90; 150 leaves time to read a two-line caption before the cursor moves. It is fixed, so no gameplay change can bring a press under 90.
- **Watch: idle 270 (4.5 s) after each beat's command, under the same caption.** The outcome animates there: the fold's reveal (planets turning known a few frames after it), the colony ring, transit dots along the new slipway, the year box and the year's earnings, the tech panel closed and the map again. One size for every beat keeps the script regular; the beats differ by their command's own frames only.
- **Closing card: idle 180 (3 s)**, exactly section 2's size, after the tech beat's watch.
- **Total: 2736 frames** with step 5's command lengths (156 frames of commands). A gameplay change would have to add 864 frames to the commands to reach the cap.

The implementer changes an idle size only when `pad_author` refuses the script or a beat fails with these sizes (longer idles shift the frames on which the game stream is drawn, which could change, say, a fold's reveal rolls). Then: the smallest change that passes, every section 2 rule kept, the total within 2400 to 3600, and the step note says which size moved and why. If no sizes within those bounds make the five beats pass, or the demo cannot meet the pacing or the 3600-frame limit, **stop and ask the user** (the step's stop rule): never relax a number, change a beat's command to one section 2 does not list, or re-cut the record.

### `pico8/demo.json`

- Generated by `pad_author pico8/demo.pad pico8/demo.json --no-aim-budget`, committed.
- Its header carries `"start": "resume"` and `"initial_save"` (step 5) and `"captions"` (H-6-3), then `"frames"`, one frame per line.
- **The aim budget does not apply.** The 25% aim-mode budget is a realism rule of the worst-case games (section 4 of the pad metrics); every proof, the record proof included, runs without it, and it does not change the output bytes. The demo starts from a record, which starts in aim mode, and its long idles would count against a budget meant for a played game.
- **`make pad-author` regenerates it.** The target's loop over `worst-case/*.pad` gains `pico8/demo.pad` → `pico8/demo.json`, with `--no-aim-budget` for that script only (the worst-case games keep their budget). Its legend line says so. `make pad-author SCRIPT=pico8/demo.pad [STOP_AT=<label>]` works as for any script.
- `pad_author`'s own self-check (it replays its output with `replay` and requires the same state) already runs on every regeneration.

### `pico8/demo.txt`

Written like `tests/e2e/pad_tech_costs.txt`, in English, readings taken from `replay` (`make state EVENTS=…` of the file cut at a label with `make pad-author SCRIPT=pico8/demo.pad STOP_AT=<label>`, and the framebuffer dump), never typed from memory:

1. **Summary.** The seven parts, the total frame count, that it plays from a record, on the default bindings, without cheat.
2. **The record.** Cut, never typed, from `worst-case/full_game.pad` at its `demo-record` label (year 3411's end browsing): `make pad-author SCRIPT=worst-case/full_game.pad STOP_AT=demo-record OUT=tmp/demo_record.json` then `make replay EVENTS=tmp/demo_record.json SAVE_HEX=pico8/demo_record.hex`, with the file's size and sha256 (step 2's note), and its `make state` reading: year 3412 month 1, credits 86, science 12, techs space folding and geneseeds, 16 colonies, 18 slipways. Regenerating `full_game` never moves the demo: the hex is frozen.
3. **How it is made and checked.** `make pad-author` (or `make pad-author SCRIPT=pico8/demo.pad OUT=pico8/demo.json`) regenerates the JSON; `make demo-check` checks it.
4. **Each part,** with the caption's frame range as written in the `"captions"` member, the frames of the command's presses, and what the frames show (planets turning known and their ids, the colony and its cost, the slipway and its cost, the year box and the new year's credits and science, the tech and its cost).
5. **The end state** at the last frame: year, credits, science, techs, colonies, slipways, cheat activations 0, "one more of each than the record".
6. **Why the idles have their sizes** (the pacing decision above, in two or three sentences).

The `.txt` is prose for people; no check reads it. When a regeneration moves frames, the `.txt` is updated in the same commit, as for an e2e.

### `make demo-check`

A new target, outside `make test-all`, listed in the Makefile legend and `.PHONY`. Files under `tmp/demo-check/`. It builds `tmp/pad_author`, `tmp/replay` and `tmp/test_demo_check` and runs:

1. **The checker's own test**, `test_demo_check` (see Testing Decisions), first: a broken checker must not pass a broken demo.
2. **Regeneration.** `pad_author pico8/demo.pad tmp/demo-check/demo.json --no-aim-budget`, then `cmp` with `pico8/demo.json`. A difference prints `demo-check: pico8/demo.json is not what pico8/demo.pad generates (make pad-author)`.
3. **Content.**
   - The record's state: `STATE=tmp/demo-check/record.txt pad_author pico8/demo.pad tmp/demo-check/record.json --stop-at probe --no-aim-budget`. This is the state after the opening card's empty frames, confirmed by `pad_author`'s own replay self-check: the record's counts, read by the same writer as the end state.
   - The end states: `REPLAY_STARTS=2 STATE=tmp/demo-check/end.txt` `replay pico8/demo.json` (capture to `/dev/null`, dump into `tmp/demo-check/`), giving `end.txt.start1` (first playback) and `end.txt` (second, after a return and 30 idle title frames). This is step 5's tool, on the committed file.
   - `sh tools/demo_check.sh pico8/demo.json tmp/demo-check/record.txt tmp/demo-check/end.txt.start1 tmp/demo-check/end.txt`.
4. Both checks always run and print their verdict; the target fails if either fails (as `make end-state` examines every replay before failing). Success prints one line per check.

It does not rewrite any committed file. Its cost is two `pad_author` runs (one of ~2736 frames, one of 300, each with its self-check replay) and one two-start replay: a few seconds.

### The content checker, `tools/demo_check.sh`

A POSIX shell script with one `awk` process, in the style of `tools/end_state.sh`. Usage: `demo_check.sh <demo.json> <record state> <end state>...` (one or more end states). It prints `demo-check: <file>: <reason>` for every failure, then exits 1 if any, 0 otherwise. It checks:

- **Frames.** The number of frame lines in `<demo.json>`'s `"frames"` array is between 1 and 3600.
- **Captions** (from the `"captions"` member, one caption per line, H-6-3):
  - exactly seven, with these texts in this order (lines joined by `\n` as stored): section 2's table, pinned in the script;
  - the first starts at 0; each starts where the previous ends (the captions tile the demo, no `caption -`);
  - the last ends at the frame count and lasts exactly 180 frames (the closing card).
  - The 150-frame and 90-frame rules are `pad_author`'s (H-6-4) and are not repeated here.
- **Counts,** for each end state against the record state, read from the state lines the replay's `STATE` writes:
  - `clock year`: end = record + 1;
  - `count colonies`, `count slipways`, `count techs`: end = record + 1 each;
  - `cheat activations`: 0 in the end state (and in the record state, which must have none either).
  - A missing line, a missing file or a `stopped` line is a failure, never a pass.
- "Exactly one more", not "at least one more": the demo shows one of each, so a second colony or a year passing on its own is a changed demo, which the check reports.

Pinning the caption texts in the checker duplicates section 2's table on purpose: the table is the spec, the script is the thing checked against it.

### Guardrails for this step

- No change to `src/`. The step adds `pico8/demo.pad`, `pico8/demo.json`, `pico8/demo.txt`, `tools/demo_check.sh`, `tests/test_demo_check.c`, and Makefile lines (`make pad-author`'s loop and legend, `demo-check` and its legend, the test binary's rule).
- Runs: `make test-all`; `make pad-author` with no diff in `worst-case/` and a regenerated `pico8/demo.json` identical to the committed one; `make pad-author-proof` (step 6's captioned record proof included); `make demo-check`. `make end-state` and `make full-game` are not required (no core change).
- Every `tests/e2e/*.sha256`, `.end-state` and `worst-case/` file unchanged. `make test` and `make e2e` gain nothing.
- **The step note records:** the total frame count; the `"captions"` member as generated (seven `from`/`to` pairs); each beat's command frames; the record's and the end's year, colonies, slipways, techs and cheat activations (both starts); any idle size changed and why; the `make demo-check` run time; and one by-hand failing run of `make demo-check` (a tmp copy of a state file with one count edited, fed to `tools/demo_check.sh`), not committed.

## Implementation Chunks

1. **The demo.** `pico8/demo.pad` with its labels, captions and the decided idles; `pico8/demo.json` generated and committed; `make pad-author`'s loop and legend gain it (with `--no-aim-budget`). Done when `make pad-author` regenerates `worst-case/` and `pico8/demo.json` with no diff, and `make state EVENTS=pico8/demo.json REPLAY_STARTS=2` shows the four counts one above the record's and zero cheat activations at both ends. This chunk proves the stop rule is not hit; if it is, stop here and ask.
2. **The check.** `tools/demo_check.sh`, `tests/test_demo_check.c` and its Makefile rule, the `demo-check` target and its legend line. Done when `make demo-check` passes on chunk 1's files.
3. **The explanation.** `pico8/demo.txt`, its readings taken from cut files and dumps of chunk 1's demo.

## Testing Decisions

- A good test drives the checker through its public interface, the command line, on hand-written files, and reads only its exit status and output, never its internals.
- **`tests/test_demo_check.c`** (new), modelled on `tests/test_end_state.c`: it writes a small `demo.json` (a handful of frames, seven caption lines in H-6-3's layout) and state files to `tmp/test_demo_check_work/`, runs `sh tools/demo_check.sh`, and checks:
  1. a passing set (one end state, then two end states) exits 0;
  2. each failure exits 1 and names its reason: 3601 frames; a caption missing; a caption misspelt; two captions swapped; a gap between captions; the closing caption 179 frames, or not ending at the last frame; colonies, slipways, techs or year +0, and +2; a cheat activation in an end state; the second end state wrong while the first is right; a missing count line; a `stopped` state; a missing file.
  - The fixtures are small (a few frames), except the over-the-cap case, which writes 3601 frame lines; the caption ranges of each fixture match its own frame count.
- **Where it runs: `make demo-check` only**, not `make test`. Precedent: `test_live_slots` is not in `UNIT_TESTS` and runs only from `make sanitize-net`. The checker exists for `demo-check`, and CLAUDE.md forbids any slowdown of `make test`, however small a shell-forking test would be.
- **The real check** is `make demo-check` passing on the committed demo, run by plan-pilot from step 8 on. Its failing direction for the regeneration is proven in step 13 (one frame of `demo.json` altered); this step's note records one by-hand failing content run.
- **`pad_author`'s self-check** (its replay of its own output) and `make pad-author-proof` cover the caption rules and the record start; nothing of them is retested here.
- **Prior art:** `tests/test_end_state.c` (a shell checker tested on hand-written state files), `tools/end_state.sh` (one `awk` process, `check` reporting every failure), the `*.record.pad` rule of `make pad-author-proof` (pad_author's state, then `REPLAY_STARTS=2`).
- **Timing:** `make test` and `make e2e` unchanged (nothing added to them); measured once by step 1's method for the note.

## Out of Scope

- Compiling the demo (step 8), the default table beside the compiled one (step 9), the attract state machine (step 10), drawing captions and the `demo` label (step 11), the attract e2e (step 12), the full-cycle replay with the `-O0` sanitize binary and its hash (step 13), docs (step 14).
- Any change to `pad_author`, `replay`, the caption rules or `src/`. If the demo needs one, that is a stop-and-ask, not part of this step.
- Re-cutting the record or changing a beat's command beyond section 2's list.
- Removing or changing `tools/pad_author_proofs/demo_beats.record.pad`: it stays as step 5's (and step 6's) proof.
- A check that `pico8/demo.txt` matches the JSON: it is prose, kept current by hand like the e2e `.txt` files.
- Fixing the `v` glyph, audio, the PSX build.

## Further Notes

- Section 2's captions all pass step 6's text rules: the longest line is `each one needs and makes goods` (30 characters), no `v`, `:` has a glyph.
- The labels are clock-free: they add no frame, so the `probe` label's stop state is the same whatever comes after it.
- The step 5 proof measured the beats at 1085 frames; the demo uses the same commands from the same record, so the same command lengths are expected. If the longer idles change a command's length (a different reveal, say), the total moves by those frames only.
- The demo's year beat assumes no year passes on its own during the first three actions: step 5's proof reached exactly year 3413 with the same actions, and idles do not advance the clock. The content check's "exactly + 1" catches the day this stops holding.

### Hypotheses

Step 6 is being implemented while this PRD is written. What this PRD assumes about its outcome, from its PRD (`docs/prd/triage/phase1-step6-caption-command.md`). If step 6's note says otherwise, the note wins and only the named part of this PRD follows it.

- **H-6-1 (syntax):** `caption <text>` takes the rest of the line raw (no 8-word limit); `/` breaks the line and spaces around it are trimmed; `caption -` clears and fails when nothing is shown. A caption may be given before the first frame. If the syntax differs, `demo.pad`'s `caption` lines follow it; nothing else changes.
- **H-6-2 (ranges):** `from` and `to` are 0-based indexes into the output's `"frames"`, `from` inclusive, `to` exclusive; a `caption` run when `N` frames have been emitted closes the shown caption at `N` and opens the new one at `N`; the caption still shown at the end of a whole run closes at the frame count. If the convention differs (an inclusive `to`, say), the checker's tiling and "ends at the last frame, lasts 180" rules follow the new convention.
- **H-6-3 (member):** `"captions"` is a header member before `"frames"`, after `"start"` and `"initial_save"`, one caption per line as `{ "from": F, "to": T, "text": "…" }`, `\n` the only escape, absent when there is no caption. The checker reads it line by line. If the layout differs, the checker parses the layout step 6's note fixes; if the member is placed after `"frames"`, the checker reads it there.
- **H-6-4 (pacing checks):** `pad_author` refuses a caption shown fewer than 150 frames and any non-zero frame in the first 90 frames of a shown caption; a caption cut by `--stop-at` is closed unchecked; the closing card's 180 frames and the 3600-frame cap are not checked by `pad_author`. So this step's checker checks the 180 and the 3600 and not the 150 and 90. If step 6 also checks the 180 or the 3600, the checker keeps its own check anyway (cheap, and it checks the committed file, not the script).
- **H-6-5 (stop with a caption shown):** `pad_author --stop-at probe` succeeds with the opening caption open (cut at 300 frames, unchecked, written or not into the prefix's member), and its `STATE` file is written. The content check reads only the state, so how the cut caption is written does not matter. If a stop with a shown caption is refused, the record's state is read at a label placed after an `idle 1` that comes before the opening `caption` line (the opening card becomes `idle 1`, label, `caption`, `idle 299`: still 300 frames, the opening caption from frame 1, and the checker's "first caption starts at 0" rule becomes "at 1"). The state there is still the record's.
- **H-6-6 (reader):** `replay` loads a file with `"captions"` exactly as without it, including with `"start": "resume"` and `REPLAY_STARTS=2`, and `pad_author`'s self-check replays a captioned output. If not, that is a step-6 defect to fix there, not here.
- **H-6-7 (captioned proof):** `tools/pad_author_proofs/demo_beats.record.pad` carries section 2's seven captions after step 6 and passes `make pad-author-proof`. This step relies on it only as evidence that the captioned beats play from the record; `demo.pad` does not include it.
- **H-6-8 (`--try`):** `--try` lines accept `caption` with the same checks, so the implementer may try a changed idle size with its caption before editing the script.

### Decisions taken without a human (guideline rules)

No human answers questions in this run. Each decision below was taken under `.claude/skills/guideline/SKILL.md` ("make the plan work", "keep it safe", "the smaller change"):

- **Length by idles, not by more actions:** opening 300, lead 150, watch 270 per beat, closing 180, 2736 frames. Section 2 fixes the beats; more actions would be more beats. The watch is where the game animates the outcome, which is what a viewer should see.
- **One idle size per role, not tuned per beat.** Fewer numbers to justify and keep; a beat's own command frames are the only difference.
- **Idle sizes change only when a rule refuses or a beat fails,** smallest change, total kept in 2400–3600, recorded in the note; otherwise the stop rule.
- **The record's counts come from `pad_author --stop-at probe`'s state,** not from numbers typed into the checker: mechanical, read by the same state writer as the end, and right if the record is ever re-cut. Its prefix holds only empty frames after the resume.
- **"One more" means exactly one more,** for year, colonies, slipways and techs; cheat activations must be 0.
- **The content check plays both starts (`REPLAY_STARTS=2`),** as the record proof does: cheap, and the attract mode starts the demo many times in one process.
- **The checker also checks the 3600 cap, the captions' order and text, the tiling and the closing 180.** The step's "Verified" line lists them, `pad_author` does not check them (H-6-4), and they cost nothing inside the same `awk` pass. They belong to the plan's second check ("reaches those counts"), extended to what this step verifies.
- **`demo.json` is generated with `--no-aim-budget`** in `make pad-author` and `make demo-check`: the budget is a worst-case realism rule; every proof skips it; it does not change the bytes.
- **`make pad-author` regenerates `pico8/demo.json`,** so the plan's guardrail "no diff in `worst-case/` or `pico8/demo.json`" and section 4's "Done when" line are one command.
- **The checker's test runs in `make demo-check`, not `make test`** (precedent `test_live_slots`).
- **Both `demo-check` checks always run,** the target failing if either does, so a regenerated-but-broken demo and a stale-but-correct one are told apart.
- **One label per part** in `demo.pad`, for the record's state and for the `.txt`'s readings.
- **No `caption -` in the demo:** the captions tile it, and the checker requires that, so a later edit cannot leave a stretch without text unnoticed.
