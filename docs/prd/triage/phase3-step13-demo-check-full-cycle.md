---
title: "make demo-check, full cycle"
description: "make demo-check replays the attract cycle only with a 3-frame threshold and a tmp fixture with no save and no hash, so the real 1200-frame cycle, the returns and the player's save over two complete demos are never checked frame for frame."
status: needs-triage
---

## Problem Statement

Plan 18, phase 3, step 13, verbatim:

> 13. **`make demo-check`, full cycle.** Extend step 7's target with a `worst-case/`-style replay of the full cycle (default threshold: 1200 idle frames, the whole demo, the return, a second start), the target outside `make test-all` that replays it with the `-O0` sanitize binary to its end, checks its hash, and checks the content before trusting the hash: at the end of each demo playback the state shows step 7's counts (one more colony, slipway, year and tech than the record), and after each return the title is back with the disk save unchanged. The second start need not draw like the first (the visual stream is never reseeded), but its game state must match. Verified by the target passing, then failing once (recorded in the step note, not committed) when one frame of `demo.json` is altered. Method: the target also runs step 10's interruption sweep, and passes an explicit dump path under `tmp/demo-check/` to every `replay` it launches.

What `make demo-check` does at HEAD 5e1e0f0 (step 11 done):

- `tests/test_attract_demo` plays the whole compiled demo twice through the attract mode in-process, with a threshold of 3, and runs the interruption sweep. It also writes `tmp/demo-check/attract_cycle.json`: a fixture of empty frames with `"attract_after": 3`, **no `initial_save`**, never committed.
- `replay` plays that fixture with `ATTRACT_STATE`. `tools/demo_check.sh` and `tools/end_state.sh` check both playbacks' end states.
- `pad_author` regenerates `pico8/demo.json` byte for byte, and `pico8/demo.json` is replayed from the resume twice (`REPLAY_STARTS=2`).

What nothing checks yet:

- **The default threshold.** No replay idles 1200 frames on the title. A change to the idle count, its reset after a return, or the 1200 default would pass.
- **The pixels of a whole cycle.** No hash covers two complete demos, their captions, both returns and the title between them. A gameplay or drawing change that keeps the four counts would pass. Measured below: one D-pad press moved inside the closing card keeps every count and changes only the hash.
- **The returns in a replay.** The fixture has no save on disk. The replay never observes the title after a return, or the stored record across the cycle.
- **The default dump.** The `REPLAY_STARTS=2` replay writes the default `tmp/replay_dump.txt`. That is why `make demo-check` must never run beside `make test`. `replay` refuses any capture argument with `REPLAY_STARTS=2`, so it cannot be given a dump path.

## Solution

From the maintainer's side:

1. A committed replay, `worst-case/attract_cycle.json`, plays the full attract cycle with the default threshold. It starts on the title, with a player's save on disk, and has 7994 empty frames:
   - 1200 idle title frames;
   - the whole demo;
   - the return;
   - 1200 idle title frames;
   - the whole demo again;
   - the return;
   - 120 title frames.
2. Its capture hash is committed beside it, as `worst-case/attract_cycle.json.sha256`, in the e2e `.sha256` format.
3. `make demo-check` replays it to its last frame twice:
   - with the `-O0` `replay` that `make full-game` and `make e2e` use. This run gives the capture, the states and the saves;
   - with the ASan+UBSan `replay_sanitize`, for a clean exit.
4. The target checks the content first:
   - at each playback's last frame: one more year, colony, slipway and tech than the record, and no cheat;
   - both playbacks end in the same normalized game state, the one the resumed replay of `pico8/demo.json` ends in;
   - on each return frame, and at the file's end: the title is back (no game: the state equals `worst-case/title_idle.json.end-state`);
   - on each return frame, and at the file's end: the stored record is the file's `initial_save`, byte for byte;
   - no third playback;
   - the sanitize run is clean;
   - the committed replay is exactly what its generator writes.
5. Only when all of that passes does it compare the capture hash with the reference. A missing reference fails.
6. The interruption sweep of step 10 still runs (in `tests/test_attract_demo`). The pad_author regeneration and the resumed replay of `pico8/demo.json` also stay.
7. Every `replay` the target launches gets an explicit dump path under `tmp/demo-check/`. The default dump is never written.
8. The wall time stays at or under step 11's `make demo-check` time: the long runs go in parallel.

## User Stories

1. As a maintainer, I want a replay of the full attract cycle with the default threshold, so that the 1200-frame idle count is checked as players meet it, not only at 3.
2. As a maintainer, I want that replay committed under `worst-case/`, so that anyone can replay it with `make replay EVENTS=worst-case/attract_cycle.json` and read it in a review.
3. As a maintainer, I want the replay to contain only empty frames, so that every demo start, demo frame and return in it comes from the core's attract mode and none from the file.
4. As a maintainer, I want the replay to start on the title with a player's save on disk, so that "the disk save is unchanged" has something to compare.
5. As a maintainer, I want that save to be a record already cut and frozen in the suite, so that it is never typed and never moves with gameplay.
6. As a maintainer, I want the file generated by a committed program and checked byte for byte against its regeneration, so that a demo of another length can never leave a stale cycle in `worst-case/`.
7. As a maintainer, I want the generator to use the shared `"attract_after"` writer, so that the plan's reuse rule holds and the header is spelled as recordings spell it.
8. As a maintainer, I want the capture hash of the whole cycle committed, so that any change to what the demo, the captions, the tag or the title draw is caught, even when the counts still match.
9. As a maintainer, I want the hash taken with the `-O0` `replay` that `make e2e` and `make full-game` use, so that the reference follows the same binary as every other hash.
10. As a maintainer, I want the cycle also replayed by the ASan+UBSan binary, so that the starts and returns over 8000 frames are memory-checked, not only the 176 frames of the e2e.
11. As a maintainer, I want the hash compared only after the content checks pass, so that a broken demo is reported as broken, never as "a new hash to baseline".
12. As a maintainer, I want a missing hash reference to fail the target, so that deleting the reference cannot turn the check off.
13. As a maintainer, I want the state at the last frame of each playback, so that step 7's counts are checked on both playbacks of the default-threshold cycle.
14. As a maintainer, I want both playbacks' normalized states compared with the resumed replay's end, so that the second start is shown to reach the same game as the first.
15. As a maintainer, I want the second playback allowed to draw differently, so that the never-reseeded visual stream does not make the check flaky. Only its game state must match.
16. As a maintainer, I want the state written on the frame of each return, so that "the title is back" is observed on that exact frame.
17. As a maintainer, I want the stored record written on the frame of each return, so that the save is compared byte for byte at both returns, not only at the end of the file.
18. As a maintainer, I want the title state compared with `worst-case/title_idle.json.end-state`, so that "no game is running" is checked against an existing, committed reference.
19. As a maintainer, I want the file's last frame checked too (state and stored record), so that the 120 title frames after the second return are shown to stay on the title with the save untouched.
20. As a maintainer, I want the target to fail on a third playback, so that a counter that restarts too early is caught.
21. As a maintainer, I want an interrupted playback to write no return file, so that the numbered files always pair one completed playback with its return.
22. As a maintainer, I want the interruption sweep of step 10 kept in the target, so that the five interruptions still prove the swallow, the `resume` row and the save after an interruption.
23. As a maintainer, I want every `replay` launched by the target to write its dump under `tmp/demo-check/`, so that the target never touches the default dump that `make test` reads.
24. As a maintainer, I want `replay` to accept `/dev/null` as the capture with `REPLAY_STARTS=2`, so that the resumed replay can take an explicit dump path without a meaningless capture.
25. As a maintainer, I want every check to run and report even after an earlier one failed, so that one run shows every broken part.
26. As a maintainer, I want the failure messages to name the file and the command that rebaselines it, so that a legitimate demo change is easy to accept after its content is checked.
27. As a maintainer, I want `make demo-check` no slower than at step 11, so that plan-pilot can keep running it on every demo-touching step.
28. As a maintainer, I want `make test` and `make e2e` untouched in time, so that the fast loop stays fast (`CLAUDE.md`).
29. As a maintainer, I want the Makefile legend of `make demo-check` to describe the new checks, so that the legend stays true.
30. As a reviewer, I want the step note to record a deliberate failure (one frame of `demo.json` altered in a scratch copy), so that the target is shown to catch a change the counts miss.
31. As a reviewer, I want the frame plan and the measured hash in the step note, so that the reference can be judged without trusting it.
32. As a maintainer, I want every existing `*.sha256`, `.end-state` and `worst-case/` file unchanged, so that the step only adds a check.
33. As the step 14 writer, I want the legend already describing `demo-check`, so that step 14 only adds the README paragraph.

## Implementation Decisions

### Where the cycle lives

- **`worst-case/attract_cycle.json`, committed**, with **`worst-case/attract_cycle.json.sha256`**. It is the plan's "`worst-case/`-style replay": a long, committed replay checked outside the suite. `worst-case/title_idle.json` is the precedent of a `worst-case/` replay that has no `.pad`.
- **It does not join `FULL_GAME_EVENTS`.** So `make full-game`, `make state`, `make end-state`, `make worst-case-peaks`, `make save-worst-case` and `make pad-metrics-worst-case` are unchanged. `make demo-check` owns the file and replays it "as `make full-game` does": same binary, to the last frame, a non-zero exit is a failure. Joining the list would need a peaks review, an `.end-state` and slower targets, none of which the step asks for.
- **No `.end-state` file**: the end of the file is checked against `worst-case/title_idle.json.end-state` (see below).
- `make pad-author` reads only `worst-case/*.pad`, so it never touches the new file and stays no-diff.

### The frame plan (7994 frames, all `{ "pad": [] }`)

Measured on a clean export of HEAD 5e1e0f0, on a file built exactly as below. Frame indexes are 0-based.

| Frames | What the core does | Observed (state of a file cut after that frame) |
|---|---|---|
| f0–1199 | Title, idle count 1…1200 | after f1199: `clock year - month -`, 0 colonies |
| f1200 | **First start**: record resumed, demo frame 0 | after f1200: year 3412, 16 colonies |
| f1200–3935 | Demo frames 0–2735 | after f3935: year 3413, 17 colonies; `ATTRACT_STATE` `.1` |
| f3936 | **First return** (the frame after the last one) | after f3936: no game |
| f3937–5136 | Title, idle count 1…1200 | after f5136: no game |
| f5137 | **Second start** | after f5137: year 3412, 16 colonies |
| f5137–7872 | Demo frames 0–2735 | after f7872: year 3413, 17 colonies; `.2` |
| f7873 | **Second return** | after f7873: no game |
| f7874–7993 | 120 title frames (under 1200: no third start) | end: `worst-case/title_idle.json.end-state`'s lines |

- In general, with a demo of D frames and a threshold T: the starts are at T and 2T+D+1, the returns at T+D and 2T+2D+1, and the total is 2(T+D+1)+120. This is the formula `test_attract_demo` already uses, with 120 trailing frames instead of 10.
- The stored record equals the `initial_save` byte for byte after f1199, f1200, f3935, f3936, f5136, f5137, f7872, f7873 and f7993 (all measured with `SAVE_HEX`).
- Both playbacks' end states, normalized, are equal. The raw files differ only in `zone`/`cap` peak lines, which normalization drops.
- 159 814 bytes, in the size range of `worst-case/full_game*.json` (about 121 KB to 126 KB).

### The header and the save

- Header, in the session recorder's order: `"input": "pad"`, then `"attract_after": 1200`, then `"initial_save": "<hex>"`, then `"frames"`.
- **`"attract_after": 1200` is written explicitly**, even though a missing member means 1200. It is spelled by `replay_source_format_attract_after`, and recordings write it the same way. A change of the default can then never silently change this file's meaning.
- **The `initial_save` is `tests/e2e/pad_tech_costs.json`'s, read through `replay_source`'s accessors** (the plan's reuse rule: no new shell parsing):
  - year 3407, credits 100, 3921 bytes;
  - a record cut from a real game and frozen ("cut, never typed");
  - a galaxy other than the demo record's, so a demo that wrote its own record over the save would show;
  - its file's `.sha256` must stay unchanged by plan rule, so its save never moves.
- Not the in-process forgiving game of `test_attract_demo`: that save is regenerated by gameplay code, and the committed cycle would then move with every gameplay change.

### How the file is produced: `test_attract_demo` writes it, `demo-check` compares it

- **`tests/test_attract_demo` stays the generator**. It already writes the full-cycle fixture with `p8_input_save` and the shared `"attract_after"` writer. Its fixture changes from "threshold 3, no save, 10 trailing frames" to the frame plan above. The threshold-3 fixture and its replay are dropped: the default-threshold cycle checks everything they checked, and more.
- Its command line becomes `test_attract_demo <out fixture.json> <save source events.json>`. The Makefile passes `tests/e2e/pad_tech_costs.json`, so the save's origin can be read in the Makefile.
- Beside the fixture, it writes `<out fixture>.save.hex`: the `initial_save` as one lowercase hex line ending with a newline, the format `SAVE_HEX` writes. The return checks compare against it.
- The demo length comes from the compiled demo (`demo_data_frame_count`), never from a constant, so a regenerated demo of another length gives another file, and the `cmp` below catches the stale commit.
- **Not `pad_author`**:
  - its input step runs on the default threshold, so an `idle 1200` on the title would start the demo inside the author's own run;
  - it has no command for a title-start `initial_save`;
  - a `worst-case/*.pad` would join `make pad-author`'s regeneration loop;
  - the file is pure empty frames, with nothing to author.
- **Not a hand-written file or a scratch script**: the length follows the demo, so a program in the repository must be able to rewrite it.
- `make demo-check` compares `tmp/demo-check/attract_cycle.json` (regenerated) with `worst-case/attract_cycle.json` using `cmp`. On a difference it prints: `demo-check: worst-case/attract_cycle.json is not what test_attract_demo writes (cp tmp/demo-check/attract_cycle.json worst-case/attract_cycle.json, then rebaseline its hash)`.

### `replay`: the returns, and an explicit dump with `REPLAY_STARTS=2`

Two small changes to `src/cmd/replay.c`, the tool step 10 named for steps 12 and 13.

- **`ATTRACT_STATE` also writes two files on the return frame of each completed playback:**
  - the return frame is the frame after the one that wrote `<file>.<n>`;
  - `<file>.<n>.return`: the state in `STATE`'s format, with the cheat count of that playback;
  - `<file>.<n>.return.hex`: the stored record in `SAVE_HEX`'s format (one hex line). It is written by the same writer as `SAVE_HEX`, and the "save:" summary lines go to stdout, as with `SAVE_HEX`;
  - a file that ends on a playback's last frame writes no `.return` for it;
  - an interrupted playback writes nothing, as today.
- **With `REPLAY_STARTS=2`, a capture argument of exactly `/dev/null` is accepted as "no capture"**, so a dump path can follow it. Any other capture path is still refused with today's message. `/dev/null` is already how `make full-game`, `make state` and `make sanitize-net` say "no capture". This is the smallest change that gives the resumed replay an explicit dump. A new environment variable or a `-` placeholder were rejected as new conventions.
- Nothing else in `replay` changes. The capture, the dump, `STATE`, `SAVE_HEX` and the refusals stay as they are.

### The target

`make demo-check` gains these prerequisites:

- `worst-case/attract_cycle.json` and its `.sha256`;
- `tmp/replay_sanitize`;
- `tests/e2e/pad_tech_costs.json`;
- `worst-case/title_idle.json.end-state`.

It stays outside `make test-all`.

**Runs** (all paths under `tmp/demo-check/`, cleared at the start):

1. `test_demo_check` (fast, first, as today).
2. Started in the background at once (the committed cycle no longer waits for `test_attract_demo`):
   - **a.** `test_attract_demo` writes `attract_cycle.json` and `attract_cycle.json.save.hex`, then runs its two in-process playbacks and the **interruption sweep** (fold 480, colonise 945, slipway 1380, year 1830, research 2280). Log: `attract_demo.log`.
   - **b.** `pad_author pico8/demo.pad demo.json --no-aim-budget` (regeneration, as today). Log: `regenerate.log`.
   - **c.** The `-O0` `tmp/replay`, on `worst-case/attract_cycle.json`. Capture `attract_cycle.bin`, dump `attract_cycle.dump.txt`. With `ATTRACT_STATE=attract.txt`, `STATE=attract_cycle.end.txt` and `SAVE_HEX=attract_cycle.end.hex`. Log: `attract_cycle.log`. When it exits 0, the hash of the capture goes to `attract_cycle.sha256`, in `make e2e`'s way (keep sha256's exit status, hash only).
   - **d.** `tmp/replay_sanitize`, on `worst-case/attract_cycle.json`, capture `/dev/null`, dump `attract_cycle.sanitize.dump.txt`, log `attract_cycle.sanitize.log`.
3. In the foreground, as today:
   - `pad_author … --stop-at probe` with `STATE=record.txt`;
   - `REPLAY_STARTS=2 STATE=end.txt tmp/replay pico8/demo.json /dev/null end.dump.txt`. This is the new explicit dump.

**Checks**, every one always run, each failure printed as `demo-check: <file>: <reason>`, the exit status set at the end:

1. **The resumed demo** (today's): `tools/demo_check.sh pico8/demo.json record.txt end.txt.start1 end.txt`.
2. **The in-process tests**: `test_attract_demo` exited 0. Its asserts cover both playbacks, the overlay and the sweep.
3. **The playbacks of the cycle**:
   - replay c exited 0;
   - `tools/demo_check.sh pico8/demo.json record.txt attract.txt.1 attract.txt.2` passes: counts +1, no cheat;
   - there is no `attract.txt.3`;
   - `tools/end_state.sh check` of `attract.txt.1` and `attract.txt.2` against the normalized `end.txt` passes: **the second start reaches the same game state as the first**.
4. **The returns**:
   - `tools/end_state.sh check` of `attract.txt.1.return`, `attract.txt.2.return` and `attract_cycle.end.txt` against `worst-case/title_idle.json.end-state` passes: no game, no count, no cheat, so the title is back;
   - `cmp` of `attract.txt.1.return.hex`, `attract.txt.2.return.hex` and `attract_cycle.end.hex` with `attract_cycle.json.save.hex` passes: the disk save is unchanged, byte for byte, at both returns and at the end.
5. **Sanitize**: replay d exited 0, else its log is printed.
6. **The committed cycle**: `cmp` of the regenerated fixture with `worst-case/attract_cycle.json` passes.
7. **The demo's source** (today's): `cmp` of the regenerated `demo.json` with `pico8/demo.json` passes.
8. **The hash, last, only if checks 1–6 passed**:
   - it compares `attract_cycle.sha256` with `worst-case/attract_cycle.json.sha256`;
   - a missing reference fails;
   - a difference prints both hashes and the baseline command (`cp tmp/demo-check/attract_cycle.sha256 worst-case/attract_cycle.json.sha256`, after reading the content checks above);
   - if a content check failed, it prints `demo-check: hash not compared: the content checks failed`, which counts as a failure. Check 7 does not gate the hash: the cycle plays the compiled `pico8/demo.json`, whatever `demo.pad` says.

- `tools/frames_check.sh` is not reused. It tolerates a missing reference and names `make e2e-baseline`, and both are wrong here. The comparison is two `cat`s in the recipe, as `frames_check.sh` does it.
- The capture (77 MB) stays in `tmp/demo-check/` until the next run clears it, so a human can compare it with a reference capture.
- **The dumps**: every `replay` the target launches has its own dump in `tmp/demo-check/` (`attract_cycle.dump.txt`, `attract_cycle.sanitize.dump.txt`, `end.dump.txt`). `pad_author` launches its own `replay` with a dump under `tmp/pad-author/`. That run is the author's, not the target's, and stays as it is. So the rule "never alongside `make pad-author`" still holds. The plan's "never alongside `make test-all`" guardrail is unchanged too.
- **No new baseline target.** The two `cp` commands are printed by the failures and written in the legend. A `demo-check-baseline` target would be new surface for two files.

### The Makefile legend

`make demo-check`'s entry is rewritten to say what is true, following this PRD's checks:

- the committed cycle and its hash;
- the two binaries;
- the return files and the save comparison;
- the title reference;
- the regeneration `cmp`s;
- the hash compared last, only after the content checks;
- the two baseline commands;
- the explicit dumps.

The `make replay` entry gains the `.return`/`.return.hex` files of `ATTRACT_STATE` and the `/dev/null` capture with `REPLAY_STARTS=2`. Step 14 then only adds the README paragraph.

### Decisions taken without a human (`.claude/skills/guideline/SKILL.md`)

- **"The `-O0` sanitize binary"**: no such binary exists. `make full-game` uses the `-O0` `tmp/replay`, and `make sanitize-net` uses the `-O2` ASan+UBSan `tmp/replay_sanitize`. DECISION: the hash comes from `tmp/replay`, as the plan's "as `make full-game` does" says and as every e2e hash does. `tmp/replay_sanitize` also replays the cycle for a clean exit. WHY: "make the plan work" covers both readings. Measured cost: 4.5 s of CPU in parallel, the same capture hash as `-O0`, no wall-time increase on 4 cores.
- **Committed file plus a regeneration `cmp`, generated by `test_attract_demo`**: it is the pattern `pico8/demo.json` already follows in this target, and it reuses the existing writer. WHY: the plan's reuse rule, and the smaller change against teaching `pad_author` a title-start save and the attract threshold.
- **Not in `FULL_GAME_EVENTS`**: WHY: the step names `make demo-check` as the target that replays it. Joining would widen six other targets.
- **The save from `pad_tech_costs.json`**: WHY: a frozen cut record already in the suite (step 12's PRD made the same choice). Gameplay changes cannot move it.
- **The return observed by `ATTRACT_STATE`, not by cut copies of the file**: cut prefixes would cost about 4000 more replayed frames and a second generated file. The extension is a few lines in the tool step 10 named for this step, and it observes the exact return frame. WHY: the smaller runtime and the existing tool.
- **"Title is back" = the title state reference plus the save bytes**: the `resume` row itself is proven by behaviour in `test_attract_demo` (same target) and by step 12's e2e. The replay's hash pins the row's pixels. WHY: the step asks for "the title is back with the disk save unchanged", and both are measured.
- **`/dev/null` accepted with `REPLAY_STARTS=2`**: WHY: the step's method requires an explicit dump for every `replay`, and the smallest change uses the existing "no capture" spelling.
- **The deliberate failure in a scratch copy of the repository**, never in the working tree: the compiled demo unit is rebuilt from `pico8/demo.json`. A scratch copy cannot leave an altered file to be committed or a stale unit behind.

## Implementation Chunks

1. **`replay`: return files and an explicit dump with `REPLAY_STARTS=2`.**
   - Red: `test_replay_from_resume` asserts that `REPLAY_STARTS=2 … <events> /dev/null <dump>` succeeds and writes `<dump>`, and that a real capture path is still refused.
   - Red: `test_replay_from_title`'s interrupted-demo case also asserts that no `.1.return` or `.1.return.hex` is written.
   - Green: the two changes in `replay.c`. The `.return` files of a completed playback are proven in chunk 3, because a completed playback costs 2736 frames, too slow for `make test`.
   - Done when `make test-all` passes and `make test` is not slower.
2. **The generator and the committed cycle.**
   - `test_attract_demo` writes the default-threshold fixture and its `.save.hex` from its second argument.
   - Commit `worst-case/attract_cycle.json`, copied from a run.
   - Check the frame plan with cut copies (the table's readings, state and `SAVE_HEX`) before going further.
3. **The target.**
   - The runs, the checks and the legend as above.
   - First run with no reference: every content check passes, and the target fails only on the missing reference.
   - Copy the hash, run again: it passes.
   - A third run after `rm -rf tmp/demo-check` gives the same hash.
   - Then the deliberate failure in a scratch copy, recorded in the note.

## Testing Decisions

- A good test here observes the replay from outside, as a human could:
  - the states `replay` writes;
  - the stored record in `SAVE_HEX`'s format;
  - the capture's hash;
  - the exit statuses.

  Nothing reads the attract module's fields. The only accessors used are step 10's (`input_step_attract_phase`, `input_step_attract_demo_frame`), inside `replay`.
- **Modules tested**:
  - `replay`'s two changes: unit-tested in `make test` for the cheap cases (refusals, interrupted playback, `/dev/null` with a dump), and in `make demo-check` for the completed-playback files;
  - the generator: by the `cmp` with the committed file;
  - the whole cycle: by the target.
- **Prior art**:
  - today's `make demo-check` recipe (background runs, `failed=` accumulation, `tools/demo_check.sh`, `tools/end_state.sh check`);
  - `make e2e`'s `frames_hash` (hash only, sha256's status kept);
  - `make full-game`'s replay to the last frame with `/dev/null` and an explicit dump;
  - `tests/test_replay_from_title.c` and `tests/test_replay_from_resume.c` for `replay`'s environment variables and refusals.
- **The deliberate failure** (a scratch copy of the repository, never committed):
  - alter **demo frame 2700** of `pico8/demo.json` (line 2715 of the file at HEAD, inside the closing card, 144 frames after its caption, so `demo_generate`'s pacing rule accepts it) from `{ "pad": [] }` to `{ "pad": [4] }`;
  - run `make demo-check`. Expected, as measured on the export: the compiled demo is rebuilt; both playbacks still end at 3413/17/19/3 with the same normalized state, so the content checks pass; check 7 fails (`demo.pad` no longer generates the file); check 8 fails, with the capture hash `c7e2916d511728e1…` against the reference's `999f9cd6573c180e…`;
  - this shows the hash catching a change the counts miss. Step 7's note already recorded a count-breaking change (frame 943 blanked) failing the content checks;
  - the note records the outputs. If `[4]` changes a count with the final code, the implementer picks another D-pad button and records which.
- **Guardrail runs** (plan section 2; this step changes `replay` and the target, not the core):
  - `make test-all`, then `make demo-check` after it, never at the same time;
  - `make full-game` and `make end-state` (both run `replay`);
  - `make pad-author` with no diff in `worst-case/` or `pico8/demo.json`;
  - `git status worst-case tests/e2e`: only the two new `worst-case/` files.
- **Timing**:
  - `make test`: three runs, step 1's method, against 8.2 s;
  - `make demo-check`: three runs, against step 11's 13.3 s. Prototype on the export, 4 cores: today's target 11.5 s; the new runs (sweep, regeneration, `-O0` cycle with capture and hash, sanitize cycle, resumed replays) 9.8 s.
- **The step note records**:
  - the frame plan as committed;
  - the generator's command;
  - the reference hash and the commands that produced it;
  - the outputs of the deliberate failure;
  - the timings;
  - any deviation.

## Out of Scope

- The README paragraph (step 14).
- Adding the cycle to `FULL_GAME_EVENTS`, `make state` or the peaks table.
- Teaching `pad_author` a title-start save or the attract threshold.
- A `resume` row check by pixels or by entity inside the replay (proven by `test_attract_demo` and step 12's e2e).
- Moving `pad_author`'s internal replay dumps out of `tmp/pad-author/`.
- Removing `test_replay_from_resume`'s use of the default dump, or the Makefile ordering that works around it.
- Any change to the attract mode, the overlay, the demo, `pico8/demo.json` or `tools/demo_check.sh`'s rules.
- A `demo-check-baseline` target.

## Further Notes

- Every reading in this PRD was taken on a clean `git archive` of HEAD 5e1e0f0 in the scratchpad, with `tmp/replay`, `tmp/replay_sanitize` and `tmp/test_attract_demo` built from it. The file was built exactly as the frame plan says, with `pad_tech_costs.json`'s `initial_save`:
  - **capture hash `999f9cd6573c180e82191f29fa144bc19920e6d4839df23bf5651b78a3400543`**, the same from `-O0` and from the sanitize binary, and the same on a second `-O0` run;
  - `-O0` replay with capture 4.2 s, sanitize replay 4.5 s;
  - `test_attract_demo` 8.5 s.

  This hash is what the implementer should expect. It is still adopted only after the content checks pass on the final code.
- `ATTRACT_STATE` writes `.1` and `.2` on f3935 and f7872 today. The `.return` files will be written on f3936 and f7873.
- The 120 trailing frames are far under the threshold, so the file ends on the settled title, which a human can read in `attract_cycle.dump.txt`.

### Hypotheses

Step 12 (`docs/prd/triage/phase2-step12-attract-e2e.md`) is being implemented while this PRD is written. This PRD reads the code at HEAD 5e1e0f0. Below is what it assumes about step 12's outcome. If step 12's note says otherwise, the note wins, and only the named part of this PRD follows it.

- **H-12-1 (no core change):** step 12 changes no file under `src/`, `pico8/` or the demo unit's inputs. So the attract timings, the frame plan above and the measured hash still hold. If step 12 changed the core or the overlay, re-measure the frame plan and the hash, content checks first.
- **H-12-2 (the save source frozen):** `tests/e2e/pad_tech_costs.json`, its `initial_save` included, is unchanged by step 12, as the plan's guardrail requires. If step 12 adds `tests/e2e/attract.json` with the same save (its PRD's decision), nothing here changes. The cycle keeps reading `pad_tech_costs.json`, the older file.
- **H-12-3 (demo-check untouched):** step 12 does not change `make demo-check`, `tests/test_attract_demo.c`, `src/cmd/replay.c`, `tests/test_replay_from_title.c` or `tests/test_replay_from_resume.c`. This PRD's chunks start from their HEAD versions.
- **H-12-4 (the short cycle is step 12's):** `tests/e2e/attract.json` covers the threshold-3 start, the interruption, the swallow and the `resume` row by behaviour inside `make e2e`. That is why this step drops the threshold-3 fixture and does not check the `resume` row in the replay.
- **H-12-5 (no `worst-case/` change):** step 12 adds nothing to `worst-case/` and changes no `FULL_GAME_EVENTS` entry. So `worst-case/title_idle.json.end-state` is still the title reference, and the two new files are the only `worst-case/` additions.
- **H-12-6 (`test_live_slots` aside):** step 12's change to `tests/test_live_slots.c` (following `"attract_after"`) does not read `worst-case/`. So the new file does not enter `make sanitize-net` through it. The cycle's sanitize run is this target's own.
