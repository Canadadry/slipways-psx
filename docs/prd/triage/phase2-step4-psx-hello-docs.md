---
title: "docs/psx-hello-world.md describes the PSX toolchain, emulator and capture that work"
description: "The only PSX doc still says nothing has ever run, points at a GCC build from GNU mirrors and a windowed emulator, and is silent on the setup, boot, capture and check targets plan 9 made work."
status: needs-triage
---

## Step

Plan `docs/plan-9.md`, phase 2 — Toolchain and image, step 4 (verbatim):

> 4. **Docs.** `docs/psx-hello-world.md` rewritten to describe what actually works.

And the matching "Done when" line: "`docs/psx-hello-world.md` describes the working toolchain, emulator and capture."

## Problem Statement

`docs/psx-hello-world.md` is the doc CLAUDE.md, `psx-hello/CMakeLists.txt` and plan 9 point at for the PSX side. As committed, every paragraph of it is false or stale:

- It says none of the steps has been run, because the sandbox blocks the GNU mirrors and the emulators. Steps 1.1 to 2.2 ran all of them in this sandbox.
- It describes `make psn00bsdk-toolchain-setup` as a ~30 min GCC + binutils build from `ftpmirror.gnu.org` with 45-minute timeouts. Step 2.1 replaced that with the pinned prebuilt PSn00bSDK 0.24 archive (~7 s, sha256-checked); `ftpmirror.gnu.org` is no longer reached.
- It tells the reader to boot in a windowed emulator (`pcsx-redux --no-ui … --run --test-mode`, or DuckStation, no$psx) and confirm by eye. The working path is headless: `make psx-boot` writes a capture and a TTY file, and the check targets read them. Windows are forbidden to agents.
- It knows nothing of the CD image (`make psx-hello-cd`), the capture format, the five-condition proof, the committed reference (step 2.3), nor of the traps the step notes found (`.bin` not reproducible, TTY files holding temp paths, a `.cue` naming a missing `.bin` dropping to the OpenBIOS shell).
- Its `elf2x` trap did not occur with the pinned release (step 2.1 note: "`elf2x` did not crash"); its workaround describes a fix nobody needs.

CLAUDE.md's Docs rule says `docs/` only describes what is true or still to do. Plans 10 and 11 build on this toolchain, emulator and capture; their first reader needs one page saying how to install, build, boot, capture and check, and what is not checked yet.

## Solution

`docs/psx-hello-world.md` rewritten from scratch (same path, so the three existing pointers stay valid), describing, for a reader with a clean checkout:

1. What `psx-hello` is and what it does (GPU double buffer, BIOS pad driver, the PICO-8 mouse mapping, the known TTY line).
2. What is pinned and where (`tools/psx_sources.txt`), the components in one short table, and the network facts that shaped them.
3. The two setup targets, into which directories, how long, and their idempotence/refusal behavior.
4. The two build targets (`.exe`, `.bin/.cue`).
5. The headless boot and its three output files, with the capture format.
6. The check targets in one table: what each proves, how long it takes; the committed reference and how it is reset.
7. Traps, each one a sentence a later plan would otherwise rediscover.
8. What is still to do: the pad mapping is not checked by any target; scripted pad input works in a trial only.

No other file changes for the doc's sake except where a hypothesis below forces it (see "Other docs").

## User Stories

1. As an agent starting plan 10, I want one page listing the commands from a clean checkout to a checked `psx-hello` boot, so that I do not reconstruct them from three step notes and a Makefile legend.
2. As an agent, I want the setup commands in the order they must run, with their install directories, so that a first run works without trial and error.
3. As an agent, I want each setup's duration stated (emulator ~10 min, SDK seconds), so that I set a timeout that fits and do not kill a healthy build.
4. As an agent, I want to know that the setups exit 0 at once on a complete install and refuse a non-empty directory without their stamp, so that I neither rebuild for nothing nor delete a directory to "fix" a refusal blindly.
5. As an agent, I want to know the emulator setup uses `sudo apt` unless `PSX_EMULATOR_SKIP_APT` / `PSN00BSDK_SKIP_APT` is set, so that I am not surprised by a privilege prompt.
6. As an agent, I want to know that no check target ever runs a setup, so that I never trigger a ten-minute build by accident.
7. As an agent, I want the build targets named with their outputs, so that I know where the `.exe`, `.bin` and `.cue` land.
8. As an agent, I want to know how to boot any `.exe` or `.cue` headless and where the capture and TTY go, so that plan 10 can boot its own binaries the same way.
9. As an agent, I want the capture format written down (header line, then raw bytes of the displayed frame), so that I can read or compare a capture without opening the Lua script.
10. As an agent, I want to know that a 600-frame boot takes ~11 s, at real-time pace, so that I budget check runs correctly.
11. As an agent, I want each check target listed with what it proves, so that I pick the right one.
12. As an agent, I want the five conditions of the `psx-hello` proof named, so that I can read a failure line without opening the script.
13. As an agent, I want the committed reference and its baseline target described, so that I know how an intended visual change is accepted, and that no hash is ever written by hand.
14. As an agent, I want to be told never to hash the `.bin`, so that I do not build a check that fails on every build.
15. As an agent, I want to be told never to compare TTY files whole, only grep them, so that per-boot temp paths do not fail a check.
16. As an agent, I want to know that a `.cue` naming a missing `.bin` does not fail the boot but lands deterministically in the OpenBIOS shell, so that I do not take a deterministic capture for a successful boot.
17. As an agent, I want to know the emulator needs the upstream headless fix on top of its pin, so that a pin bump does not drop it and bring back a segfault on disc boots.
18. As an agent, I want to know there is no Sony BIOS and no license data on the disc, and why that works (OpenBIOS does not check it), so that I do not go looking for either.
19. As an agent, I want to know which hosts the toolchain needs and that `make psx-net-check` probes them, so that a network failure is diagnosed in seconds.
20. As an agent, I want to know that GitHub web/API pages and the PCSX-Redux prebuilt host are unreachable here, so that I do not try to switch to prebuilt emulator binaries.
21. As an agent, I want the doc to say the pad mapping (Cross, D-pad, Circle) is not checked by any target, so that I do not believe it is proven.
22. As the implementer of plans 10 and 11, I want to know that scripted pad input works headless (`setOverride` / `clearOverride` from the vsync listener) but is not committed, so that I know where to start.
23. As a human, I want the doc to say that playing `psx-hello` in a windowed emulator is a human's job, so that the boundary with agents stays clear.
24. As a maintainer, I want no hash values duplicated in the doc, so that a rebaseline never leaves the doc wrong.
25. As a maintainer, I want the doc to point at the Makefile legend and the scripts for details rather than copy every option, so that it does not drift.
26. As a maintainer, I want the stale `elf2x` section gone, so that `docs/` holds only what is true or still to do.
27. As a maintainer, I want CLAUDE.md's PSX line, the README and the Makefile legend touched only if they become wrong, so that this step stays a doc rewrite.
28. As the plan-pilot run, I want the step verified by a mechanical cross-check of every target and path the doc names against the committed tree, so that the doc cannot name something that does not exist.
29. As the plan-pilot run, I want `make test-all` unchanged in result and timing, so that the plan rule holds.
30. As a human auditing plan 9 later, I want the step note to list the H-2.3-k results and every decision, so that I can audit the step without rerunning it.

## Step 2.3's outcome (hypotheses)

This PRD is written while step 2.3 is still being implemented, from its committed PRD (`docs/prd/triage/phase2-step3-psx-hello-reference-check.md`) and the committed tree only. Every assumption about step 2.3's outcome is a hypothesis below. Before starting, the implementer checks each one against the step 2.3 note in section 6 of `docs/plan-9.md`, the Makefile and the tree as committed, and follows the stated fallback where one does not hold. Each result (holds / did not hold, and what was done) goes into the step note.

- **H-2.3-1 — Step 2.3 landed and passed.** The step 2.3 note says the check target exited 0, and exited 1 on a deliberately altered reference. *If not:* the run has stopped at step 2.3 per the plan rule; this step is not started. If it is reached anyway with 2.3 failed, it fails with a report rather than documenting a check that does not work.
- **H-2.3-2 — Target names.** The check target is `make psx-hello-e2e` and the reference writer is `make psx-hello-e2e-baseline`, both in `.PHONY`, both outside `make test-all`. *If not:* the doc uses the names the Makefile actually has.
- **H-2.3-3 — Reference file.** The committed reference is `psx-hello/psx-hello-cd.vram.sha256`, one line holding the sha256 of the whole first CD capture (header included), overridable through `PSX_HELLO_REFERENCE`. *If not:* the doc names the actual path and variable; if the reference covers something other than the whole capture file, the doc says what.
- **H-2.3-4 — Comparison script.** The comparison is `tools/psx_reference_check.sh <reference> <capture>`, usable on any capture. *If not:* the doc names whatever does the comparison, or omits a script name if the comparison is inline in the Makefile.
- **H-2.3-5 — Proof before comparison.** `psx-hello-e2e` depends on `psx-hello-cd-check` (fresh image build, five-condition proof) and on no setup target; the baseline target has the same dependency and is the only writer of the reference. *If not:* the doc describes the actual order; if the comparison can run without the proof, that is a plan-rule breach to report, not to document.
- **H-2.3-6 — Frame guard.** Both targets refuse a `PSX_FRAMES` other than the 600 frames the reference was taken at. *If not:* the doc says the reference holds for 600 frames only, without claiming a guard.
- **H-2.3-7 — Messages and timing.** A mismatch names the expected and actual hashes and `make psx-hello-e2e-baseline`; a missing reference fails (not skipped); a run takes ~35 s. *If not:* the doc states the actual behavior and the time from the step note.
- **H-2.3-8 — Legend done by step 2.3.** The Makefile legend already has entries for both new targets. *If not:* this step adds them, in the style of the `psx-hello-cd-check` entry; it is the only Makefile change this step may make.
- **H-2.3-9 — Step 2.3 left the docs alone.** Step 2.3 did not change `docs/psx-hello-world.md`, the README or CLAUDE.md (its PRD puts the doc rewrite out of scope). *If not:* this step starts from the committed version and keeps any true sentence step 2.3 added.
- **H-2.3-10 — Reference value consistent with step 2.2.** The step 2.3 note records that the committed hash equals the CD capture recorded in step 2.2. *Either way:* the doc quotes no hash value (decision below), so a difference only goes into this step's note as an observation.

## Implementation Decisions

### Scope of files

- One file rewritten: `docs/psx-hello-world.md`, same path. It stays the single PSX doc; no new doc is created.
- **CLAUDE.md**: its PSX line ("`psx-hello/` is a toolchain test project — see `docs/psx-hello-world.md`") stays true after the rewrite; unchanged.
- **README.md**: mentions no PSX target (it is the PC target's doc, and PSX is paused); unchanged.
- **Makefile legend**: unchanged unless H-2.3-8 fails. The doc does not duplicate the legend's per-option detail; it points at it.
- **`psx-hello/CMakeLists.txt`**: its pointer comment to the doc stays valid; unchanged.
- **`docs/plan-9.md`**: only the step note is appended. Its Rules line "`psx-hello/` and `tools/install_psn00bsdk_toolchain.sh`, never run … The known `elf2x` trap … are in that doc" describes the plan's starting point and is not edited; the plan file goes when plan 9 is done.

### Content decisions

- **Title**: `# psx-hello: PSX toolchain, emulator and capture`.
- **No hash values** in the doc (no `b4934260…`, `d5b36d08…`, `.exe` hash): they live in the step notes and the committed reference file. The doc names the reference file and the baseline target instead.
- **Versions and pins** are named once (PCSX-Redux at its pinned commit plus the upstream headless fix, OpenBIOS from the same tree, PSn00bSDK 0.24 with prebuilt `mipsel-none-elf` GCC 12.3.0 and mkpsxiso 2.03) with `tools/psx_sources.txt` named as the single source of truth for URLs and pins. Commit hashes and sha256 pins are not repeated.
- **Durations** are approximate and labelled as such: emulator setup ~10 min; SDK setup seconds plus the download; a 600-frame boot ~11 s; each `psx-hello` check ~35 s; `psx-emulator-check` ~1 min; `psx-hello-e2e` per H-2.3-7.
- **Sections**, in order:
  1. *What `psx-hello` is*: `psx-hello/` (`main.c`, `CMakeLists.txt`, `iso.xml`, `system.cnf`); 320×240 double buffer, BIOS pad driver, pad status text, the PICO-8 mouse mapping (Cross toggles mouse mode, D-pad moves the cursor while it is on, Circle flashes a click); the TTY line `psx-hello: main loop started`, which the checks and plan 10 grep for verbatim.
  2. *Sources and network*: the component table (emulator, BIOS, SDK/toolchain/mkpsxiso, CD license data: none, test media for the emulator check); the hosts (`github.com` git and release downloads through `release-assets.githubusercontent.com`, `psx.arthus.net`, the Ubuntu noble archive and security hosts); `make psx-net-check` probes them; GitHub web/API pages and the PCSX-Redux prebuilt host are unreachable from the sandbox, which is why the emulator is built from source. A host outside that list is added to `tools/psx_sources.txt` and the probe, never reached ad hoc.
  3. *Setup*: `make psx-emulator-setup` (into `PSX_EMULATOR_DIR`, default `third_party/pcsx-redux`) and `make psn00bsdk-toolchain-setup` (into `PSN00BSDK_DIR`, default `third_party/psn00bsdk`); both run `psx-net-check` first, use `sudo apt` unless their `*_SKIP_APT` variable is set, exit 0 at once on a stamped install, refuse a non-empty directory without the stamp. The emulator finds OpenBIOS next to its binary: no BIOS placement, no Sony BIOS.
  4. *Build*: `make psx-hello` (`psx-hello/build/psx-hello.exe`, reconfigured from scratch each time, fails naming the SDK setup when the SDK is missing); `make psx-hello-cd` (`psx-hello.bin/.cue` with `SYSTEM.CNF` booting `PSXHELLO.EXE`; no license data on the disc, OpenBIOS does not check it); `make psx-hello-clean`.
  5. *Boot and capture*: `make psx-boot [PSX_EXE=… | PSX_ISO=…]`; headless (`-cli`, software GPU, interpreter), a throwaway HOME per boot; `PSX_FRAMES` (600) vsyncs; outputs `PSX_OUT.vram`, `.tty.txt`, `.log`; capture = one `psx-vram <w> <h> <depth>` line then the raw bytes of the displayed frame (what `PCSX.GPU.takeScreenShot()` returns); `PSX_BOOT_TIMEOUT` (120 s).
  6. *Checks*: one table — `psx-emulator-check` (the emulator itself, against the pinned test media), `psx-hello-check` (`.exe`), `psx-hello-cd-check` (CD image), `psx-hello-e2e` (H-2.3-2: CD proof, then sha256 against the committed reference) — with what each proves and its time. The five conditions named (`magic` for the `.exe` or `cue` for the image, `deterministic`, `differs-from-empty`, `not-uniform`, `tty-line`). Then the reference: one line in the file of H-2.3-3, written only by `make psx-hello-e2e-baseline` after a passed proof, committed with the change that moves the frame; valid for the pinned emulator, BIOS, SDK and 600 frames. All these targets stay outside `make test-all` and never run a setup.
  7. *Traps* (one sentence each): never hash the `.bin` (mkpsxiso stamps the build time; the capture is stable); never compare TTY files whole (emulator log lines hold per-boot temp paths), only grep; a `.cue` naming a missing `.bin` boots the OpenBIOS shell deterministically, which only the `cue` and `tty-line` conditions catch; the emulator pin needs the upstream headless fix on top or `-cli` segfaults on a disc boot; the emulator pin is the last commit before SDL3, which Ubuntu noble lacks.
  8. *Still to do*: no target drives the pad, so the mouse mapping is unchecked; scripted pad input (`PCSX.SIO0.slots[1].pads[1].setOverride(…)` / `clearOverride` from the `GPU::Vsync` listener) worked headless and deterministically in a step 1.2 trial, nothing committed — plans 10 and 11 pick it up. Playing `psx-hello` in a windowed emulator is a human's job; agents never open a window.
- **Dropped**: the `elf2x` trap and its `objcopy` workaround (did not occur with the pinned 0.24 release, step 2.1 note); the GCC-from-source description and its timeouts; the `pcsx-redux --no-ui` command; the DuckStation / no$psx / ePSXe / MAME advice; the "nothing has been run" paragraph. Git keeps them.
- **Style**: English, plain Markdown, no hashes, no emojis, one table for components and one for checks, the rest short paragraphs or lists; line-wrapped like the other docs in `docs/`. It names Makefile variables and defaults only where a reader needs them to run something; everything else defers to the Makefile legend and the scripts' usage headers.
- No question needed a `guideline` decision.

### Step note

- Appended to section 6 of `docs/plan-9.md` under `### Step 2.4`: what the doc now covers (section list), what was dropped and why, the files checked and left unchanged (CLAUDE.md, README, legend, CMakeLists), the cross-check result, the H-2.3-k results, any deviation.

## Testing Decisions

- A good check here tests what a reader relies on: every command and path the doc names exists and does what the doc says. It does not test prose.
- No unit test and nothing added to `make test` / `make e2e`: the change is a doc.
- **Mechanical cross-check, run once by hand and recorded in the note:**
  - every `make <target>` the doc names is in the Makefile's `.PHONY` line;
  - every repository path the doc names (`psx-hello/…`, `tools/…`, the reference file) exists in the committed tree, except build/output paths under `psx-hello/build/`, `tmp/` and `third_party/`, which are listed in `.gitignore`;
  - every Makefile variable the doc names (`PSX_EMULATOR_DIR`, `PSN00BSDK_DIR`, `PSX_FRAMES`, `PSX_OUT`, `PSX_BOOT_TIMEOUT`, `PSX_EXE`, `PSX_ISO`, `PSX_HELLO_CUE`, `PSX_HELLO_REFERENCE`, the `*_SKIP_APT` variables) is set or read in the Makefile or the scripts it calls;
  - the doc contains none of `ftpmirror`, `--no-ui`, `--test-mode`, `DuckStation`, `none of the steps`, `elf2x crashed`, and no 64-hex or `[0-9a-f]{8}…` hash;
  - every duration and fact in the doc is traceable to a step note of plan 9 (1.1 to 2.3) or a committed script header.
- No boot is needed to verify the doc: its facts come from the step notes, which recorded runs. If an emulator and SDK install happen to be present, running the doc's command sequence (`make psx-hello-cd-check`, then the H-2.3-2 target) is a bonus check, recorded if done.
- `make test-all` still passes, timing unchanged (nothing it reads changed).
- Prior art: the step notes of plan 9 (the facts), `docs/raylib-vendoring.md` (a setup doc for a vendored dependency in this repo's style).

## Out of Scope

- Any change to the PSX targets, scripts, `psx-hello` sources, pins or the reference (steps 1.1 to 2.3).
- Committing pad-input scripting, or checking the mouse mapping (plans 10 and 11).
- Editing `docs/plan-9.md` beyond the step note, or `docs/plan-10.md` / `docs/plan-11.md`.
- Documenting the game's PSX port (does not exist yet) or audio.
- Rewording CLAUDE.md, the README or the Makefile legend beyond what H-2.3-8 may force.

## Further Notes

- Facts used, all from committed step notes: emulator built from source (~10 min, 610 s measured), OpenBIOS found next to the binary, `-cli -stdout -softgpu -interpreter -run`, 600-frame boot ~11 s, TTY temp paths, pad on port 1 present headless (`PORT 1: TYPE=4` in the capture), `setOverride` trial, SDK setup ~7 s, `elf2x` fine, `.bin` 1059 × 2352-byte `MODE2/2352` sectors and time-stamped, CD capture equal to the `.exe` capture, missing-`.bin` fallback to the OpenBIOS shell.
- The doc deliberately quotes no hash, so step 2.3's exact reference value (H-2.3-10) cannot make it wrong; only the target names, the reference path and the comparison behavior (H-2.3-2 to H-2.3-7) feed into its text.
