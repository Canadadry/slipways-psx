---
title: "Separate random streams for visual effects"
description: "Particles, the probe emitter, transit dots and the vanishing flicker share the game's random stream, so the camera path and frame timing change which planets a probe reveals."
status: needs-triage
---

## Problem Statement

The port has one random number generator, the PICO-8 `rnd` emulated by the p8 runtime. Game decisions and visual-only effects both draw from it:

- **Game decisions:** the galaxy's planet positions (galaxy), the vanish roll and the type picker's random walk used when a probe or fold reveals a planet (planet, gen), and the background tile map, built once at game start (background).
- **Visual effects:** the particle emitter's spawn count (particle), each probe particle's angle and radius (probe, which the title screen's emitter uses too), each harvest burst particle (planet_family's burst), the 8% transit-dot roll on each slipway end (slipway_display), and the flicker of a vanishing planet (planet_display).

Most visual effects only draw while their entity is drawn on screen, and the emitters draw every frame. So the camera path and the number of frames between two game actions change the game stream, and with it the next revealed planet's type. Adding 5 idle frames to `pad_probe` was measured to turn a revealed ocean planet into a forgeworld. Plan 12 has to re-author all 17 pad replays with a new control scheme while keeping their game end states. That cannot work while every extra frame or camera pixel rerolls the galaxy.

For a developer re-authoring a replay, the symptom is that any input change moves game outcomes. For a player, panning the camera rerolls the next planet, so the game is not the same game for the same decisions.

## Solution

Visual-only effects draw from a second random stream, the **visual stream**, which no game decision reads. The existing stream becomes the **game stream** and keeps its current API and seed. Within a game, the camera path, frame timing, particles and transit dots no longer move the game stream. Only game actions do: placing a probe, a fold's reveal, the vanish roll, the type picker, and galaxy generation.

The save stays version 1. Its `rnd_state` is the game stream. The visual stream is not saved: after a resume, particles already restart from nothing, so the visual stream being in another state changes nothing a save check compares. `docs/save-format.md` says so.

This changes the random state for every e2e replay once. Each replay is replayed and judged against the situations its `.txt` lists. It is either re-baselined (hash and `.txt` readings) or re-authored until it produces every listed situation again. Then `make full-game`, `make save-worst-case`, `make save-fuzz` and `make rng-sync-probe` are rerun, and `make coverage` is compared against the baseline taken before this step.

## User Stories

1. As a developer re-authoring a replay, I want extra idle frames inside a game to leave every revealed planet type unchanged, so that I can change timing without rerolling the galaxy.
2. As a developer re-authoring a replay, I want a different camera path to leave game outcomes unchanged, so that the new jump-cursor scheme can reach the same end state as the old free camera.
3. As a developer, I want particles to keep looking as random as before, so that the visual effects are not visibly degraded.
4. As a developer, I want the probe emitter, the title screen's emitter and the harvest burst to draw from the visual stream, so that an emitter on screen never moves the game stream.
5. As a developer, I want the transit-dot roll on slipways to draw from the visual stream, so that the number of slipways on screen and the frame count do not move the game stream.
6. As a developer, I want the flicker of a vanishing planet to draw from the visual stream, so that whether a vanishing planet is on screen does not move the game stream.
7. As a developer, I want the galaxy layout, the vanish roll and the type picker to keep drawing from the game stream, so that game decisions stay reproducible from the saved state.
8. As a developer, I want the background tile map to keep drawing from the game stream at game start, so that the save's `background_seed` keeps its meaning and the resume rebuilds the same map.
9. As a player, I want every new game started from the title to still get a galaxy that depends on when I start it, so that two launches do not always give me the same galaxy.
10. As a player who recorded a session, I want my recording to replay to the same game, so that the measurement tool and bug reports can rely on it.
11. As a player resuming a save, I want the resumed game to make the same game decisions as the original, so that resuming is invisible to the rules.
12. As a developer, I want the save format to stay version 1 with the same bytes, so that existing saves and the `pad_resume_at_start` fixture still decode.
13. As a developer, I want `docs/save-format.md` to say that `rnd_state` is the game stream and that the visual stream is not saved, so that the doc stays true.
14. As a developer, I want a unit test showing that the same game-stream draws come out with and without visual draws between them, so that a regression is caught in `make test`.
15. As a developer, I want a unit test showing that each visual draw site leaves the game-stream state untouched, so that a new effect drawing from the wrong stream is caught.
16. As a developer, I want the game-stream draws and the visual-stream draws to never move each other's state, so that the separation goes both ways.
17. As a developer, I want the probe particle order test to keep checking that `rnd(angle_slices)` comes before `rnd(0.5)`, now on the visual stream, so that its trap stays guarded.
18. As a developer, I want the "hiding particles draws no random number" test to check both streams, so that hiding pixels stays a pure display switch.
19. As a developer, I want every e2e replay compared with its `.txt` after the change, so that no replay silently loses a situation it was written to cover.
20. As a developer, I want a replay that still produces every situation in its `.txt` to be re-baselined (its `.json.sha256` and the `.txt` readings), so that unchanged coverage costs no re-authoring.
21. As a developer, I want a replay that no longer produces a situation (for example a colony on a planet whose type changed) to be re-authored until it does, so that the e2e keep their meaning.
22. As a developer, I want the shared prefixes (`pad_probe` → `pad_colonise` → `pad_slipway`, and the other verbatim chains) fixed first and reused verbatim, so that a shared prefix is re-authored once.
23. As a developer, I want every re-authored replay to stay within its frame limit (180 frames, or 600 when listed in the long-exceptions file), so that `make e2e` does not get slower.
24. As a developer, I want re-authored inputs never to type the cheat sequence by accident, and the replays that type it on purpose to keep doing so the same number of times, so that cheat activations stay the same.
25. As a developer, I want every test that pins frames, positions or counts inside an e2e replay to be re-derived in the same commit with the same intent, so that no assertion is weakened or removed.
26. As a developer, I want `make rng-sync-probe` to still show that the saved state is enough to stay in sync after dropping unsaved state, so that the save's completeness stays proven.
27. As a developer, I want the `pad_resume_at_start` fixture re-cut if `pad_research`'s last record changed, so that the resume replay resumes the game it describes.
28. As a developer, I want `make full-game`, `make save-worst-case` and `make save-fuzz` to pass after the change, so that the long games still run clean and still save and load.
29. As a developer, I want `make coverage` to show no line left unhit that the baseline hit (lines this step deletes or rewrites excepted, the rewritten form being hit), so that re-authored replays with a changed galaxy still cover what they covered.
30. As a developer reading the plan later, I want the step note to say which replays were re-baselined, which were re-authored and why, and the coverage outcome, so that later steps start from facts.
31. As a developer, I want `make test` and `make e2e` to stay as fast as before, so that the loop stays fast.
32. As a developer, I want the visual stream's seed to be a fixed constant at boot, so that every headless replay is deterministic and its hash is stable.

## Implementation Decisions

- **Two streams, one generator.** The p8 runtime keeps its single global `rnd` (the game stream) with the same API, seed and LCG. It gains one function that draws from an explicit `uint32_t` state the caller passes in, with the same LCG and scaling as `p8_fixed_rnd`. `p8_fixed_rnd` is rewritten on top of it, so the game stream's draws stay identical bit for bit. No new constants, so `constants-check` is unaffected. The state is a value, not a pointer field, so `pointer-guard` is unaffected.
- **A new small game module, `visual_random`**, owns the visual stream as a file-scope `uint32_t`, set at boot to a fixed seed different from the game stream's. It exports a draw of `rnd(x)`, a draw of `rnd(1)`, and a state getter and setter for tests, named by the `module_verb[_object]` rule (for example `visual_random_draw`, `visual_random_draw_one`, `visual_random_state`, `visual_random_set_state`). Nothing reseeds it at game start, title, restart or resume. Its state continues from boot.
- **Draw sites moved to the visual stream:**
  - the emitter's spawn loop (`while rnd() < n`), which covers the per-frame emitters and the burst's 140-particle construct;
  - the three draws in the probe particle spawn (angle slices, half and radius), in the same order. The title screen's emitter uses it too;
  - the burst particle spawn (radius, quantize roll, angle, life);
  - the transit-dot 8% roll on each slipway end, still drawn before the served-planets test;
  - the vanishing planet's flicker roll in the planet draw.
- **Draw sites that stay on the game stream:** galaxy positions; the random walks of the type picker, the vanish roll and the background (the shared random walk keeps drawing from the game stream, unchanged); the background's per-tile draws.
- **Background decision (it is in the step's list, kept on the game stream on purpose).** Its tile map is drawn once per game, before the galaxy, at a fixed point that no camera or timing can move, so it does not break the step's goal. Moving it would change the meaning of the save's `background_seed`, which is the game stream's state just before the map, restored by the loader, and checked by the resume's assert and by the save-state test. It would buy nothing towards the goal. The background therefore draws from the game stream as today. Only the step's per-frame visual effects move.
- **The title keeps the galaxy time-dependent.** Today the title screen's emitter draws every frame, so the galaxy of a new game depends on how many frames the title was shown. A human gets a different galaxy each launch, and a recording still replays, because the boot seed is fixed and the title frames are recorded. With the emitter on the visual stream, every game started from boot would get the same galaxy. To keep the current property, the title screen's draw takes exactly **one game-stream draw per drawn frame** (a seed tick), its result unused. So the galaxy depends only on the title's frame count and the game stream left by earlier games. It no longer depends on particle counts, and in-game camera and timing never move it. Rejected: a fixed galaxy per boot (the same galaxy every launch for a player); a wall-clock seed (recordings stop replaying); seeding the game from the visual stream (brings back camera and timing dependence for games started after a restart).
- **Save.** The format, the version (1) and the encoder and decoder bytes are unchanged. `rnd_state` is captured and restored as today and is now only the game stream. The visual stream is not saved, because nothing a save check compares depends on it: particles and shakes are dropped by the save check's unsaved-state drop, and snapshots are taken with particle pixels hidden. `docs/save-format.md` gets the two facts: `rnd_state` is the game stream, and visual effects draw from a stream the record does not hold. `save_state.c` needs no change. If the implementation finds a save check that compares a visual-stream-dependent value, the decision becomes saving it as a new field and bumping the version, with the `pad_resume_at_start` re-cut its `.txt` describes.
- **Comments that state the old coupling are corrected in the same commit:** the resume check's notes that a probe's emitter moves `rnd_state`, and the transit-dot order note, which now concerns the visual stream. Workaround code that stays harmless (copying the record into the original side's snapshot) is left as is.
- **The `rng-sync-probe`.** It keeps comparing the game stream of run A and run B on every frame of both windows. Its "rnd_state never moved over the window" assertion checked that the window exercises the draws the drop could disturb. Those draws are now on the visual stream, so the assertion moves to the visual stream: run A's visual stream must move over the window. A second check is added: run B's game stream equals run A's on every frame. If `pad_colonise` is re-authored, the save frames and last frames of the burst and quiet cases are re-derived so that each case's own assertions still hold (a live, on-screen burst at the save frame; a slipway and live probe particles in the quiet case).
- **Coverage baseline.** It is taken before any change of this step, as the plan's guardrail requires: `make coverage` on the current HEAD of branch `claude/epic-hawking-oi6wnl` (`6346485`, after step 1's fix and its step note), over the 17 pad replays. Copy `tmp/coverage/lines.txt` aside (for example under `tmp/coverage-baseline-1b/`, gitignored, reproducible from that commit). After the change, every baseline line with hit 1 must be hit again. Lines are matched through the step's own diff: an untouched line keeps its identity, and a line the step rewrites (a draw site moved to the visual stream) must be hit in its new form. Any newly unhit line means a replay is re-authored until it is hit. No new make target: the comparison is a one-off in `tmp/`.
- **The e2e procedure, in this order:**
  1. Replay every e2e and read its new frames and `make state` against its `.txt`.
  2. Shared prefixes first: `pad_probe`, then `pad_colonise` (which starts with it), then `pad_slipway` (which copies `pad_colonise` f0-98 verbatim), then the other verbatim chains the `.txt` files declare. A prefix re-authored once is pasted verbatim into the replays that share it.
  3. A replay that still produces every situation of its `.txt` gets its `.json.sha256` re-baselined and its `.txt` readings (planet ids, types, positions, colours, credits, state lines) rewritten from the new frames.
  4. Otherwise its inputs are re-authored until it does, under plan section 2's situation guardrail: every listed situation is kept, because this step removes no mechanic. Each replay stays within its frame limit and types the cheat exactly as many times as before. The `.txt` notes that compare with the old keyboard galaxies ("the galaxy depends on the frame the game starts") are kept true or cut.
  5. Pinned values in tests that load an e2e replay (at least the live-slots test on `pad_pause_restart`, the state test on `pad_tech_costs` and `pad_research`, the rng-sync probe on `pad_colonise`, and every other test that names a file under the e2e directory or `worst-case/`) are re-derived in the same commit, each assertion keeping its event at its new frame.
  6. If `pad_research`'s last record changes, the `pad_resume_at_start` fixture is re-cut with the two commands in its `.txt`, and its checks rerun.
- **The worst-case games** are not hashed. They must still run clean under `make full-game`, `make save-worst-case` and `make save-fuzz`. If one stops (an input that relied on a planet type, or a zone cap), its input is fixed minimally in this step. Plan section 2 marks them stale only from step 5.
- **Plan step note.** The implementer's report gives the content of step 1b's note: which replays were re-baselined, which re-authored and why, any first differing reading, the coverage comparison result, and the outcome of the four long checks.

## Implementation Chunks

1. **Baseline and stream, no behaviour change.** Take the coverage baseline on `6346485` (the current HEAD of `claude/epic-hawking-oi6wnl`), before editing anything. Add the explicit-state draw to the p8 runtime, rewrite the global `rnd` on it, and add the `visual_random` module and its unit test. No draw site moves yet, so every e2e hash is unchanged and `make test-all` passes. One commit.
2. **The switch and every replay, in one commit** (`make test-all` must pass on every commit, and every hash changes at once). Move the five draw sites to the visual stream and add the title's one-draw-per-frame seed tick. Update the probe-order test, the hidden-particles test, the rng-sync probe, the corrected comments and `docs/save-format.md`. Then run the e2e procedure above: prefixes first, then every other replay re-baselined or re-authored, pinned test values re-derived, and the fixture re-cut if needed.
3. **Long checks and coverage.** Run `make full-game`, `make save-worst-case`, `make save-fuzz`, `make rng-sync-probe` and `make coverage` against the chunk 1 baseline. Any failure or newly unhit line is fixed by re-authoring (a replay or a worst-case input) in a follow-up commit. If everything passes, nothing is committed beyond the report for the step note.

## Testing Decisions

- A good test drives the public interface (the two streams' draw and state functions, and the public draw sites such as the emitter spawn, the probe and burst particle spawns, the transit-dot spawn and the planet draw) and asserts on observable results: sequences of drawn values and the streams' states. It never looks at the static fields.
- **New unit test for the `visual_random` module** (one test file per module, as the tests directory does):
  - seed the game stream, draw N values. Reseed it to the same value and draw the same N values with a varying number of visual draws between them. The two sequences are equal (the plan's verification);
  - visual draws leave the game-stream state unchanged, and game draws leave the visual-stream state unchanged;
  - the explicit-state draw on a copy of the game stream's state gives the same values as the global `rnd`, so the LCG is shared and the game stream is unchanged bit for bit;
  - each moved draw site (emitter spawn with a non-zero count, the probe particle spawn, the burst particle spawn, the transit-dot spawn, the planet draw of a vanishing planet) leaves the game-stream state unchanged and moves the visual stream.
- **Updated tests:** the probe particle order test redraws its expected sequence from the visual stream. The hidden-particles save-check test asserts both streams. The rng-sync probe changes as decided above.
- **Prior art:** the hidden-particles save-check test (an "draws no random number" assertion on the stream state), the gen test (save and restore the stream state around draws), and the probe particle order test (the same state reached twice, then sequences compared).
- **E2e:** every replay re-baselined or re-authored as described. `make e2e` is the gate, and the `.txt`-to-`.txt` comparison in the same commit is the situation check.
- **Out of `make test-all`:** `make full-game`, `make save-worst-case`, `make save-fuzz`, `make rng-sync-probe` and `make coverage`, all rerun in chunk 3. No test is added to `make test` or `make e2e` that makes them slower: the new unit test is a few thousand LCG steps.

## Step 1 outcome

Step 1 (the slipgate □ press-retry fix) is committed, and its step note is in the plan's section 6. Facts this step relies on:

- The fix (`6059d8e`, "give up the drag on a full slipgate") is in the pad navigation layer and the slipgate's trade code only. It touches no random draw site, no save format and no visual effect, so the draw-site list above is complete.
- No e2e was added or changed: there are still the 17 pad replays, with the `.txt` files of that commit as the reference situations.
- The regression test is a unit test in the pad navigation tests. It pins no replay, so it is not on the pinned-value list of the e2e procedure.
- The coverage baseline is taken on the current HEAD of branch `claude/epic-hawking-oi6wnl` (`6346485`, "docs: plan 12 step 1 note", which follows the fix), over the 17 pad replays.

## Out of Scope

- Any pad mapping change (steps 3 onwards), the end-state check (step 2a) and the feasibility study (step 2b). Step 2b checks afterwards that the 5-idle-frame edit no longer changes `pad_probe`'s reveal; this step does not have to prove it beyond the unit test.
- Re-authoring the worst-case games beyond what keeps the four long checks passing.
- Saving the visual stream, unless a save check proves it is needed (see the save decision).
- Changing the boot seed of the game stream, the LCG, or any game rule.
- Audio, PSX, mouse input.

## Further Notes

- The background is the one file of the step's list that keeps the game stream. The decision and its reason are above: the step's goal (camera and timing independence) holds without moving it, and moving it would change a save field's meaning for nothing.
- The title seed tick replaces a variable number of title draws (one per spawned particle, plus the spawn-loop rolls) with exactly one per drawn frame. So the galaxy of a pad replay depends only on the frame its title X lands on. That makes later re-authoring more robust: a title path re-authored to the same X frame keeps its galaxy.
- Every e2e hash changes in chunk 2. Galaxies change for every replay because the game stream at game start differs: the title draws differ, so planet positions move as well as types. Expect most planet ids, positions and types in the `.txt` readings to change, and plan the re-authoring time accordingly, prefixes first.
