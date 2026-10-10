---
title: "Personal recordings flagged: a game built with bindings.cfg marks every recording it writes"
description: "A recording made by a game compiled with a personal bindings.cfg looks exactly like a default-bound one, so replay would play it under the default table and show a game that never happened."
status: needs-triage
---

## Problem Statement

Plan 17 step 17 (Phase 4, replays that prove the remap). The step's text, from `docs/plan-17.md` section 3, with its amendment after phase 2:

> 17. **Personal recordings flagged.** A `game` built with a personal `bindings.cfg` writes `"bindings": "personal"` through the recorder's header member. The flag comes from the generated header. Verified by a unit test on the recorder with both header variants, and by checking that the `game` binary built with a personal file carries the flag and the one built without it does not. Per step 2.7's note there is no generated header: the flag is a symbol of the generated unit, read only in `src/cmd/game.c`, which hands it to the recorder as an input (as `session_recorder_set_initial_save` does). The recorder names no generated symbol, so its unit test sets both variants directly. `p8_input_append` takes a single header member, so the recorder writes `initial_save` and `bindings` together.

Section 2's locked decision this step implements:

- "Personal recordings are flagged. A `game` built with a personal `bindings.cfg` writes `"bindings": "personal"` in every recording's header. `replay` refuses such a recording with a clear message. A personal `keymap.cfg` needs no flag: recordings store buttons, which come after the keymap." Step 16 implements the refusal; this step writes the flag.
- Rejected: "Copying the personal file next to each recording".
- Out of scope: `game.c`'s replay-then-play path.

Today, at HEAD 67a88d5 (steps 1–15 done):

- `tmp/game` links `tmp/bindings/game.c`, generated from `bindings.cfg` when it exists, else from `bindings.default.cfg`. Nothing in the generated unit or in the binary says which file it came from: only `make raylib`'s console line does, and it is gone once the game runs.
- Every session writes `tmp/events_recorded_<time>.json` through `session_recorder`. The header holds `"input": "pad"` and, when the session started on a save, `"initial_save"`. A recording made with personal controls is indistinguishable from a default-bound one.
- Once step 16 lands, `replay` refuses `"bindings": "personal"`, but no recording ever carries it, so a personal recording replays silently under the default table.

## Solution

- The bindings generator gains an option that marks the unit it writes as personal. Every generated unit now defines, beside `bindings_compiled`, a flag `bindings_compiled_personal`: 1 in the unit generated from a personal `bindings.cfg`, 0 everywhere else. The Makefile passes the option exactly when it compiles `bindings.cfg` into `game`, under the same condition that makes `make raylib` print `(personal)`.
- `game.c` is the only code that reads the flag. Right after it hands the startup save to the recorder, it hands the flag too, through a new recorder setter.
- The recorder knows nothing of bindings files or generated symbols: it takes a yes/no input before the first frame. When it is set, the recording's header holds `"bindings": "personal"`. When it is not, the file is byte-identical to what the recorder writes today.
- `p8_input_append` keeps its single header-member parameter. The recorder joins the members it has (`bindings`, then `initial_save`) into one text and passes that.
- A player who builds with a personal `bindings.cfg` keeps playing and recording as before. Their recordings now say how they were made, and `replay` (step 16) refuses them with a clear message instead of replaying a different game.

## Hypotheses on step 16's outcome

Step 16 ("Fixtures in replay", PRD `docs/prd/triage/phase4-step16-fixtures-in-replay.md`) is being implemented in parallel. This PRD was written from HEAD 67a88d5, where step 15 is done and step 16 is not. Everything below about step 16 comes from its PRD, not from committed code.

- **H-16-1. Spelling of the flag.** Step 16 refuses exactly the header member `"bindings": "personal"`: the key `"bindings"`, a colon, optional blanks, and the JSON string `personal` with no escape. This step writes `"bindings": "personal"` (one space after the colon, as every other header member).
- **H-16-2. Where the member is looked for.** Step 16's parser looks for `"bindings"` only in the header (before the first `"frames":`), in any position among the other header members, and accepts it next to an `"initial_save"` member. It refuses a second `"bindings":` before `"frames":`; this step writes the member once.
- **H-16-3. Where the parser lives and its interface.** The member parser is in the `replay_source` module, behind `replay_source_read_bindings(source, fallback, out, report)`, called after a successful `replay_source_open`. It returns a status of the `REPLAY_SOURCE_BINDINGS_` family, with `PERSONAL` for this member and `DEFAULT` when there is no member, and its report message holds `recorded with a personal bindings.cfg`.
- **H-16-4. The shared loader is unchanged.** `replay_source_open` still loads a recording that holds `"bindings": "personal"` (status `REPLAY_SOURCE_LOADED`), and still parses its `initial_save` whatever the member order. `game.c`'s replay-then-play path therefore still opens personal recordings, as section 2's Out of scope requires.
- **H-16-5. No shared source file.** Step 16 does not touch `src/devtools/session_recorder.{c,h}`, `src/cmd/game.c`, `src/cmd/bindings_generate.c`, `src/game/bindings.h`, `src/p8/input.h`, `tests/test_recording_continuity.c` or `bindings.default.cfg`. This step does not touch `src/cmd/replay.c`, `src/devtools/replay_source.{c,h}`, `tests/test_replay_source.c`, `tests/test_headless_capture.c`, `tests/test_live_slots.c` or `tests/test_resume_check.c`.
- **H-16-6. Separate Makefile neighbourhoods.** Step 16 changes the `make e2e ->` and `make replay ->` legend entries, the `test_headless_capture` rule and the e2e frames block. This step changes the `make raylib ->` legend entry and the `$(BINDINGS_GAME_UNIT)` recipe near the end of the file (plus one variable next to `BINDINGS_CHOSEN`). The `test_recording_continuity` link line stays as it is at HEAD: it already links `src/devtools/replay_source.c` and `src/game/bindings.c`, which is all the round-trip test needs, provided step 16 adds no new source file to `replay_source`'s dependencies.
- **H-16-7. Order.** Step 16 is committed before this step starts (plan order). "This step's base HEAD" includes it, and timings compare against that commit.

**Rule if a hypothesis is wrong.** Follow the committed code at this step's base HEAD. If step 16 refuses a different spelling, this step writes the spelling step 16's committed code refuses, and the step note records the difference from section 2. If the reader's name, statuses or message differ, the round-trip test (Testing Decisions, test 6) uses what is committed. If step 16 is not committed when this step starts, implement everything except that round-trip test and record it in the note as owed. If a Makefile hunk conflicts, keep step 16's lines and reapply this step's change around them. If `test_recording_continuity` needs another source file to link the reader, add it to that one link line.

## Starting point (checked at 67a88d5)

- `src/cmd/bindings_generate.c`: `bindings_generate <input.cfg> <output.c>`, exactly two arguments, else usage on stderr and exit 2. It parses with `bindings_parse`, writes `const bindings_t bindings_compiled = …` to a temporary file and renames it. Input limit 65536 bytes.
- Makefile: `$(BINDINGS_DEFAULT_UNIT)` (`tmp/bindings/default.c`) is generated from `bindings.default.cfg` and linked into `replay`, `pad_author` and the tests. `$(BINDINGS_GAME_UNIT)` (`tmp/bindings/game.c`) is generated from `BINDINGS_CHOSEN` (`bindings.cfg` if it exists, else `bindings.default.cfg`), depends on the chosen file, the stamp `tmp/bindings.source` and the generator, and is linked into `tmp/game` only. `make raylib` prints `bindings: $(BINDINGS_CHOSEN)` plus ` (personal)` when `bindings.cfg` exists.
- `src/game/bindings.h` declares `extern const bindings_t bindings_compiled;` with a comment saying which unit defines it. `bindings_compiled` is named in `src/cmd/game.c`, `src/cmd/replay.c`, `src/devtools/pad_author.c`, the generator's output string and tests.
- `src/devtools/session_recorder.{c,h}`: `session_recorder_t { path, frame_count, tail_offset, initial_save[], initial_save_count }`. `session_recorder_open` and `session_recorder_close` reset the count, offset and save count. `session_recorder_set_initial_save` asserts it runs before the first frame. `session_recorder_record_frame` builds the `"initial_save": "<hex>"` member on the first frame only when a save was set, and passes it (or NULL) to `p8_input_append`. A NULL member keeps the file byte-identical to `p8_input_save`'s.
- `src/p8/input.h` documents `p8_input_append`'s `header_member` as "one JSON member such as `"key": "value"`, without indent or comma, written after "input" in a fresh file only; NULL for none". `input_json.c` writes it as `JSON_HEADER` + two-space indent + member + `,\n` + `  "frames": [\n`.
- `src/cmd/game.c`: `raylib_backend_init` opens the recorder on a fresh `tmp/events_recorded_<time>.json`. `game.c` then reads the startup save and calls `session_recorder_set_initial_save(&backend.recorder, …)` before `cart_load`, long before the first recorded frame. One recording per run.
- `src/devtools/replay_source.c`: `parse_initial_save` finds `"initial_save":` by `strstr` before `"frames":`, so another header member before or after it does not disturb it. Unknown members are ignored by `p8_input_load`.
- The recorder's tests live in `tests/test_recording_continuity.c` (there is no `test_session_recorder.c`): the basic recording, resume, the `initial_save` round trips through `replay_source`, and the byte-identity of a recording without a save. `tests/test_marker_coherence.c` and `tests/test_input_step.c` use the recorder without a save and are not affected.
- `bindings.default.cfg`'s top comment explains the personal `bindings.cfg` override.

## User Stories

1. As a player who built the game with a personal `bindings.cfg`, I want every recording my game writes to say so, so that nobody replays it as if it had been played with the default controls.
2. As a player with the default bindings, I want my recordings to stay exactly as they are today, byte for byte, so that nothing I already do changes.
3. As a player with only a personal `keymap.cfg`, I want my recordings unflagged, so that they replay normally: they store buttons, which come after the keymap.
4. As the developer, I want `replay` (step 16) to refuse a flagged recording, so that a game played with personal controls is never mistaken for a default-bound replay.
5. As the developer, I want the flag to come from the compiled bindings unit, so that the recording states what the binary was built with, not what files happen to sit on disk while it runs.
6. As the developer, I want the flag set by the same condition that makes `make raylib` print `(personal)`, so that the console line and the recordings can never disagree.
7. As the developer, I want creating or deleting `bindings.cfg` to regenerate the flag through the existing stamp, so that a stale binary never flags wrongly.
8. As the developer, I want the default unit linked into `replay`, `pad_author` and the tests to carry the flag at 0, so that every generated unit has the same shape and the default never claims to be personal.
9. As the developer, I want the generator to refuse an unknown option with its usage line and exit 2, so that a typo in the Makefile fails loudly.
10. As the developer, I want `game.c` to be the only code that reads the flag, so that the recorder and the game core stay free of generated symbols.
11. As the developer, I want the recorder to take the flag as a plain input before the first frame, like the startup save, so that its unit test can set both variants directly.
12. As the developer, I want setting the flag after the first frame to fail an `assert` with a message, so that a late call cannot silently produce an unflagged file.
13. As the developer, I want opening or closing the recorder to clear the flag, so that a reused recorder never flags a recording it was not told to.
14. As the developer, I want the flagged header to hold `"bindings": "personal"` exactly once, before `"frames"`, so that step 16's reader finds it and refuses nothing else.
15. As the developer, I want a flagged recording that also has a startup save to carry both members in one header, so that the flag never costs the save.
16. As the developer, I want a flagged recording to still load through `replay_source_open` with its frames and save intact, so that `game.c`'s out-of-scope replay-then-play path behaves as before.
17. As the developer, I want a round-trip test from the recorder to step 16's reader, so that the writer and the refusal can never drift apart on the spelling.
18. As the developer, I want `p8_input_append`'s signature unchanged and its header comment to say what text it accepts, so that the recorder's joined members follow a written contract.
19. As the developer, I want a binary proof without launching anything, so that I know the `game` built with a personal file carries the flag at 1 and the one built without it carries 0.
20. As the developer, I want the shared objects linked into both `game` and `replay` to carry no flag, so that the flag lives only in the generated units, as `bindings_compiled` does.
21. As the developer, I want `bindings.default.cfg` to say that a personal file flags recordings and that `replay` refuses them, so that a player learns it where they learn how to override the file.
22. As the developer, I want the `make raylib ->` legend to mention the flag, so that the Makefile help stays true.
23. As the developer, I want `make test` and `make e2e` unchanged in duration and every `.sha256` unchanged, so that this step costs the loop nothing.

## Implementation Decisions

### The flag in the generated unit

- Name: `bindings_compiled_personal`, type `const int`, value 1 or 0. Declared `extern` in `src/game/bindings.h` right after `bindings_compiled`, under a comment stating the contract: 1 only in the unit generated from a personal `bindings.cfg`, which only `game` links; read only by `src/cmd/game.c`, which hands it to the recorder.
- Every generated unit defines it: the generator writes `const int bindings_compiled_personal = 0;` or `= 1;` after the table. No binary other than `game` reads it; the default unit carries 0 so the units keep one shape.
- Generator command line: `bindings_generate [--personal] <input.cfg> <output.c>`. Three arguments without the option, four with `--personal` first. Anything else (an unknown option, the option in another position, a wrong count) prints `usage: bindings_generate [--personal] <input.cfg> <output.c>` and exits 2. The option changes only the flag's value, never the table. The generated header comment line stays `Generated by bindings_generate from <path>. Do not edit.`
- The generator does not guess personal from the input's file name: the Makefile decides, as it already decides which file to compile.

### Makefile

- A variable next to `BINDINGS_CHOSEN`: `BINDINGS_PERSONAL_OPTION := $(if $(wildcard bindings.cfg),--personal)`. The `$(BINDINGS_GAME_UNIT)` recipe becomes `$(TMP_DIR)/bindings_generate $(BINDINGS_PERSONAL_OPTION) $(BINDINGS_CHOSEN) $@`. The condition is the same `$(wildcard bindings.cfg)` that `make raylib`'s `(personal)` line uses.
- No new prerequisite: the flag changes exactly when the chosen file changes, which the stamp `tmp/bindings.source` already tracks.
- `$(BINDINGS_DEFAULT_UNIT)`'s recipe is unchanged (no option, flag 0).
- The generator binary changes, so the first build after this step regenerates `tmp/bindings/default.c` and relinks what links it, once. Steady-state `make test` and `make e2e` do no more work.
- The `make raylib ->` legend gains one clause: a `game` built with `bindings.cfg` writes `"bindings": "personal"` in every recording, which `replay` refuses.

### Recorder (`src/devtools/session_recorder.{c,h}`)

- New field `int personal_bindings` in `session_recorder_t`. `session_recorder_open` and `session_recorder_close` set it to 0.
- New function `session_recorder_set_personal_bindings(recorder, personal)`. `personal` is 0 or 1 (`assert`). It asserts `frame_count == 0` with a message in the style of `session_recorder_set_initial_save`'s ("the bindings flag is written into the header with the first frame"). The header comment states: before the first frame; 1 writes `"bindings": "personal"` into the header, 0 (the default after open) writes nothing.
- The recorder names no generated symbol and does not include `game/bindings.h`.
- Header text on the first frame, one string passed as `p8_input_append`'s `header_member`:
  - flag 0, no save: NULL (file byte-identical to `p8_input_save`, as today);
  - flag 0, save: `"initial_save": "<hex>"` (as today);
  - flag 1, no save: `"bindings": "personal"`;
  - flag 1, save: `"bindings": "personal"` + `,\n  ` + `"initial_save": "<hex>"`.
  So a flagged file starts `{\n  "input": "pad",\n  "bindings": "personal",\n` and the save, when present, follows on its own line before `"frames"`. The bindings member comes first because it is short and is what a human looks for first; the order is free for both readers (Starting point, H-16-2).
- The existing `format_initial_save_member` grows into one function that formats the whole header text (caller frees, NULL when there is nothing to write or no memory, as today's contract). On an allocation failure the frame is dropped exactly as today (returns 0, count unchanged).
- `session_recorder_record_frame` builds the header only on the first frame, as today; resumed and later frames never repeat it (`p8_input_append` ignores the member once the file is not fresh).

### `p8_input_append` contract (`src/p8/input.h`)

- Signature and behaviour unchanged; `input_json.c` is not touched. The header comment is reworded to state what it accepts: one or more JSON members as text, several joined by `,\n  ` by the caller, without the leading indent or the trailing comma; NULL for none, which keeps the file byte-identical to `p8_input_save`'s. A comment-only edit in `p8/input.h` touches no button constant, so the guardrail's generator and metrics commands are not required.

### `game.c`

- Right after `session_recorder_set_initial_save(&backend.recorder, …)`, call `session_recorder_set_personal_bindings(&backend.recorder, bindings_compiled_personal)`. That is the only line that reads the flag. Nothing else in `game.c` changes: the replay-then-play path, its messages and `replay_source_open` stay as they are. A personal recording handed back to `game` still loads and replays there with whatever table `game` compiled (out of scope); the new recording that session writes carries the flag of the binary that wrote it.
- No new console message: the `make raylib` line already tells the player which file was compiled.

### `bindings.default.cfg`

- The paragraph on the personal `bindings.cfg` gains one sentence: a game built with it writes `"bindings": "personal"` in every recording it makes, and `replay` refuses such recordings. Comment-only: the generated table is unchanged, and `test_bindings` still checks the default unit against the parse of the file.

### Where the flag is read, written and refused

| Place | Role |
|---|---|
| `src/cmd/bindings_generate.c` | writes `bindings_compiled_personal` into every unit, 1 only with `--personal` |
| Makefile | passes `--personal` for `tmp/bindings/game.c` when `bindings.cfg` exists |
| `src/game/bindings.h` | declares it |
| `src/cmd/game.c` | the only reader; hands it to the recorder |
| `src/devtools/session_recorder.c` | writes `"bindings": "personal"` when told to; names no generated symbol |
| `replay_source_read_bindings` (step 16) | refuses it in `replay` and the in-process replayers |
| `replay_source_open` | ignores it (H-16-4); `game.c`'s replay-then-play path unchanged |

## Implementation Chunks

1. **The recorder writes the flag.** The field, the setter, the joined header text, the `p8_input_append` comment, and the unit tests in `tests/test_recording_continuity.c` (tests 1–6 below). Nothing sets the flag outside tests yet. `make test` green, every `.sha256` unchanged.
2. **The flag comes from the compiled unit.** The generator option and symbol, the `bindings.h` declaration, the Makefile variable, recipe and legend clause, the `game.c` call, the `bindings.default.cfg` sentence. Then the binary proof and the one-off checks of Testing Decisions, recorded in the step note.

## Testing Decisions

- Good tests here drive public interfaces only: the recorder through `session_recorder_*`, read back as file bytes and through `p8_input_load`, `replay_source_open` and step 16's `replay_source_read_bindings`. They pin header bytes and round trips, never the formatting helper.
- **`tests/test_recording_continuity.c`** (module `session_recorder`, whose tests already live there; prior art: `assert_initial_save_round_trips` and `test_recorder_without_initial_save_writes_no_key`). Each test records `source_frames` to its own `tmp/test_recording_continuity_*.json` and removes it.
  1. **Flag with a save.** Set a save and the flag. The file starts with exactly `{\n  "input": "pad",\n  "bindings": "personal",\n  "initial_save": "<hex>",\n  "frames": [\n`. `"bindings"` occurs once in the whole file, after recording every frame. `replay_source_open` gives `REPLAY_SOURCE_LOADED`, serves every frame in order, and `replay_source_seed_storage` writes the same save bytes (pins H-16-4 and that the flag never costs the save).
  2. **Flag without a save.** The file starts with exactly `{\n  "input": "pad",\n  "bindings": "personal",\n  "frames": [\n`, loads with `p8_input_load` with every frame, and seeds an empty storage.
  3. **No flag, both variants.** With the flag set explicitly to 0: without a save the file is byte-identical to `p8_input_save`'s (extend the existing no-save test with the explicit call, keeping a variant without it); with a save, the file holds no `"bindings"` (add the check to `assert_initial_save_round_trips`). This pins that default players' recordings do not change.
  4. **Open and close clear the flag.** Open, set the flag, close; open the same recorder on a new path and record without setting it: no `"bindings"` in the file.
  5. **Resume keeps one header.** A flagged session resumed from a flagged recording (through `run_session`'s pattern, the flag set before the loop): the resumed file holds `"bindings"` exactly once and every frame in order.
  6. **Round trip to the refusal (H-16-3).** A flagged recording read back with `replay_source_read_bindings` (any local fallback table) gives `REPLAY_SOURCE_BINDINGS_PERSONAL`, and an unflagged one gives `REPLAY_SOURCE_BINDINGS_DEFAULT`. This is the only test that names a step-16 symbol; see the rule under Hypotheses if it differs.
  - The late-setter `assert` is not tested (no death tests in the suite); its message is the documentation.
- **No unit test for the generator option or for `game.c`.** `game.c` builds only under `make raylib`, and the generator's no-option path is exercised by every build of `tmp/bindings/default.c`. The option is proved on the binaries:
- **Binary proof, nothing launched (recorded in the step note):**
  1. With a valid personal `bindings.cfg` (for example the default with 7 and 8 swapped), `make raylib` prints `bindings: bindings.cfg (personal)`. `nm -S tmp/game` shows `bindings_compiled_personal` (4 bytes, read-only data) and `objdump -s` of those bytes reads 1. The same dump on `tmp/replay` reads 0.
  2. After deleting `bindings.cfg`, `make raylib` prints `bindings: bindings.default.cfg`, regenerates `tmp/bindings/game.c` through the stamp, and the dump on `tmp/game` reads 0.
  3. `nm -A --defined-only` over the objects linked into `tmp/replay` finds `bindings_compiled_personal` only in `tmp/obj/tmp/bindings/default.o`; no shared object carries it.
  4. `git grep -n bindings_compiled_personal -- src tests` lists only the declaration in `bindings.h`, the generator, and the one read in `game.c`.
  If raylib is not vendored in the container, run `make raylib-vendor-setup` first (as steps 2.7 and 3.15 did); if that is impossible, prove points 1–2 on `tmp/bindings/game.o` built by a direct `make tmp/obj/tmp/bindings/game.o`-style target and record the gap.
- **One-off checks (recorded in the step note, not committed):**
  1. `tmp/bindings_generate --bogus bindings.default.cfg tmp/x.c` and a call with the option last both exit 2 with the usage line and write nothing.
  2. A no-change `make raylib` after the proof compiles nothing.
  3. With step 16 committed: a recording written by the recorder with the flag set (for example by test 1 with its file kept once) fed to `tmp/replay` exits 1 with step 16's personal message and writes no capture.
  4. `git diff <base HEAD> -- src/cmd/game.c` shows only the added setter call; `src/devtools/replay_source.c` is untouched.
- **Guardrails:** `make test-all` green with every `tests/e2e/*.json.sha256` unchanged and `make end-state` unchanged. `make raylib` links. With a personal `bindings.cfg` present, `make test-all` stays green and `make pad-author` followed by `git diff --exit-code worst-case/` shows nothing (only `game` compiles the personal file). This step touches no `src/game/pad_*`, `src/devtools/pad_*`, `src/devtools/input_json.c` or button constant, so the generator and metrics commands are not otherwise required.
- **Timings:** `make test` and `make e2e` on a warm build (after the one-time regeneration of the default unit), at least five alternating runs of this step's HEAD and its base HEAD on the same machine. Means go in the note. Neither may be slower beyond noise.

## Out of Scope

- Refusing the flag in `replay` and the in-process replayers: step 16.
- `game.c`'s replay-then-play path, including what it does with a flagged recording (section 2, Out of scope).
- A flag for a personal `keymap.cfg` (section 2: recordings store buttons, after the keymap).
- Copying the personal file into or next to a recording (section 2, Rejected).
- Changing `p8_input_append`'s signature or `input_json.c`; sharing the `"bindings"` spelling as a constant between the recorder and `replay_source` (the round-trip test pins their agreement instead).
- A console message from `game` about personal bindings, and any README change (step 20 owns the README).
- Fixture replays in `tests/e2e/`: steps 18 and 19.

## Further Notes

- Why a separately defined symbol and not a header: step 2.7's note. A header under `tmp/` would need an include path beyond `-Isrc`, and keeping the definition in the generated unit keeps it out of the objects shared by `game` and `replay`, which the binary proof checks.
- Why the Makefile and not the generator decides "personal": the Makefile already owns the choice of file and the `(personal)` line; one condition drives both, so they cannot disagree.
- `tests/test_marker_coherence.c` indexes markers by `session_recorder_frame_count`, which this step does not change: the header is written with the first frame and the count stays the number of frames.
