---
title: "pad_author starts from a record, at the attract mode's start point"
description: "pad_author cannot start a script from a save record, and no tool plays a file from a resume instead of the title, so the demo's frames cannot be authored or checked from pico8/demo_record.hex."
status: needs-triage
---

## Problem Statement

Plan 18 step 5: **`pad_author` starts from a record, at the attract mode's start point.**

The demo plays from the record that step 2 committed, `pico8/demo_record.hex`. The attract mode does not reach it through the title. It calls `cart_resume_game` on the compiled record and feeds the demo's frames straight after it. Two things are missing:

- **`pad_author` cannot start from a record.** Every script begins on the title (`title`, `new-game`, or an `include` that reaches them). `pad_author` writes no `initial_save`, and the e2e files that have one (`pad_tech_costs`, `pad_resume_at_start`) were cut by hand with `make replay SAVE_HEX=…` and start on the title. Their first frames press Up and X on the `resume` row. A demo authored that way would carry a title prefix the attract mode never plays.
- **No tool plays a file from the resume.** `replay` writes a file's `initial_save` to storage and always starts on the title. Fed the demo's frames, it would play them on the title: the first press would pick a difficulty row, not fold space. `make state`, `make replay SAVE_HEX=…`, `pad_author`'s self-check and every later check of the demo's content (steps 7, 10 and 13) depend on that tool.

Until both exist, the demo script (step 7) cannot be written, and nobody can tell whether section 2's beats can be played from the record within 3600 frames.

## Solution

1. **A `from-record <file.hex>` script command.** It must come before the script's first frame. It reads the committed hex file, resumes the record at the attract mode's start point, and makes `pad_author` write the record into its output's `initial_save`. The output also gets a new header member, `"start": "resume"`. Its frames start at the first frame after the resume: there is no title prefix.
2. **The start point, defined in one place.** A small devtools module, `record_start`, does the start: it decodes the record, calls `cart_resume_game`, and re-initialises the input step on the default bindings table, which is step 4's contract. The module also does the return: `cart_return_to_title`, then the input step on the compiled table. Both `pad_author` and `replay` call it, so the two cannot drift apart.
3. **`replay` plays a record-based file from the resume.** When the header says `"start": "resume"`, `replay` boots, starts the record through `record_start` without writing it to storage, and plays the frames. `make replay`, `make state` and `pad_author`'s self-check work on such a file with no other change. This is **the one tool** the plan asks this step to name. Every later check of the demo's content uses it.
4. **Step 4's two-start cycle, in the same tool.** With `REPLAY_STARTS=2`, `replay` plays the file, returns to the title, idles 30 frames, starts the record again and plays the file again. It writes the state at the end of each playback.
5. **A proof.** `tools/pad_author_proofs/demo_beats.record.pad` plays section 2's beats from the record, in order, with the existing high-level commands, without captions, and with idle gaps of about the pacing sizes. `make pad-author-proof` gains a fourth kind of proof, `*.record.pad`. The proof passes when `pad_author` writes the file, its self-check (one start) is byte-identical, and the two-start replay reaches `pad_author`'s own end state at both ends.

Investigation for this PRD ran the beat line on a throwaway harness from a fresh resume of the record, with the input step re-initialised. Every beat succeeds, the rough total is **1415 frames** (limit 3600), and the second start of a two-start cycle reaches the same normalised end state. Step 2 does not need a re-cut (see Further Notes).

## User Stories

1. As the step-7 author, I want a `pad_author` script to start from `pico8/demo_record.hex`, so that I can write `pico8/demo.pad` from the record without typing a title prefix.
2. As the step-7 author, I want the record's path written relative to the script, as `include` paths are, so that `pico8/demo.pad` reads `from-record demo_record.hex`.
3. As the step-7 author, I want the output's frames to begin with the first frame after the resume, so that `demo.json`'s frames are exactly what the attract mode feeds.
4. As the step-7 author, I want the output's `initial_save` to hold the record's bytes, so that `demo.json` carries its own start and step 8 compiles it from one file.
5. As the step-7 author, I want `pad_author`'s closed-loop commands (`fold`, `colonise`, `slipway`, `year`, `research`, `idle`) to work unchanged after the resume, so that the demo is written like any other script.
6. As the step-7 author, I want `--try` to work on a record-based script, so that I can test beat variants without regenerating everything.
7. As the step-7 author, I want `--stop-at` and `make pad-author SCRIPT=…` to work on a record-based script, so that I author the demo the way `full_game` is authored.
8. As the step-7 author, I want a rough beat total measured now, so that I know the demo fits in 3600 frames before I write it.
9. As the step-10 implementer, I want the start point defined in one module, with its contract written down, so that the attract module reproduces exactly what the tool did.
10. As the step-10 implementer, I want the start to reset the input step on the default table, so that the demo's first frame sees the pad navigation state step 4 fixed.
11. As the step-10 implementer, I want the tool to start the record without writing it to storage, so that the tool's run is as close as possible to the attract mode, which never stores the record.
12. As the step-7 and step-13 checker, I want `make state EVENTS=<record-based file>` to give the state at the end of the demo, so that I read the colony, slipway, year and tech counts there.
13. As the step-13 checker, I want a two-start mode in the same tool, so that a second demo in one process is checked against the first before the attract module exists.
14. As a maintainer, I want `replay` to refuse a record-based file whose header is inconsistent (no `initial_save`, an unknown `start` value, a repeated member), so that a hand-edited file fails loudly instead of playing on the title.
15. As a maintainer, I want every file without `"start"` to replay exactly as today, so that every `*.sha256`, `.end-state` and `worst-case/` file stays unchanged.
16. As a maintainer, I want `pad_author`'s output for title-based scripts to stay byte-identical, so that `make pad-author` and `make pad-author-proof` show no diff in `worst-case/` or `tests/e2e/`.
17. As a maintainer, I want `pad_author` to refuse `from-record` after a frame, a second `from-record`, a missing file or a record that does not decode, each with the usual `file:line` message, so that a mistyped script never produces a half-title demo.
18. As a maintainer, I want each refusal that `pad_author` can reach from a script to have a `.refused.pad` proof, so that `make pad-author-proof` keeps them working.
19. As a maintainer, I want the new unit tests in `make test` to be cheap, so that `make test` stays at step 1's timing.
20. As a maintainer, I want the slow proof (about 1400 frames, played three times) to run only in `make pad-author-proof`, outside `make test-all`, so that the fast loop does not pay for it.
21. As a maintainer, I want a record-based file to play on the default bindings only, so that no fixture or personal table can change what the demo does.
22. As a reviewer, I want the step note to name the tool and give the proof's rough total and end counts, so that step 7 starts from known numbers.
23. As a player (later), I want the demo built from the same start the tools checked, so that what I watch on the title is the game the checks verified.

## Implementation Decisions

- **Modules changed or added:**
  - **`record_start`** (new, devtools). It is the only code that starts or leaves a record-based run. Three calls:
    - `record_start_read_hex(path, bytes, capacity, error, error_size)` reads one line of hex digits, an even count, at most `SAVE_STATE_MAX_BYTES` bytes, then a newline and nothing else. It returns the byte count, or 0 with a message `"<path>: <reason>"`: missing file, odd count, non-hex character, too long, trailing data, or bytes that do not decode.
    - `record_start_resume(step, bytes, count)` decodes into the module's own static `save_state_t`, calls `cart_resume_game`, then runs `input_step_init(step, &bindings_compiled)`. It asserts `!bindings_compiled_personal`, because tools link the default table. It returns 0 if the bytes do not decode, and then touches nothing. Contract: it runs at the top of a frame, either right after boot's `cart_init` or after at least one `cart_update60` since the last return (H-4-4).
    - `record_start_return(step)` calls `cart_return_to_title` (H-4-1), then `input_step_init(step, &bindings_compiled)`. It runs between frames only.
  - **`replay_source`** reads a new header member, `"start"`. Missing means the title. `"resume"` means the resume. Any other value, a repeated member, or `"resume"` without `initial_save` fails the load. A new accessor, `replay_source_start()`, returns `REPLAY_SOURCE_START_TITLE` or `REPLAY_SOURCE_START_RESUME`. It returns the title when there is no recording, after a failed load and after close, following `replay_source_attract_after`'s pattern. `replay_source_seed_storage` **erases** storage for a resume start: the record is the demo's start, not a player's save.
  - **`p8_input_save`** gains a header-members argument. `NULL` writes today's bytes. Otherwise it is bare JSON members, written after `"input": "pad"`, under the same convention and assert as `p8_input_append`'s `header_member`. Several members are joined by `",\n  "`. Every existing caller passes `NULL`.
  - **`replay` (the command)**:
    - For a resume start it boots (`cart_load`, `cart_init`), calls `record_start_resume` with the header's `initial_save` before the first frame, and plays every frame from there. It does not play a title frame first.
    - It refuses a resume-start file that has a `"bindings"` member, with a `replay:` message: the demo plays on the default table (section 2).
    - New environment variable `REPLAY_STARTS`, set to `1` (the default) or `2`, for resume-start files only. With `2`, after the last frame `replay` calls `record_start_return`, plays 30 empty title frames, calls `record_start_resume` again and plays every frame again.
    - With `REPLAY_STARTS=2` and `STATE` set, the state at the end of the first playback goes to `<STATE>.start1`, and the end of the second to `STATE`.
    - `REPLAY_STARTS` refuses any other value, a title-start file, a capture path and `PAD_METRICS`: the two-start mode is for state checks only. `SAVE_HEX`, `REPLAY_FILL` and the dump read the end of the last playback.
  - **`pad_author`** gains the command `from-record <file.hex>`:
    - The path is resolved like `include`'s, relative to the script, or the working directory for a `--try` line.
    - It fails with `from-record: before the first frame only` when a frame has been emitted. It fails with `from-record: already starting from '<path>'` on a second use, and with `from-record: <reader message>` on a bad file.
    - On success it calls `record_start_resume` on `pad_author`'s own input step and keeps the bytes in `pad_author_t`.
    - The `pad_author` command writes them as `"start": "resume"` then `"initial_save": "<lowercase hex>"`, through `p8_input_save`'s header members. A script without `from-record` writes exactly today's bytes.
    - Commands that need the title (`title`, `new-game`) already fail when a game runs, so no new check is needed for them.
  - **Makefile:**
    - `record_start.c` joins the `pad_author` and `replay` sources.
    - `make pad-author-proof` gains `PAD_AUTHOR_RECORD_PROOFS` (`tools/pad_author_proofs/*.record.pad`). For each one: `pad_author --no-aim-budget`, whose self-check (`replay`, one start) must be byte-identical. Then `REPLAY_STARTS=2 STATE=… tmp/replay`. Then `pad_author`'s state is normalised into a reference with `tools/end_state.sh normalize`, and `tools/end_state.sh check` runs on both `<STATE>.start1` and `STATE` against it.
    - Legend lines for `make replay`/`make state` (`"start": "resume"`, `REPLAY_STARTS`) and `make pad-author-proof` (`*.record.pad`).
- **The start point (the contract this step fixes).**
  - A record-based output's frame 0 is the first frame after `cart_resume_game`, in the same loop turn: input step, `cart_update60`, `cart_draw` (H-4-3). The input step is what `input_step_init` gives on the default table (H-4-2).
  - The tools start right after boot's `cart_init`, with **no title frame**. Investigation: a resume straight after boot works, because boot's `cart_init` does not flip the empty game zone. Starts after 0, 1 and 37 title frames gave identical normalised end states. Only particle and session counters differed, from the visual stream (step 4's Further Notes, items 1 and 8).
  - The attract mode will start after at least its threshold of title frames. Its game state is the same, and its pixels may differ: section 2 accepts this, since the visual stream is never reseeded.
- **Why `"start": "resume"` and not an inference from `initial_save`.** `pad_tech_costs.json` and `pad_resume_at_start.json` have an `initial_save` and start on the title. Only an explicit member tells the two apart without touching a committed e2e. The member is absent from every existing file, so their replays cannot change.
- **Why storage is erased for a resume start.** The attract mode keeps the player's save in storage and never stores the record. Nothing the demo's frames do reads storage, since they never visit the title. An erased store keeps `SAVE_HEX` at the end showing only what the demo wrote. Until step 10, the demo's actions still write saves, as step 4 recorded.
- **Why the two-start cycle lives in `replay` and not in step 4's harness.** Decided per the guideline rules, as no human answers: "make the plan work", "the smaller change". Step 4's harness is unit-test code in `tests/test_cart.c`, with static helpers and frames generated in the test (H-4-6). It cannot be fed a file that `pad_author` writes at run time. `replay` already plays files, writes `STATE`, and is the tool step 7 needs. Two calls of `record_start` plus a 30-frame idle reproduce step 4's cycle with the same core calls (`cart_resume_game`, `cart_return_to_title`, `input_step_init`). If step 4 lands a reusable harness outside the tests, the implementer calls it from `replay` instead and records that in the step note.
- **What "reaches `pad_author`'s own end state both times" means.**
  - The first start: byte-identical state files. This is `pad_author`'s existing self-check, which now goes through the resume.
  - Both ends of the two-start run: equal after `tools/end_state.sh normalize`. Normalising drops camera, screen, visible, zone, budget and cap. Step 4's proof showed that the zone and cap peaks and the visual stream differ across a cycle, and the game state does not (H-4-7).
- **The proof script**, `tools/pad_author_proofs/demo_beats.record.pad`, uses the beats of section 2 with step 2's PRD decisions: the probe beat is a `fold`, because a probe is already live, and the slipway links a new colony to an existing one.

  ```
  from-record ../../pico8/demo_record.hex
  idle 150                      # opening card
  idle 90; fold 118 307; idle …  # probe, at least 150 frames in all
  idle 90; colonise p32; idle …
  idle 90; slipway p32 p55; idle …
  idle 90; year; idle …          # Select hold
  idle 90; research xenofoods; idle …
  idle 180                      # closing card
  ```

  The `idle …` after each beat pads that beat to at least 150 frames (section 2's caption duration). One command per line in the committed file; the `;` above is only for brevity. The planets, the point and the tech come from step 2's note. If one fails from the record, the implementer follows the plan: go back to step 2 and cut at another year, under its stop rule.
- **Refusals reachable from a script** each get a `.refused.pad` proof with an `# expect:` line: `from_record_late` (a frame before it), `from_record_twice`, `from_record_missing` (no such file). A malformed or undecodable hex file is covered by `record_start`'s unit tests, since a committed bad hex fixture would only duplicate them.
- **Guardrail runs.** This step changes `replay`, which `make e2e`, `make end-state` and `make full-game` use. It runs `make test-all`, `make end-state`, `make full-game`, `make pad-author` (no diff in `worst-case/`) and `make pad-author-proof` (no diff in `tests/e2e/`). It does not touch `src/game/`.
- **The step note records:**
  - the tool's name and its contract: `replay` with `"start": "resume"`, through `make replay`/`make state`, and `REPLAY_STARTS=2` for the cycle;
  - the start-point contract above;
  - the proof's total frames and end counts (year, colonies, slipways, techs, credits) against the record's;
  - the `make test` timing against step 1's minimum (8.2 s).

## Implementation Chunks

1. **Header member and writer.** `replay_source` reads `"start"` (`replay_source_start`, erase on a resume start). `p8_input_save` takes header members. Unit tests in `test_replay_source` and `test_input`. Nothing uses them yet, and every `.sha256` is unchanged.
2. **`record_start` module.** Read hex, resume with the input-step reset, return. Unit tests in a new `tests/test_record_start.c`. Needs `cart_return_to_title` from step 4 (H-4-1).
3. **`replay` from the resume.** The resume start, the `"bindings"` refusal, and `REPLAY_STARTS=2`. A new `tests/test_replay_from_resume.c` drives `tmp/replay` on files it writes in `tmp/`, as `test_replay_from_title` does.
4. **`pad_author` `from-record`.** The command, the output header, the refusal proofs, `demo_beats.record.pad`, the `*.record.pad` kind in `make pad-author-proof`, and the Makefile legend.

## Testing Decisions

- A good test drives each module through its public interface only: `record_start`'s three calls, `replay_source_open`/`replay_source_start`/`replay_source_seed_storage`, the `replay` binary's arguments and environment, and `pad_author` scripts. It reads results through public readers: `state_write`, `save_state_capture`/`save_state_encode`, `save_storage_read`, the title's buttons, the input step's struct as `input_step_init` builds it, and the files the binaries write. It never reads a module's statics.
- **`test_replay_source`** (existing, `make test`):
  - `"start"` missing gives the title;
  - `"resume"` with `initial_save` gives the resume, and `seed_storage` leaves storage empty;
  - an unknown value, a non-string, a repeated member, or `"resume"` without `initial_save` fails the load, and the accessor then reads the title;
  - after close it reads the title.
- **`test_input`** (existing): `p8_input_save` with `NULL` writes the same bytes as before (the existing byte tests keep passing unchanged). With members, the file has them after `"input": "pad"`, loads back with `p8_input_load`, and `replay_source` reads them.
- **`test_record_start`** (new, `make test`, a few dozen frames):
  1. Reading `pico8/demo_record.hex` gives bytes that decode. A missing file, an odd count, a non-hex character, trailing data and undecodable bytes each return 0 with a message naming the path. Fixtures are written to `tmp/` by the test.
  2. After some title frames that press buttons, which dirties the input step, `record_start_resume` gives a running game. `save_state_capture` plus encode gives the record's bytes back (assert equality only, no record numbers). The input step equals a freshly `input_step_init`ed one on the default table.
  3. `record_start_return` gives a title with no game. After one `cart_update60`, a second `record_start_resume` gives the same encoded bytes again.
  4. Undecodable bytes return 0 and leave the title untouched.
- **`test_replay_from_resume`** (new, `make test`). It writes a resume-start file from `pico8/demo_record.hex` with about 30 empty frames, and runs `tmp/replay` on it:
  - `STATE` shows a running game, not the title;
  - `REPLAY_STARTS=2` writes `.start1` and the final state, equal after `end_state.sh normalize`;
  - `REPLAY_STARTS=3`, `REPLAY_STARTS=2` on a title-start file, `REPLAY_STARTS=2` with a capture path, and a resume-start file with `"bindings"` each fail with a `replay:` message.

  A title-start fixture, `title_pick_reasonable.json`, replays exactly as before (`test_replay_from_title` already holds its bytes). About 100 frames over a handful of runs, a few tens of milliseconds.
- **`make pad-author-proof`** (outside `make test-all`): the record proof and the three refusal proofs. Existing frame and end-state proofs stay byte-identical.
- **Prior art:**
  - `tests/test_replay_from_title.c` and `tests/test_replay_usage.c` (driving `tmp/replay` from a unit test);
  - `tests/test_replay_source.c` (`attract_after` cases, step 3);
  - `tests/test_save_load.c` and `src/devtools/save_check.c` (resume, then compare encoded bytes);
  - `tools/pad_author_proofs/*.refused.pad` (`# expect:` refusals);
  - the `*.end-state.pad` rule of `make pad-author-proof` (`tools/end_state.sh check`).
- **Timing:** `make test` with nothing to rebuild, three runs, minimum, must not exceed step 1's 8.2 s beyond noise. `make e2e` is unaffected: the only change on its path is one more header lookup per file.
- **Every `tests/e2e/*.json.sha256`, `.end-state` and `worst-case/` file unchanged**, checked by `make test-all`, `make end-state`, `make full-game`, `make pad-author` and `make pad-author-proof`.

## Out of Scope

- The `caption` command and the `captions` member (step 6). The proof has no captions.
- `pico8/demo.pad`, `pico8/demo.json` and `pico8/demo.txt` (step 7), and `make demo-check` (steps 7 and 13).
- The compiled demo (step 8), the default table beside the compiled one in `game` (step 9), and the attract module (step 10). `record_start` is a devtools module. Step 10 may move its start into the core, but this step does not anticipate that.
- Cutting the save during the demo (step 10). The tool's runs still write saves, as the game does today.
- Any change to `src/game/`, to `cart_init` or `cart_resume_game`, or to the session recorder. Recordings always start on the title.
- Re-cutting `pico8/demo_record.hex`. Investigation found no beat failing, so step 2's stop rule does not come into play.
- Converting the existing hand-cut `initial_save` e2e files to `from-record`. They start on the title on purpose.

## Further Notes

### Investigation (throwaway harness under `tmp/`)

- **From the label**, with `pad_author --try` on `include worst-case/full_game.pad demo-record` (2908 frames), the line `idle 150; fold 118 307; idle 90; colonise p32; idle 90; slipway p32 p55; idle 90; year; idle 90; research xenofoods; idle 180` succeeds. Frames per command: `fold` 71, `colonise` 20, `slipway` 10, `year` 45, `research` 16.
- **From a fresh resume of `pico8/demo_record.hex`**, the input step re-initialised on the default table: a harness linked against `pad_author`'s objects called `cart_resume_game`, then `pad_author_run_line` with the beats padded to section 2's pacing:
  - an opening of 150;
  - for each beat, 90 frames before the press and enough after it to make 150;
  - a closing of 180.

  Result: **1415 frames**, every beat ok. End state: year 3413 month 1, credits 105, science 8, income 42, techs space folding, geneseeds, xenofoods; 17 colonies, 19 slipways, 74 planets. The record has year 3412, 16 colonies, 18 slipways and 2 techs: one more colony, slipway, year and tech, as step 7 needs.
- **Title frames before the start** (0, 1, 37): normalised end states are identical. The raw state files differ only in `zone particles` and some `cap` lines.
- **Two starts** (the steps of `cart_return_to_title` from step 4's PRD, inlined, then input step re-init, 30 idle frames, resume, the same 1415 frames): the normalised end state equals the first start's.
- A rough demo total of 1415 frames against the 3600 limit leaves step 7 room to reach its aim of about 2700 with longer captions.

### Hypotheses

Step 4 is not done while this PRD is written. What this PRD assumes about it, from its PRD (`docs/prd/next/phase1-step4-two-attract-starts.md`):

- **H-4-1:** `cart.h` exports `void cart_return_to_title(void)`. It releases every live entity (abandon, flush, forget the registries) and then calls `cart_init`. It may only run between frames, never from a handler inside `ui_update` or `update_entities`. `record_start_return` is a thin wrapper around it. If step 4 names it otherwise, the wrapper follows the name.
- **H-4-2:** The input-step contract: at each start the whole input step (pad navigation and cheat) is what `input_step_init` gives on the **default** table, and on each return it is re-initialised on the compiled table. Neither `cart_init`, `cart_resume_game` nor `cart_return_to_title` does it; the caller does. `record_start` is that caller in the tools.
- **H-4-3:** The start and the return run at the top of a frame, between one frame's `cart_draw` and the next frame's `input_step_run`. The first demo frame is the frame that follows the resume in the same loop turn. This PRD's "frame 0 of a record-based output" is that frame.
- **H-4-4:** A start after a return needs at least one `cart_update60` in between. Otherwise `flip_zone_flip` asserts ("flip while the old generation awaits its release"). `replay`'s two-start mode idles 30 frames, and `test_record_start` idles at least one.
- **H-4-5:** Step 4 changes neither `cart_init` nor `cart_resume_game` (its Out of Scope). So the boot start this PRD relies on still works: boot's `cart_init` flips nothing, and `cart_resume_game` follows at once. If step 4 did change that, the tools would play one empty, unrecorded title frame before the start, and this PRD's contract would otherwise hold.
- **H-4-6:** Step 4's harness is unit-test code in `tests/test_cart.c`, with no entry point that takes a file. Step 5 therefore reproduces its cycle in `replay` (Implementation Decisions).
- **H-4-7:** Step 4's proof holds: two starts in one process give equal save bytes and state dumps (session lines dropped), and differ only in the visual stream and session counters. That is why the two-start check compares normalised states, not raw state files or pixels.
- **H-4-8:** Step 4 touches only `cart.{c,h}`, `tests/test_cart.c` and `test_cart`'s Makefile rule. It does not touch `pad_author`, `replay`, `replay_source`, `input_json`, `input_step` or the proofs, so the two steps' changes do not overlap.
- **H-4-9:** `bindings_compiled` is the default table in every binary this step builds (`replay`, `pad_author`, the tests), as it is today, and step 4's tests assert `!bindings_compiled_personal` the same way. Step 9's second table does not exist yet. `record_start` asserts it.

If H-4-1 is false and there is no `cart_return_to_title` (step 4 stopped under its stop rule), the two-start mode and `record_start_return` cannot be built. Chunks 1, 3 (one start only) and 4 can still land. The implementer stops before the two-start check and asks, as the plan's stop rule for step 4 requires.

### Decisions taken without a human (guideline rules)

- The tool is `replay`, taught by a header member, not a new binary: the smaller change, and `make state`/`make replay` follow for free.
- No title frame before a tool's start (H-4-5): the smaller change, and investigation showed the game state does not depend on it.
- Storage is erased, not seeded with the record, for a resume start.
- `REPLAY_STARTS` accepts only 1 or 2: the plan needs two starts, nothing more.
- Record paths are relative to the script, like `include`.
