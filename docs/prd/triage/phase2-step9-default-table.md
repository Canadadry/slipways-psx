---
title: "Default table beside the compiled one"
description: "The attract demo must always play on the default bindings, but a game built with a personal bindings.cfg links only its personal table, so the core has no default table to switch pad_nav to."
status: needs-triage
---

## Problem Statement

Plan 18, phase 2, step 9:

> 9. **Default table beside the compiled one.** `game` links the default bindings table as well as its compiled one, and `input_step` can switch `pad_nav` between them. Nothing switches yet. Verified as plan 17 step 7 verified its tables, on the built binaries without launching anything: with a swapped personal `bindings.cfg`, `game` holds both the personal and the default table; `replay` holds the default. Every `.sha256` unchanged.

Section 2 of the plan locks: **"The demo always plays on the default bindings."** The demo is a recorded stream of button numbers authored by `pad_author` against `bindings.default.cfg`. Played through a personal table, the same buttons mean other actions: a player who swapped two buttons would watch the demo open the wrong menu, never fold, and drift away from its captions.

Today the core cannot honour that rule:

- **Each binary links exactly one table, under one name.** `bindings_generate` writes a unit defining `bindings_compiled` (and `bindings_compiled_personal`). `replay`, `pad_author` and the tests link the unit generated from `bindings.default.cfg`; `game` links the unit generated from `bindings.cfg` when it exists. In a personal `game` the default table is simply absent, and linking the default unit beside the game unit would define `bindings_compiled` twice.
- **Only the entry points name a table.** `src/cmd/game.c`, `src/cmd/replay.c`, `pad_author` and `record_start` hand `&bindings_compiled` (or a fixture table, in `replay`) to `input_step_init`; nothing in `src/game/` names a table. The attract mode lives in the core (step 10) and must reach the default table from there.
- **`input_step` forgets the session's table.** `input_step_init` copies the table into `pad_nav` and keeps nothing else. After the demo, the return must put back the table the session started with: `bindings_compiled` in `game`, but in `replay` the e2e fixture table read from the header (`replay_source_read_bindings`), which `input_step` cannot recover.
- `record_start` (step 5) dodges the question by asserting `!bindings_compiled_personal` and using `bindings_compiled` both ways, which is only true of the tools.

## Solution

1. **A second generated unit, from `bindings.default.cfg` only, defines a new symbol `bindings_default`.** `bindings_generate` gains a `--default` option that writes `const bindings_t bindings_default` and nothing else. The Makefile generates it as `tmp/bindings/core.c` and adds it to `ENGINE_SRCS`, so every binary that links the engine (`game`, `replay`, `pad_author`, every engine test) holds the default table under its own name, whatever its compiled table is. No binary defines a symbol twice: `bindings_compiled` stays in exactly one unit per binary, `bindings_default` in exactly one unit per binary.
2. **`input_step` keeps the session's table and can switch.** `input_step_init(step, table)` keeps its signature and meaning, and also keeps its own copy of `table` as the session's table. A new `input_step_switch_bindings(step, which)` puts the input step back to the state `input_step_init` gives, with `pad_nav` on either the default table or the session's table, and the session's table kept for the way back. Step 4's contracts (re-init at the start and at the return) become one call each in step 10.
3. **Nothing switches yet.** No caller of the switch exists outside the tests; `game`, `replay`, `pad_author` and `record_start` are unchanged; every `.sha256`, every `.end-state` and `worst-case/` stay byte-identical.
4. **Proof on the built binaries, nothing launched**, as plan 17 did: with a personal `bindings.cfg` swapping two actions, `tmp/game` defines `bindings_compiled` (the swapped table, personal flag 1) and `bindings_default` (the default table); `tmp/replay` defines both with the default table's bytes; after deleting the file, `game`'s two tables are equal again.

## User Stories

1. As a player with a personal `bindings.cfg`, I want the demo to play on the default bindings, so that the demo I watch does what its captions say.
2. As a player with a personal `bindings.cfg`, I want my own bindings back the moment the demo returns to the title, so that the title menu answers my buttons.
3. As a player without a personal file, I want nothing to change, so that my game plays exactly as before.
4. As the step-10 implementer, I want the default table reachable from `src/game/` under a stable name, so that the attract module in the core needs no table from its caller.
5. As the step-10 implementer, I want one call that resets the input step onto the default table at the demo's start, so that step 4's "re-init at start" contract is a single line.
6. As the step-10 implementer, I want one call that resets the input step onto the session's table at the return, so that step 4's "re-init at return" contract is a single line and needs no table from the caller.
7. As the step-10 implementer, I want the session's table kept inside `input_step` by value, so that `replay`'s e2e fixture table comes back after a demo as surely as `game`'s personal table.
8. As the step-10 implementer, I want the switch to reset the cheat state and the pad navigation exactly as `input_step_init` does, so that the second demo start does not inherit the first demo's cursor or ids (step 4's list, item 4).
9. As the step-10 implementer, I want switching to the table already in use to be allowed and to reset just the same, so that the attract module needs no bookkeeping of which table is active.
10. As a maintainer, I want the default table generated only from `bindings.default.cfg`, never from `bindings.cfg`, so that no personal file can reach the demo through any binary.
11. As a maintainer, I want the default table's unit rebuilt when `bindings.default.cfg` or the generator changes, and never otherwise, so that a no-change `make`, `make test` and `make raylib` recompile nothing.
12. As a maintainer, I want creating or deleting `bindings.cfg` to leave the default table's unit untouched, so that only the game unit follows the personal file, as plan 17 designed it.
13. As a maintainer, I want every binary to define `bindings_default` exactly once and `bindings_compiled` exactly once, so that no link fails on a duplicate and no binary silently picks one of two tables.
14. As a maintainer, I want `bindings_compiled` and `bindings_compiled_personal` unchanged in name, meaning and generator output, so that plan 17's personal flag, `replay`'s refusal of personal recordings and every existing caller keep working.
15. As a maintainer, I want `bindings_generate` to refuse `--default` together with `--personal`, so that a confused Makefile edit cannot write a personal default table.
16. As a maintainer, I want a unit test proving `bindings_default` is the parse of `bindings.default.cfg`, so that the new unit is proven like the old one.
17. As a maintainer, I want a unit test proving `bindings_default` and `bindings_compiled` coexist in one binary and are equal when the compiled table is the default, so that the two-unit link is exercised by `make test` on every commit.
18. As a maintainer, I want unit tests of the switch through `input_step`'s public interface, so that the behaviour step 10 relies on is pinned before step 10 exists.
19. As a maintainer, I want every existing `*.sha256`, `.end-state`, `worst-case/` file and `pico8/demo.json` unchanged, so that the step is proven to change no behaviour.
20. As a maintainer, I want `make test` and `make e2e` not slower than step 1's baseline, so that the loop stays fast (`CLAUDE.md`).
21. As a maintainer, I want `make pointer-guard`, `make button-guard`, `make data-guard`, `make float-grep`, `make float-guard`, `make alloc-guard` and `make constants-check` to keep passing with unchanged counts, so that the new table adds no pointer, no button name and no float to the core.
22. As a reviewer, I want the binary proof (which symbols each binary defines, which bytes each table holds) recorded in the step note, so that "game holds both tables" is evidence, not a claim.
23. As a reviewer, I want the proof repeated after deleting `bindings.cfg`, so that the personal table is shown to leave `game` too.
24. As a PSX porter, I want the default table as plain `const` data generated beside the core, so that the console build links it like any other core unit, with no file read.
25. As plan-pilot, I want `make demo-check` to keep passing, so that the demo's chain is shown untouched by a change on its path (`input_step`).

## Implementation Decisions

### The symbol: `bindings_default`

- **Name: `bindings_default`**, declared in the bindings module's header next to `bindings_compiled` as `extern const bindings_t bindings_default;`. It always holds `bindings.default.cfg`. `bindings_compiled` keeps meaning "the table this binary was built with" (personal or default), `bindings_compiled_personal` its flag.
- Rejected names: anything ending in `_default_table`, `_default_string` and the like. `make data-guard` refuses `\b[a-z0-9_]+_default_(fixed|table|vec|string|box|list|bool|hf)\b` in `src/` and `tests/` (a trap for test function names too: `test_..._default_table` at a word end matches).
- **No pointer**: an `extern` declaration and a definition in a generated unit under `tmp/` (not scanned): `make pointer-guard` keeps its counts, `make button-guard` sees no `P8_PAD_BUTTON_*` in `src/game/` (the masks live in the generated unit).

### The generator: `bindings_generate --default`

- `usage: bindings_generate [--personal | --default] <input.cfg> <output.c>`. With `--default` the unit holds the header line, the two includes and `const bindings_t bindings_default = { .masks = { … } };` and nothing else (no personal flag: that flag describes `bindings_compiled`). Without the option, or with `--personal`, the output is byte-identical to today's.
- `--personal` and `--default` together, or either one repeated, is a usage error (exit 2), as a wrong argument count is today.
- The table writer is shared: only the symbol name and the flag line differ. Same parse, same refusal messages, same temporary-file-then-rename write.
- The generator does not check the input's file name: the Makefile is the only caller and passes `bindings.default.cfg`. The invariant "never from `bindings.cfg`" is held by the Makefile rule (below) and checked by the binary proof.

### Makefile

- **`BINDINGS_CORE_UNIT := $(TMP_DIR)/bindings/core.c`**, next to `BINDINGS_DEFAULT_UNIT`, with the rule `$(BINDINGS_CORE_UNIT): bindings.default.cfg $(TMP_DIR)/bindings_generate` running `mkdir -p $(@D)` then `bindings_generate --default`. No stamp and no `FORCE`: its input is fixed, unlike the game unit's, so `bindings.cfg` appearing or disappearing never touches it.
- **`$(BINDINGS_CORE_UNIT)` joins `ENGINE_SRCS`.** Every target built from `ENGINE_SRCS` (including `DATA_ENGINE_SRCS`, `REPLAY_SRCS`, `PAD_AUTHOR_SRCS`, `GAME_SRCS`, and their asan, sanitize and coverage object trees) then holds `bindings_default`. Why `ENGINE_SRCS` and not per target: `input_step.c` is in `ENGINE_SRCS` and now names `bindings_default`, and every engine binary links `input_step.o` (objects are listed, not archived), so every one needs the definition. Appending it per target would mean editing about eighty test rules.
- **Not added to `GAME_SRCS` separately.** `GAME_SRCS` already includes `ENGINE_SRCS`, and `tmp/game` is compiled from sources in a single `$(CC)` command: listing the unit twice would define `bindings_default` twice. Trap for step 10, which will likely move step 8's `$(DEMO_UNIT)` from `GAME_SRCS` into the core the same way (Further Notes).
- **`CORE_SRCS` filters `$(BINDINGS_CORE_UNIT)` out**, as it filters the host tools out. `float-grep` (`-MM` over `CORE_SRCS`, partition of `src/` only) and `float-guard` run with no prerequisite on generated files, so a `tmp/` path in `CORE_SRCS` would fail on a clean tree or under `-j`. The unit is integer data generated from a `.cfg`; the guards lose nothing. The reason goes in the commit message (`CLAUDE.md` § Comments: in a `Makefile` only help for reading a command stays).
- **`.d` includes: no change.** The default, asan, sanitize and coverage `-include` lines already read `tmp/bindings/*.d`, which covers `tmp/bindings/core.d`; the ubsan tree builds no engine target.
- **`test_bindings`'s rule** gains `$(BINDINGS_CORE_UNIT)` (it links `src/game/bindings.c` and the default unit, not `ENGINE_SRCS`).
- **The `make raylib` legend** gains a few words: every binary, `game` included, also links `bindings.default.cfg` as the default table the demo plays on. The rest of the legend is step 14's.
- `tmp/bindings/core.c` is never committed (plan 17's pattern: no sync check). `bindings.cfg` stays gitignored.

### `input_step`: the session's table and the switch

- **`input_step_t` gains `bindings_t session_bindings`**, by value (about 1.3 KB; the struct already holds a copy inside `pad_nav`). No pointer field: `make pointer-guard` counts data pointers in `src/game/` struct fields with strict equality and no step may raise one.
- **`input_step_init(step, table)`**: signature and visible behaviour unchanged (zeroes the step, `cheat_init`, `pad_nav_init` on `table`); it also copies `*table` into `session_bindings`.
- **New `void input_step_switch_bindings(input_step_t *step, input_step_bindings_t which)`** with
  `typedef enum { INPUT_STEP_BINDINGS_SESSION, INPUT_STEP_BINDINGS_DEFAULT } input_step_bindings_t;`
  Contract: afterwards the step is exactly what `input_step_init` on the chosen table gives (`INPUT_STEP_BINDINGS_DEFAULT` → `bindings_default`, `INPUT_STEP_BINDINGS_SESSION` → the copy kept at init), except that `session_bindings` is kept. It is the reset that step 4's contracts require at the demo's start and at its return, folded with the table choice: step 4 showed that switching the table without resetting `pad_nav` would let the second start inherit the previous demo's ids and cursor, so there is deliberately no "swap the table only" call.
  - Implementation shape: save the session table, run `input_step_init` on the chosen table, restore the session table. Calling it on the table already in use is legal and resets the same way.
  - An out-of-range `which` is an `assert` with a message.
- **No query of the active table.** Step 10's attract module knows whether it is playing; a getter would be an untested extra. Tests read the table through the existing `pad_nav_bindings`.
- **Callers unchanged.** `game.c`, `replay.c`, `pad_author.c`, `record_start.c` keep `input_step_init(&step, …)` as today. `record_start` keeps its `!bindings_compiled_personal` assertion and its use of `bindings_compiled`: migrating it to the switch is not needed by this step (the tools never link a personal table) and stays step 10's choice.

### Why this design (guideline decisions)

No human answers questions in this run; each decision below was taken under `.claude/skills/guideline/SKILL.md` ("make the plan work", "keep it safe", "the smaller change"):

- **A new symbol in a new unit rather than renaming or merging the existing ones**: keeps `bindings_compiled`, its flag and all ~40 references untouched, and makes duplicate definitions impossible by construction (one symbol, one unit, one binary).
- **The default unit in `ENGINE_SRCS` rather than tables handed in by every caller**: the alternative (`input_step_init` taking two tables) changes about 25 call sites in `src/cmd/`, `src/devtools/` and `tests/`; the core naming its own default table is what the plan's "attract mode in the core" needs, and step 10 will need the same for the demo data.
- **The session's table stored by value in `input_step`** rather than the switch naming `bindings_compiled`: in `replay` the session table can be an e2e fixture, and `bindings_compiled` is not defined in engine-only test binaries.
- **The switch resets the step**: step 4's note (item 4) requires a re-init at start and at return; one call does both jobs and leaves no way to switch without resetting.
- **`CORE_SRCS` filters the generated unit out**: the four guards read `CORE_SRCS` without building generated files; filtering is one word, making them generate it would touch four rules.
- **No-raylib fallback for the binary proof** (Testing Decisions): the plan's "on the built binaries" needs `tmp/game`, which needs a vendored raylib this environment may lack; the fallback proves the same symbols and bytes on the exact units `tmp/game` links, and the note says which variant ran.

## Implementation Chunks

1. **Generator option and the core unit.** `bindings_generate --default`, `bindings_default` declared in the bindings header, `BINDINGS_CORE_UNIT` and its rule, `ENGINE_SRCS`, the `CORE_SRCS` filter, `test_bindings`'s link and its two new cases, the legend words. Nothing in `src/game/` names the symbol yet. Done when `make test-all` passes and a second `make test` compiles nothing.
2. **The switch.** `session_bindings`, `input_step_switch_bindings` and the enum, the `test_input_step` cases. Done when `make test-all` passes, the guardrail runs below are clean, and the binary proof is recorded.

## Testing Decisions

- A good test drives a module through its public interface and reads only its results: tables through `bindings_mask` / `pad_nav_bindings`, the step's bytes against a freshly initialised step. No test reads the generator's C text.
- **`tests/test_bindings.c`** (extended, links the core unit beside the default unit):
  - `bindings_default` equals the parse of `bindings.default.cfg`, for every mode and action (the existing check for `bindings_compiled`, applied to the new symbol);
  - in this binary, which links the default compiled table, `bindings_default` and `bindings_compiled` are equal mask for mask and `bindings_compiled_personal` is 0. This is the in-suite proof that both units link into one binary.
- **`tests/test_input_step.c`** (extended), with the existing swap table (`jump.enter_aim = 8`, `jump.open_menu = 7`) as the session table:
  - after `input_step_init` on the swap table, switching to the default puts `bindings_default`'s masks in `pad_nav` (enter_aim on 7, open_menu on 8);
  - switching back to the session's table puts the swap table back, even when the caller's table was zeroed after init (the step keeps its own copy);
  - a step that ran some frames (cursor moved, cheat input under way), then switched, is byte-identical (`memcmp`) to a fresh `input_step_init` on the same session table followed by the same switch: the switch resets as init does and keeps only the session table;
  - switching to the table already in use resets the same way;
  - `input_step_init` alone is unchanged: the existing three table tests stay as they are.
- Both binaries are already in `UNIT_TESTS`; the new cases run in microseconds.
- **Binary proof, by hand, recorded in the step note** (as plan 17 step 7 asked and plan 17 step 17 did with `tmp/game` and `tmp/replay`), nothing ever launched:
  1. Try `make raylib-vendor-setup` once, unless step 8's note says raylib is already vendored here.
  2. Create `bindings.cfg`: a copy of `bindings.default.cfg` with the buttons of `jump.enter_aim` and `jump.open_menu` swapped (7 and 8). `make raylib` prints `bindings: bindings.cfg (personal)`.
  3. `nm -S --defined-only` on `tmp/game` and `tmp/replay`: each defines `bindings_compiled`, `bindings_compiled_personal` and `bindings_default` exactly once. Read each table's bytes at its address and size (`objdump -s --start-address --stop-address`): `game`'s `bindings_default` equals `replay`'s `bindings_compiled` and `bindings_default`; `game`'s `bindings_compiled` differs from them in exactly the two swapped masks; `game`'s flag reads 1, `replay`'s 0. Among `replay`'s objects, only `tmp/bindings/core.o` defines `bindings_default` and only `tmp/bindings/default.o` defines `bindings_compiled`.
  4. With the personal file present: `make test-all` green, `make pad-author` with no diff in `worst-case/` or `pico8/demo.json`, `make demo-check` passing.
  5. Delete `bindings.cfg`, `make raylib` prints `bindings: bindings.default.cfg`; `game`'s two tables now have equal bytes and the flag reads 0. `tmp/bindings/core.c` kept its date through both rebuilds.
  - **Fallback when raylib cannot be vendored** (no network): `tmp/game` cannot link. Then generate `tmp/bindings/game.c` (`make tmp/bindings/game.c`) with the personal file present, compile it and the core unit to objects, combine them with `ld -r` into a throwaway object under `tmp/` (a duplicate definition would fail there), and run steps 3 and 5 on that object instead of `tmp/game`; `make -p -q tmp/game` must show both `tmp/bindings/game.c` and `tmp/bindings/core.c` among `tmp/game`'s prerequisites. The step note says which variant ran.
- **Guardrail runs** (plan section 2: the step changes `input_step`): `make test-all`; `make end-state`, `make full-game` and `make pad-author` with no diff in `worst-case/` or `pico8/demo.json`; `make demo-check` (the step touches the demo's path). Every `tests/e2e/*.sha256` unchanged.
- **Timing**: `make test` with nothing to rebuild and `make e2e` after `rm -rf tmp/frames`, three runs each, step 1's method, compared with step 1's minimums (8.2 s, 1.39 s) and the latest step notes' ranges.
- **No-rebuild**: a second `make test` prints no compile line; `make -q tmp/bindings/core.c tmp/obj/tmp/bindings/core.o` exits 0; creating or deleting `bindings.cfg` regenerates `tmp/bindings/game.c` only.
- **Prior art**: `tests/test_bindings.c` (generated unit vs. parse of the committed file), `tests/test_input_step.c` (swap table, `pad_nav_bindings`, own copy), plan 17 step 17's binary proof (flag bytes read in `tmp/game` and `tmp/replay`, defining objects counted).

## Out of Scope

- Any caller of `input_step_switch_bindings` outside the tests: the attract state machine, its start and return (step 10).
- Changing `game.c`, `replay.c`, `pad_author`, `record_start` or the session recorder; migrating `record_start` to the switch or dropping its assertion.
- Linking step 8's demo unit into the core or into `replay` (step 10).
- Drawing anything (step 11), the attract e2e (step 12), `make demo-check`'s full cycle (step 13), docs beyond the legend's few words (step 14).
- Any change to `bindings.default.cfg`, to `bindings_compiled`'s generation, to plan 17's stamp, or to the keymap units.
- A getter for the active table, or a table switch that does not reset the step.

## Further Notes

- `bindings_default` is linked into every engine binary but read only by `input_step_switch_bindings` until step 10. Its bytes are the same in every binary, built from the one committed file.
- The step note should record the proof variant, the symbol reads, the timings and the guardrail runs.
- **For step 10:** the start becomes `input_step_switch_bindings(step, INPUT_STEP_BINDINGS_DEFAULT)` then `cart_resume_game`, and the return `cart_return_to_title` then `input_step_switch_bindings(step, INPUT_STEP_BINDINGS_SESSION)` (step 4's order). `record_start` could then use the same calls and drop its personal-table assertion; that is step 10's call.

### Hypotheses

Step 8 (compiled demo, `docs/prd/triage/phase1-step8-compiled-demo.md`) is being implemented while this PRD is written. What this PRD assumes about its outcome; if step 8's note says otherwise, the note wins and only the named part of this PRD follows it.

- **H-8-1 (done first):** step 8 is committed before step 9 starts, and step 9 builds on its Makefile, not on the one read for this PRD.
- **H-8-2 (Makefile overlap):** step 8 adds `DEMO_UNIT := $(TMP_DIR)/demo/demo.c` and its rules next to `BINDINGS_DEFAULT_UNIT`, appends `$(DEMO_UNIT)` to `GAME_SRCS` (not `ENGINE_SRCS`), and adds `$(OBJ_DIR)/tmp/demo/*.d` to the default tree's `-include` line. Step 9 touches the same block (a second unit next to `BINDINGS_DEFAULT_UNIT`), `ENGINE_SRCS`, `CORE_SRCS` and `test_bindings`'s rule, but not `GAME_SRCS` nor any `-include` line. The edits are adjacent, not overlapping; if step 8 put its rules elsewhere or changed `GAME_SRCS`'s shape, step 9 places `BINDINGS_CORE_UNIT` next to `BINDINGS_DEFAULT_UNIT` wherever it is.
- **H-8-3 (no double listing):** step 8 does not put `$(DEMO_UNIT)` in `ENGINE_SRCS`. If it did, `GAME_SRCS` must not also list it (single-command compile of `tmp/game`), a step-8 defect to report, not to work around here. Conversely, when step 10 moves the demo unit into the core it must drop it from `GAME_SRCS`, exactly as this step does not list `BINDINGS_CORE_UNIT` there.
- **H-8-4 (generator closure):** `demo_generate` links no `ENGINE_SRCS` and no bindings unit (step 8's link-closure decision), so the new core unit never reaches it and cannot create a build cycle. `test_demo_data` links `ENGINE_SRCS` with `$(BINDINGS_DEFAULT_UNIT)` and thereby gains the core unit with no edit and no duplicate.
- **H-8-5 (untouched files):** step 8 does not modify `src/game/bindings.{c,h}`, `src/game/input_step.{c,h}`, `src/game/pad_nav.*`, `src/cmd/bindings_generate.c`, `tests/test_bindings.c` or `tests/test_input_step.c`. If it did, step 9 builds on its version.
- **H-8-6 (raylib):** step 8's note says whether `make raylib-vendor-setup` worked in this environment. If raylib is vendored (`third_party/` is gitignored and stays), step 9 runs the proof on the real `tmp/game`; if step 8 fell back, step 9 tries once more and otherwise uses the fallback above.
- **H-8-7 (timing reference):** step 8's note holds the latest `make test` timings; step 9 compares with step 1's minimum and step 8's range.
- **H-8-8 (guards):** step 8's float-grep exclusion for `src/game/demo_data.h` and its new tests do not interact with this step's `CORE_SRCS` filter or guard counts.
