---
title: "psx-hello CD boot checked against a committed reference"
description: "Nothing fails when the psx-hello CD image stops drawing the frame it drew when it was proved, because no reference of that boot's capture is committed and no target compares against one."
status: needs-triage
---

## Step

Plan `docs/plan-9.md`, phase 2 — Toolchain and image, step 3 (verbatim):

> 3. **Check target.** A `make` target, outside `make test-all`, builds the image, boots it, and fails if the capture's sha256 differs from its committed reference, like the e2e. The reference is committed only from a boot that passed step 2.2's proof: the target runs that same check on the image before comparing the sha256. Verified: it passes, and fails on a deliberately altered reference.

## Problem Statement

Plan 9 is done when, from a clean checkout with the toolchain and emulator installed into empty directories, "the step 2.3 target passes". After step 2.2 (see "Step 2.2's outcome": written in parallel, so every fact about it is a hypothesis), the CD image can be built and proved to boot `psx-hello`, but:

- The proof only says the boot is deterministic, differs from an empty boot, is not one colour and prints the known line. It does not say *what* is drawn: a change to `psx-hello`, the SDK pin, the emulator pin or the boot script that alters the frame passes it silently.
- The game side has an answer to this (`make e2e`: the sha256 of each capture against a committed `<name>.json.sha256`, set by `make e2e-baseline`). The PSX side has none: no reference is committed and no target compares.
- The plan forbids a reference taken from an unproved boot (a black screen or a halted BIOS is deterministic too), so the comparison cannot run on its own: it must sit behind step 2.2's proof.
- The `.bin` cannot be the reference: `mkpsxiso` writes the build time into the volume descriptors, so two builds of the same sources give different `.bin` files (step 2.2's trial). Only the capture is stable.

## Solution

- A committed reference file, `psx-hello/psx-hello-cd.vram.sha256`, holding one line: the sha256 of the capture (the whole `.vram` file, header included) of a 600-frame CD boot of `psx-hello`.
- A new `make psx-hello-e2e`, outside `make test-all`: it depends on step 2.2's `make psx-hello-cd-check` (which builds the image fresh, boots it twice and nothing once, and runs the five-condition proof), so a failed proof stops it before any comparison. Then it hashes the proved capture `run1.vram` and compares it with the reference: exit 0 on a match, exit 1 with the expected and actual hashes on a mismatch or a missing reference.
- A new `make psx-hello-e2e-baseline`, the PSX counterpart of `make e2e-baseline`: same dependency on the proof, then writes the proved capture's sha256 into the reference file. This is the only way the reference is written, which makes the plan's rule ("committed only from a boot that passed step 2.2's proof") mechanical rather than a matter of discipline.
- The comparison lives in one small script, `tools/psx_reference_check.sh <reference> <capture>`, so its messages and failure paths are testable by hand without a boot.

The step is verified by running `make psx-hello-e2e`: it exits 0; then by running it with `PSX_HELLO_REFERENCE` pointed at an altered copy of the reference under `tmp/`: it exits 1 naming both hashes, without the committed file ever being edited.

## User Stories

1. As the plan-pilot run, I want one `make` target that builds the image, proves the boot and compares its capture to a committed reference, so that the plan's "Done when" names a single command.
2. As the plan-pilot run, I want that target outside `make test-all`, so that `make test` and `make e2e` stay untouched and no slower.
3. As the plan-pilot run, I want the target to run step 2.2's proof before comparing, so that a reference can never be matched by a boot that failed the proof.
4. As the plan-pilot run, I want the target to reuse step 2.2's check target rather than copy its conditions, so that a fix to the proof applies everywhere at once.
5. As the plan-pilot run, I want the target to build the image fresh each time, so that it checks the current sources and not a stale build.
6. As a maintainer, I want the capture hashed, not the `.bin`, so that a build-time stamp in the image never fails the check.
7. As a maintainer, I want the whole capture file hashed, header included, so that a change of resolution or colour depth fails the check even if the pixels hash the same.
8. As a maintainer, I want the reference to be one line holding only the hash, like `tests/e2e/*.json.sha256`, so that both reference kinds read the same way.
9. As a maintainer, I want the reference committed next to the `psx-hello` sources, so that a change to `psx-hello` and its new reference land in the same commit and the file sits away from `tests/e2e/`, whose globs it must not disturb.
10. As a maintainer, I want a baseline target that writes the reference only after the proof passed, so that the plan's rule on the reference holds by construction.
11. As a maintainer, I want the baseline target to print the hash it wrote and the path, so that I can see what I am about to commit.
12. As an agent, I want a mismatch to print the expected hash, the actual hash and the reference path, so that I can tell a changed frame from a missing file at a glance.
13. As an agent, I want a mismatch to name `make psx-hello-e2e-baseline` as the way to accept an intended change, so that the fix is obvious, as `make e2e` does for its references.
14. As an agent, I want a missing reference to fail the check (not be skipped as `make e2e` skips it), so that the plan's "Done when" cannot pass vacuously.
15. As an agent, I want a missing or empty capture to fail the comparison with its own message, so that a broken dependency is not reported as a hash mismatch.
16. As an agent, I want the reference path overridable on the command line, so that the deliberately altered reference of the verification lives in `tmp/` and the committed file is never edited.
17. As an agent, I want the target to fail at once when `PSX_FRAMES` is not the frame count the reference was taken at, so that a different frame count is reported as such and not as a changed frame.
18. As an agent, I want the target to never run a setup target, so that a ten-minute emulator build never starts as a side effect.
19. As an agent, I want the target to fail through step 2.2's messages when the SDK or the emulator is missing, so that the fix (the setup target) is named.
20. As an agent, I want a success line naming the matched hash, so that a passing run is visible in a log.
21. As the plan-pilot run, I want the check to exit non-zero on any failure, so that the step fails as the plan rule demands.
22. As a maintainer, I want the new targets in the Makefile legend, so that they are found like every other target.
23. As the implementer of plans 10 and 11, I want the comparison script to take a reference and any capture, so that later PSX checks can reuse it.
24. As a human reading the plan later, I want the step note to record the committed hash, the run time, the altered-reference failure and every deviation, so that I can audit the step without rerunning it.
25. As a maintainer, I want nothing generated committed besides the one-line reference, so that the repository holds no capture or image.

## Step 2.2's outcome (hypotheses)

This PRD is written while step 2.2 is still being implemented, from its committed PRD (`docs/prd/triage/phase2-step2-psx-hello-cd-image.md`) and the committed tree only. Every assumption about step 2.2's outcome is a hypothesis below. Before starting, the implementer checks each one against the step 2.2 note in section 6 of `docs/plan-9.md`, the Makefile and `tools/psx_hello_check.sh` as committed, and follows the stated fallback where one does not hold. Each result (holds / did not hold, and what was done) goes into the step note.

- **H-2.2-1 — Step 2.2 landed and passed.** `make psx-hello-cd` and `make psx-hello-cd-check` exist in the Makefile, and the step 2.2 note says `make psx-hello-cd-check` exited 0. *If not:* this step cannot be verified; it fails with a report, per the plan rule (no guess).
- **H-2.2-2 — The check target builds the image fresh.** `psx-hello-cd-check` depends on `psx-hello-cd`, which depends on `psx-hello` (`cmake --fresh` and a full build, image included), and on no setup target. *If not:* `psx-hello-e2e` adds the missing `psx-hello-cd` prerequisite itself, so that it still "builds the image"; it never adds a setup target.
- **H-2.2-3 — Output directory and file names.** `psx-hello-cd-check` writes into `$(TMP_DIR)/psx/hello-cd-check`, emptied at the start of each run, and the capture of the first image boot is `run1.vram` there (with `run2.vram`, `empty.vram`, and `<name>.tty.txt` beside them). *If not:* the comparison reads the first image boot's capture from wherever step 2.2 actually puts it; the design does not change.
- **H-2.2-4 — Exit status is the proof.** `tools/psx_hello_check.sh` with `--iso` exits 0 only when all five conditions hold (and then prints `psx-hello-check: all five conditions hold`), and non-zero otherwise; so a failed proof stops `make` before the comparison. `psx-hello-e2e` relies on the exit status, never on parsing that line. *If not (exit 0 on a failed condition):* the comparison script is not run unless the success line is present in the check's output; recorded as a deviation.
- **H-2.2-5 — The CD capture is stable across builds.** Two `--iso` boots of two separate builds of the image give byte-identical captures, although the `.bin` differs (step 2.2's PRD trial: `b4934260…` both times, `.bin` `f3ba1bff…` vs `0b724a5d…`). *If not (the capture changes with each build):* a hashed reference is impossible with this image tool; the step fails with a report naming the differing captures, rather than hashing something weaker.
- **H-2.2-6 — Shared variables.** `psx-hello-cd-check` uses the same `PSX_EMULATOR_DIR`, `PSX_HELLO_TTY_LINE`, `PSX_FRAMES` (default 600) and `PSX_BOOT_TIMEOUT` as `psx-hello-check`, and the CD path variable is `PSX_HELLO_CUE` (default `psx-hello/build/psx-hello.cue`). *If not:* the frame guard of this step reads whichever variable step 2.2 actually passes as the frame count.
- **H-2.2-7 — Expected reference value.** The CD capture is 320×240, depth 16, and its sha256 is the one step 2.2's note records (predicted `b4934260…`, equal to the side-loaded `.exe` capture). *If the note records another value:* the baseline target is still the only source of the reference; the step note says the value differs from the PRD's prediction. Not a failure.
- **H-2.2-8 — No name clash.** Step 2.2 adds `system.cnf` and `iso.xml` to `psx-hello/` and changes its `CMakeLists.txt`, but commits no reference file, no `psx-hello-e2e*` target and no `tools/psx_reference_check.sh`; the CMake glob (`*.c`) and the image description (explicit file list) do not pick up a new `.sha256` file in `psx-hello/`. *If a clash exists:* the new names get a suffix and the step note records it.

## Implementation Decisions

### What is hashed, and where the reference lives

- The reference is the sha256 of `run1.vram` from step 2.2's check output: the whole file, `psx-vram <w> <h> <depth>` header line plus raw bytes, exactly what `psx-hello-check:` already prints a sha256 for. `run2.vram` is not hashed: the proof's `deterministic` condition already made it byte-identical to `run1.vram`.
- Never the `.bin`, the `.cue`, the `.exe` or a TTY file: the `.bin` carries a build timestamp, and the TTY file carries per-boot temporary paths (step 1.2 note).
- Committed file: `psx-hello/psx-hello-cd.vram.sha256`, one line, the 64-hex-digit hash and a newline, nothing else (the format of `tests/e2e/*.json.sha256`). It is not put in `tests/e2e/`: that directory's `*.json` glob and checks belong to the game's replays, and `tests/` is flat by rule.
- The reference holds for the pinned emulator, BIOS, SDK and 600 frames. A change to any pin, to `psx-hello`, or to `tools/psx_boot.sh` / `tools/psx_capture.lua` that changes the frame needs a new baseline, committed with that change.

### Comparison script: `tools/psx_reference_check.sh`

- Signature: `psx_reference_check.sh <reference> <capture>`. Exactly two arguments, else the usage line and exit 1.
- Fails (exit 1, one line on stderr, prefix `psx-reference:`) when the capture is missing or empty; when the reference is missing (message names `make psx-hello-e2e-baseline`); when the reference is not one 64-hex-digit line.
- On a mismatch: exit 1, lines naming the reference path, the expected hash, the actual hash, and that `make psx-hello-e2e-baseline` accepts an intended change.
- On a match: one line `psx-reference: <capture> matches <reference> (<hash>)`, exit 0.
- Uses `sha256sum`, falling back to `shasum -a 256` as the Makefile's `SHA256_CMD` does; the hash is taken with the `${h%% *}` idiom, keeping the exit status of the hash command (as the Makefile's `frames_hash` does).
- Comments follow CLAUDE.md: a usage header stating the contract (exit codes, prefix) and nothing else.

### Makefile

- New variables: `PSX_HELLO_REFERENCE ?= psx-hello/psx-hello-cd.vram.sha256` (overridable, for the altered-reference verification) and `PSX_HELLO_REFERENCE_FRAMES := 600` (the frame count the reference was taken at).
- New phony target `psx-hello-e2e`, depending on `psx-hello-cd-check` (H-2.2-2) and on no setup target. Its recipe first fails, with a message naming both values, if `PSX_FRAMES` differs from `PSX_HELLO_REFERENCE_FRAMES`, then runs `tools/psx_reference_check.sh $(PSX_HELLO_REFERENCE) $(TMP_DIR)/psx/hello-cd-check/run1.vram` (H-2.2-3). Because the frame guard runs after the prerequisite, a wrong `PSX_FRAMES` costs one proof run; accepted, it is a misuse path.
  - The guard can instead sit in a tiny prerequisite target ordered before `psx-hello-cd-check` if the implementer finds it no more complex; either is acceptable, the step note says which.
- New phony target `psx-hello-e2e-baseline`, depending on `psx-hello-cd-check`, same frame guard; it writes the hash of `run1.vram` into `$(PSX_HELLO_REFERENCE)` with `SHA256_CMD` and the `frames_hash` idiom, then prints `psx-hello-e2e: reference set in <path> (<hash>)`.
- Both join the `.PHONY` line. Neither is a prerequisite of `test`, `e2e` or `test-all`.
- Legend entries next to `psx-hello-check` (and step 2.2's entries): `psx-hello-e2e` (builds the image, runs psx-hello-cd-check, compares the sha256 of its first capture with PSX_HELLO_REFERENCE; fails on a failed proof, a mismatch or a missing reference; never runs a setup, ~35 s, outside the suite) and `psx-hello-e2e-baseline` (same proof, then sets the reference; never run by any other target).
- Name choice: `psx-hello-e2e` / `psx-hello-e2e-baseline` mirror `e2e` / `e2e-baseline`, as the plan says "like the e2e"; `psx-hello-cd-check` was taken by step 2.2 for the proof alone.

### Committing the reference

- The implementer runs `make psx-hello-e2e-baseline` once (proof passes, file written), then `make psx-hello-e2e` (exit 0), and commits the reference file in the same commit as the targets. No hash is ever written by hand.

### Step note

- Appended to section 6 of `docs/plan-9.md` under `### Step 2.3`: the committed hash and the capture header; whether it equals step 2.2's recorded CD capture and step 2.1's `.exe` capture (`b4934260…`); the run time of `make psx-hello-e2e`; the hand-tested failure paths and their output; the H-2.2-k results; any deviation from this PRD. No `guideline` decision was needed for this PRD.

## Testing Decisions

- A good test here checks what the plan's "Done when" relies on: the target's exit status and its messages for a match, a mismatch and a refused proof — never the emulator's or `mkpsxiso`'s internals.
- No unit test in `tests/` and nothing added to `make test` / `make e2e`: everything is emulator-bound, and the fast loop must stay untouched.
- **The step's verification** (with step 2.1's SDK install and step 1.2's emulator install in place): `make psx-hello-e2e-baseline` exits 0 and writes the reference; `make psx-hello-e2e` exits 0 with the `matches` line; then `make psx-hello-e2e PSX_HELLO_REFERENCE=tmp/psx/altered.sha256`, where that file is a copy of the reference with one hex digit changed, exits 1 with the expected and actual hashes. The committed file is never edited.
- **Failure paths, verified once by hand** and recorded in the note:
  - `tools/psx_reference_check.sh` on a missing reference, on a reference holding a non-hash line, on a missing capture and on an empty capture: each exits 1 with its own message (no boot needed: synthetic files under `tmp/`);
  - `make psx-hello-e2e PSX_HELLO_TTY_LINE='no such line'`: the proof fails (`tty-line`), `make` stops, and no `psx-reference:` line is printed — the comparison never ran;
  - `make psx-hello-e2e PSX_FRAMES=10`: fails on the frame guard naming 10 and 600;
  - `make test-all` still passes, with its timing unchanged.
- Prior art: `make e2e` / `make e2e-baseline` and `tools/frames_check.sh` (hash file per capture, expected/actual on divergence, a baseline target as the only writer); `tools/psx_hello_check.sh` (failure paths hand-tested the same way in step 2.1).

## Out of Scope

- Changing step 2.2's proof, its script, its targets or the image (step 2.2).
- Making the `.bin` byte-reproducible, or hashing it.
- A reference for the side-loaded `.exe` boot (`make psx-hello-check` stays a proof without a reference).
- Rewriting `docs/psx-hello-world.md` (step 2.4).
- Scripted pad input, several frame counts or several references (plans 10 and 11).
- Any change to `make test`, `make e2e`, `make test-all`, `tests/e2e/` or `tools/frames_check.sh`.
- The game itself and audio.

## Further Notes

- A trial was not run for this PRD: no emulator or SDK install is available in this session (the emulator takes ~10 min to build), and step 2.2 is being implemented in parallel. The facts it would have shown (stable CD capture across builds, its hash) come from step 2.2's PRD trial and are carried as H-2.2-5 and H-2.2-7.
- Determinism across install directories (the plan's "Done when" uses fresh, empty ones) rests on step 1.2's boot design: each boot runs in a throwaway HOME, with the software GPU and the interpreter. If a fresh install ever gives a different capture, the step note records it and that is a failure to report, not a reason to rebaseline.
