---
title: "PSn00bSDK toolchain setup and a booted psx-hello.exe"
description: "Nothing installs the pinned PSn00bSDK release or builds psx-hello with it, and nothing proves the resulting .exe runs in the windowless emulator and prints over TTY."
status: needs-triage
---

## Problem Statement

Plan 9 needs `psx-hello` compiled and proved to run, headless, before it can be packed into a CD image (step 2.2) and checked against a committed reference (step 2.3). Today:

- `make psn00bsdk-toolchain-setup` runs a script that was never run: it builds GCC 12.2.0 and binutils 2.40 from source off `ftpmirror.gnu.org` (a host outside the step 1.1 list, so forbidden by the plan's network rule), clones PSn00bSDK at an unpinned `HEAD`, and installs everything system-wide under `/usr/local` with `sudo`. It cannot be run into an empty install directory, and it ignores the pinned release step 1.1 chose.
- `make psx-hello` calls CMake with no toolchain file, so `psn00bsdk_add_executable` is undefined and the configure fails, whatever is installed.
- `psx-hello` prints nothing over TTY, so even a boot cannot show that its `main` ran, and plan 10 step 1 has no proof that a program of ours reaches the TTY file it will read the game state from.
- Nothing boots the `.exe` in the step 1.2 emulator, nor checks that its capture is deterministic and different from a boot with nothing loaded.

## Solution

- `make psn00bsdk-toolchain-setup` is rewritten: it installs the pinned `PSn00bSDK-0.24-Linux.zip` release (prebuilt `mipsel-none-elf` GCC 12.3.0, binutils, `elf2x`, `mkpsxiso`, libpsn00b) into an install directory (`PSN00BSDK_DIR`, default under the gitignored `third_party/`), with its URL and sha256 read from the step 1.1 sources list. No GCC is compiled, nothing is installed system-wide except a few apt packages (skippable), and it runs `make psx-net-check` first.
- `make psx-hello` configures `psx-hello/` with the SDK's CMake toolchain file from that install directory and builds `psx-hello/build/psx-hello.exe`.
- `psx-hello` prints one known line over TTY once, after its GPU and pad setup: `psx-hello: main loop started`.
- A new `make psx-hello-check` builds the `.exe`, boots it twice and boots nothing once, through the step 1.2 boot script, and checks: the `.exe` starts with the PS-EXE magic; the two `.exe` captures are byte-identical; the `.exe` capture differs from the empty boot's capture; the `.exe` capture is not a single uniform color; the known line is in the TTY file.

The step is verified by removing the install directory, running `make psn00bsdk-toolchain-setup` into it, then `make psx-hello-check`: it exits 0.

A trial in a scratch directory (no install, no repository change) already showed: the pinned zip has the expected sha256 (79 MB, one top-level directory `PSn00bSDK-0.24-Linux/`); its host binaries need only the host libc/libstdc++ of Ubuntu 24.04; `cmake -S psx-hello -B <dir> -DCMAKE_TOOLCHAIN_FILE=<sdk>/lib/libpsn00b/cmake/sdk.cmake` with the host's CMake 3.28 and either generator configures in 0.2 s and builds in under a second; `elf2x` did not crash (the trap of `docs/psx-hello-world.md` was seen with binutils 2.42, the release ships its own); the `.exe` starts with `PS-X EXE`; three builds in three different build directories gave the same `.exe` sha256 (`5a29fb31…` before the TTY line was added); `printf` in libpsn00b is the BIOS call A(3Fh); the libpsn00b in this release already logs its own lines through it (`psxgpu: setup done, default mode is …`), so the known line must be our own text; a `printf` of the known line compiles and the string is in the `.exe`.

## User Stories

1. As the plan-pilot run, I want one `make` target that installs the PSn00bSDK toolchain, so that every later step builds with the same compiler.
2. As the plan-pilot run, I want the setup to install the exact release pinned in step 1.1, so that a `.exe` built today matches one built in plan 11.
3. As the plan-pilot run, I want the setup to read the URL and sha256 from the sources list, so that the probe, the list and the setup never drift apart.
4. As the plan-pilot run, I want the setup to verify the archive's sha256 before unpacking it, so that a changed or truncated download is refused.
5. As the plan-pilot run, I want the setup to run the network probe first, so that a blocked host fails at once and is named.
6. As the plan-pilot run, I want the setup to reach no host outside the step 1.1 list, so that the plan's network rule holds.
7. As the plan-pilot run, I want no GCC compiled, so that the setup takes seconds and follows the plan's prebuilt rule.
8. As the plan-pilot run, I want the setup to work into an empty directory, so that the verification proves nothing left over from a trial is needed.
9. As the plan-pilot run, I want the setup to refuse a non-empty directory it did not fill, so that a half-unpacked or foreign tree is never mistaken for a good install.
10. As the plan-pilot run, I want a second setup into a complete install to do nothing and exit 0, so that re-running it is harmless.
11. As the plan-pilot run, I want a failed setup to leave no completion stamp, so that the next run never trusts a broken tree.
12. As the plan-pilot run, I want the setup to check that the installed compiler runs and reports 12.3.0, so that a broken unpack fails the setup, not the build.
13. As a maintainer, I want nothing installed under `/usr/local` and no `sudo` except for apt, so that the toolchain lives in one deletable directory.
14. As a maintainer, I want the apt step skippable, as for raylib, so that a machine that already has the packages needs no `sudo`.
15. As an agent, I want `make psx-hello` to build against the installed SDK with one command, so that I never pass a toolchain path by hand.
16. As an agent, I want `make psx-hello` to fail with a message pointing at the setup target when the SDK is missing, so that the fix is obvious.
17. As an agent, I want `make psx-hello` to reconfigure from scratch each time, so that a CMake cache pointing at a deleted or older SDK never survives.
18. As an agent, I want `make psx-hello` to accept another SDK directory through a variable, so that the verification can use a fresh one.
19. As the implementer of plan 10 step 1, I want `psx-hello` to print one known line over TTY, so that I know a program of ours reaches the TTY file I will read the game state from.
20. As an agent, I want the known line printed once, not every frame, so that the TTY file stays small and the search is unambiguous.
21. As an agent, I want the known line printed after the GPU and pad setup, so that its presence proves `main` got past initialization.
22. As an agent, I want the known line to be text the SDK never prints, so that the search cannot match a library log line.
23. As the plan-pilot run, I want `make psx-hello-check` to boot the `.exe` twice and compare the captures byte for byte, so that the capture is proved deterministic.
24. As the plan-pilot run, I want the check to boot nothing once and compare, so that a capture equal to an empty boot fails.
25. As the plan-pilot run, I want the check to fail a capture of one uniform color, so that a hang on a cleared screen is not mistaken for `psx-hello` drawing its text.
26. As the plan-pilot run, I want the check to search the TTY file for the known line as a fixed string, so that the proof that `main` ran is exact.
27. As the plan-pilot run, I want the check to verify the `.exe` starts with the PS-EXE magic, so that an `elf2x` failure is named as such and not as an emulator failure.
28. As the plan-pilot run, I want the check to evaluate every condition before concluding, with one line per failure, so that one run reports everything that is wrong.
29. As the plan-pilot run, I want the check to exit non-zero if any condition fails, so that the step fails as the plan rule demands.
30. As the plan-pilot run, I want the check to print the sha256 of each capture and of the `.exe` on success, so that the step note records them.
31. As the plan-pilot run, I want the check never to trigger the emulator setup or the toolchain setup, so that a long install never starts as a side effect.
32. As the implementer of step 2.2, I want the `.exe` at a fixed path, so that the CD image target picks it up.
33. As the implementer of step 2.3, I want the boot of `psx-hello` already proved deterministic, so that a committed reference sha256 means something.
34. As a maintainer, I want `make test` and `make e2e` untouched, so that the fast loop stays fast.
35. As a maintainer, I want the new and changed targets in the Makefile legend, so that they are found like every other target.
36. As a maintainer, I want the install directory, the build directory and every boot output in gitignored places, so that nothing large or generated is committed.
37. As a human reading the plan later, I want the step note to record the setup time, the `.exe` and capture hashes, the TTY line found and every hypothesis that turned out false, so that I can audit the step without rerunning it.

## Hypotheses on step 1.2's outcome

Step 1.2 is being implemented in parallel. This PRD relies only on its committed PRD (`docs/prd/triage/phase1-step2-psx-headless-emulator.md`) and on the committed step 1.1 note and sources list. When step 1.2 lands, the implementer of this step checks each hypothesis against the actual step 1.2 note, scripts and Makefile, and follows step 1.2 where it differs: the hypothesis is the fallback, never the authority.

- **H-1.2-1 — Step 1.2 passed.** `make psx-emulator-setup` and `make psx-emulator-check` exist and passed in this sandbox; the step 1.2 note is in section 6 of `docs/plan-9.md`. If step 1.2 failed, the plan stopped there and this step is never run.
- **H-1.2-2 — Install directory.** The emulator install directory is the Make variable `PSX_EMULATOR_DIR` (default a `pcsx-redux` directory under `third_party/`), holding a completion stamp and the OpenBIOS image. This step reads that variable and never installs or rebuilds the emulator.
- **H-1.2-3 — Boot interface.** A boot script in `tools/` (the one `make psx-boot` runs) takes the install directory, `--exe <file>` or no input for an empty boot, a frame count and an output prefix; it makes input paths absolute itself. It writes `<prefix>.vram` (capture), `<prefix>.tty.txt` (TTY) and `<prefix>.log` (stderr). This step calls that script directly, once per boot, and adds no second boot path.
- **H-1.2-4 — Boot termination.** The boot ends by `PCSX.quit()` at the frame count (default 600, `PSX_FRAMES`) under a wall-clock timeout (`PSX_BOOT_TIMEOUT`, default 120 s); it exits 0 whenever a capture was written, even of a black screen, and non-zero, naming its input, on a crash, a timeout or a missing capture. `psx-hello` loops forever, so it relies on this.
- **H-1.2-5 — Boot fails fast without an install.** The boot script fails with a message pointing at `make psx-emulator-setup` when the install directory has no stamp or no BIOS; this step relies on that and adds no check of its own.
- **H-1.2-6 — Capture format.** One ASCII header line `psx-vram <width> <height> <depth>` ending in `\n`, then the raw bytes of `PCSX.GPU.takeScreenShot()`'s data. The uniform-color condition reads the depth from the header (16 bits: 2 bytes per pixel; 24 bits: 3 bytes per pixel).
- **H-1.2-7 — Determinism holds for any PS-EXE.** Each boot is isolated (fresh working directory, `HOME`, no surviving config or memory card), with `-interpreter -softgpu`, so two boots of the same `.exe` are byte-identical as they were for `test-all.exe`. If two `psx-hello` boots differ, the implementer bisects by frame count, as step 1.2's PRD says, before declaring the step failed.
- **H-1.2-8 — The empty boot works.** A boot with no input under OpenBIOS writes a capture, and that capture is not the `psx-hello` dark-blue screen with text.
- **H-1.2-9 — BIOS `printf` reaches the TTY file.** Under the pinned PCSX-Redux and OpenBIOS, text printed through the BIOS A(3Fh) `printf` (the one libpsn00b's `printf` jumps to) lands in the emulator's stdout, so in `<prefix>.tty.txt`, as `test-all.exe`'s line did. The TTY file may also hold libpsn00b's own log lines and emulator log lines; the check searches for one line, it does not compare the file.
- **H-1.2-10 — Pad port.** Whether the emulator presents a pad on port 1 headless changes only which text `psx-hello` draws (`PORT 1: TYPE=…` or `NO PAD DETECTED`); either is deterministic, and no condition depends on it. The step note records which one the capture shows.
- **H-1.2-11 — Outputs under `tmp/psx/`.** Boot outputs live under the gitignored `tmp/psx/`; this step writes its own under `tmp/psx/hello-check/`.
- **H-1.2-12 — Check style.** `make psx-emulator-check` reports one line per failed condition (`psx-emulator-check: <condition>: <detail>`) and evaluates all conditions before concluding; this step's check uses the same style with its own prefix, and does not modify step 1.2's check script.
- **H-1.2-13 — Sources list unchanged for the toolchain.** Step 1.2 leaves the `psn00bsdk` line of the sources list (URL and sha256) and the `release-assets.githubusercontent.com` host line as step 1.1 committed them.

If any hypothesis is false in a way this step cannot follow (for instance, the boot script cannot boot a PS-EXE without a disc), the step fails with a report, per the plan rule.

## Implementation Decisions

### Setup: `make psn00bsdk-toolchain-setup`

- The existing setup script in `tools/` is rewritten in place (same name, same target name, which the plan fixes). The old GCC-from-source path, its `ftpmirror.gnu.org` downloads, the unpinned PSn00bSDK clone and every `sudo make install` are removed.
- The script takes two arguments: the install directory and the sources list. The Makefile passes `PSN00BSDK_DIR` (new variable, default a `psn00bsdk` directory under `third_party/`, already gitignored) and `PSX_SOURCES`.
- The target depends on `psx-net-check`, as step 1.2's setup does.
- Steps, in order:
  1. apt install of `cmake make unzip curl` with `sudo`, skipped when `PSN00BSDK_SKIP_APT` is set (the raylib vendoring pattern). Noble's CMake 3.28 meets the SDK's 3.21 minimum.
  2. The `psn00bsdk` line of the sources list gives the URL and sha256; a missing line fails the setup naming the component. No URL or hash is written in the script.
  3. Download with `curl` (fail on HTTP errors, follow redirects: the release redirects to `release-assets.githubusercontent.com`, already in the list) into a fresh temporary directory under `tmp/`; no download cache, so a verification run truly starts from nothing.
  4. sha256 check against the pin; a mismatch fails naming the component and the two hashes.
  5. Unzip into a temporary directory next to the install directory; the archive must hold exactly one top-level directory, whose contents are moved into the install directory (so the install directory holds `bin/`, `lib/`, `include/`, `share/`, `libexec/`, `mipsel-none-elf/` directly). Then the temporary directories are removed.
  6. Smoke test: the installed `mipsel-none-elf-gcc --version` runs and reports `12.3.0`; `elf2x` and `mkpsxiso` are present and executable.
  7. A completion stamp written last in the install directory, holding the archive's sha256.
- **Re-runs**, the same rule as step 1.2's setup: a stamp naming the pinned sha256 → print "already installed", exit 0; a non-empty directory without a matching stamp → fail and tell the user to remove it, deleting nothing; missing or empty → full install.
- The `elf2x` trap of `docs/psx-hello-world.md` did not occur in the trial with the release's own binutils; the `objcopy` workaround is not built. If it occurs, the check's magic condition names it, and the implementer applies the documented workaround and records it in the step note.

### Build: `make psx-hello`

- Configures with `cmake --fresh -S psx-hello -B psx-hello/build -DCMAKE_TOOLCHAIN_FILE=<PSN00BSDK_DIR>/lib/libpsn00b/cmake/sdk.cmake`, then builds; default generator (Unix Makefiles, host `make`). `--fresh` costs 0.2 s and guarantees the cache never points at a previous SDK directory.
- Before configuring, it fails with a message pointing at `make psn00bsdk-toolchain-setup` when the SDK stamp is missing.
- The SDK's toolchain file finds the compiler through its own install-relative hints before `PATH`, so no `PATH` change is needed; the step relies on that (seen in the trial).
- The output is `psx-hello/build/psx-hello.exe` (gitignored directory), unchanged from today.
- `psx-hello-clean` is kept as is.

### `psx-hello` source

- One `printf` of the exact line `psx-hello: main loop started` followed by a newline, once, after `init_context` and the pad setup, right before the main loop; plus `#include <stdio.h>`. Nothing else in the program changes (the existing `DrawPrim` pointer-type warning is left alone).
- The line text is a contract with the check and with plan 10: one short comment says the check searches for it verbatim.

### Check: `make psx-hello-check`

- A new shell script in `tools/`, run by a new target that depends on `psx-hello` (a sub-second build), and not on either setup target.
- Arguments: the emulator install directory (`PSX_EMULATOR_DIR`), the `.exe`, the known line (Make variable `PSX_HELLO_TTY_LINE`, default `psx-hello: main loop started`), the frame count (`PSX_FRAMES`, default 600 as in step 1.2), and an output directory under `tmp/psx/hello-check/` that the script empties first.
- **Three boots**, sequential, through the step 1.2 boot script (H-1.2-3): the `.exe` twice, nothing once.
- **Five conditions**, all evaluated before concluding, one line per failure (`psx-hello-check: <condition>: <detail>`), exit 1 if any failed, 0 otherwise:
  1. `magic`: the `.exe`'s first 8 bytes are `PS-X EXE`.
  2. `deterministic`: the two `.exe` captures are byte-identical.
  3. `differs-from-empty`: the first `.exe` capture differs from the empty boot's capture.
  4. `not-uniform`: the first `.exe` capture holds at least two distinct pixel values (pixel size from the header's depth, H-1.2-6), so a cleared screen with no text fails.
  5. `tty-line`: the first `.exe` boot's TTY file contains the known line, searched as a fixed string.
  A boot that failed counts as a failure of every condition that needs it; the others are still checked.
- On success it prints the sha256 of the `.exe` and of each capture, and the capture's width, height and depth.
- The script is standalone; step 1.2's check script is not modified (H-1.2-12). Step 2.2 may later reuse this script's conditions for the CD image.

### Makefile and repository

- New phony target `psx-hello-check`; new variables `PSN00BSDK_DIR`, `PSN00BSDK_SKIP_APT` (documented in the legend), `PSX_HELLO_TTY_LINE`. Legend entries updated for `psn00bsdk-toolchain-setup` (what it installs, where, outside the suite) and `psx-hello`, and added for `psx-hello-check`.
- None of these targets is in `test`, `e2e` or `test-all`.
- No `.gitignore` change: `third_party/`, `tmp/` and `psx-hello/build/` are already ignored.
- Comments in the scripts follow CLAUDE.md: only traps (why `--fresh`, why the stamp holds the sha256) and contracts.
- `docs/psx-hello-world.md` is not touched (step 2.4 rewrites it).

### Step note

- The implementer appends the step 2.1 note to section 6 of `docs/plan-9.md`: the setup command and its wall time from an empty directory; the `.exe` sha256 and whether it is the same across build directories; the three capture sha256 and the capture's width, height and depth; the TTY line found and which other lines the TTY file holds; which text the capture shows (H-1.2-10); every H-1.2-k that turned out false and what was followed instead.

## Implementation Chunks

1. **Toolchain and build.** The rewritten setup script, `make psn00bsdk-toolchain-setup` (with its dependency on `psx-net-check`), `make psx-hello` with the toolchain file and the stamp check, the known `printf` line, legend entries. Done when a setup into an empty `PSN00BSDK_DIR` under `tmp/` writes its stamp, a second run exits 0 at once, and `make psx-hello PSN00BSDK_DIR=<that dir>` produces an `.exe` starting with `PS-X EXE` and containing the known line.
2. **Check, verification, note.** The check script and `make psx-hello-check`; the step's verification; the hand-tested failure paths; the step note. Done when the verification passes against the step 1.2 emulator install.

## Testing Decisions

- A good test here checks what later steps rely on: the `.exe` that is produced, the files a boot writes and the facts the check asserts, never the SDK's or the emulator's internals.
- No unit test is added to `tests/`: everything is network-, toolchain- or emulator-bound, and `make test` / `make e2e` must stay untouched and no slower.
- **The step's verification:** remove `PSN00BSDK_DIR` (or point it at a fresh directory under `tmp/`), run `make psn00bsdk-toolchain-setup`, then `make psx-hello-check` against the step 1.2 emulator install: it exits 0.
- **Failure paths, verified once by hand** and recorded in the note:
  - the check with `PSX_HELLO_TTY_LINE` set to a line the program never prints: exits 1, one `tty-line` failure, the four other conditions pass;
  - `make psx-hello` with `PSN00BSDK_DIR` pointing at an empty directory: fails pointing at the setup target;
  - the setup into a non-empty directory with no stamp: fails and deletes nothing;
  - the setup with a sources list whose `psn00bsdk` sha256 is altered (a copy under `tmp/`, passed through `PSX_SOURCES`): fails on the hash, leaves no stamp.
- Prior art: step 1.2's setup and check scripts (stamp, re-run rule, one line per failed condition), `tools/install_raylib_vendor.sh` (apt skip variable), `tools/e2e_limits.sh` (every item checked before concluding), `tools/psx_net_check.sh` (reading the sources list).

## Out of Scope

- Installing, rebuilding or changing the emulator, its BIOS, its boot script or its check (step 1.2).
- The CD image, `mkpsxiso`, `SYSTEM.CNF` and license data (step 2.2), though `mkpsxiso` comes with the installed release.
- A committed reference sha256 for any capture (step 2.3).
- Rewriting `docs/psx-hello-world.md` (step 2.4), even though its build instructions become stale in this step.
- Scripted pad input to `psx-hello` (plans 10 and 11); the check uses the pad state the emulator presents by default.
- Building GCC, binutils or PSn00bSDK from source; any host outside the step 1.1 list.
- The game itself and audio.

## Further Notes

- The `.exe` was byte-identical across three build directories in the trial, so its sha256 is a stable fact to record; it is not committed as a reference (step 2.3 commits the capture's).
- libpsn00b's own TTY logs (for example `psxgpu: setup done, default mode is …`) are expected in the TTY file before the known line; they are not an error and the check ignores them.
- The capture is taken at the step 1.2 default frame (600 vsyncs); `psx-hello` reaches its main loop within a few frames, so 600 leaves wide margin.
- Written in parallel with step 1.2's implementation: see "Hypotheses on step 1.2's outcome".
