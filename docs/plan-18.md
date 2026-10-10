# Plan 18 — Attract mode: a captioned demo on the title screen

## 1. Goal

Left alone on the title screen, the game plays a short captioned demo of itself, like an arcade attract mode, which teaches what the game is about. Any button brings the title back, with the player's save untouched.

Goals that justify every rule below:

- **The demo is the real game.** It is a recorded pad stream played by the engine, never a video. It always matches the current code, and a gameplay change that breaks it is caught by a check.
- **The e2e replays stay the source of truth.** The attract mode lives in the core, so `game`, `replay` and the e2e behave the same, and every existing `*.sha256` stays unchanged.
- **One mechanism on every platform.** The demo is compiled in: the PSX (paused, not abandoned) cannot open a JSON file.
- **The demo never lies.** No cheat, no button name that may be wrong for the player, no money from nowhere.

## 2. Rules

### Guardrails

- `make test-all` passes on every commit. `make test` and `make e2e` never get slower (`CLAUDE.md`). Every slow check of this plan goes in `make demo-check`, outside `make test-all`.
- Every existing `tests/e2e/*.json.sha256`, every `.end-state` and every `worst-case/` file stays unchanged by every step. No existing replay idles 1200 frames on the title (e2e replays are capped at 600 frames, `worst-case/title_idle.json` idles 300), so none may start the demo. A step that changes one is wrong. Never re-baseline to make it pass.
- `make test-all` does not run the `worst-case/` games. Every step that changes the core (step 4 is the first likely one) (`src/game/`, `input_step`, the draw, the save path) also runs, outside `make test-all`: `make end-state`, `make full-game`, and `make pad-author` with no diff in `worst-case/` or `pico8/demo.json`. From step 10 on, such a step also runs `make raylib` (link only, never run; raylib is vendored since step 8) and checks that `$(DEMO_UNIT)` appears exactly once in `GAME_SRCS`'s expansion. A step's `make demo-check` runs only after its `make test-all` has finished, never at the same time (both write `tmp/replay_dump.txt`).
- Reuse (from step 10 on): the compiled demo is read through `demo_data.h`, replays through `replay_source`'s accessors, states through `tools/end_state.sh`; a tool that must write `"attract_after"` shares one header-member writer with `session_recorder` rather than adding a copy; no new shell parsing of `demo.json`.
- Never launch the raylib app (`CLAUDE.md`). `make raylib` only compiles and links. Only a human watches the demo in `game`.
- Everything written is in English. Comments follow `CLAUDE.md`'s comment rule.

### Prerequisite

- This plan builds on plan 17 (remappable pad, branch `claude/loving-brahmagupta-ulwvej` while in progress): replays with button numbers (`"pad": [5, 7]`), `bindings.default.cfg`, the `title` mode, the host generator pattern of `src/cmd/bindings_generate.c` writing a C unit in `tmp/`, and a personal `bindings.cfg` compiled into `game` only. The user merges plan 17 into `master` (merging is the user's job, never an agent's). Step 1 checks it and stops if it is not done.

### Locked decisions

**Trigger and cycle**

- **Attract mode, not a tutorial.** There is no "how to play" row. The demo starts by itself after **1200 frames (20 s)** with no button down on the title, and loops: title, 20 s, demo, title, 20 s, demo…
- **What counts as the title:** no game running and no box open. Any button down there resets the counter.
- **Interruption:** any button down during the demo returns to the title. That press is **swallowed**: the core sees empty frames until every button is released, so it neither moves a row nor picks one on the title.
- **End of the demo:** after the closing card, return to the title and restart the counter.
- **Return to the title** goes through the path the game already uses to rebuild the title (`cart_init`, as "again?" and restart do), which re-reads the disk save: the `resume` row comes back exactly as before the demo.

**The demo runs live, in the core**

- The demo is a **recorded pad stream fed to the engine** in place of the player's frames, starting from a **save record compiled in**, loaded with `cart_resume_game` (which reseeds the RNG from the record, so the galaxy does not depend on how long the title ran).
- **The attract mode lives in the core** (`src/game/attract.{c,h}`, called from `input_step` / `cart`), driven only by the input frames, never in `src/cmd/game.c`. Otherwise a `game` session where the player waited on the title would replay differently in `replay`. The session recorder keeps the **raw** frames (empty during the demo, then the interrupting press).
- **The save is cut off during the demo:** `cart_update60` skips `save_at_frame_end` while the demo plays. Without this the demo would overwrite or erase the player's game (`save_at_frame_end` writes after every action and erases at a game's end).
- **The demo always plays on the default bindings.** `game` links both its compiled table (personal or default) and the default table; during the demo `pad_nav` uses the default, and the compiled table again on return. A personal `bindings.cfg` must never make the demo play nonsense.

**Content**

- **A single segment from a mid-game record.** The record has a few colonies, enough science for a tech, unexplored space, a planet to colonize and two colonies that can be linked. The demo does not show the empty first minute of a new game: accepted.
- **Beats, in order:** opening card, probe (and fold), colonize, slipway, end of year (Select hold), **buy a tech**, closing card.
- **Captions, exactly** (lowercase like the cart's own texts; at most 2 lines of at most 32 characters; no button name; no letter `v`):

| Beat | Line 1 | Line 2 |
|---|---|---|
| Opening | `build a trading empire` | `across the stars` |
| Probe | `probe the dark` | `to find new planets` |
| Colonize | `colonize planets:` | `each one needs and makes goods` |
| Slipway | `link colonies with slipways` | `so goods can flow` |
| End of year | `end the year:` | `trade earns money and science` |
| Tech | `spend science on new tech` | |
| Closing | `slipways` | `press any button` |

- **Trap — the letter `v`:** `src/p8/gfx.c` draws `v` with the `u` glyph (`case 'V': case 'v': return FONT_ROWS_U;`), so "discover" would read "discouer". Captions avoid `v`, and the generator refuses it.
- **Pacing:** each caption appears at least **90 frames** before the first button press that follows it, and stays at least **150 frames**. The closing card stays **180 frames**. The demo lasts at most **3600 frames (60 s)**; aim for about 2700 (45 s).

**Display**

- Captions are drawn **in the PICO-8 framebuffer** with `p8_print`, on a dark band, after the game's draw and inside the core's draw, so the headless dump and the e2e hashes see them and the PSX gets them. The band must not cover the top HUD (buttons at y≈1) or the build bar (icons at y≈116); its exact place is fixed in the step that draws it.
- A **`demo` label** sits in a corner during the whole playback, so nobody takes it for their own game.

**Authoring and data**

- **Written by an agent as a `pad_author` script**, not played by a human: it is reproducible and regenerated after a gameplay change. The high-level commands exist (`probe`, `fold`, `colonise`, `slipway`, `year`, `research`, `idle`); low-level `tap` / `hold` only where none fits.
- **The starting record** is played by a prefix of `worst-case/full_game.pad` (`include … until <label>`), cut at a year that meets the content conditions above, then **frozen** into a committed file ("cut, never typed", as `tests/e2e/pad_tech_costs.txt` describes its record). Regenerating `full_game` never moves the demo. The command that produced the record is written in the demo's `.txt`.
- **The demo script** `pico8/demo.pad` starts from that record and adds a **`caption <text>`** command (a line break written `/`; `caption -` clears it). `pad_author` computes each caption's frame range itself, so a regenerated demo keeps its captions aligned, and refuses a script that breaks the pacing rule.
- **Chain:** `pico8/demo.pad` → `pad_author` → `pico8/demo.json` (committed, reviewable: `initial_save`, `frames`, `captions` as `{from, to, text}`) → host generator `src/cmd/demo_generate.c` → `tmp/demo/demo.c` (a C unit, rebuilt by `make`, never committed), linked into the core. Same pattern as plan 17's bindings: no generated file is committed, so no sync check is needed.

**Verification**

- **Header member `"attract_after": N`** in replays and recordings: the core's idle threshold. Missing means 1200. `game` writes `1200` in every recording, so its sessions replay faithfully. Only test files declare a short threshold.
- **`make e2e`** gets one new replay, `tests/e2e/attract.json`, under 180 frames, with `"attract_after": 3`: the demo starts (record loaded, first caption, `demo` label), plays about 150 frames, a press returns to the title, the press is swallowed, the `resume` row is there and the disk save is unchanged.
- **`make demo-check`**, outside `make test-all`: the full cycle (idle, the whole demo, return, a second start) replayed to its end with the `-O0` sanitize binary as `make full-game` does, its own hash, and `pad_author` regenerating `pico8/demo.json` byte for byte. plan-pilot runs it in every step that touches the demo, `src/game/pad_*`, `src/devtools/pad_*` or gameplay, like plan 17 runs the generator proofs.

### Rejected

- **An explicit "how to play" row (tutorial):** the user wants a passive teaser. A tutorial would need pacing, skipping and resuming.
- **Pre-rendered frames (a video):** about 15 MB for 30 s before compression, a new format, and it goes stale silently.
- **A new game with a fixed seed plus a silent cheat for the tech:** the teaching demo would lie about the money.
- **A new game for the first beats, then a cut to a record for the tech:** two segments, two sources, a transition to write.
- **Captions as a raylib overlay:** invisible to headless checks and absent on PSX.
- **Button glyphs (× ○ △ □) in captions, or placeholders resolved against the player's bindings:** a personal `bindings.cfg` makes them wrong or needs a keymap-dependent glyph, and plan 17 removes the PSX vocabulary. The demo shows *what* to do; the README and `bindings.default.cfg` say *with which button*.
- **Captions triggered by game events:** more code; drift is caught by `demo-check` anyway, and `caption` in the script keeps frames aligned.
- **Loading `demo.json` at runtime:** the PSX has no file system for it.
- **A committed generated header with a sync check:** plan 17's generated-unit pattern makes it unneeded.
- **The attract mode in the `game` driver:** recordings would diverge in `replay`.
- **A test-only threshold compiled into `replay`:** `replay` would no longer replay `game`'s recordings faithfully. The header member is per file.
- **The full demo in `make e2e`, or a new frame tier in `tools/e2e_limits.sh`:** it would make `make e2e` slower.
- **No full-demo check at all:** a gameplay change would break the demo unseen.
- **A demo played and recorded by a human:** depends on the user's time and takes, and cannot be regenerated.

### Out of scope

- Fixing the `v` glyph of the font: a separate fidelity question, which may change e2e hashes.
- An in-game choice of the timeout or of the demo.
- Audio (`CLAUDE.md`).
- The PSX build itself: the demo data is compiled in and ready, nothing more.

## 3. Phases and steps

### Phase 0 — Base

1. **Base check.** Plan 17 is in `master` (its bindings module, `bindings.default.cfg`, button-number replays), and this branch is based on it. `make test-all` passes. Time `make test` and `make e2e` on this base: the baseline of every "not slower" check. If plan 17 is not merged, stop and ask the user. Verified by the commands' output and the timings in the step note.

### Phase 1 — Demo data

2. **The starting record.** Add a label to `worst-case/full_game.pad` (a label is clock-free: `make pad-author` must regenerate `worst-case/full_game.json` with no diff) at a year where, as `make state` shows, a tech is affordable from science, unexplored space remains reachable by a probe, an uncolonized planet is reachable, and two colonies can be linked. Cut the record there with `make replay SAVE_HEX=…` and commit it as `pico8/demo_record.hex`. Nothing in it depends on the tool steps after it. If no year of `full_game` meets all four conditions, stop and ask the user: never relax a condition or take the record from elsewhere. Verified by `make state` on the record and the four conditions written in the step note with their numbers.
3. **`attract_after` header member.** The replay reader (`src/devtools/replay_source.c`, `src/devtools/input_json.c`) reads `"attract_after"` (a positive integer; anything else is a load error) and exposes it, 1200 when missing; the session recorder writes `"attract_after": 1200`. Nothing uses the value yet. Verified by unit tests (missing, present, invalid) and every `.sha256` unchanged.
4. **Core proof: two attract starts in one process.** On `pico8/demo_record.hex` alone, before any tooling, a test harness runs in one process: some idle title frames, `cart_resume_game` of the record from the input path, a few frames, a return through `cart_init`, more idle title frames, then the same resume again (today these only run from button handlers inside `ui_update`). The pad navigation state at each start is the one the attract start will have. Verified: the second start's game state matches the first, and the title after each return is the one `cart_init` builds; the step note lists each piece of state the proof showed `cart_init` and the resume do not rebuild (the visual stream, entity ids, zones, …), and step 10 takes that list as an input. If the proof cannot pass without departing from a section 2 decision (the resume through `cart_resume_game`, the return through `cart_init`, the attract mode in the core), stop and ask the user, with what the proof showed.
5. **`pad_author` starts from a record, at the attract mode's start point.** If `pad_author` cannot yet start a script from a save record (as `pad_tech_costs` starts from its `initial_save`), add it: a script command naming a committed hex file, written into the output JSON's `initial_save`. Contract, fixed here because the attract mode feeds frames straight after `cart_resume_game`, not from the title: a record-based script's output frames start at the first frame after the resume, with the pad navigation state of step 4, never with a title prefix that navigates to `resume`. This step also delivers, and names in its note, **the one tool that plays a record-based output from the resume** (step 4's harness, or `replay` and `make state` taught to start such a file by the resume instead of the title): today `replay` only writes `initial_save` to storage and starts on the title, so it would play the demo's frames on the title. Every later check of the demo's content uses that tool. Verified by a proof script in `tools/pad_author_proofs/`, run on `pico8/demo_record.hex`, which plays section 2's beats in order with the existing high-level commands, without captions but with idle gaps of about the pacing sizes (the rough total goes in the step note; if a beat fails from the record or the total exceeds 3600 frames, go back to step 2 and cut at another year, under its stop rule), and whose output frames, fed by step 4's harness through its two-start cycle, reach `pad_author`'s own end state both times; and `make pad-author-proof` plus `make pad-author` with no diff in `worst-case/`.
6. **`caption` command.** `pad_author` accepts `caption <text>` (`/` breaks the line, `caption -` clears) and writes `"captions": [{"from": F, "to": T, "text": "…"}]` into its JSON; the replay reader accepts and ignores the member. It refuses, with the generator's usual `file:line` message and a `.refused.pad` proof each: a line over 32 characters, more than 2 lines, the letter `v`, a character the font has no glyph for, a caption shown fewer than 150 frames, or a button press fewer than 90 frames after a caption appears. Verified by the proofs, `make pad-author-proof`, and `make pad-author` with no diff in `worst-case/`.
7. **The demo script.** Write `pico8/demo.pad` from the record with the beats and captions of section 2, and generate the committed `pico8/demo.json`, with a `pico8/demo.txt` describing each beat's frames, as e2e `.txt` files do, and the command that cut the record. If the demo cannot meet the beats, the pacing or the 3600-frame limit, stop and ask the user rather than relax a number or a condition. Verified with step 5's tool at the end of `demo.json`: one more colony, one more slipway, one more year, one more invented tech than the record, no cheat frame; total frames at most 3600; captions in section 2's order and text. The step also creates `make demo-check`, outside `make test-all`, with two checks so the plan-pilot rule can run from step 8 on: `pad_author` regenerates `pico8/demo.json` byte for byte, and playing `demo.json` with step 5's tool reaches those counts.
8. **Compiled demo.** `src/cmd/demo_generate.c` turns `pico8/demo.json` into `tmp/demo/demo.c` (the record's bytes, one button mask per frame, the captions), following `bindings_generate`'s pattern, with a Makefile rule that rebuilds it when the JSON changes. It refuses what step 6 refuses (a hand-edited JSON) with `file: message`. Verified by a unit test that the compiled data equals the JSON read by the replay reader, a no-change `make raylib` recompiling nothing, and `make test` not slower.

### Phase 2 — Attract mode in the core

9. **Default table beside the compiled one.** `game` links the default bindings table as well as its compiled one, and `input_step` can switch `pad_nav` between them. Nothing switches yet. Verified as plan 17 step 7 verified its tables, on the built binaries without launching anything: with a swapped personal `bindings.cfg`, `game` holds both the personal and the default table; `replay` holds the default. Every `.sha256` unchanged.
10. **Attract state machine.** Starting from step 4's list of state not rebuilt, `src/game/attract.{c,h}`: title idle counter, threshold from the header member, reset on any button down, start, frame substitution from the compiled demo, interruption, swallowed press until every button is up, end after the last frame, return to the title through `cart_init`, `save_at_frame_end` skipped while playing, default bindings while playing. Not drawn yet. Verified by unit tests through the module's interface for each of these, including: a disk save written before the demo is byte-identical after an interrupted demo and after a complete one, and the title has its `resume` row again; a press held from the demo does not move or pick a title row; and feeding the whole compiled demo through the attract module reaches step 7's end state (the counts step 5's tool gave at the end of `demo.json`). Every existing `.sha256` unchanged. Method (plan-review after phase 1): the full-demo feed runs in a test binary of `make demo-check`, not in `make test`; the start and return sequence lives once in the core and `record_start_resume`/`record_start_return` become thin calls to it; the step fixes and names how a tool reads the attract phase and demo frame from outside the module and which tool writes the state on the demo's last frame before the return in a title-start replay (steps 12 and 13 use it); and, once, recorded in the note, the demo is interrupted at one frame inside each command range of step 7's note, each followed by the return, the title and `resume` row check and a byte comparison of the disk save.
11. **Captions and label.** Draw the current caption on its band and the `demo` label, in the core's draw. Choose the band's place (not over the top HUD or the build bar) and write it in the step note. Verified by a unit test reading the framebuffer for a caption frame and a captionless frame, and every existing `.sha256` unchanged.
12. **The attract e2e.** Add `tests/e2e/attract.json` (under 180 frames, `"attract_after": 3`, an `initial_save` so the `resume` row is checked) with its `.txt`, `.sha256` and end state, covering section 2's `make e2e` line. Verified by `make e2e`, `tools/e2e_limits.sh` passing without an exception, and the `.txt` naming the frames that show the demo start, the first caption, the return and the swallowed press. `make e2e` not slower than step 1's timing beyond this file's own frames.

### Phase 3 — Full check and docs

13. **`make demo-check`, full cycle.** Extend step 7's target with a `worst-case/`-style replay of the full cycle (default threshold: 1200 idle frames, the whole demo, the return, a second start), the target outside `make test-all` that replays it with the `-O0` sanitize binary to its end, checks its hash, and checks the content before trusting the hash: at the end of each demo playback the state shows step 7's counts (one more colony, slipway, year and tech than the record), and after each return the title is back with the disk save unchanged. The second start need not draw like the first (the visual stream is never reseeded), but its game state must match. Verified by the target passing, then failing once (recorded in the step note, not committed) when one frame of `demo.json` is altered. Method: the target also runs step 10's interruption sweep, and passes an explicit dump path under `tmp/demo-check/` to every `replay` it launches.
14. **Docs.** The README gains one short paragraph: left alone 20 s, the title plays a demo; any button returns. The Makefile legend lists `demo-check`. `docs/` describes nothing finished. Verified by reading.
15. skill: rework-comments — the files this plan created or rewrote: the source, test and tool files of `git diff --name-only 6e5584c..HEAD -- src tests tools`, computed at this step and listed in its note (generated `tmp/` files excluded).

## 4. Done when

- `make test-all` passes, every `tests/e2e/*.json.sha256` from before step 2 is unchanged, and `tests/e2e/attract.json` is green.
- `make demo-check` passes.
- `make pad-author` regenerates `worst-case/` and `pico8/demo.json` with no diff.
- With a personal `bindings.cfg` that swaps two actions, `make raylib` links a `game` that holds both tables, and `make test-all` and `make demo-check` are unchanged.
- `make test` and `make e2e` are not slower than at step 1, apart from `attract.json`'s own frames in `make e2e` (timings in step 1's and the last step's notes).
- A human has watched the demo in `game` and accepted it (asked for at the end, never done by an agent).

## 5. Open questions

(none)

## 6. Step notes

### Step 1

- Plan 17 is in `master` (#67, then #69 and c92cfc5); this branch fast-forwarded to `origin/master` (c92cfc5). `bindings.default.cfg`, `src/game/bindings.{c,h}`, `src/cmd/bindings_generate.c` and button-number replays are present.
- `make test-all` passes: 19 e2e green.
- Baseline timings (4 cores, wall clock, three runs, a background read-only agent running at the same time): `make test` with nothing to rebuild 8.2 / 8.8 / 9.2 s; `make e2e` after `rm -rf tmp/frames` (every replay re-run, binaries built) 1.78 / 1.46 / 1.39 s. Later "not slower" checks use the same method and compare with the minimum (8.2 s, 1.39 s).

### Step 2

- Label `demo-record` added to `worst-case/full_game.pad` after the 7th `bar` (year 3411's end browsing), before `research slipgates`; `make pad-author` regenerates `worst-case/` with no diff.
- Cut: `make pad-author SCRIPT=worst-case/full_game.pad STOP_AT=demo-record OUT=tmp/demo_record.json` then `make replay EVENTS=tmp/demo_record.json SAVE_HEX=pico8/demo_record.hex`. 3852 bytes, sha256 `61c39e56…83ae`, re-cut byte-identical.
- `make state` on the record: year 3412 month 1, credits 86, science 12 (+5/year), techs space folding and geneseeds, 16 colonies (14 colonized planets), 18 slipways, 49 unknown and 9 known planets, a live probe, p46 still `vanishing` (gone one frame later; accepted).
- The four conditions, each checked with `pad_author --try` from the label, no cheat: tech affordable — `research xenofoods` (10) and `research slipgates` (12) succeed; unexplored space — `fold 118 307` costs 3 and reveals 3 planets (49 → 46 unknown); planet to colonize — `colonise p32` succeeds (6 of 9 known planets work); two colonies to link — `slipway p23 p33` succeeds (17 unlinked pairs possible).
- The whole beat line `fold 118 307; idle 120; colonise p32; slipway p32 p55; year; research xenofoods` succeeds: year 3413, 17 colonies, 19 slipways, 3 techs.
- PRD decisions: the probe beat is a `fold` (a probe is already live and `pad_author` refuses a second); "two colonies can be linked" counts colonies already in the record. 3410, 3414, 3419, 3420, 3422, 3424 and 3425 also met all four conditions; 3412 has more science margin than 3410 and is less crowded than the later years.
- Bookkeeping: this note landed one commit late (in step 3's first commit), after a refused amend broke the command chain.

### Step 3

- `replay_source` parses `"attract_after"` from the header only (before `"frames"`): digits 1-9 then digits, at most `INT_MAX`, followed by a separator; `0`, signs, leading zeros, fractions, exponents, strings, `null`, overflow and a repeated member fail the load. `replay_source_attract_after()` returns `REPLAY_SOURCE_ATTRACT_AFTER_DEFAULT` (1200) when missing, with no recording, after a failed load and after close.
- The session recorder writes `"attract_after": 1200` as the first header member of every recording, then `bindings`, then `initial_save`.
- Deviation: `src/devtools/input_json.c` changes by one assertion (the PRD said none): `p8_input_append` required a header member ending in `"`; it now refuses a trailing comma or whitespace, its stated intent. `p8_input_save` and `pad_author` output are untouched, so `worst-case/` has no diff.
- Three tests of `test_recording_continuity.c` froze the old header bytes and were updated.
- `make test` 8.6–8.8 s (the step 1 baseline method gives 8.2–9.2 s); the new cases cost a few ms. Every `.sha256` unchanged.

### Step 4

- `tests/test_cart.c` proves two attract starts in one process on `pico8/demo_record.hex`: 30 idle, start, 30 played, return, 37 idle, start, 30 played, return. The starts match (save bytes, dump, screen); each returned title equals the boot title, keeps the `resume` row, and storage is unchanged. A tech-panel case and an empty-storage case (no `resume` row) too. Binary 0.27 s.
- New `cart_return_to_title` (release live entities, flush the exit queue, forget registries, then `cart_init`): a bare `cart_init` with the tech panel open (11 live buttons) stops on `p8_fatal: zone ui button full, limit 15 slots`. Guideline decision: this still is "the return through `cart_init`" (the title is built by `cart_init`; "again?" and restart are unchanged, they never run with the panel open), so the stop rule does not trigger. Only tests call it until step 10.
- Contracts for steps 5 and 10: the start (`input_step_init` on the default table, then `cart_resume_game`) and the return (`cart_return_to_title`, then `input_step_init` on the compiled table) run at the top of a frame, before that frame's input step. At least one `cart_update60` must come between a return and a start (else `flip_zone_flip`'s assert).
- State `cart_init` and the resume do not rebuild (input for step 10):
  1. The visual random stream: never reseeded; a different stream value gives 35 pixels apart after 30 frames (vanishing planet's flicker at (61,15)); the plan accepts it.
  2. The game random state on the title: the returned title's differs from the boot title's (the demo's draws); harmless, `cart_resume_game` reseeds and the dump does not print it.
  3. Entity ids: indexes repeat, generations rise; never comparable between starts.
  4. The input step (pad navigation, cheat state): untouched by both calls; without the re-inits the second start inherits the previous demo's `pad_nav` ids and the title cursor. Step 10 must call `input_step_init` at start and return.
  5. The abandoned world's UI slots: handled by `cart_return_to_title`.
  6. The game zone's pending generation: one frame needed between return and start.
  7. The stored save: still written by demo actions through `save_at_frame_end`; step 10 cuts it off.
  8. Session peak counters (`cap`, `zone`, `budget` dump lines): rise across cycles; excluded from comparisons.
  9. Seen by reading only, not exercised: `economy_score`'s difficulty and the year banner's keep-open counter.
- Checks: `make test` 7.27–7.70 s, `make e2e` 1.25–1.32 s (under step 1's minimums); `make end-state`, `make full-game`, `make pad-author` clean, no diff in `worst-case/`, `pico8/`, `tests/e2e/`. ASan/UBSan build of `test_cart` clean.

### Step 5

- Four chunks. `replay_source` reads `"start"` (`"resume"` needs `initial_save` and erases storage before the run); `p8_input_save` takes header members (NULL keeps the old bytes). `src/devtools/record_start.{c,h}`: `record_start_read_hex`, `record_start_resume` (`cart_resume_game`, then `input_step_init` on the default table, asserting no personal bindings), `record_start_return` (`cart_return_to_title`, then `input_step_init` on the compiled table), step 4's contracts. `pad_author`'s `from-record <file.hex>` (before the first frame, once) writes `"start": "resume"` and `initial_save`; frame 0 is the first frame after the resume, no title prefix.
- **The one tool that plays a record-based output from the resume: `replay`** (`make replay`, `make state`), for files with `"start": "resume"`; a `"bindings"` member is refused there. `REPLAY_STARTS=2` reproduces step 4's two-start cycle (play, return, 30 idle title frames, start, play), writing `<STATE>.start1` and `<STATE>`; it refuses a title-start file, a capture path and `PAD_METRICS`. Every later check of the demo's content uses it.
- Proof `tools/pad_author_proofs/demo_beats.record.pad` (new `*.record.pad` kind in `make pad-author-proof`: self-check, then the two-start replay, both ends equal to `pad_author`'s normalised end state). Beats with `idle 90` before each command, padded to 150, opening 150, closing 180: **1085 frames** (PRD estimate 1415). Ranges: opening 0–149, fold 150–304, colonise p32 305–454, slipway p32 p55 455–604, year 605–754, research xenofoods 755–904, closing 905–1084. End: year 3413, 17 colonies, 19 slipways, 3 techs, credits 105, 0 cheat activations — one more of each than the record. No re-cut of step 2 needed.
- Deviations: the hex reader also accepts uppercase and refuses an empty line or no final newline; `REPLAY_STARTS=1` is accepted on a title file; the two-start replay writes the default `tmp/replay_dump.txt`, so `test_replay_from_resume` runs after `test_frames_tool` under `-j`.
- Checks: `make test` 7.0–7.9 s, `make e2e` green, `make pad-author-proof` (20 proofs), `make pad-author`, `make end-state`, `make full-game` clean; no diff in `worst-case/`, `pico8/`, `tests/e2e/`.

### Step 6

- Chunk 1: `src/devtools/pad_caption.{c,h}` (text rules, `{from, to}` track, pacing, the bare `"captions"` member writer) and `p8_font_has_glyph` in `src/p8/gfx.c` (drawing unchanged; it returns 1 for `v`, which the `v` rule refuses separately). Chunk 2: `pad_author`'s `caption <text>` (rest of the line raw, `/` breaks, trimmed) and `caption -`; `"captions"` written as a header member after `start` and `initial_save`, one caption per line, `\n` the only escape (step 11 splits on it).
- Semantics: `from` inclusive, `to` exclusive, indexes into the output's `"frames"`. A caption open at the script's end lasts to the last frame. A "button press" is any non-zero mask in a caption's first 90 frames (stricter than an edge: a held button counts). A caption shown under 150 frames is reported at the line that opened it; a run cut by a stop label skips that check. Uppercase is accepted (the font draws it).
- Refusal proofs: `caption_line_long`, `caption_lines`, `caption_letter_v`, `caption_glyph`, `caption_short`, `caption_press`. `demo_beats.record.pad` carries section 2's seven captions with byte-identical frames: 0–150, 150–305, 305–455, 455–605, 605–755, 755–905, 905–1085.
- Deviations: run ends close a copy of the track so `--try` keeps the shown caption; new public `pad_author_captions_member()`; the press message uses a correct plural ("89 frames").
- Checks: `make test` 7.2–8.0 s, `make e2e`, `make pad-author-proof` (26), `make pad-author`, `make end-state`, `make full-game` clean; no diff in `worst-case/`, `pico8/`, `tests/e2e/`.

### Step 7

- `pico8/demo.pad`: `from-record demo_record.hex`, section 2's seven captions, beats `fold 118 307`, `colonise p32`, `slipway p32 p55`, `year`, `research xenofoods`; idles opening 300, then 150 before and 270 after each command, closing 180; clock-free labels `probe`, `colonize`, `slipway`, `year`, `tech`, `closing`. **2736 frames.** `pico8/demo.json` committed, regenerated by `make pad-author` (with `--no-aim-budget`) byte for byte.
- Captions: 0–300, 300–785, 785–1225, 1225–1655, 1655–2120, 2120–2556, 2556–2736 (closing exactly 180). Commands: fold 450–515, colonise 935–955, slipway 1375–1385, year 1805–1850, research 2270–2286; each first press 150 frames after its caption appears.
- Counts with `replay` (`REPLAY_STARTS=2`), both ends: year 3413, 17 colonies, 19 slipways, 3 techs, 0 cheats; the record (`--stop-at probe`): 3412, 16, 18, 2. Note: "16 colonies" are 14 planets plus structures s1 and s2.
- `make demo-check` (outside `make test-all`, ~4 s): `test_demo_check` first, then both checks always — regeneration with `cmp`, and `tools/demo_check.sh` (frames ≤ 3600, captions' order/text/tiling/closing 180, +1 counts at both ends, no cheat). Blanking frame 943 of a copy fails both. The two-start replay writes the default `tmp/replay_dump.txt` and the regeneration uses `tmp/pad-author/demo.*`: don't run `demo-check` alongside `make test` or `make pad-author`.
- `pico8/demo.txt` gives each part's frames and readings (fold at f513 $86→$83, 49→46 unknown; p32 colonised f953; slipway f1379; year ends f1834; xenofoods f2284, science 18→8) and the record's cut commands.
- `make test` 7.6–8.1 s; `make e2e` green; no diff in `worst-case/` or `tests/e2e/`.

### Step 8

- Chunk 1: `pad_caption_read_member` (header only; 0 absent, -1 refused with the reason). Chunk 2: `src/devtools/demo_source.{c,h}`: loads a demo through `replay_source` and refuses, as `<path>: <reason>`, a non-`from-record` file, a `"bindings"` member, more than 3600 frames, no caption, a captions member not byte for byte as `pad_caption_write_member` writes it, bad ranges, and every step 6 rule replayed over the frames with step 6's messages (so a `v` reads `caption 2: caption: the letter v draws as u`).
- Chunk 3: `src/game/demo_data.h` (no function, no pointer): `DEMO_DATA_FRAME_LIMIT` 3600, `demo_data_caption_t {from, to, text[66]}`, `demo_data_record[]`/`_count`, `demo_data_pad_buttons[]` (`uint16_t`)/`demo_data_frame_count`, `demo_data_captions[]`/`_count`. `src/cmd/demo_generate.c` writes `tmp/demo/demo.c` (`$(DEMO_UNIT)`, depends on `pico8/demo.json` and the generator only, no FORCE), links host modules only. The unit is in `GAME_SRCS` only: step 10 moves it into the core (watch the double-listing trap step 9's PRD names) and deletes `demo_data.h`'s float-grep exclusion line once a core file includes it.
- `test_demo_data` (3 ms): compiled frames, record and captions equal the JSON as `replay_source` reads it.
- raylib vendored in this session (`make raylib-vendor-setup`, apt step included). No rebuild: a second `make test` and `make raylib` recompile nothing; `make -q` on the unit, its object, the generator and the test exits 0. `make -q tmp/game` stays 1 because plan 17's `tmp/bindings.source`/`tmp/keymap.source` depend on `FORCE` (pre-existing). Touching `pico8/demo.json` regenerates the unit.
- Checks: `make test-all`, `make demo-check` (4.2 s), `make pad-author-proof`, `make pad-author`, `make sanitize-net` clean; `make test` 7.86–8.89 s (min under 8.2 s).

### Step 9

- Chunk 1: `bindings_generate --default` writes only `bindings_default` (declared in `game/bindings.h`); `tmp/bindings/core.c` (`BINDINGS_CORE_UNIT`, from `bindings.default.cfg`, no stamp, no FORCE) is the first entry of `ENGINE_SRCS`, so every engine binary, `game` included, holds it; `CORE_SRCS` filters it out (float-grep/float-guard read `CORE_SRCS` without building generated files). `GAME_SRCS` does not list it (single-command compile of `tmp/game`: a double listing would define it twice — the same trap applies when step 10 moves the demo unit into the core).
- Chunk 2: `input_step_t.session_bindings` (by value) and `input_step_switch_bindings(step, INPUT_STEP_BINDINGS_DEFAULT|SESSION)`, which resets the step exactly as `input_step_init` does onto the chosen table and keeps the session copy. No caller yet.
- Binary proof on the real `tmp/game` (raylib vendored): with a personal `bindings.cfg` swapping `jump.enter_aim`/`jump.open_menu` (7↔8), `nm -S` shows `bindings_compiled`, `bindings_compiled_personal`, `bindings_default` each defined once; `objdump -s -j .rodata` shows the compiled table differs from the default only at those two masks, flag `01000000`; `tmp/replay`'s tables are equal, flag 0. `make test-all`, `make pad-author`, `make demo-check` unchanged with the personal file. The file was then deleted and `game` relinked on the default.
- Checks: `make test-all`, `make end-state`, `make full-game`, `make pad-author` (no diff), `make demo-check` (after `test-all`), `make sanitize-net` clean; `make test` 7.43–7.63 s, `make e2e` 1.28–1.44 s.
- Plan-review after phase 1 (warn-review's 18 warnings fed in): eight method proposals accepted and applied to sections 2 and 3 in this commit — `make raylib` and a single-listing check for the demo unit from step 10 on; `demo-check` never concurrent with `make test-all`; a reuse rule; step 10's full-demo feed in `demo-check`, one core start/return used by `record_start`, a read-only attract phase accessor and the tool that writes the demo's last-frame state, an interruption sweep over step 7's command ranges; step 13's explicit dump paths; step 15's scope from `git diff --name-only 6e5584c..HEAD -- src tests tools`. The other warnings (duplicated hex/file helpers, hard-coded 3600/180, `pad_author.c` size, pointer rules 12/17 in devtools) are left: off the remaining steps' path.

### Step 10

- Chunk 1: one core start and return — `input_step_start_record` (switch to the default table, `cart_resume_game`) and `input_step_return_to_title` (`cart_return_to_title`, switch back to the session table); `record_start` keeps only the hex reading and calls them, and never cuts the save. `cart_title_shown()` (flag set by `cart_init`, cleared by any game start, no box open) and `cart_cut_save()` (skips `save_at_frame_end` until `cart_return_to_title` or `cart_load`). The demo unit moved into `ENGINE_SRCS`/`CART_ONLY_SRCS`, out of `GAME_SRCS` (once in its expansion, checked) and filtered from `CORE_SRCS`.
- Chunk 2: `src/game/attract.{c,h}`, pure: `attract_frame(attract, on_title, raw_buttons, &played)` → none/start/return; phases idle, playing, swallowing; one empty frame always between a return and the next start; `attract_demo_record()`. `demo_data.h` lost its float-grep exclusion.
- Chunk 3: `input_step_t.attract`; `input_step_set_attract_after` (game and replay hand `replay_source_attract_after` over); at the top of `input_step_run` the attract mode picks the played buttons, starts (core start + `cart_cut_save`) or returns; pad navigation and cheats take the played buttons, the recorder keeps raw frames; the bindings switch keeps the attract state. Accessors for tools: `input_step_attract_phase`, `input_step_attract_demo_frame`. Shared writer `replay_source_format_attract_after` (recorder and tests).
- Chunk 4: **the tool for steps 12 and 13 is `replay` with `ATTRACT_STATE=<file>`**: on the last frame of each completed playback of a title-start replay it writes `<file>.<n>` (n from 1), `STATE`'s format, the cheat count of that playback only; nothing for an interrupted one; refused with `REPLAY_STARTS=2` or a resume-start file. `tests/test_attract_demo` (in `make demo-check` only): two full playbacks over a player's save each reach 3413/17/19/3 from 3412/16/18/2 with no cheat, then the title with its `resume` row and the save bytes untouched. Interruption sweep, each passing (title, `resume` row, swallowed press, save untouched): fold at demo frame 480, colonise 945, slipway 1380, year 1830, research 2280. `make demo-check` now ~14 s.
- Deviations: the start frame's state equals the record resumed plus demo frame 0's input step (its pad navigation moves the camera); without the cut, the fold's save write lands at demo frame 540, the next at 953 and 1379. Recordings keep `"attract_after": 1200` (plan rule): a `game` session started by replaying a short-threshold test file would not replay identically — accepted limit.
- No existing hash moved: the longest empty run is 59 frames in `tests/e2e/`, 301 in `worst-case/`. Checks: `make test-all`, `end-state`, `full-game`, `pad-author` (no diff), `raylib` + single listing, `pad-author-proof`, `sanitize-net`, `demo-check` (after `test-all`) all pass; `make test` 7.80–8.46 s (min under 8.2; `test_cart` 0.45 → 0.81 s, off the critical path), `make e2e` min 1.31 s.

### Step 11

- `src/game/demo_overlay.{c,h}`: `demo_overlay_find_caption`, `demo_overlay_draw(gfx, mem, frame)` (nothing for frame -1; draws with a copy of the graphics state, camera and fill pattern reset). `cart_set_demo_frame` / `CART_NO_DEMO_FRAME`: `input_step_run` publishes the demo frame just played (`input_step_attract_demo_frame - 1`) while playing, -1 otherwise; `cart_load` and `cart_return_to_title` reset it; `cart_draw` ends with the overlay.
- **Band: rows 96–108, full width** (just above the bottom HUD and build bar, which start at row 109; measured over all 2736 frames, the top HUD is rows 0–10 and the tech panel rows 11–87 on the left, so a band under the top HUD would hide the panel for 436 frames). Fill colour 1, text 7, lines centred, two lines at y 97 and 103, one at y 100; the closing card uses the same band. **`demo` tag:** colour 8 box at x 143–159, y 11–17 (every corner is HUD), text 7 at (144, 12), on every demo frame.
- Tests: `test_demo_overlay` (pixels on a sentinel screen: two lines, one line, closing, frame -1, camera and fill pattern); `test_cart` (no overlay on the title, frame 0 on the start, caption 0 at demo frame 299, caption 1 at 300, none on interrupting/swallowed frames); the demo-check binary (closing caption and tag on each last frame, none on the return or at each interruption, both playbacks' captions equal).
- Headless look (dump of a 4-frame fixture with `"attract_after": 3`): "build a trading empire" from x 36 at y 97, "across the stars" from x 48 at y 103, band solid colour 1 between rows 96 and 108, tag box at the top right.
- Checks: `make test-all`, `end-state`, `full-game`, `pad-author` (no diff), `raylib` + single listing, `pad-author-proof`, `sanitize-net`, `demo-check` (13.3 s, after `test-all`); `make test` min 7.61 s, `make e2e` min 1.38 s (noisy machine).

### Step 12

- `tests/e2e/attract.json` (176 frames, hand-written; header `"input"`, `"attract_after": 3`, `initial_save` copied byte for byte from `pad_tech_costs.json`, a year-3407 player's game): f0–2 idle; **f3 demo start and first caption** (shown f3–152, with the tag; state 3412/16 colonies/18 slipways); **f153 return**; **X held f153–162, swallowed** (focus stays on "forgiving"), released f163; Up at f164 reaches `resume`, X at f166 resumes the player's game. `SAVE_HEX` equals the `initial_save` after f2, f3, f152, f153, f162, f164, f165, f166, f175. Without the header member, no demo (end: year 3401, forgiving). `.txt`, `.sha256` (`68de26ab…`, made for this file alone, twice, after checking the overlay on exactly f3–152) and `.end-state` (from `make end-state-baseline`, which writes only missing ones). No `make e2e-baseline`. `tools/e2e_limits.sh` passes with no exception.
- What this e2e cannot prove: demo frames 0–149 hold no action, so the save cut and frame substitution rest on step 10's tests and `make demo-check`.
- Chunk 2: `test_live_slots` (in `make sanitize-net`) ignored the header, so it replayed `attract.json` without the demo; it now hands the threshold to the input step, asserts one e2e reaches the demo, and ends an idle window before a frame whose attract phase changes (the demo start jumped the game slots 7616 → 93880). 174 windows over 20 e2e.
- Checks: `make test-all` (20 e2e), `sanitize-net` (20 clean), `end-state` (25), `full-game`, `pad-author` (no diff), `raylib`, `demo-check` (after `test-all`); `make test` min 7.03 s, `make e2e` min 1.32 s (attract.json costs ~0.1 s CPU; noisy machine).
