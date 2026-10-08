---
title: "Windowless PSX emulator: setup, boot, capture"
description: "Plan 9 has chosen an emulator but cannot install it, boot a PS-EXE or a disc in it without a window, or read back what it drew and printed."
status: needs-triage
---

## Problem Statement

Plan 9 needs a PSX emulator that an agent can run with no window and whose output it can read: a VRAM capture and the TTY text, both as files. Step 1.1 picks the emulator, the BIOS and two prebuilt test media (a PS-EXE that prints, a CD image that draws) and pins them in a sources list, but installs nothing. Today nothing installs the emulator, nothing boots anything in it, and nothing proves that a boot is deterministic or that the capture shows what the program drew. Without that, phase 2 cannot check `psx-hello`, and plans 10 and 11 have no way to read the game state (plan 10 step 1 reads it over TTY) or to check GPU primitives against a capture (plan 10 step 4).

## Solution

Three new `make` targets, all outside `make test-all`:

- `make psx-emulator-setup` installs the pinned emulator and builds its BIOS into an install directory (`PSX_EMULATOR_DIR`, default under the gitignored `third_party/`), reading every URL and pin from the step 1.1 sources list. It first runs `make psx-net-check`, so a blocked host fails at once and is named.
- `make psx-boot` boots one PS-EXE, one CD image, or nothing, headless, for a fixed number of frames, and writes two files: a VRAM capture and the TTY output.
- `make psx-emulator-check` fetches the two prebuilt test media (sha256-checked), runs five boots and checks the four facts the plan asks for: two boots of the same `.exe` give byte-identical captures; the known text is in that boot's TTY file; two boots of the CD image give byte-identical captures; that CD capture differs from the capture of a boot with nothing loaded.

The step is verified by running the setup into an empty install directory, then the check against that directory: it passes. The step note also records whether, and how, the emulator takes scripted pad input, measured by one trial.

## User Stories

1. As the plan-pilot run, I want one `make` target that installs the emulator, so that every later step starts from the same binary.
2. As the plan-pilot run, I want the setup to install the exact pinned emulator and BIOS, so that a capture taken today matches one taken in plan 11.
3. As the plan-pilot run, I want the setup to read its URLs and pins from the step 1.1 sources list, so that the probe and the setup never drift apart.
4. As the plan-pilot run, I want the setup to run the network probe first, so that a blocked host fails in seconds with its name, not after minutes of build.
5. As the plan-pilot run, I want the setup to work into an empty directory, so that the verification proves nothing left over from a trial is needed.
6. As the plan-pilot run, I want the setup to refuse a non-empty directory it did not fill, so that a half-built or foreign tree is never mistaken for a good install.
7. As the plan-pilot run, I want a second setup into a complete install to do nothing, so that re-running it costs seconds, not the 8-minute build.
8. As the plan-pilot run, I want a failed setup to leave no completion marker, so that the next run rebuilds instead of trusting a broken tree.
9. As the plan-pilot run, I want the emulator fix commit applied by the setup, so that a disc boot does not segfault headless.
10. As the plan-pilot run, I want the fix to fail the setup if it does not apply cleanly, so that an unpatched emulator is never installed silently.
11. As the plan-pilot run, I want the BIOS to be OpenBIOS built from the same tree, so that no Sony BIOS is ever downloaded.
12. As an agent, I want to boot a PS-EXE with one command and get two files, so that I can read the result without a window.
13. As an agent, I want to boot a `.cue` with the same command, so that discs and executables are checked the same way.
14. As an agent, I want to boot with nothing loaded, so that I have a baseline capture to compare against.
15. As an agent, I want the boot to stop after a fixed number of frames, so that a capture is always taken at the same moment.
16. As an agent, I want a wall-clock timeout on every boot, so that a hung emulator fails instead of blocking the run.
17. As an agent, I want a boot that times out or crashes to fail with a message naming the input, so that the report says what broke.
18. As an agent, I want the boot never to open a window and never to need an X server, so that the CLAUDE.md ban on windows holds.
19. As an agent, I want every boot isolated from configuration, memory cards or caches left by an earlier boot, so that two boots of the same input are byte-identical.
20. As an agent, I want the capture to carry its width, height and depth, so that two captures of different modes never compare equal by accident.
21. As an agent, I want the capture to be the displayed VRAM area as raw bytes, so that its sha256 is a stable reference for step 2.3 and plan 10.
22. As an agent, I want the TTY file to hold only what the program printed, so that a search for a known line is a plain text search.
23. As the implementer of plan 10 step 1, I want the TTY output in a file, so that I can read the game state printed by the core.
24. As the implementer of step 2.1, I want to boot `psx-hello.exe` with the same command, so that its capture and TTY are checked like the test media.
25. As the implementer of step 2.2, I want to boot `psx-hello.cue` with the same command, so that the image is checked like BuzzyBee.
26. As the implementer of step 2.3, I want the capture format fixed now, so that the committed reference sha256 means the same thing later.
27. As the plan-pilot run, I want `make psx-emulator-check` to fetch the test media and verify their sha256, so that a changed file is refused.
28. As the plan-pilot run, I want the check to evaluate every condition before concluding, so that one run reports every failure.
29. As the plan-pilot run, I want each failed condition to print one line, so that the report is exact.
30. As the plan-pilot run, I want the check to exit non-zero if any condition fails, so that the step fails as the plan rule demands.
31. As the author of plans 10 and 11, I want the step note to say whether scripted pad input works headless and how, so that I know what to build on.
32. As a maintainer, I want `make test` and `make e2e` untouched, so that the fast loop stays fast.
33. As a maintainer, I want the three targets in the Makefile legend, so that they are found like every other target.
34. As a maintainer, I want the install directory and every boot output in gitignored places, so that nothing large or generated is committed.
35. As a maintainer, I want the test media never committed, so that the repository stays free of third-party binaries.
36. As a human reading the plan later, I want the step note to record the measured hashes, timings and pad finding, so that I can audit the step without rerunning it.

## Hypotheses on step 1.1's outcome

Step 1.1 is being implemented in parallel. This PRD relies only on its committed PRD (`docs/prd/triage/phase1-step1-psx-sources-net-check.md`). Each assumption below is labelled; when step 1.1 lands, the implementer of this step checks each one against the actual step 1.1 note and code, and follows step 1.1 where it differs (the hypothesis is the fallback, never the authority).

- **H-1.1-1 — Sources list location and variable.** One text file in `tools/` holds every source; the Makefile names it in a `PSX_SOURCES` variable, which `make psx-net-check` reads. This step reads the same variable and adds no second list.
- **H-1.1-2 — Sources list format.** One line per source, whitespace-separated: component name, kind (`git`, `file`, `apt`, `host`), URL, pin (commit for `git`, sha256 for `file`, series for `apt`); `#` starts a comment line. Lines are found by component name; this step assumes names that identify, at least, the emulator base commit, the emulator fix commit, the Ubuntu archive, the ps1-tests archive, `buzzy.bin` and `buzzy.cue`. The exact names are taken from the committed file.
- **H-1.1-3 — Emulator pins.** PCSX-Redux at base commit `9eb544100311010c68af9049f57bf1f718961774`, plus the change of commit `307177ba56cdc5938f2a6f2a954753992ad13287` applied on top; both fetched from `https://github.com/grumpycoders/pcsx-redux` (two `git` lines on the same URL). The submodules (35, all on `github.com`) are pinned by the base commit and are not listed.
- **H-1.1-4 — The fix commit is fetchable by its sha.** The session's git proxy serves a fetch of a single commit by sha from that repository (the 1.1 trial applied it); if not, the setup fetches the branch that contains it.
- **H-1.1-5 — Build dependencies.** The Ubuntu 24.04 "noble" packages `build-essential pkg-config libcapstone-dev libfreetype-dev libglfw3-dev libavcodec-dev libavformat-dev libavutil-dev libswresample-dev libcurl4-openssl-dev libuv1-dev zlib1g-dev`, plus `gcc-mipsel-linux-gnu` and `g++-mipsel-linux-gnu` for OpenBIOS, are enough to build both; they come from `archive.ubuntu.com` / `security.ubuntu.com`, pinned to the series only.
- **H-1.1-6 — Build commands.** The emulator builds with the repository's `pcsx-redux` make target (about 8.5 minutes on 4 cores in the trial), and OpenBIOS with a make in its `src/mips/openbios` directory (about 1 second), giving a 512 KiB `openbios.bin`.
- **H-1.1-7 — Command line and BIOS lookup.** The pinned build accepts `-cli` (no window), `-stdout` (TTY to stdout), `-softgpu`, `-interpreter`, `-run`, `-loadexe <file>`, `-iso <file>`, `-dofile <lua>`; with no BIOS configured it finds `openbios.bin` by its own fallback lookup, and runs with `DISPLAY` unset. Where that fallback looks (next to the binary, the working directory, or elsewhere) is read from the source at the pinned commit; the boot script places or points to the BIOS accordingly.
- **H-1.1-8 — Lua capture API.** At the pinned commit, a script run by `-dofile` can listen to the `GPU::Vsync` event, call `PCSX.GPU.takeScreenShot()` (returning the displayed area's bytes, width, height and bit depth) and `PCSX.quit()` (exit 0). Raw `PCSX.GPU.getVRAM()` does not exist at this commit.
- **H-1.1-9 — Trial results.** The patched build printed `Auto test all GTE valid opcodes/registers` on stdout for `test-all.exe`, then looped forever (so a boot must end by `PCSX.quit()` or a timeout); two BuzzyBee disc boots captured at vsync 600 gave the same 320x240 capture, and a boot with no disc gave a different one. This step re-measures all of it; nothing in the check relies on those hashes.
- **H-1.1-10 — Test PS-EXE.** `gte/test-all/test-all.exe` inside the ps1-tests `build-158` `tests.zip`, sha256-pinned in the list; the known line is `Auto test all GTE valid opcodes/registers`, printed before any test runs.
- **H-1.1-11 — Test CD image.** BuzzyBee v1.1, `buzzy.bin` and `buzzy.cue` from `psx.arthus.net`, each sha256-pinned; the `.cue` names the `.bin` by its plain file name, so both sit in one directory. It carries no Sony license data.
- **H-1.1-12 — Net check target.** `make psx-net-check` exists, is outside `make test-all`, passes in this sandbox, and exits non-zero naming any unreachable host. This step makes the setup depend on it.
- **H-1.1-13 — Step 1.1 note.** Section 6 of `docs/plan-9.md` holds the step 1.1 note with the choices above and the host list; this step's note follows it.
- **H-1.1-14 — Pad input unmeasured.** `PCSX.SIO0.slots[n].pads[m].setOverride(button)` exists in the Lua API at the pinned commit but was never tried; this step's trial is the first measurement.

If any hypothesis is false in a way the step cannot follow (for instance, the emulator in the list cannot boot headless, or a pin is missing), the step fails with a report, per the plan rule; it never swaps in an unmeasured source.

## Implementation Decisions

### Setup: `make psx-emulator-setup`

- A new shell script in `tools/`, run by the target, in the style of the raylib vendoring script. It takes the install directory and the sources list as arguments; the Makefile passes `PSX_EMULATOR_DIR` (default: a `pcsx-redux` directory under `third_party/`, already gitignored) and `PSX_SOURCES`.
- The target depends on `psx-net-check`: no download starts while a listed host is unreachable.
- Steps, in order:
  1. apt install of the H-1.1-5 packages with `sudo`, skipped when `PSX_EMULATOR_SKIP_APT` is set (same pattern as `RAYLIB_VENDOR_SKIP_APT`). apt is the only system-wide change; everything else lands in the install directory.
  2. Clone of the emulator URL into the install directory, checkout of the base commit (detached), recursive submodule init.
  3. Fetch of the fix commit from the same URL, and its diff against its parent applied to the working tree; a diff that does not apply cleanly fails the setup with a message naming the fix commit. The fix is not committed into the clone.
  4. Build of the emulator, then of OpenBIOS (H-1.1-6), each under a `timeout` (45 minutes, as the existing toolchain script uses) so a hung build fails.
  5. The BIOS placed where the boot finds it (H-1.1-7).
  6. A completion stamp written last in the install directory, holding the two commits it was built from.
- **Re-runs.** A directory whose stamp names the same two commits: the setup prints that it is already installed and exits 0. A non-empty directory without a matching stamp (partial build, other commits, foreign files): the setup fails and tells the user to remove it. A missing or empty directory: full install. No automatic deletion.
- The setup reads every URL and pin from the sources list (H-1.1-1, H-1.1-2); no URL or commit is written in the script. A pin missing from the list fails the setup naming the component.

### Boot: `make psx-boot`

- A second shell script in `tools/`, plus one Lua script in `tools/` that the emulator runs with `-dofile`.
- **Interface.** Arguments: the install directory, an input (`--exe <file>`, `--iso <cue>`, or neither for an empty boot), a frame count, and an output prefix. Make variables: `PSX_EXE`, `PSX_ISO` (mutually exclusive; both set is an error), `PSX_FRAMES` (default 600), `PSX_OUT` (default a prefix under `tmp/psx/`). It writes `<prefix>.vram` (the capture) and `<prefix>.tty.txt` (the TTY). Phase 2 and plans 10 and 11 call this same script.
- **Command line.** `-cli -stdout -softgpu -interpreter -run`, plus `-loadexe` or `-iso`, plus `-dofile` of the capture script (H-1.1-7). The interpreter, not the dynarec: it is what the 1.1 trial measured as deterministic, and speed does not matter here.
- **Capture script.** Counts `GPU::Vsync` events; at the frame count given (passed through the environment, which the script reads), it writes the capture file and calls `PCSX.quit()`. The capture file format, fixed now for step 2.3 and plan 10: one ASCII header line `psx-vram <width> <height> <depth>` ending in `\n`, then the raw bytes of `takeScreenShot()`'s data, unchanged. The header makes a mode change a different file, and the raw tail keeps the format trivial to decode.
- **TTY file.** The emulator's stdout, redirected to the TTY file. stderr goes to a separate log next to it (`<prefix>.log`), so emulator diagnostics never pollute the TTY text. If the pinned build writes its own log lines to stdout as well, the implementer records it in the step note and keeps them: the check searches for a line, it does not compare the whole file.
- **Isolation.** Each boot runs in a fresh temporary working directory created by the script, with `HOME` and `XDG_CONFIG_HOME` pointing into it and `DISPLAY` / `WAYLAND_DISPLAY` unset; the directory is removed after the boot. No configuration file, memory card image or cache survives from one boot to the next. Input paths are made absolute before the change of directory.
- **Termination.** The emulator runs under `timeout` (default 120 seconds, `PSX_BOOT_TIMEOUT`). The boot fails, with one message naming the input, when: the emulator exits non-zero (including a timeout's 124, or a crash), or the capture file was not written. A boot that ends normally exits 0 even if the program drew nothing: a black screen is a deterministic, valid capture (plan step 2.1 says so).
- Before booting, the script checks that the install directory holds a completion stamp and a BIOS, and fails with a message pointing at `make psx-emulator-setup` otherwise.

### Check: `make psx-emulator-check`

- A third shell script in `tools/`.
- **Test media.** Fetched into a gitignored directory under `tmp/` from the URLs in the sources list (H-1.1-10, H-1.1-11), each verified against its sha256 pin; a file already present with the right sha256 is not fetched again; a mismatch deletes it and fails naming the component. `test-all.exe` is extracted from the ps1-tests zip (the zip is the pinned file). Downloads go through the proxy and its CA bundle, the same way the probe reaches them.
- **Five boots**, each through the boot script with the default frame count, outputs under `tmp/psx/check/`: the `.exe` twice, the `.cue` twice, nothing once.
- **Four conditions**, all evaluated before concluding, one line per failure (`psx-emulator-check: <condition>: <detail>`), exit 1 if any failed, 0 otherwise:
  1. The two `.exe` captures are byte-identical.
  2. The first `.exe` boot's TTY file contains the known line (H-1.1-10), searched as a fixed string.
  3. The two `.cue` captures are byte-identical.
  4. The `.cue` capture differs from the empty boot's capture.
  A boot that failed counts as a failure of every condition that needs it, and the other conditions are still checked.
- On success it prints the sha256 of each capture, which the implementer copies into the step note.
- The known line and the frame count are Make variables with the H-1.1-10 value and 600 as defaults, so step 2.1 can reuse the condition with its own line.

### Scripted pad input (recorded, not built)

- One trial, by hand, recorded in the step note: a copy of the capture script that also calls `setOverride` on pad 1 of slot 1 for a short window (Start, then Cross) during the BuzzyBee boot, captured at a frame after the window; compared to the plain BuzzyBee capture at the same frame. The note records: whether the call exists and runs headless, whether the capture changed, whether two runs with the override are byte-identical, and the exact Lua calls used. A negative result does not fail the step. Nothing from the trial is committed beyond the note.

### Makefile and repository

- Three new phony targets, `psx-emulator-setup`, `psx-boot`, `psx-emulator-check`, outside `test`, `e2e` and `test-all`, each with a legend entry at the top in the existing style (what it does, what it needs, "outside the suite").
- `make psx-emulator-check` does not depend on the setup target (an 8-minute build must never start as a side effect of a check); it fails at once if the install is missing.
- Nothing is added to `.gitignore` if the install directory and outputs land under `third_party/` and `tmp/`, which are already ignored.
- Comments in the new scripts follow CLAUDE.md: only traps (for example why the fix commit is applied, why the interpreter) and contracts.

### Step note

- The implementer appends the step 1.2 note to section 6 of `docs/plan-9.md`: the setup command and its wall time from an empty directory; the capture format; the five capture sha256 and the TTY line found; whether any H-1.1-k turned out false and what was followed instead; the pad input finding (whether, and how).

## Implementation Chunks

1. **Setup.** The setup script and `make psx-emulator-setup` (with its dependency on `psx-net-check` and its legend entry). Done when a run into an empty `PSX_EMULATOR_DIR` under `tmp/` completes and writes its stamp, and a second run exits 0 at once.
2. **Boot.** The boot script, the capture Lua script and `make psx-boot`. Done when booting `test-all.exe`, the BuzzyBee `.cue` and nothing each writes a capture and a TTY file, against the chunk 1 install.
3. **Check, pad trial, note.** The check script and `make psx-emulator-check`; the pad trial; the step note. Done when the check passes against the chunk 1 install (the step's verification), and fails as expected on the hand tests below.

## Testing Decisions

- A good test here checks what an agent relies on: the files a boot writes and the facts the check asserts, never the emulator's internals.
- No unit test is added to `tests/`: everything here is network-, build- or emulator-bound, and `make test` / `make e2e` must stay untouched and no slower. None of the new targets is in `make test-all`.
- **The step's verification:** remove the install directory used for the check (a fresh one under `tmp/`), run `make psx-emulator-setup` into it, then `make psx-emulator-check` against it: it exits 0.
- **Failure paths, verified once by hand** during the step and recorded in the note:
  - the check with the known line set to a string the program never prints: exits 1, one line naming the TTY condition, the other three conditions still reported as passing;
  - a boot with a frame count that cannot be reached before a short `PSX_BOOT_TIMEOUT`: the boot fails naming its input;
  - `make psx-boot` against an install directory with no stamp: fails pointing at the setup target;
  - the setup into a non-empty directory with no stamp: fails without deleting anything.
- Prior art: `tools/e2e_limits.sh` (every item checked before concluding, one message per failure, exit 1), `tools/install_raylib_vendor.sh` (vendoring into `third_party/`, apt skip variable), and the step 1.1 probe script.

## Out of Scope

- Choosing or changing sources, pins or hosts (step 1.1). A host this step needs that is not in the list is added to the list and the probe, per the plan rule.
- The PSn00bSDK toolchain, `psx-hello`, `mkpsxiso`, any CD image of ours (phase 2).
- A committed reference sha256 for any capture (step 2.3 commits the `psx-hello` one). The check compares boots with each other, not with a reference.
- A reusable scripted-pad replay feature: only the trial and its note (plans 10 and 11 build on the finding).
- Raw full-VRAM dumps (not available at the pinned commit), audio, CD audio tracks.
- Rewriting `docs/psx-hello-world.md` (step 2.4).

## Further Notes

- The plan's "prefer prebuilt binaries" rule is about GCC; the emulator is built from source because its prebuilt host is denied (recorded in step 1.1's PRD).
- The setup's 8-minute build is why the check never triggers it, and why the setup skips a complete install.
- Determinism risks the isolation decision addresses: PCSX-Redux keeps a configuration file and may create memory card images on first run; both would differ between a first and a second boot if they survived. If two boots still differ after isolation, the implementer bisects by frame count (an earlier capture frame) before declaring the step failed, and records the finding.
- Written in parallel with step 1.1's implementation: see "Hypotheses on step 1.1's outcome".
