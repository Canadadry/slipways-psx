---
title: "psx-hello CD image and its boot proof"
description: "Nothing packs psx-hello into a .bin/.cue with a SYSTEM.CNF, and nothing proves such an image boots psx-hello deterministically in the windowless emulator."
status: needs-triage
---

## Step

Plan `docs/plan-9.md`, phase 2 — Toolchain and image, step 2 (verbatim):

> 2. **CD image.** A `make` target produces `psx-hello.bin/.cue` (with a tool like `mkpsxiso`), including the license and `SYSTEM.CNF`. Verified: two boots give byte-identical captures, as in step 1.2, with the same proof that `psx-hello` ran as in step 2.1. The proof is step 2.1's check script, extended to take the CD image as well as the `.exe`, not a second script.

## Problem Statement

Plan 9 ends on a `.bin/.cue`, the form a PSX build ships in, and step 2.3 commits the sha256 of the capture of that image's boot. After step 2.1, `psx-hello` exists only as a PS-EXE side-loaded by the emulator (`-loadexe`), which skips the BIOS's CD boot path entirely:

- No target packs `psx-hello.exe` into a disc image. The installed PSn00bSDK release ships `mkpsxiso` 2.03 and a CMake helper for it, but `psx-hello/` uses neither, and has no `SYSTEM.CNF` and no image description.
- Nothing proves that the BIOS finds `SYSTEM.CNF` on the disc, loads the executable it names, and that `psx-hello` then runs as it does when side-loaded.
- Step 2.1's check script (`tools/psx_hello_check.sh`) only accepts `--exe`, so step 2.3 has no proved image boot to take its reference from.

## Solution

- `psx-hello/` gains a `SYSTEM.CNF` and an `mkpsxiso` image description, and its CMake project declares the CD image with the SDK's own `psn00bsdk_add_cd_image` helper. The image holds two files: `SYSTEM.CNF` and the executable as `PSXHELLO.EXE` (ISO 9660 8.3 name). No license data, per the step 1.1 note (see Implementation Decisions).
- A new `make psx-hello-cd` builds `psx-hello/build/psx-hello.bin` and `psx-hello/build/psx-hello.cue` against the installed SDK.
- Step 2.1's check script already takes its input as a flag (`--exe <file>`); it is extended to accept `--iso <cue>` as well, the same two flags as the step 1.2 boot script. With `--iso` it boots the image twice and nothing once, and checks the same proof as for the `.exe`: identical captures, a capture different from the empty boot, not one uniform color, the known TTY line present. The `.exe`-only `magic` condition runs only with `--exe`; with `--iso` a `cue` condition on the image files takes its place.
- A new `make psx-hello-cd-check` runs that script on the image; `make psx-hello-check` keeps checking the `.exe`, through the same script.

The step is verified by running `make psx-hello-cd-check`: it exits 0. `make psx-hello-check` still exits 0.

A trial in the scratchpad (no repository change; a copy of `psx-hello/` with the same `printf` step 2.1 committed, the pinned SDK unzipped, the step 1.2 emulator install and the committed boot script) already showed:

- `psn00bsdk_add_cd_image(psx-hello-iso psx-hello iso.xml DEPENDS psx-hello system.cnf)` with the template's image description (two files, `<dummy sectors="1024"/>`) builds a 2 490 768-byte `.bin` (1059 sectors, `MODE2/2352`) and a 72-byte `.cue` in the build directory, in well under a second.
- The `.exe` is byte-identical across two build directories (`b79ccaeb…`); the `.bin` is **not** (`f3ba1bff…` vs `0b724a5d…`): `mkpsxiso` writes the build time into the volume descriptors. Only the capture is a reference (step 2.3), never the `.bin`.
- Two `--iso` boots of two different builds gave the same capture, `b4934260…`, 320×240, depth 16, three distinct pixel values; the empty boot gave `d5b36d08…` (step 1.2's); the side-loaded `.exe` gave `b4934260…` too.
- The TTY file of a CD boot holds OpenBIOS's `BOOTSTRAP LOADER` lines (`setup file    : cdrom:SYSTEM.CNF;1`, `BOOT =	cdrom:\PSXHELLO.EXE;1`, `boot file     : cdrom:\PSXHELLO.EXE;1`), then `psxgpu: setup done, default mode is NTSC`, `PS-X Control PAD Driver`, `psx-hello: main loop started`. Two TTY files differ (the emulator logs the image's absolute path), as step 1.2's note warns.
- A `SYSTEM.CNF` naming a missing file (`NOPE.EXE`): OpenBIOS prints a register dump and `=== halting ===`, no vsync ever reaches frame 600, the boot times out after `PSX_BOOT_TIMEOUT` (120 s) and writes no capture; the boot script exits 1.
- Three sequential 600-frame boots take ~32 s.

## User Stories

1. As the plan-pilot run, I want one `make` target that builds `psx-hello.bin/.cue`, so that every later step boots the same kind of image.
2. As the plan-pilot run, I want the image built with the `mkpsxiso` that ships in the pinned SDK release, so that no new tool or host is needed.
3. As the plan-pilot run, I want the image built through the SDK's own CMake helper, so that the image and the `.exe` come from one configure and one build directory.
4. As the plan-pilot run, I want the image target to reach no network host, so that the plan's network rule holds trivially.
5. As the plan-pilot run, I want the image to hold a `SYSTEM.CNF` naming the executable, so that the BIOS boots it the way a retail disc boots.
6. As the plan-pilot run, I want the executable stored under an 8.3 name, so that the BIOS's ISO 9660 reader finds it.
7. As the plan-pilot run, I want the `SYSTEM.CNF` to use the SDK template's `TCB`, `EVENT` and `STACK` values, so that the kernel is set up as the SDK expects.
8. As the plan-pilot run, I want the decision not to include license data recorded with its reason, so that the plan's "including the license" is answered, not silently dropped.
9. As an agent, I want `make psx-hello-cd` to fail pointing at the setup target when the SDK is missing, so that the fix is obvious.
10. As an agent, I want the `.bin` and `.cue` at fixed paths in the gitignored build directory, so that the check and step 2.3 pick them up without arguments.
11. As an agent, I want the `.cue` to reference the `.bin` by a relative name, so that the boot script can be given the `.cue` from any working directory.
12. As the plan-pilot run, I want the same check script to take `--iso <cue>`, so that the CD image is proved by exactly the conditions that proved the `.exe`.
13. As the plan-pilot run, I want the CD check to boot the image twice and compare the captures byte for byte, so that the image boot is proved deterministic.
14. As the plan-pilot run, I want the CD check to boot nothing once and compare, so that an image the BIOS rejects is not mistaken for `psx-hello`.
15. As the plan-pilot run, I want the CD check to fail a capture of one uniform color, so that a hang on a cleared screen is not mistaken for `psx-hello` drawing.
16. As the plan-pilot run, I want the CD check to find the known line in the TTY file, so that `main` is proved to have run from the disc.
17. As the plan-pilot run, I want the CD check to verify the `.cue` names an existing `.bin` whose size is a whole number of 2352-byte sectors, so that a broken image is named as such and not as an emulator failure.
18. As the plan-pilot run, I want a boot that fails (for instance a `SYSTEM.CNF` naming a missing file, which halts the BIOS) to fail every condition that needs it, with the others still checked, so that one run reports everything that is wrong.
19. As the plan-pilot run, I want the CD check to exit non-zero if any condition fails, so that the step fails as the plan rule demands.
20. As the plan-pilot run, I want the CD check to print the sha256 of each capture and the capture's size and depth on success, so that the step note records them and step 2.3 can take its reference from a passing run.
21. As the implementer of step 2.3, I want the CD check to be callable as one script with `--iso`, so that the check target runs that same proof before comparing the reference.
22. As the plan-pilot run, I want `make psx-hello-check` to keep working on the `.exe` with the same conditions and output, so that step 2.1's verification still holds.
23. As a maintainer, I want one check script for both inputs, so that a fix to the proof applies to the `.exe` and the image at once.
24. As the plan-pilot run, I want the CD check never to trigger the emulator setup or the toolchain setup, so that a long install never starts as a side effect.
25. As the plan-pilot run, I want the CD check's outputs in their own directory under `tmp/psx/`, so that the `.exe` check and the CD check never overwrite each other's files.
26. As a maintainer, I want `make test` and `make e2e` untouched, so that the fast loop stays fast.
27. As a maintainer, I want the new targets in the Makefile legend, so that they are found like every other target.
28. As a maintainer, I want nothing generated committed, so that the repository holds only the image's sources (`SYSTEM.CNF` and its description).
29. As a human reading the plan later, I want the step note to record the image size, the capture hashes, the TTY boot lines and every deviation from this PRD, so that I can audit the step without rerunning it.

## Step 2.1's outcome

This PRD was first written in parallel with step 2.1, with each assumption about its outcome recorded as a hypothesis `H-2.1-k`. Step 2.1 has since landed. Every hypothesis was checked against the committed step 2.1 note, `tools/psx_hello_check.sh`, the Makefile and `psx-hello/`; the decisions below follow the actual outcome.

- **H-2.1-1 — Step 2.1 passed: holds.** `make psn00bsdk-toolchain-setup`, `make psx-hello` and `make psx-hello-check` exist and passed; the note is in section 6 of `docs/plan-9.md`.
- **H-2.1-2 — SDK install directory: holds.** `PSN00BSDK_DIR`, default `third_party/psn00bsdk`, with the release tree at its root and a `psn00bsdk.stamp`.
- **H-2.1-3 — Build command: holds.** `make psx-hello` fails without the stamp, then runs `cmake --fresh -S psx-hello -B psx-hello/build -DCMAKE_TOOLCHAIN_FILE=<abs PSN00BSDK_DIR>/lib/libpsn00b/cmake/sdk.cmake` and `cmake --build psx-hello/build`. That builds every `ALL` target, so it also builds the CD image once it is declared.
- **H-2.1-4 — No `elf2x` workaround: holds.** `elf2x` did not crash; the `.exe` (`b79ccaeb…`) comes from the CMake build and is identical across build directories.
- **H-2.1-5 — Known TTY line: holds.** `psx-hello: main loop started`, printed once after the pad setup, in `PSX_HELLO_TTY_LINE`.
- **H-2.1-6 — Check script signature: did not hold as stated.** The signature is `tools/psx_hello_check.sh <install-dir> --exe <file> <known-line> <frames> <out-dir>`: the input is already a flag, checked by a `case` that accepts only `--exe`. The script takes exactly six arguments and exits at once if the input file is missing. This step therefore does not redesign the argument list. It adds `--iso` to that `case`, and `make psx-hello-check` stays unchanged.
- **H-2.1-7 — Check behavior: holds, with two deviations recorded in the step 2.1 note.** The boots are named `run1`, `run2` and `empty`. The conditions are `magic`, `deterministic`, `differs-from-empty`, `not-uniform` and `tty-line`, all evaluated before concluding. Each failure prints one `psx-hello-check: <condition>: <detail>` line on stderr, and a failed boot fails only the conditions that need it. The two deviations: each passing condition also prints a `psx-hello-check: <condition>: ok` line, and the script calls `tools/psx_boot.sh` directly. On success it prints the input's sha256, then for each capture its header (`psx-vram <w> <h> <depth>`) and sha256, then `psx-hello-check: all five conditions hold`. It empties its output directory first.
- **H-2.1-8 — Sources untouched: holds.** Step 2.1 changed only `psx-hello/main.c` (the `printf` and `#include <stdio.h>`); `CMakeLists.txt` is unchanged, and there is no `SYSTEM.CNF` or image description.
- **H-2.1-9 — Exe boot capture: holds.** 320×240×16, `b4934260…` twice; empty boot `d5b36d08…`. A pad is present on port 1 (`PORT 1: TYPE=4`).

## Implementation Decisions

### Image sources in `psx-hello/`

- A `system.cnf` (lowercase source name, as the SDK template) holding exactly four lines: `BOOT=cdrom:\PSXHELLO.EXE;1`, `TCB=4`, `EVENT=10`, `STACK=801FFFF0` (the template's values; the BIOS reads them, the trial's TTY echoed them).
- An `iso.xml` image description, trimmed from the SDK template: one data track; identifiers `system` and `application` `PLAYSTATION` (required for the disc to be recognized), `volume` and `volume_set` `PSX_HELLO`, `publisher` `SLIPWAYS`, `data_preparer` `PSN00BSDK ${PSN00BSDK_VERSION}`; a root holding `SYSTEM.CNF` (source `${PROJECT_SOURCE_DIR}/system.cnf`) and `PSXHELLO.EXE` (source the built `psx-hello.exe`, relative to the build directory); the template's `<dummy sectors="1024"/>` padding. The template's explanatory comments are not copied (CLAUDE.md comment rule); the one trap kept is that names must be 8.3.
- **No license data.** The plan step says "including the license"; step 1.1 recorded the alternative the plan allows: PSn00bSDK ships none, OpenBIOS does not check it, and Sony's license data cannot be committed. The image has no `<license>` element. The step note records this as a deviation from the step's wording, settled by step 1.1's note. If a later plan targets real hardware or a Sony BIOS, it adds the license then.
- `psx-hello/CMakeLists.txt` adds one call: `psn00bsdk_add_cd_image(psx-hello-iso psx-hello iso.xml DEPENDS psx-hello system.cnf)`. Image file names are therefore `psx-hello.bin` / `psx-hello.cue` in `psx-hello/build/`, matching the plan's wording. The helper runs the SDK's `mkpsxiso` (found through the toolchain file's own hints) with `-y`, and the `.cue` it writes names the `.bin` relatively (`FILE "psx-hello.bin" BINARY`).
- `psx-hello/main.c` is not touched by this step.
- The `.bin` is not reproducible byte for byte (timestamps in the volume descriptors); this step does not try to make it so. Nothing compares `.bin` hashes.

### Build: `make psx-hello-cd`

- New phony target `psx-hello-cd`, depending on `psx-hello` (which, per H-2.1-3, already configures fresh and builds every `ALL` target, the image included), whose recipe runs `cmake --build psx-hello/build --target psx-hello-iso` (a no-op when up to date) and fails if either `psx-hello/build/psx-hello.bin` or `psx-hello/build/psx-hello.cue` is missing.
- New Make variable `PSX_HELLO_CUE`, default `psx-hello/build/psx-hello.cue`, used by the check target and later by step 2.3.
- `psx-hello-clean` already removes the build directory, image included; unchanged.

### Check: one script, two inputs

- `tools/psx_hello_check.sh` is extended in place; no second script. Its signature stays `<install-dir> <--exe|--iso> <file> <known-line> <frames> <out-dir>`: `--iso` joins `--exe` in the `case` on the input flag, and the usage text names both. The flag is passed through unchanged to each `tools/psx_boot.sh` call, which already accepts both. The existing early exit on a missing input file now also covers a missing `.cue`.
- **Boots**, unchanged in names, number and order: `run1` and `run2` of the input, then `empty`.
- **Conditions**, still five per run, all evaluated before concluding, same `ok` / failure lines as today:
  1. With `--exe`: `magic`, unchanged. With `--iso`: `cue` instead (`magic` is not run). The `cue` condition requires the `.cue`'s `FILE` line to name a `.bin` that exists next to the `.cue`, is non-empty and has a size that is a multiple of 2352. It sits where `magic` sits, after the boots.
  2. to 5. `deterministic`, `differs-from-empty`, `not-uniform`, `tty-line`: unchanged code, applied to whichever input was booted.
- **Success output**, unchanged in form: the input's sha256 line, then for each capture its header and sha256, then `psx-hello-check: all five conditions hold` (still true in both modes, so step 2.3 can rely on one success line). With `--iso` an extra sha256 line for the `.bin` follows the `.cue`'s. It is informational only, because the `.bin` changes on every build.
- The message prefix stays `psx-hello-check:` in both modes. The input path in the first success line and the output directory tell the two runs apart.
- No condition reads OpenBIOS's `BOOTSTRAP LOADER` lines. With `-iso` and no `-loadexe`, the disc is the only way the `.exe` reaches memory, so `tty-line` already proves the CD path. The step note quotes those lines.
- Comments follow CLAUDE.md. The header's usage line is updated. One trap comment is allowed on `cue`: the `.bin` hash is not stable.

### Makefile

- `psx-hello-check` is unchanged: it already passes `--exe psx-hello/build/psx-hello.exe`.
- New phony target `psx-hello-cd-check`, depending on `psx-hello-cd` and on no setup target. It runs the script with `--iso $(PSX_HELLO_CUE)` and the same `PSX_EMULATOR_DIR`, `PSX_HELLO_TTY_LINE`, `PSX_FRAMES` and `PSX_BOOT_TIMEOUT` as `psx-hello-check`, with the output directory `$(TMP_DIR)/psx/hello-cd-check`.
- Legend entries added for `psx-hello-cd` and `psx-hello-cd-check` (~32 s, outside the suite), next to the `psx-hello-check` entry. That entry is left as is unless it describes the script as `.exe`-only.
- `psx-hello-cd` and `psx-hello-cd-check` join the `.PHONY` list. None of these targets is in `test`, `e2e` or `test-all`. No `.gitignore` change (`psx-hello/build/` and `tmp/` are ignored).

### Step note

- The implementer appends the step 2.2 note to section 6 of `docs/plan-9.md`. It records:
  - the image size in bytes and sectors, and whether the `.bin` differs across two builds;
  - the three capture sha256 values and the capture's width, height and depth, and whether the CD capture equals step 2.1's `.exe` capture (`b4934260…`);
  - the `BOOT` and `boot file` TTY lines and the known line found;
  - the no-license deviation and its reason;
  - the hand-tested failure paths;
  - any deviation from this PRD.

  No `guideline` decision was needed for this PRD.

## Implementation Chunks

1. **Image.** `system.cnf`, `iso.xml`, the `psn00bsdk_add_cd_image` call, `make psx-hello-cd`, `PSX_HELLO_CUE`, legend entry. Done when `make psx-hello-cd` produces a `.bin` whose size is a multiple of 2352 and a `.cue` naming it, and one `make psx-boot PSX_ISO=psx-hello/build/psx-hello.cue` writes a TTY file containing the known line.
2. **Check, verification, note.** `--iso` in the check script's input `case`, the `cue` condition in place of `magic` for it, the extra `.bin` sha256 line, `make psx-hello-cd-check`, legend entries; the verification; the hand-tested failure paths; the step note. Done when both `make psx-hello-cd-check` and `make psx-hello-check` exit 0.

## Testing Decisions

- A good test here checks what later steps rely on: the image files that are produced and the facts a boot of them shows, never `mkpsxiso`'s or the emulator's internals.
- No unit test is added to `tests/`: everything is toolchain- or emulator-bound, and `make test` / `make e2e` must stay untouched and no slower.
- **The step's verification:** with step 2.1's SDK install and step 1.2's emulator install in place, `make psx-hello-cd-check` exits 0; `make psx-hello-check` still exits 0 (the `.exe` mode was not broken by the extension).
- **Failure paths, verified once by hand** and recorded in the note:
  - `make psx-hello-cd-check` with `PSX_HELLO_TTY_LINE` set to a line the program never prints: exits 1, one `tty-line` failure, the four other conditions pass;
  - the script in `--iso` mode on a copy of the `.cue` under `tmp/` whose `FILE` line names a missing `.bin`: exits 1 with a `cue` failure, and each boot-dependent condition fails as a failed boot (run with a low `PSX_BOOT_TIMEOUT` if the emulator hangs instead of exiting);
  - the script with an unknown input flag (for instance `--bin`): refuses with the usage message naming `--exe` and `--iso`;
  - `make psx-hello-check` after the change: still exits 0 with `magic` among its five conditions, and no `cue` line.
  The halting-BIOS case (a `SYSTEM.CNF` naming a missing file: boot timeout, no capture) was observed in this PRD's trial and is not repeated: it would need a repository file edited and restored.
- Prior art: `tools/psx_hello_check.sh` (the script extended; its failure paths were hand-tested the same way in step 2.1), `tools/psx_emulator_check.sh` (boots both an `.exe` and a `.cue` through `tools/psx_boot.sh`), `tools/psx_boot.sh` (its `--exe`/`--iso` flags).

## Out of Scope

- A committed reference sha256 and the target comparing against it (step 2.3).
- Making the `.bin` byte-reproducible.
- License data, a Sony BIOS, or anything aimed at real hardware.
- CD-DA tracks, extra files on the disc, or any change to `psx-hello/main.c`.
- Installing, rebuilding or changing the emulator, its boot script, its check, or the SDK install (steps 1.2 and 2.1).
- Rewriting `docs/psx-hello-world.md` (step 2.4).
- Scripted pad input (plans 10 and 11).
- The game itself and audio.

## Further Notes

- In the trial the CD boot's capture equalled the side-loaded `.exe`'s (`b4934260…`): `psx-hello` draws the same frame 600 either way. That is an observation, not a condition: a later change to timing could legitimately split them.
- The SDK's CMake prints a `GNUInstallDirs` developer warning at configure time; it is the SDK's, harmless, and left alone.
- First written in parallel with step 2.1; its hypotheses were checked once step 2.1 landed: see "Step 2.1's outcome". Only H-2.1-6 (the script's signature) did not hold as stated, and the decisions follow the committed script.
