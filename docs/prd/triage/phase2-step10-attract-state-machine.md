---
title: "Attract state machine"
description: "Left alone on the title, the core must start the compiled demo by itself, play it on the default bindings without touching the player's save, and give the title back on any button, but no code drives the demo from the input frames yet."
status: needs-triage
---

## Problem Statement

Plan 18, phase 2, step 10:

> 10. **Attract state machine.** Starting from step 4's list of state not rebuilt, `src/game/attract.{c,h}`: title idle counter, threshold from the header member, reset on any button down, start, frame substitution from the compiled demo, interruption, swallowed press until every button is up, end after the last frame, return to the title through `cart_init`, `save_at_frame_end` skipped while playing, default bindings while playing. Not drawn yet. Verified by unit tests through the module's interface for each of these, including: a disk save written before the demo is byte-identical after an interrupted demo and after a complete one, and the title has its `resume` row again; a press held from the demo does not move or pick a title row; and feeding the whole compiled demo through the attract module reaches step 7's end state (the counts step 5's tool gave at the end of `demo.json`). Every existing `.sha256` unchanged.

Every piece is there except the machine that ties them together:

- The demo is compiled (step 8: `demo_data_record`, `demo_data_pad_buttons`, `demo_data_frame_count` in `tmp/demo/demo.c`), but the unit is linked into `game` only, and nothing in the core reads it.
- The threshold is read (step 3: `replay_source_attract_after`, 1200 when missing; recordings write 1200), but nothing hands it to the core.
- Two starts in one process are proven (step 4), but only by a test harness and by `src/devtools/record_start.c` (step 5), which each spell the start and the return themselves. Step 4's note lists what the start and the return must also rebuild: the input step (item 4) and the stored save (item 7, still written by demo actions through `save_at_frame_end`).
- The default table and the switch back to the session's table are being added (step 9: `bindings_default`, `input_step_switch_bindings`), with no caller yet.

A player who leaves the title alone sees nothing happen, and a `game` session cannot yet be replayed with a demo in it.

## Solution

From the player's side:

1. On the title (no game running, no box open), the core counts frames with no button down. Any button down resets the count; leaving the title resets it too.
2. When the count reaches the threshold (1200 frames by default, or the `"attract_after"` of the replayed file), the next frame starts the demo: the compiled record is resumed and the compiled pad stream is played on the default bindings, one recorded mask per frame, in place of the player's frames.
3. While the demo plays, the player's save on disk is never written nor erased.
4. Any button down during the demo returns to the title on that frame. That press, and everything held with it, is swallowed: the core sees empty frames until every button is up, so no title row moves or is picked.
5. After the demo's last frame, the next frame returns to the title on its own.
6. The returned title is built by `cart_init` (through `cart_return_to_title`) and re-reads the disk: the `resume` row is exactly as before the demo. The player's own bindings are back. The count starts again from zero, so the demo loops: title, 20 s, demo, title, 20 s, demo…
7. Nothing is drawn yet (no caption, no `demo` label: step 11).

From the code's side, the attract mode lives in the core and is driven only by the input frames: `input_step_run` asks the attract module what to play each frame. `game`, `replay`, `pad_author` and every test that runs frames go through it, so a `game` session that idled on the title replays identically in `replay`. The start and the return are one core sequence each, shared with `record_start`, so `replay`, `REPLAY_STARTS=2`, `make pad-author-proof` and `make demo-check` run the exact sequence the attract mode runs. No existing replay idles 1200 frames on the title, so every `*.sha256`, `.end-state` and `worst-case/` file stays unchanged.

## User Stories

1. As a player, I want the title to start a demo by itself after 20 seconds with no button down, so that I learn what the game is about without reading anything.
2. As a player, I want any button I press on the title to restart the 20 seconds, so that the demo never starts while I am choosing a row.
3. As a player, I want the demo never to start while a game runs, so that a long pause in my game is never taken for an idle title.
4. As a player, I want the demo never to start while a box is open, so that a box left open is never interrupted by a demo.
5. As a player, I want the demo to always show the same galaxy, so that it does not depend on how long the title ran.
6. As a player with a personal `bindings.cfg`, I want the demo to play on the default bindings, so that it does what it was recorded to do.
7. As a player with a personal `bindings.cfg`, I want my own bindings back as soon as the title returns, so that the menu answers my buttons.
8. As a player, I want any button to bring the title back at once, so that I never wait for the demo to finish.
9. As a player, I want the button that stopped the demo to do nothing on the title, so that it neither moves the cursor nor starts a game.
10. As a player, I want a button I keep holding after stopping the demo to stay ignored until I let go, so that a long press does not pick a row.
11. As a player, I want the title back after the demo's last frame, so that the demo loops like an arcade attract mode.
12. As a player, I want my saved game untouched by the demo, byte for byte, so that `resume` brings back exactly my game.
13. As a player, I want the `resume` row back on the title after the demo, so that I can go on with my game.
14. As a player with no saved game, I want the title after the demo to have no `resume` row, so that the demo is never offered as my game.
15. As a player, I want the title after the demo to count 20 s again before the next demo, so that the loop is steady.
16. As a maintainer, I want the attract mode in the core and driven only by the input frames, so that `game`, `replay` and the e2e behave the same.
17. As a maintainer, I want the session recorder to keep the raw frames (empty during the demo, then the interrupting press), so that `replay` rebuilds the demo from the same frames and the recording stays small.
18. As a maintainer, I want the threshold to come from the replayed file's `"attract_after"` (1200 when missing) in both `game` and `replay`, so that a test file can declare a short one and a recording replays faithfully.
19. As a maintainer, I want the core's default threshold and the replay reader's default to be one constant, so that they can never drift apart.
20. As a maintainer, I want the start (default table, then the record resumed) and the return (`cart_return_to_title`, then the session's table) written once in the core, so that the tools and the attract mode run the same sequence.
21. As a maintainer, I want `record_start` reduced to reading the hex file and calling the core's start and return, so that `replay`, `REPLAY_STARTS=2`, `make pad-author-proof` and `make demo-check` prove the attract mode's own sequence.
22. As a maintainer, I want the attract module's decisions (count, start, substitute, interrupt, swallow, end) testable without a cart, so that their timing is pinned frame by frame at no cost.
23. As a maintainer, I want the integration (save bytes, `resume` row, bindings, swallowed press) tested in `make test` on short segments, so that `make test` stays fast.
24. As a maintainer, I want the whole compiled demo fed through the attract module in a test run by `make demo-check`, never by `make test`, so that the full check exists without slowing the loop.
25. As a maintainer, I want that full feed to reach step 7's end state (year 3413, 17 colonies, 19 slipways, 3 techs, no cheat) at the end of each of two playbacks, so that the attract path is shown to play the demo exactly as `replay` does.
26. As a maintainer, I want the save bytes compared after a complete demo, so that the save cut-off covers the whole demo, closing card included.
27. As a maintainer, I want one interruption inside each command range of the demo (fold, colonise, slipway, year, research) checked once, so that no command leaves a write or a box behind on the return.
28. As a maintainer, I want a read-only accessor for the attract phase and the demo frame, so that tools and tests read where the demo is without frame arithmetic.
29. As the step 12 and 13 implementer, I want `replay` to write the state on the demo's last frame before each return of a title-start replay, so that the full-cycle check reads the demo's end state without computing frame numbers.
30. As a maintainer, I want one writer of the `"attract_after"` header member shared by the session recorder and the tests, so that no third copy of its format appears.
31. As a maintainer, I want the demo unit linked into the core (every engine binary) exactly once, so that `game`, `replay`, `pad_author` and the tests hold the same demo and no link defines it twice.
32. As a maintainer, I want `demo_data.h`'s float-grep exclusion deleted once a core file includes it, so that the float partition stays exact.
33. As a maintainer, I want every existing `*.sha256`, `.end-state`, `worst-case/` file and `pico8/demo.json` unchanged, so that the step is shown to change no existing replay.
34. As a maintainer, I want `make test` and `make e2e` not slower than step 1's baseline, so that the loop stays fast (`CLAUDE.md`).
35. As a maintainer, I want `make pointer-guard`, `button-guard`, `random-guard`, `data-guard`, `float-grep`, `float-guard`, `alloc-guard` and `constants-check` to keep passing with unchanged counts, so that the attract module adds no pointer, button name, random draw or float to the core.
36. As a PSX porter, I want the attract mode in portable core C over compiled data, so that the console gets it with no file read.
37. As plan-pilot, I want `make raylib` to link `game` with the demo in the core, so that the interactive binary is shown to build (never run).
38. As a reviewer, I want the step note to record the interruption sweep, the timings and the guardrail runs, so that the claims are evidence.

## Implementation Decisions

### Where the attract mode hooks in

- **In `input_step_run`, at its very top**, before `pad_nav_step` and `cheat_step`. Every driver already calls `input_step_run` once per frame, before `cart_update60`, and records the raw frame before it (`game.c` records with `raylib_backend_record_frame` before the input step). So the start and the return run "at the top of a frame, before that frame's input step" (step 4's contract) with no driver loop change, and the recorder keeps the raw frames with no change.
- **Not in `cart_update60`**: the substituted buttons must go through `pad_nav_step`, which runs before `cart_update60`. **Not in the drivers' loops** (`src/cmd/game.c`, `src/cmd/replay.c`): plan section 2, "never in `src/cmd/game.c`". The drivers only hand over the threshold (below).
- The **buttons fed to the rest of `input_step_run`** (pad navigation, `played_frame.pad_buttons`, `cheat_step`) are the attract module's output, not `raw->pad_buttons`. Mouse fields are already dropped by `input_step_run` (the played frame is zeroed and only pad buttons are copied); the attract mode looks at `raw->pad_buttons` only. "Any button down" is `raw->pad_buttons != 0`: no button is named (`make button-guard`).

### The module: `src/game/attract.{c,h}`, a pure state machine

`attract.c` decides; it calls no cart function and does not touch the input step. That keeps the dependency one way (`input_step` → `attract`, `input_step` → `cart`) and lets every timing rule be tested without a cart.

Shape (decision-rich part only):

```c
#define ATTRACT_AFTER_DEFAULT 1200

typedef enum { ATTRACT_PHASE_IDLE, ATTRACT_PHASE_PLAYING, ATTRACT_PHASE_SWALLOWING } attract_phase_t;
typedef enum { ATTRACT_ACTION_NONE, ATTRACT_ACTION_START, ATTRACT_ACTION_RETURN } attract_action_t;

typedef struct {
    int attract_after;   /* >= 1 */
    int idle_frames;     /* empty title frames counted since the last reset */
    attract_phase_t phase;
    int demo_frame;      /* next index into demo_data_pad_buttons while playing */
} attract_t;

void attract_init(attract_t *attract, int attract_after);
/* Top of a frame. Returns the action the caller must run before this frame's input step;
   *played_buttons gets the buttons the core plays this frame. */
attract_action_t attract_frame(attract_t *attract, int on_title, int raw_buttons, int *played_buttons);
attract_phase_t attract_phase(const attract_t *attract);
int attract_demo_frame(const attract_t *attract);
/* The compiled record, decoded into the module's static storage; asserts it decodes. */
const save_state_t *attract_demo_record(void);
```

- No pointer field, no file-scope pointer: `make pointer-guard` keeps its counts. The decoded record is a file-scope `static save_state_t` (as `cart.c`'s `g_save_state`), decoded at each start: deterministic, no cache to invalidate. A decode failure is an `assert` with a message (the compiled record is proven by `test_demo_data` and by the tests below).
- `attract_init` asserts `attract_after >= 1` (the reader refuses 0, and a threshold of 0 would start a demo with no `cart_update60` since the return, against `flip_zone_flip`'s assert).

**Transition table** (evaluated once per frame, at its top; `b` = raw buttons, `t` = on the title):

| Phase | Condition | Action | Played buttons | Then |
|---|---|---|---|---|
| IDLE | `b != 0` or `!t` | none | `b` | `idle_frames = 0` |
| IDLE | `b == 0`, `t`, `idle_frames >= attract_after` | START | `demo[0]` | PLAYING, `demo_frame = 1` |
| IDLE | `b == 0`, `t`, otherwise | none | 0 | `idle_frames++` |
| PLAYING | `b != 0` | RETURN | 0 | SWALLOWING, `idle_frames = 0` |
| PLAYING | `b == 0`, `demo_frame == demo_data_frame_count` | RETURN | 0 | IDLE, `idle_frames = 0` |
| PLAYING | `b == 0`, otherwise | none | `demo[demo_frame]` | `demo_frame++` |
| SWALLOWING | `b != 0` | none | 0 | stays |
| SWALLOWING | `b == 0` | (as IDLE) | (as IDLE) | IDLE, then the IDLE rows apply to this frame |

Consequences, each a test:
- With `attract_after = N`, frames 0 … N-1 of an empty title are played empty and frame N is the demo's frame 0 (step 12's `"attract_after": 3` starts the demo on frame 3).
- Frame 0 of the demo is the first frame after the resume, exactly as step 5's contract and `replay`'s `"start": "resume"` play it.
- The demo's last frame (`demo_data_frame_count - 1`) is played; the frame after it returns, played empty on the title.
- Between any return and the next start, at least `attract_after + 1 >= 2` frames run `cart_update60` (step 4 item 6).
- The interrupting frame and every frame with a button still down play empty; the first all-up frame counts as the first idle frame.
- A press on the return frame of a completed demo is an interruption (PLAYING's first row wins), so it is swallowed too.

### Where the state lives, and the threshold

- **`input_step_t` gains `attract_t attract`**, by value. The input step is the per-session object every driver and test already owns, so there is no global attract state to reset between sessions in one process.
- **`input_step_init(step, table)`**: unchanged signature; also `attract_init(&step->attract, ATTRACT_AFTER_DEFAULT)`. `pad_author`, which writes no `"attract_after"`, thus runs the same default threshold that `replay` reads for its output.
- **New `input_step_set_attract_after(input_step_t *step, int frames)`**: `attract_init` with that threshold (phase IDLE, count 0). Called by the drivers right after their init.
- **`input_step_switch_bindings` (step 9) keeps `attract` as well as `session_bindings`**: the switch is called by the start and the return, in the middle of the attract machine's own transitions. Its implementation shape (save, `input_step_init`, restore) saves and restores both. This widens step 9's contract by one field (H-9-3).
- **Read-only accessors for tools and tests**: `input_step_attract_phase(const input_step_t *)` and `input_step_attract_demo_frame(const input_step_t *)`, thin wrappers over `attract_phase` / `attract_demo_frame`. Tools never read the struct fields directly.
- **One default constant**: `REPLAY_SOURCE_ATTRACT_AFTER_DEFAULT` becomes defined as `ATTRACT_AFTER_DEFAULT` (`replay_source.h` includes `game/attract.h`, a devtools-to-core include as `record_start` already does). Value unchanged, 1200.
- **Drivers** (one line each, configuration only, no attract logic):
  - `game.c`: after `input_step_init(&step, &bindings_compiled)`, `input_step_set_attract_after(&step, replay_source_attract_after(&replay))` (1200 with no recording).
  - `replay.c`: after the initial init (title start) or the initial resume (below), `input_step_set_attract_after(&step, replay_source_attract_after(&source))`.
  - `pad_author`: unchanged (default).
- **Recordings keep writing 1200** (plan rule, step 3). Known limit, accepted: a `game` session that starts by replaying a file declaring another threshold (a test file) runs that threshold but records 1200, so its recording would not replay identically. Only test files declare one; writing the session's threshold instead would contradict the plan's "game writes 1200 in every recording". Recorded here, not fixed.

### "The title": `cart_title_shown()`

- New `int cart_title_shown(void)` in `cart`: 1 when the title built by `cart_init` is up and no game has started since, and no box is open.
  - "No game running": a cart-file flag set at the end of `cart_init`, cleared by `start_world` (so by `cart_start_game` and `cart_resume_game`, hence by the demo's own start) and by `cart_load`.
  - "No box open": `shown_message_box() == NULL` and no live `TAG_MESSAGE_BOX` entity (the condition `save_check` already uses). The title's rows are buttons opened by `message_box_open_box`, not a shown message box, so the bare title counts.
- Why a flag and not `main_economy_typed() == NULL`: unit tests that run `input_step_run` on a bare world (no `cart_init`, e.g. `test_pad_nav`) have no economy either; with the flag they never count, so no test can start a demo in a world with no title.
- `input_step_run` passes `cart_title_shown()` to `attract_frame`. `input_step.c` and `attract.c` are listed in `CART_ONLY_SRCS` (`attract.c` is new; `input_step.c` already is).

### One start and one return, in the core, shared with `record_start`

- **New in `input_step`** (it owns the switch and may call `cart`):
  - `void input_step_start_record(input_step_t *step, const save_state_t *state)`: `input_step_switch_bindings(step, INPUT_STEP_BINDINGS_DEFAULT)`, then `cart_resume_game(state)`.
  - `void input_step_return_to_title(input_step_t *step)`: `cart_return_to_title()`, then `input_step_switch_bindings(step, INPUT_STEP_BINDINGS_SESSION)`.
  - Both at the top of a frame only (step 4's contract); the header says so. `pad_nav_init` reads no world state, so the order between the switch and the cart call is free; step 4's order is kept anyway.
- **`input_step_run`** runs, on `ATTRACT_ACTION_START`: `input_step_start_record(step, attract_demo_record())` then `cart_cut_save()`; on `ATTRACT_ACTION_RETURN`: `input_step_return_to_title(step)`.
- **`record_start` keeps only the hex reading and the decode**:
  - `record_start_resume(step, bytes, count)`: decode (returns 0 and touches nothing when it fails, as today), then `input_step_start_record(step, &decoded)`. New precondition: `step` was initialised by `input_step_init` (on the default table for the tools). Its `!bindings_compiled_personal` assertion goes: the switch names `bindings_default` itself.
  - `record_start_return(step)`: `input_step_return_to_title(step)`.
  - `replay.c`'s resume path calls `input_step_init(&step, &bindings)` before `record_start_resume` (a `"start": "resume"` file is already refused with a `"bindings"` member, so `bindings` is the default table). `pad_author` already inits its step before `from-record`.
- **The one difference, by design**: `record_start` does not cut the save. `make replay SAVE_HEX=…` and `make state` on a record-based file keep their current behaviour; the cut is an attract rule ("while the demo plays"), not a property of the start. The attract mode adds exactly one call (`cart_cut_save`) after the shared start.
- `record_start_resume` and `record_start_return` keep the attract state (the switch keeps it). `REPLAY_STARTS=2`'s 30 title frames between starts run the file's threshold; a `"start": "resume"` file never declares one (`pad_author` does not write it), so they run 1200 and never start a demo.

### The save cut-off

- `cart` gains a file-scope flag and `void cart_cut_save(void)`. `cart_update60` skips `save_at_frame_end` while the flag is set. Only `cart_return_to_title` and `cart_load` clear it.
- Not cleared by `cart_init`: if gameplay drift ever made the demo reach "again?" or pick "new game", the player's save must still not be written or erased; only the attract's return ends the cut. `demo-check` catches such drift anyway.
- `cart_resume_game` and `cart_return_to_title` write nothing; `cart_init` only reads storage. So the bytes on disk before a start are the bytes after the return, which the tests compare.

### The demo unit in the core

- **`$(DEMO_UNIT)` joins `ENGINE_SRCS`** (as step 9 adds its `BINDINGS_CORE_UNIT`), and **`CART_ONLY_SRCS`**, so `DATA_ENGINE_SRCS` binaries do not depend on the demo generator. Every binary built from `ENGINE_SRCS` (`replay`, `pad_author`, `game`, the engine tests, their asan/sanitize/coverage trees) holds it.
- **Removed from `GAME_SRCS`'s explicit list**: `tmp/game` is compiled from sources in one `$(CC)` command, so a second listing would define every demo symbol twice. Proof recorded in the step note: `$(DEMO_UNIT)` appears exactly once in `GAME_SRCS`'s expansion (`make --eval` printing it, or `make -p -q tmp/game`), and `make raylib` links.
- **Removed from `test_demo_data`'s rule** (now redundant; `$^` would dedupe it, but one listing is the rule).
- **`CORE_SRCS` filters `$(DEMO_UNIT)` out**: `float-grep`, `float-guard`, `alloc-guard` and `constants-check` read `CORE_SRCS` with no prerequisite on generated files (same reason as step 9's filter). The unit is integer data.
- **`tools/float_grep_exclusions.txt`: delete the `src/game/demo_data.h` line** in the same commit that makes `attract.c` include it, or `float-grep` exits 2 on "both".
- **`.d` includes**: add `tmp/demo/*.d` to the asan, sanitize and coverage `-include` lines (the default tree already has it), so a change to `demo_data.h` rebuilds the demo object in every tree.
- No build cycle: `demo_generate` links host modules only (no `ENGINE_SRCS`, no demo unit).
- Rebuild cost: `make pad-author` rewrites `pico8/demo.json`; if that bumps its date, the next build regenerates the demo unit and relinks the engine binaries once. A no-change `make test` still compiles nothing.

### The `"attract_after"` header member writer

- One writer, in `replay_source` (it owns the member's reading and its default): `int replay_source_format_attract_after(char *out, size_t size, int frames)` writing `"attract_after": N` (no leading comma, the format step 3's recorder writes). The session recorder uses it in place of its own `snprintf` (bytes unchanged, so step 3's recorder tests and every recording are byte-identical). The demo-check test uses it to write its full-cycle fixture with `p8_input_save`. No other copy.

### The tool that writes the demo's end state: `replay`'s `ATTRACT_STATE`

- `ATTRACT_STATE=<file>` on a title-start replay: at the end of every frame that played the demo's last frame (read through `input_step_attract_phase` / `input_step_attract_demo_frame`: still PLAYING and `demo_frame == demo_data_frame_count` after `input_step_run`; the return comes at the top of the next frame), `replay` writes the state, as `STATE` writes it with the cheat count, to `<file>.<n>`, `n` counting completed playbacks from 1. An interrupted playback writes nothing.
- Refused, with `replay`'s usual message and exit 2, on a `"start": "resume"` file and with `REPLAY_STARTS=2` (one way to cycle per run). Accepted with `STATE`, a capture, and `PAD_METRICS`.
- The numbered suffix is the playback count, a fact of the run, not a mode of the tool: steps 12 and 13 read `<file>.1`, `<file>.2` instead of computing frame numbers. `make replay` / `make state` pass it through like `REPLAY_STARTS`; the Makefile legend gains a few words for it (the rest of the legend is step 14's).

### Unchanged

- `game.c` and `replay.c`'s frame loops; the session recorder's frames and bytes; `pad_author`'s frames; `bindings_default`, `bindings_compiled`; the draw (step 11). `REPLAY_STARTS=2` keeps its name and output.

### Decisions taken without a human (guideline, `.claude/skills/guideline/SKILL.md`)

- **Hook in `input_step_run`** rather than a new per-frame call in each driver: every driver already calls it at the right point, so `game`, `replay`, `pad_author` and the tests cannot diverge (plan: "called from `input_step` / `cart`", "never in `src/cmd/game.c`").
- **A pure state machine plus thin sequences in `input_step`**, rather than `attract.c` calling the cart and the switch itself: `attract` → `input_step` → `attract` would be a cycle, and the codebase breaks such cycles (help's bridges to cart).
- **State in `input_step_t`**, kept by the switch, rather than a module global: no hidden state between sessions in one process, and no new reset call in the drivers.
- **A cart flag for the title**, rather than "no economy": safe in the bare-world unit tests.
- **Save cut set by the attract, cleared only by the return**: keeps the player's save safe even under gameplay drift.
- **Recordings stay at 1200** (plan's letter); the test-file-in-`game` limit is recorded instead of widening the step.
- **`record_start` does not cut the save**: the tools' `SAVE_HEX` behaviour is unchanged; the start and the return themselves are shared.
- **The full-demo feed outside `make test`** (plan-pilot's accepted method change): `make test` never gets slower.

## Implementation Chunks

1. **The core plumbing, no behaviour change.** Demo unit into `ENGINE_SRCS` / `CART_ONLY_SRCS`, out of `GAME_SRCS` and `test_demo_data`'s rule, filtered from `CORE_SRCS`; `.d` lines; `input_step_start_record` / `input_step_return_to_title`; `record_start` reduced to them (and `replay.c`'s init before the resume); tests of `test_record_start` that froze the old whole-struct equality updated. `cart_title_shown`, `cart_cut_save` and the flag, not called yet. Done when `make test-all`, `make pad-author-proof`, `make demo-check` (after `make test-all`, never alongside), `make raylib` and the single-listing check pass, with no diff in `worst-case/`, `pico8/`, `tests/e2e/`.
2. **The state machine.** `src/game/attract.{c,h}` and `tests/test_attract.c` (pure, no cart). The `demo_data.h` float-grep line deleted here. Not hooked yet. Done when `make test` passes.
3. **The hook.** `attract_t` in `input_step_t`, `input_step_init` default, `input_step_set_attract_after`, the switch keeping `attract`, the accessors, the call at the top of `input_step_run`, `cart_cut_save` on start; `REPLAY_SOURCE_ATTRACT_AFTER_DEFAULT` from `ATTRACT_AFTER_DEFAULT`; the shared member writer used by the recorder; `game.c` and `replay.c` hand the threshold over; integration tests in `make test`. Done when `make test-all` passes and every `.sha256` is unchanged.
4. **The full checks and the tool.** `replay`'s `ATTRACT_STATE`; the demo-check binary (full feed, complete-demo save bytes, interruption sweep, the full-cycle fixture replayed with `ATTRACT_STATE` and checked by `tools/demo_check.sh`); `make demo-check` runs it. Done when `make demo-check` passes, plus every guardrail run below.

## Testing Decisions

- A good test drives the public interface and reads results only: `attract_frame`'s action and played buttons; for the integration, `input_step_run` + `cart_update60` + `cart_draw` frames as the drivers run them, then the storage bytes, the title's buttons by name, the pad navigation's bindings through `pad_nav_bindings`, the state through `state_write` / `save_check`. No test reads `attract_t`'s fields directly.

### `tests/test_attract.c` (new, in `UNIT_TESTS`; pure: links `attract.c`, the demo unit and what `save_state_decode` needs, no cart run)

- With threshold N: N empty title frames return NONE and play 0; frame N returns START and plays `demo_data_pad_buttons[0]`.
- A button down on idle frame k < N plays that button and restarts the count (the start moves by k+1 frames); so does an off-title frame.
- No count off the title: any number of empty frames with `on_title = 0` never starts.
- Substitution: the frames after START play `demo_data_pad_buttons[1]`, `[2]`, …, whatever `on_title` says.
- End: after the last demo frame, the next empty frame returns RETURN and plays 0; the phase is IDLE and the next start comes N+1 frames later (iterating the 2736 frames of the pure machine costs microseconds).
- Interruption at a few demo frames (first, middle, last): RETURN, played 0, phase SWALLOWING.
- Swallow: held buttons play 0 and never start; the first all-up frame counts as idle frame 1.
- `attract_demo_record()` decodes and equals `demo_data_record` re-encoded (or its decode equals `save_state_decode` of the bytes).
- Threshold 1: a return is still followed by at least two frames before the next start.

### Integration, in `make test` (extends `tests/test_cart.c`, which already boots over a player's save, reads storage and finds title buttons; short segments only)

- Over a player's save (the existing `start_game_with_a_save` setup), with threshold 3 set through `input_step_set_attract_after`:
  - 3 empty frames, then the start frame: the live game is the record (`save_check` against `demo_data_record`), the pad navigation is on `bindings_default`, the title buttons are gone.
  - about 150 demo frames, then one button down: the title is back with its `resume` row, the stored bytes and the storage write count are unchanged, the pad navigation is on the session's table.
  - The press held 20 more frames, then released, then 10 empty frames: the title's pad focus and state dump equal those of a title returned with no press (no row moved, no game started: `cart_title_shown()` still 1).
- With a session table that swaps two jump-mode actions (the swap table of `test_input_step`): during the demo `pad_nav_bindings` equals `bindings_default`; after the return it equals the swap table.
- Over an empty storage: after an interrupted demo, no `resume` row and storage still empty.
- A save written during the demo would be visible: a demo segment covering a command range that sets `save_pending` (the fold at frames 450–515, reached in about 520 frames, still short) leaves the storage write count unchanged.
- Threshold from the drivers' path: `replay_source_attract_after` on a fixture with `"attract_after": 3` handed to `input_step_set_attract_after` starts on frame 3 (can live in `test_input_step`, which links `replay_source`).
- `input_step_init` alone: `input_step_attract_phase` is IDLE; `input_step_switch_bindings` keeps the attract phase, count and threshold (extends step 9's switch tests).
- `record_start`'s tests: resume and return now keep the attract state; their "fresh step" comparisons are updated to compare with `input_step_init` followed by the same switch (a test freezing the old reset is updated, `CLAUDE.md`).
- Session recorder: step 3's header-byte tests pass unchanged with the shared writer.
- Budget: these cases add well under a second of `-O0` frames; `make test` is timed against step 1's minimum.

### The demo-check binary (new, `tests/test_attract_demo.c`, **not** in `UNIT_TESTS`, run first by `make demo-check`)

- **Full feed, two playbacks**: over a player's save, threshold 3, empty frames until the demo ends by itself (start + 2736 frames + return), checked at the last demo frame through the accessors: `state_write` shows year 3413, 17 colonies, 19 slipways, 3 techs, 0 cheat activations (from the record's 3412/16/18/2); after the return the title has its `resume` row and the stored bytes and write count are unchanged (the complete-demo save check). Then the second playback: same counts; the game state at its last frame matches the first (`save_check_compare`; the visual stream is never reseeded, step 4 item 1, so screens are not compared).
- **Interruption sweep**, once per command range of step 7's note: one frame inside fold 450–515, colonise 935–955, slipway 1375–1385, year 1805–1850, research 2270–2286 (e.g. 480, 945, 1380, 1830, 2280), each in its own cycle: interrupt, release, a few title frames; the title and its `resume` row are back and the storage bytes are byte-identical. The frames chosen and the result go in the step note.
- **Full-cycle fixture through `replay`**: the binary writes `tmp/demo-check/attract_cycle.json` with `p8_input_save` and the shared member writer (`"attract_after": 3`, empty frames for two complete playbacks plus a few title frames); the recipe replays it with `ATTRACT_STATE=tmp/demo-check/attract.txt` and runs `tools/demo_check.sh pico8/demo.json tmp/demo-check/record.txt tmp/demo-check/attract.txt.1 tmp/demo-check/attract.txt.2`, reusing step 7's count checks (no new shell parsing of `demo.json`).
- `make demo-check` keeps its other two checks; it runs only after `make test-all` finished, never concurrently (the shared `tmp/replay_dump.txt`).

### Guardrail runs (plan section 2; this step changes `input_step`, `cart` and the save path)

- `make test-all`; then, never alongside it, `make demo-check`; `make end-state`, `make full-game`, `make pad-author` and `make pad-author-proof` with no diff in `worst-case/`, `pico8/demo.json`, `tests/e2e/`. Every `tests/e2e/*.sha256` unchanged.
- `make raylib` (raylib is vendored, step 8's note): links `tmp/game`, never run; the `GAME_SRCS` single-listing check recorded.
- `make sanitize-net` and an ASan/UBSan build of `test_cart` and the demo-check binary: clean.
- Timing: `make test` with nothing to rebuild and `make e2e` after `rm -rf tmp/frames`, three runs each, step 1's method, compared with 8.2 s and 1.39 s.
- Prior art: `tests/test_cart.c` (step 4's two-start harness, `save_check`, storage bytes, `title_button`), `tests/test_input_step.c` (swap table, `pad_nav_bindings`), `tests/test_record_start.c`, `tests/test_replay_from_resume.c` (driving `tmp/replay` with env variables), `tests/test_demo_data.c`, `tools/demo_check.sh`.

## Out of Scope

- Drawing the captions or the `demo` label (step 11); `demo_data_captions` is not read here.
- `tests/e2e/attract.json` (step 12); the `worst-case/`-style full-cycle replay with its hash and the default 1200 threshold (step 13); the README and the full Makefile legend (step 14).
- Writing the session's threshold in recordings, or refusing `"attract_after"` in `"start": "resume"` files.
- Teaching `pad_author` about the attract mode (it runs the default threshold; no script idles 1200 frames on the title).
- Any change to `bindings_default`, the bindings generator, `demo_generate` or `pico8/demo.json`.

## Further Notes

- **Why no existing hash moves**: with the default 1200, a demo needs 1200 consecutive empty title frames. Measured on every file: the longest run of empty frames anywhere is 59 in `tests/e2e/` (`pad_message_box.json`), 301 in `worst-case/` (`full_game_pause_idle.json`, in a game), 300 in `title_idle.json`; no file declares `"attract_after"`. Existing unit tests that run frames idle far less on the title (step 4's harness: 37).
- Step 4's list, as handled here: 1 visual stream — accepted, screens not compared between playbacks; 2 game random state — reseeded by `cart_resume_game`; 3 entity ids — never compared; 4 input step — reset by the switch at start and return; 5 UI slots — `cart_return_to_title`; 6 pending generation — at least `attract_after + 1` updates between return and start; 7 stored save — cut; 8 peak counters — excluded from comparisons; 9 `economy_score`'s difficulty and the year banner's keep-open counter — the demo record sets the difficulty at each start, and the banner is covered by the second playback's state matching the first.
- `pad_author` runs the attract machine like every driver; a future script idling 1200 frames on the title would start the demo in `pad_author` and in `replay` alike, so they stay consistent.
- The step note records: the single-listing proof, the interruption sweep frames, the timings, the guardrail runs, and any deviation.

### Hypotheses

Step 9 (`docs/prd/triage/phase2-step9-default-table.md`) is being implemented while this PRD is written. What this PRD assumes about its outcome; if step 9's note says otherwise, the note wins and only the named part of this PRD follows it.

- **H-9-1 (symbol):** `extern const bindings_t bindings_default;` is declared in the bindings header and defined by a generated unit (`BINDINGS_CORE_UNIT`, `tmp/bindings/core.c`) listed in `ENGINE_SRCS` and filtered out of `CORE_SRCS`. The demo unit follows the same pattern next to it.
- **H-9-2 (switch):** `input_step_switch_bindings(step, INPUT_STEP_BINDINGS_DEFAULT | INPUT_STEP_BINDINGS_SESSION)` exists, resets the step as `input_step_init` on the chosen table does, and is implemented as "save the session table, `input_step_init`, restore". If it is shaped differently, this step still makes it keep `attract`.
- **H-9-3 (widened contract):** step 9's switch keeps only `session_bindings`; this step makes it keep `attract` too and updates step 9's "byte-identical to a fresh init followed by the same switch" test accordingly (both sides then carry the same attract state).
- **H-9-4 (`session_bindings`):** `input_step_t` holds `bindings_t session_bindings` by value; `input_step_init` copies the caller's table into it. `replay`'s fixture table comes back after a demo through it.
- **H-9-5 (no callers):** step 9 leaves `game.c`, `replay.c`, `pad_author`, `record_start` and their `input_step_init` calls unchanged, and `record_start` keeps its personal-table assertion, which this step removes.
- **H-9-6 (Makefile):** step 9 does not touch `GAME_SRCS`, `CART_ONLY_SRCS`, the `-include` lines or `test_demo_data`'s rule; its `CORE_SRCS` filter is a `filter-out` this step extends with `$(DEMO_UNIT)`.
- **H-9-7 (timing reference):** step 9's note holds the latest `make test` / `make e2e` timings; this step compares with step 1's minimums and step 9's range.
