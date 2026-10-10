---
title: "Fixtures in replay: a replay names its bindings file, personal recordings are refused"
description: "The replay tool always plays with the compiled default bindings, so no e2e replay can prove that a remapped table drives the game, and a recording made with a personal bindings.cfg would replay silently wrong."
status: needs-triage
---

## Problem Statement

Plan 17 step 16 (Phase 4, replays that prove the remap). The step's text, from `docs/plan-17.md` section 3, with its amendment after phase 2:

> 16. **Fixtures in `replay`.** `replay` reads the header member `"bindings": "<file>.cfg"` from the replay's directory with the parser of step 6, and refuses `"bindings": "personal"` with a clear message. The Makefile makes each named fixture a prerequisite of its replay's frames. Verified by unit tests (fixture found, fixture missing, personal refused) and by `make e2e` unchanged. Also inventory every place that loads `tests/e2e/*.json` (`git grep tests/e2e`, `opendir`, the callers of `replay_source_open`: `replay.c`, `game.c`, `test_live_slots`, `test_input_step`, `test_resume_check`, `test_marker_coherence`, `test_recording_continuity`) and record in the note, for each, whether it honours `"bindings"`: the in-process e2e replayers read the fixture too, or skip fixture replays explicitly. The note says where the `personal` refusal lives with respect to `game.c`'s out-of-scope replay-then-play path, which must not change. The fixture prerequisites run no command per replay at Makefile parse time, or the note measures what that costs. Timings of `make test` and `make e2e` over alternating runs against the step's base HEAD go in the note.

Section 2's locked decisions this step implements:

- "A replay may name a fixture. The header member `"bindings": "swap_aim_menu.cfg"` names a file in the replay's directory. `replay` reads it at runtime with the same parser module. Without the member, the replay uses the compiled default. The fixture is a Makefile prerequisite of its replay's frames, so editing it replays the file. Fixtures are frozen test data, never a player's file."
- "Personal recordings are flagged. […] `replay` refuses such a recording with a clear message." (Step 17 writes the flag; this step refuses it.)
- Rejected: inline bindings in the header, mixing the fixture's bytes into the frames hash, compiling every e2e fixture into `replay`.

Today, after steps 1–14:

- `tmp/replay` hands `&bindings_compiled` (the default unit) to `input_step_init` unconditionally. A header member `"bindings"` is ignored: the loaders find their members by `strstr` before `"frames":` and skip anything they do not know.
- Steps 18 and 19 need a replay that runs a swapped table and one that runs a table with an unbound action. Without this step they would replay with the default and the test would prove nothing.
- In-process replayers (`test_live_slots`, `test_resume_check`) also hand `&bindings_compiled` to `input_step_init`. Once a fixture replay exists in `tests/e2e/`, they would play it under the wrong table and pass or fail for the wrong reason.
- Nothing stops a recording made with a personal `bindings.cfg` (step 17) from replaying under the default table, giving a game that never happened.

## Solution

- A recording's header may hold `"bindings": "<name>.cfg"`. `<name>.cfg` is a plain file name, resolved in the recording's own directory. `replay` reads that file and parses it with `bindings_parse` (step 6). The table drives the run instead of the compiled default. Without the member, nothing changes: the compiled default drives the run, and every existing `.sha256` stays byte-identical.
- `"bindings": "personal"` makes `replay` stop before the first frame with a message naming the recording and saying it was recorded with a personal `bindings.cfg`. It exits 1. A missing, unreadable or invalid fixture, or a malformed member, also stops it with a clear message and exit 1. An invalid fixture prints the parser's `file:line: message`.
- The reading and checking live in one function of the `replay_source` module. It is called by `replay` and by the two in-process e2e replayers (`test_live_slots`, `test_resume_check`), so every replayer that can meet a fixture replay plays it with its fixture. `replay_source_open`, which `game.c`'s replay-then-play path calls, is not changed. `game.c` does not call the new function, so that out-of-scope path behaves exactly as before.
- Make learns the fixture the way it learns a header from the compiler: when `replay` writes a capture for `make e2e`, it also writes a small dependency file next to the capture, listing the fixture as a prerequisite of that capture. The Makefile includes those files with `-include $(wildcard …)`. No command runs per replay at parse time. Today no e2e names a fixture, so no dependency file exists and `make e2e` reads nothing more.
- The step note records the inventory of every e2e loader and whether it honours `"bindings"`, where the `personal` refusal lives, and timings against the step's base HEAD.

## Hypotheses on step 15's outcome

Step 15 ("Keymap compiled in", PRD `docs/prd/triage/phase3-step15-keymap-compiled-in.md`) is being implemented in parallel. This PRD was written from `HEAD` d079f76, where step 14 is done and step 15 is not. Step 15's scope, as handed over: `src/platform/keymap.*`, `src/cmd/keymap_generate.c`, `src/cmd/game.c`, the Makefile (keymap stamp and units, `-include`), `tests/test_keymap.c`, `.gitignore`.

- **H-15-1. No shared source file.** Step 15 does not touch `src/cmd/replay.c`, `src/devtools/replay_source.{c,h}`, `tests/test_replay_source.c`, `tests/test_headless_capture.c`, `tests/test_live_slots.c` or `tests/test_resume_check.c`. This step does not touch `src/cmd/game.c`, `src/platform/` or `tests/test_keymap.c`.
- **H-15-2. Separate Makefile neighbourhoods.** Step 15 changes the `make raylib ->` legend entry, the `test_keymap` rule and the bindings generator block after it, the `raylib` block near the end, and the `-include` lines at the very end. This step changes the `make e2e ->` and `make replay ->` legend entries, the `test_headless_capture` rule's prerequisites, and the e2e block (the `$(FRAMES_DIR)/%.bin` rule and one new `-include` line right after it, not at the end of the file). No hunk overlaps. The `test_replay_source` rule, which sits right after the bindings block, is not touched by this step.
- **H-15-3. `game.c`'s replay lines unchanged by step 15.** Step 15 rewrites only `game.c`'s keymap loading. Its `replay_source_open` call and the replay-then-play loop are as in d079f76. The "must not change" check of this step is `git diff <this step's base HEAD> -- src/cmd/game.c src/devtools/replay_source.c` showing no change to `replay_source_open` and no call to the new function in `game.c`.
- **H-15-4. Link lines unchanged by step 15.** `REPLAY_SRCS` and the `test_live_slots`, `test_resume_check`, `test_replay_source` and `test_headless_capture` link lines are as in d079f76. The keymap is linked only into `tmp/game` and `test_keymap`.
- **H-15-5. Timing base.** "The step's base HEAD" is the commit this step starts from, which includes step 15. Timings are compared against that commit, not against d079f76.

**Rule if a hypothesis is wrong.** Follow the committed code at this step's own base HEAD. If a Makefile hunk conflicts, keep step 15's lines and reapply this step's change around them. If step 15 changed `game.c`'s replay path after all, this step still leaves it alone, and the note records the difference.

## Starting point (checked at d079f76)

- `src/cmd/replay.c`: `replay <events.json> [<capture.bin> [<dump.txt>]]`. It opens the recording with `replay_source_open` (an `assert` on failure), seeds the storage, runs `cart_load`/`cart_init`, opens the capture, then calls `input_step_init(&step, &bindings_compiled)`. Optional outputs are driven by environment variables: `STATE`, `PAD_METRICS`, `REPLAY_FILL`, `SAVE_HEX`. Its top comment documents them. Capture write failures exit 1.
- `src/devtools/replay_source.{c,h}`: `replay_source_open` reads the file once to parse `"initial_save"` from the header (the text before the first `"frames":`), then calls `p8_input_load`. A malformed `initial_save` fails the load. It has its own `read_whole_file`. Other header members are ignored. It does not include `game/bindings.h`.
- `src/devtools/input_json.c`: `p8_input_load` checks `"input": "pad"` before `"frames":` and parses frames. Unknown header members are ignored. `p8_input_append` can write one header member (step 17 will use it).
- `src/game/bindings.h`: `bindings_parse(file_name, text, out, error)` is pure. `bindings_error_t.text` is the whole `file:line: message`. `bindings_compiled` is named only in `src/cmd/`, `pad_author.c` and tests. `bindings_generate` refuses inputs over 65536 bytes.
- Makefile: `$(FRAMES_DIR)/%.bin: % $(TMP_DIR)/replay` runs `./$(TMP_DIR)/replay $< $@ $(@:.bin=.dump.txt)`. `.DELETE_ON_ERROR` is set. Object `.d` files are included at the end with `-include $(wildcard …)`. `test_live_slots` (ASan, outside `make test`, run by `make sanitize-net` and `make unit-run-test_live_slots`) and `test_resume_check` (`make resume-check`) link `src/devtools/replay_source.c` and `$(ENGINE_SRCS)` (which holds `src/game/bindings.c`). `test_replay_source` links `src/game/bindings.c` explicitly. `test_headless_capture` runs `./tmp/replay` through `system()`.
- `make test` 4.64 s and `make e2e` 0.25 s at the end of phase 2 (step 2.14 note).

## User Stories

1. As the developer writing step 18's swap twin, I want a replay to name a bindings fixture in its header, so that the twin runs under a swapped table and proves the remap reaches the game.
2. As the developer writing step 19's unbound-action replay, I want the fixture to be parsed with the same rules as `bindings.default.cfg`, so that a missing line means "unbound" in the replay exactly as in the game.
3. As the developer, I want the fixture resolved in the replay's own directory, so that `tests/e2e/x.json` naming `swap.cfg` reads `tests/e2e/swap.cfg` whatever the working directory.
4. As the developer, I want a replay without the member to run with the compiled default, so that every existing `.sha256` stays byte-identical.
5. As the developer, I want `replay` to refuse `"bindings": "personal"` before the first frame with a message saying the recording used a personal `bindings.cfg`, so that a game played with personal controls is never mistaken for a default-bound replay.
6. As the developer, I want a missing fixture to stop `replay` with a message naming the resolved path and the recording, so that a typo or a forgotten `git add` is obvious.
7. As the developer, I want an invalid fixture to stop `replay` with the parser's `file:line: message`, so that I fix the fixture as I would fix `bindings.default.cfg`.
8. As the developer, I want a member that is not a plain `<name>.cfg` file name (a path, `..`, another extension, a non-string, a second `"bindings"` member) to be refused, so that a fixture can only be a file next to its replay.
9. As the developer, I want `bindings.cfg` refused as a fixture name, so that a replay can never read a player's personal file.
10. As the developer, I want every refusal to exit 1 before the capture file is opened, so that `make e2e` never hashes a half-run capture.
11. As the developer, I want editing a fixture to make `make e2e` replay its replay, and only that one, so that a fixture change is never hidden by an up-to-date capture.
12. As the developer, I want deleting a fixture that a replay still names to make `make e2e` rerun that replay and fail with the missing-fixture message, not with make's "no rule to make target", so that the error points at the real cause.
13. As the developer, I want the fixture bytes kept out of the frames hash, so that a comment edit in a fixture turns nothing red.
14. As the developer, I want no command run per replay when the Makefile is parsed, so that `make test` and `make e2e` do not get slower as replays are added.
15. As the developer, I want no dependency file written for a replay without a fixture, and a stale one removed, so that today's `make e2e` includes nothing new.
16. As the developer, I want `make state`, `make end-state`, `make sanitize-net`, `make coverage`, `make pad-metrics`, `make full-game` and `make replay` to honour fixtures with no change of their own, so that every tool built on `replay` sees the same game as `make e2e`.
17. As the developer, I want `test_live_slots` to play each e2e with its fixture, so that its invariants are checked on the game the replay really plays.
18. As the developer, I want `make resume-check` to play a recording with its fixture, so that a resume check on a fixture replay compares the right games.
19. As the developer, I want `game.c`'s replay-then-play path left exactly as it is, so that this step does not grow scope the plan put out of it.
20. As the developer, I want a personal recording to still open through `replay_source_open`, so that the refusal lives in the replay tool and not in the shared loader `game.c` uses.
21. As the developer, I want the step note to list every loader of `tests/e2e/*.json` and whether it honours `"bindings"`, so that no in-process replayer silently ignores a fixture.
22. As the developer, I want `test_rng_sync`'s reasons for not reading fixtures written down, so that its exception is a recorded decision.
23. As the developer, I want the fixture function to take the fallback table as a parameter, so that it never names `bindings_compiled` and its unit tests use any table.
24. As the developer, I want a driver-level test proving that a fixture identical to the default gives the same capture as no fixture, and that a changed fixture gives a different one, so that I know the fixture really reaches `input_step`.
25. As the developer, I want the `make e2e ->` and `make replay ->` legend entries to mention fixtures and the personal refusal, so that the Makefile help stays true.
26. As the developer, I want `replay.c`'s top comment to document `REPLAY_DEPS` and the `"bindings"` member, so that the environment contract is next to the others.
27. As the developer, I want `make test` and `make e2e` timed over alternating runs against the step's base HEAD, so that "not slower" is measured, not assumed.
28. As a player, I want nothing to change in how the game plays or records, so that this step is invisible to me.

## Implementation Decisions

### Header member

- Key `"bindings"`, searched only in the header (before the first `"frames":`), like `initial_save`. The value must be a JSON string with no escape: optional blanks after the colon, then `"`, then the characters up to the next `"`.
- Accepted values:
  - `personal`: the personal flag of step 17. Always refused by the replayers.
  - A fixture name: 1 to 64 characters from `[A-Za-z0-9_.-]`, not starting with `.`, ending in `.cfg`, and not `bindings.cfg`. This charset excludes `/`, `\`, quotes, spaces, colons and `$`, so the name is a file in the replay's directory and is safe to write into a make dependency file.
- Refused as a bad member: a non-string value, an empty string, a backslash or any character outside the charset, another extension, a name longer than 64, `bindings.cfg`, or a second `"bindings":` before `"frames":`.
- Resolution: the directory part of the recording's path (everything up to and including its last `/`, or nothing when there is none) followed by the name. A resolved path that does not fit the module's buffer (1024 bytes) is refused as a bad member.

### `replay_source` module (`src/devtools/replay_source.{c,h}`)

- `replay_source_open`, `replay_source_seed_storage` and the struct are unchanged. `game.c` keeps calling only those.
- New function, called after a successful open: `replay_source_read_bindings(source, fallback, out, report)`.
  - `source` must be `REPLAY_SOURCE_LOADED` (an `assert` with a message otherwise).
  - It re-reads the recording file to find the member. The open does not keep the text, and re-reading leaves the code `game.c` runs untouched. The cost is one more read of a file of at most a few tens of kB per replay.
  - `fallback` is the table to use without the member. Callers pass `&bindings_compiled`. The module never names `bindings_compiled`.
  - Returns a status enum with the prefix `REPLAY_SOURCE_BINDINGS_`: `DEFAULT` (no member, `*out` = `*fallback`), `FIXTURE` (read and parsed, `*out` is the fixture's table), `PERSONAL`, `BAD_MEMBER`, `MISSING` (the resolved file cannot be opened or read), `INVALID` (`bindings_parse` failed, or the file is larger than 65536 bytes, the generator's limit). On any failure status, `*out` is unusable.
  - `report` is a small struct with two fixed buffers: the resolved fixture path (empty unless the member names a fixture, filled for `FIXTURE`, `MISSING` and `INVALID`), and a ready-to-print message (empty for `DEFAULT` and `FIXTURE`). No allocation is visible to the caller. The fixture text buffer is static inside the function, since devtools are single-threaded.
- Messages, which the unit tests pin:
  - `PERSONAL`: `<recording>: recorded with a personal bindings.cfg ("bindings": "personal"); replay runs only the default bindings or a fixture next to the recording`
  - `BAD_MEMBER`: `<recording>: bad "bindings" member: expected "personal" or a file name ending in .cfg in the recording's directory`
  - `MISSING`: `<fixture path>: cannot read the bindings fixture named by <recording>`
  - `INVALID`: the parser's `error.text` verbatim, with the resolved fixture path as `file_name`. For an oversized file: `<fixture path>: larger than 65536 bytes`.
- `replay_source.c` now includes `game/bindings.h`. Every binary linking it already links `src/game/bindings.c` (`$(ENGINE_SRCS)` or explicitly for `test_replay_source`), so no link line changes.
- The header contract comment states that `fallback` is copied on `DEFAULT`, that `*out` is unusable on failure, and that the report's buffers are filled as listed. Nothing more, per `CLAUDE.md`.

### `replay` (`src/cmd/replay.c`)

- Right after `replay_source_open` and `replay_source_seed_storage`, and before `cart_load`, call `replay_source_read_bindings(&source, &bindings_compiled, &table, &report)`. On any status other than `DEFAULT` or `FIXTURE`, print `replay: <message>` to stderr and return 1. No capture file is opened, and the dump and the optional outputs are not written.
- `input_step_init(&step, &table)` replaces `&bindings_compiled`. `pad_metrics_observe` keeps observing `step`. Pad metrics judge buttons (step 2.8), so they need no change.
- New environment variable `REPLAY_DEPS=<file>`, documented in the top comment beside the others:
  - It requires a capture path (argv[2]), which is the make target. Without one, print a usage message and return 2.
  - With a fixture: write `<capture>: <fixture path>` then `<fixture path>:` (the empty rule makes a deleted fixture force a rerun instead of a make error, like gcc's `-MP`). Each line ends in a newline. The file is written right after the bindings are resolved, before the capture is opened. A write failure prints `replay: cannot write the dependency file '<file>'` and returns 1.
  - Without a fixture: remove `<file>` if it exists (a missing file is not an error), so a replay that drops its member leaves no stale prerequisite.
- The usage line and every other behaviour are unchanged.

### Makefile

- The frames rule's recipe becomes `REPLAY_DEPS=$(@:.bin=.d) ./$(TMP_DIR)/replay $< $@ $(@:.bin=.dump.txt) > /dev/null`. The dependency file sits next to the capture: `tmp/frames/tests/e2e/<name>.json.d`.
- Right after the frames rule: `-include $(wildcard $(FRAMES_DIR)/$(E2E_DIR)/*.json.d)`. This is one glob inside make, no shell command. Today it matches nothing. With step 18 and 19 it will match one or two tiny files.
- The fixture bytes never enter the hash: `frames_hash` hashes only the capture.
- No other replay target changes. `state-run-*`, `sanitize-run-*`, `end-state`, `coverage`, `full-game`, `pad-metrics` and `make replay` call a binary built from `src/cmd/replay.c`, which reads the fixture by itself. Those targets are phony or always rerun, so they need no prerequisite.
- `test_headless_capture` gains the prerequisites `tests/e2e/pad_title.json` and `bindings.default.cfg`, which its new tests read.
- Legend: `make e2e ->` gains one sentence: a replay whose header names a fixture (`"bindings": "<file>.cfg"`, read from its directory) reruns when that fixture changes, through the `.d` file `replay` writes next to the capture. `make replay ->` gains: refuses a recording whose header says `"bindings": "personal"`.

### In-process replayers

- `tests/test_live_slots.c`, `load_e2e`: before closing its `replay_source`, call `replay_source_read_bindings` with `&bindings_compiled`, assert the status is `DEFAULT` or `FIXTURE` (with a message), and pass the result to `input_step_init`. `input_step_init` currently runs after the source is closed: move the read so the table is resolved while the source is open, and keep the init order relative to `cart_load`/`cart_init` as it is today. This covers every e2e it walks with `opendir` (twice), every named e2e and its `tests/fixtures/` paths.
- `tests/test_resume_check.c`, `boot`: the same, so `make resume-check` honours fixtures on both sides.

### Expected loader inventory (the implementer checks it against `git grep -n tests/e2e`, `opendir` and the callers of `replay_source_open`/`p8_input_load`, and records the final table in the step note)

| Loader | Loads `tests/e2e/*.json` how | Honours `"bindings"` |
|---|---|---|
| `src/cmd/replay.c` (`tmp/replay`, `replay_sanitize`, `replay_coverage`) | the file given; drives `make e2e`, `make replay`, `state`, `end-state`, `sanitize-net`, `coverage`, `pad-metrics`, `full-game`, and the `system()` calls of `test_state`, `test_live_slots`, `test_headless_capture` | yes, refuses `personal` |
| `src/cmd/game.c` | replay-then-play, `replay_source_open` only | no, out of scope, unchanged; a `personal` recording still loads there with whatever table `game` compiled |
| `tests/test_live_slots.c` | `opendir("tests/e2e")` twice, named e2e, `tests/fixtures/` paths | yes (this step) |
| `tests/test_resume_check.c` | any `EVENTS` of `make resume-check` | yes (this step) |
| `tests/test_input_step.c`, `test_marker_coherence.c`, `test_recording_continuity.c` | their own temporary recordings, written by the test, with no member | not applicable: they load no e2e |
| `tests/test_rng_sync.c` | `tests/e2e/pad_colonise.json` only, by `p8_input_load` | no, by decision: `pad_colonise.json` is regenerated by the generator (`make pad-metrics-check`), which only writes default-bound replays (section 2), so it can never name a fixture |
| `tests/test_state.c` | runs `./tmp/replay` (honours); `p8_input_load` on `pad_research` and `pad_build` only to count frames | yes, through `replay`; the frame counts do not depend on bindings |
| `tests/test_demo.c`, `test_headless_capture.c`, `test_input.c`, `test_replay_source.c` | `tests/fixtures/` or temporary files only | not applicable |
| `tools/e2e_limits.sh` | text checks (frame count, `.txt`, `.sha256`, `"input": "pad"`, no mouse field) | not applicable: no replay |
| `tools/frames_ref.sh` (`make e2e-ref`) | the replay driver built at the commit that set the reference, in a worktree of that commit | yes for any fixture replay: its reference is set at or after this step, so that commit's driver reads the fixture from the worktree |
| `src/devtools/pad_author.c` (`pad-author-proof`) | writes replays and compares them with `cmp` | not applicable: generated replays are default-bound, and a fixture replay has no `.pad` proof |

### Where the `personal` refusal lives

In `replay_source_read_bindings`, called by `replay` and the two in-process replayers. Not in `replay_source_open`. `game.c` calls only `replay_source_open`, so its replay-then-play path is unchanged, as section 2's Out of scope requires. The note states this and cites the H-15-3 diff check.

## Implementation Chunks

1. **Fixture reading in `replay_source`.** `replay_source_read_bindings`, its status enum and report, member parsing, name checks, path resolution, file read, parse, messages. Unit tests in `tests/test_replay_source.c` (see Testing Decisions). Nothing calls it yet. `make test` green, every `.sha256` unchanged.
2. **`replay` reads the fixture, Make learns it.** `replay.c` wiring, the personal and failure exits, `REPLAY_DEPS`, the top comment. The frames recipe, the `-include` line, the legend entries and the `test_headless_capture` prerequisites. Driver tests in `tests/test_headless_capture.c`. Then the one-off Makefile checks of Testing Decisions.
3. **In-process replayers.** `test_live_slots` and `test_resume_check` call the function. `make unit-run-test_live_slots` and one `make resume-check` run stay green. Then the inventory, the refusal location and the timings go in the note.

## Testing Decisions

- Good tests here drive public interfaces only: `replay_source_read_bindings` on real files written to `tmp/`, and the `replay` binary through `system()` as `test_headless_capture` already does. They pin statuses, messages, tables and files on disk, never internal helpers.
- **`tests/test_replay_source.c`** (module `replay_source`; prior art: its own tests that write recordings to `tmp/` with `p8_input_save` or inline JSON). Each test writes a recording and, when needed, a fixture into a fresh directory `tmp/test_replay_source_bindings/`. The fallback is a local table distinct from any fixture used.
  1. No member: `DEFAULT`, `*out` equals the fallback, path and message empty.
  2. Fixture found: the recording in the subdirectory names `swap.cfg`, and the file exists only there. Status `FIXTURE`, `*out` equals `bindings_parse` of the same text and differs from the fallback, the report's path is `tmp/test_replay_source_bindings/swap.cfg`. This proves resolution is against the recording's directory, not the working directory.
  3. Fixture missing: `MISSING`, with the exact message naming the resolved path and the recording.
  4. Personal refused: `PERSONAL`, with the exact message. The same file still opens with `replay_source_open` as `REPLAY_SOURCE_LOADED`, which pins that the shared loader `game.c` uses is unchanged.
  5. Invalid fixture (for example a conflict on line 2): `INVALID`, and the message is `tmp/test_replay_source_bindings/bad.cfg:2: button 7 already bound in mode jump`. An oversized fixture (65537 bytes of comments) gives the `larger than` message.
  6. Bad members, one table-driven test: `"sub/x.cfg"`, `"../x.cfg"`, `"x.txt"`, `""`, `".cfg"`, `"bindings.cfg"`, a 65-character name, `"a\"b.cfg"` written with an escape, `3`, and two `"bindings"` members. Each gives `BAD_MEMBER` with the exact message.
  7. No test for a `"bindings"` text after `"frames":`. A loadable recording cannot hold one, because `p8_input_load` accepts only `{ "pad": [...] }` frames. The search stops at `"frames":` like `initial_save`'s.
- **`tests/test_headless_capture.c`** (the `replay` driver, run for real; prior art: its `SAVE_HEX`, bad capture path and dump redirection tests). Each run is a sub-second replay, and the file runs in parallel with the other units.
  1. Same game with a default-identical fixture: copy `tests/e2e/pad_title.json` into `tmp/test_headless_capture_fixture/` with the header line `"bindings": "same.cfg"` inserted after `"input": "pad",`, and copy `bindings.default.cfg` there as `same.cfg`. The capture equals the capture of the original `tests/e2e/pad_title.json`.
  2. The fixture reaches the game: a fixture that is the default with `title.choose_row = 5` changed to `title.choose_row = 8` gives a different capture from the original. No game starts, since `pad_title` chooses the row with button 5.
  3. Personal refused: a recording with `"bindings": "personal"` exits with status 1. Stderr holds `recorded with a personal bindings.cfg`, and no capture file is created at the given path.
  4. Missing fixture: exit status 1, and stderr names the resolved path.
  5. `REPLAY_DEPS` with a fixture: the file holds exactly `<capture>: <dir>/same.cfg\n<dir>/same.cfg:\n`. Without a fixture, a pre-existing dependency file at that path is removed. `REPLAY_DEPS` without a capture path exits with status 2.
- **No new unit test for `test_live_slots`/`test_resume_check`**: they are test harnesses. Their change is covered by `make unit-run-test_live_slots` (all e2e still pass through the new call) and one `make resume-check EVENTS=tests/e2e/pad_restart_resume.json BEFORE=111 AFTER=132` printing `ok`. Step 18 records whether a wrong fixture turns `test_live_slots` red.
- **One-off checks, recorded in the step note, not committed:**
  1. With a scratch e2e `tests/e2e/zz_fixture.json` (a copy of `pad_title.json` naming a default-identical `zz_fixture.cfg`): `make tmp/frames/tests/e2e/zz_fixture.json.sha256` writes `zz_fixture.json.d`. A second `make` of that target does nothing. `touch tests/e2e/zz_fixture.cfg` makes `make -n e2e` list only that replay. Deleting the `.cfg` makes `make` rerun it and fail with the missing-fixture message, not "no rule to make target". Removing the member from the JSON reruns it and removes the `.d`. Remove every scratch file afterwards.
  2. After a clean `make e2e`, `ls tmp/frames/tests/e2e/*.d` finds nothing.
  3. The H-15-3 diff: `game.c` and `replay_source_open` unchanged by this step.
- **Guardrails:** `make test-all` green with every `tests/e2e/*.json.sha256` unchanged, and `make end-state` unchanged. `make sanitize-net` (ASan replays plus `test_live_slots`) clean. `make raylib` links if raylib is vendored (`game` links `replay_source.c`, now including `bindings.h`). This step touches no `src/game/pad_*`, `src/devtools/pad_*`, `input_json.c` or button constant, so the generator and metrics commands are not required. Running `make pad-metrics-check` once costs little and is recommended, since `replay` changed.
- **Timings:** `make test` and `make e2e` on a warm build, at least five alternating runs of this step's HEAD and its base HEAD (a `git worktree` of the base, built once) on the same machine. Means go in the note. Neither may be slower beyond noise.

## Out of Scope

- `game.c`'s replay-then-play path, including what it does with `"bindings"` or `"personal"` (section 2, Out of scope).
- Writing `"bindings": "personal"` into recordings: step 17.
- Any fixture or fixture replay in `tests/e2e/`: steps 18 and 19. This step commits no `.cfg` under `tests/` (unit and driver tests write theirs to `tmp/`).
- `e2e_limits.sh` checking that a named fixture exists: `make e2e` already fails through `replay` when it does not.
- The generator reading fixtures, and the generator writing `"bindings"` (section 2, Rejected).
- Sharing the three `read_whole_file` copies (`replay_source.c`, `input_json.c`, `bindings_generate.c`'s reader): not this step's concern. The new function reuses `replay_source.c`'s own copy.
- `test_rng_sync` reading fixtures (decision in the inventory).

## Further Notes

- Why a dependency file written by `replay` rather than a Makefile-side scan: a `$(shell grep …)` per replay, or even one over all replays, would run on every `make test`. A generated include rule would force make to re-exec whenever a JSON changes. The `.d` file is the pattern the Makefile already uses for C headers. It costs nothing until a fixture exists, and then only the read of one tiny file. The note may say this in one line instead of measuring a parse-time command, since none is added.
- `.DELETE_ON_ERROR` already removes a capture whose replay failed. A refused or broken fixture leaves no capture, so the next `make e2e` retries.
- The `bindings.cfg` name ban is the cheap enforcement of "fixtures are never a player's file" for a recording that sits at the repository root.
- Section 2's example name `swap_aim_menu.cfg` fits the charset. Step 18 uses it as is.
