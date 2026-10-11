---
title: "Reproduce the hardware-renderer stretch bug headless"
description: "The uneven-texel bug of the PSX frame's final x2 stretch is only visible to a human in DuckStation's hardware renderer; the agent needs a headless reproducer, or a recorded proof that none can run here."
status: needs-triage
---

## Step (verbatim from docs/plan-19.md § 3, step 1)

> 1. **Reproduce the bug headless.** Find an emulator renderer the agent can run without a window that shows the bug. Try these candidates, in order: DuckStation's headless regression runner with a hardware renderer and its internal resolution above 1x (the case the human saw), then PCSX-Redux's OpenGL GPU (without `-softgpu`) on a software GL (Mesa llvmpipe). A candidate counts only if its check fails on today's disc, while the same capture and check pass on that disc under the same emulator's software renderer (the control). Its capture is the renderer's output at its internal resolution, not emulated VRAM or a 320x240 readback, and the checks judge blocks of 2 x that scale. A candidate is declared exhausted only once its step note records either a capture taken at that resolution, or the concrete blocker that keeps it from producing one (command and error). Capture the title screen and the frames of `psx/gpu_captures.txt` with it, and add a check that fails on that disc: an image that is not the PC image upscaled x2 (same `offpalette`/`nonuniform` conditions). From step 2 on, each family's labels must also pass under that renderer. Once both candidates are exhausted, write it down and go to step 2; the proof then falls to the human with DuckStation (step 6).

## Problem Statement

The PSX disc draws each frame into a 160x120 area of VRAM and ends the frame with one textured quad that stretches that area x2 onto the 320x240 display. PCSX-Redux's software GPU, which every automated PSX check uses, samples that quad exactly, so `make psx-check` is green. DuckStation's hardware renderer does not: texels come out 1 or 3 pixels wide, glyph outlines get uneven, the title logo breaks up. Only the human has seen it, on a Mac. The agent has no way to see the bug, so it cannot prove that the plan's later steps fix it. Before any fix, the agent needs a renderer it can run headless that shows the bug, or a written record that no candidate can run in this container, so that the final proof passes to the human (plan step 6).

## Solution

A two-stage step. First, a cheap feasibility probe runs each candidate in the plan's order and stops on the first concrete blocker, recorded as command and error in the plan's step note. Then, only if a candidate actually runs: a pinned install target, a capture target that writes the renderer's output at its internal resolution for the title screen and the frames of the capture list, and a check that judges those captures against the PC image upscaled x2. The check must fail on today's disc and pass on the same disc under that emulator's software renderer (the control). Everything lives outside `make test-all` and outside `make psx-check`.

### What the PRD author already found (2026-10-11, this container)

This is input for chunk 1. Chunk 1 still re-runs the PCSX-Redux commands and writes the note itself.

- **Container:** 4 cores, 15 GiB RAM, no `/dev/dri`, no GPU, no X server. Mesa 25.2.8 (Ubuntu noble) is installed with `swrast_dri.so`/`kms_swrast_dri.so`. A 20-line C probe (`eglGetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA)`, then a 3.3 core context and `eglMakeCurrent` with no surface) returns `llvmpipe (LLVM 20.1.2, 256 bits)`, `4.5 (Core Profile) Mesa 25.2.8`. So a surfaceless software GL context is available headless. Lavapipe (`mesa-vulkan-drivers`) is not installed but can be installed from noble (candidate 25.2.8-0ubuntu0.24.04.4).
- **PCSX-Redux OpenGL GPU, `-cli`:** `pcsx-redux -cli -stdout -openglgpu -interpreter -run -dofile <lua>` with DISPLAY/WAYLAND_DISPLAY unset exits 139 (SIGSEGV). gdb shows a call through a null function pointer in `OpenGL::VertexBuffer::create` from `OpenGL_GPU::initBackend` (`src/core/OpenGL_GPU/gpu_opengl.cc:84`, called from `src/main/main.cc:378`). In `-cli`/`-no-ui` the emulator uses its text UI, which never creates a GL context or loads GL entry points.
- **PCSX-Redux OpenGL GPU, GUI front end:** the same command without `-cli` exits 255 with `Failed to initialize GLFW` on stderr (no display).
- **PCSX-Redux, structural:** its OpenGL GPU allocates its VRAM render target at native 1024x512 (`m_vramTexture.create(vramWidth, vramHeight, …)`, with MSAA as the only option). It has no internal-resolution scale. Even with a context, it could not render "the case the human saw" (internal resolution above 1x).
- **DuckStation regression runner:** upstream `stenzek/duckstation` (master `cd024df2`, 2026-10-11) has `src/duckstation-regtest` with `-renderer <renderer>`, `-upscale <multiplier>`, `-frames`, `-dumpdir`, `-dumpinterval`. It asks for a `Surfaceless` window, and its EGL context code supports `EGL_PLATFORM_SURFACELESS_MESA`. Its frame dump copies the display texture at internal resolution to PNG, which is the capture the plan wants. Technically it looks feasible on llvmpipe. **But it is blocked by its license and its owner's instructions, not by the code:**
  - releases ship only the Qt AppImage and no regtest binary, so it would have to be built from source;
  - the root and regtest `CMakeLists.txt` read: "SPDX-License-Identifier: CC-BY-NC-ND-4.0 + Packaging Restriction … you may not use this file to create packages or build recipes without explicit permission from the copyright holder". A committed make target that clones and builds it is such a build recipe;
  - the repository's `CLAUDE.md` states that the codebase "is not authorized for use with AI coding assistants" and tells assistants not to operate on it.

  The PRD author stopped there. The sparse clone used to read those files was deleted, and nothing was built.

On this evidence, chunk 1 is expected to declare both candidates exhausted. If it does, chunks 2 to 4 are not built.

## User Stories

1. As the human, I want the agent to find out whether any headless renderer here shows the stretch bug, so that the fix can be proven without me booting DuckStation at every step.
2. As the human, I want the candidates tried in the plan's order (DuckStation regtest first, PCSX-Redux OpenGL second), so that the case I actually saw is tried first.
3. As the human, I want each exhausted candidate's blocker written as the exact command and the exact error, so that I can check it or retry it on another machine.
4. As the human, I want the probe to stop at the first concrete blocker of each candidate, so that the agent does not spend hours building emulators.
5. As the human, I want the probe to answer the plan's open question "Can step 1 run in this container at all?", so that I know whether step 6 is the only proof.
6. As the human, I want the agent to respect DuckStation's license and its owner's instruction to AI assistants, so that the project carries no build recipe it is not allowed to have.
7. As the human, I want no window, X server or Xvfb opened, so that the CLAUDE.md rule holds even during the probe.
8. As the human, I want an offscreen GL or Vulkan context on Mesa llvmpipe/lavapipe to count as headless, so that a software GL can stand in for a GPU.
9. As the human, I want any emulator install to be a make target outside `make test-all`, with its sources pinned in `tools/psx_sources.txt`, so that the fast loop stays fast and the install can be reproduced.
10. As the human, I want no third-party binary committed, so that the repository stays source-only.
11. As the human, I want `third_party/pcsx-redux` and `third_party/psn00bsdk` left as they are, so that `make psx-check` keeps its baseline.
12. As the human, I want a candidate to count only if its check fails on today's disc and passes under the same emulator's software renderer, so that a red check means this bug and not an emulator or comparator problem.
13. As the human, I want the reproducer's capture to be the renderer's output at its internal resolution, so that it shows what the screen shows and not emulated VRAM.
14. As the human, I want the check to judge blocks of 2 x the internal scale, so that a 4x upscale is judged on 8x8 blocks of the PC pixel.
15. As the human, I want the reproducer to capture the title screen, so that the exact screen I complained about is covered.
16. As the human, I want it to capture the 14 frames of `psx/gpu_captures.txt`, so that each primitive family of steps 2 to 4 can be checked under that renderer.
17. As the human, I want the check to reuse the `offpalette` and `nonuniform` conditions of `psx-check`, so that one meaning of "the PC image upscaled x2" holds everywhere.
18. As a later step's implementer, I want a reproducer target I can run after each family, so that I can prove my family's labels pass under the hardware renderer.
19. As a later step's implementer, I want the reproducer kept outside `make psx-check`, so that the human's acceptance gate does not depend on an extra emulator.
20. As a later step's implementer, I want the step note to say plainly whether a reproducer exists, so that I know whether step 3's two-`LINE_F2` option is admissible (it needs the reproducer).
21. As a later step's implementer, I want the step note to name the target and its outputs when a reproducer exists, so that I don't have to rediscover how to run it.
22. As the plan pilot, I want this step to end with the plan's step note written, so that step 2 can start the same day whatever the outcome.
23. As the plan pilot, I want chunks 2 to 4 skipped when chunk 1 exhausts both candidates, so that no dead tooling is built.
24. As a maintainer, I want the probe's scratch work (test programs, clones, logs) kept out of the repository, so that only the note is committed.
25. As a maintainer, I want the e2e hashes and `make test-all` unchanged by this step, so that the game is untouched.
26. As the human on macOS, I want any reproducer target to fail with a clear message when its emulator is missing, so that I know which setup target to run.

## Implementation Decisions

- **Stop rule, applied literally.** A candidate is exhausted as soon as its step note records either a capture at internal resolution, or one concrete blocker: the command and its error. A license or owner prohibition quoted with its file counts as the error for a candidate that would need a build recipe. No candidate is pushed past its first blocker by patching an emulator, adding a GL context to it, or building it some other way.
- **DuckStation is not cloned, built or packaged by the agent.** Its build files forbid build recipes without the copyright holder's permission, its repository tells AI assistants not to operate on it, and no regtest binary is released. Chunk 1 records that as its blocker. It may confirm that the release assets still hold no regtest binary, but it does not clone the repository again. If the human later gets permission or builds a regtest themselves, that is a new plan decision, not part of this step.
- **PCSX-Redux is probed only as pinned and installed** (`third_party/pcsx-redux`, untouched): `-cli -openglgpu` and the GUI front end without a display, each with DISPLAY and WAYLAND_DISPLAY unset, a throwaway HOME, a timeout, and a 120-vsync Lua script that takes a screenshot and quits. Their exit codes and errors (SIGSEGV with the gdb frame, `Failed to initialize GLFW`) are the blocker. The note also records the structural fact that this GPU renders VRAM at native 1024x512 and has no internal-resolution scale. Patching the emulator to create a surfaceless EGL context is rejected: it is an emulator change, not a probe, and would still not reproduce an upscaled render.
- **No Xvfb.** It is an X server, which CLAUDE.md forbids. Mesa surfaceless EGL (llvmpipe) or lavapipe is the only accepted headless context.
- **Lavapipe is not installed by chunk 1.** No remaining candidate needs Vulkan. If a later decision brings a Vulkan candidate, its install goes into that candidate's setup target (`apt` line pinned to the noble series in `tools/psx_sources.txt`).
- **Where the outcome is written:** the plan's § 6 Step notes, under "Step 1". It holds one entry per candidate (command, exit code, error, verdict). The open question "Can step 1 run in this container at all?" in § 5 gets a one-line answer that points to the note. If both candidates are exhausted, the note says: no headless reproducer exists, step 3's two-`LINE_F2` option is therefore not admissible, and the bug's proof falls to step 6.
- **Only if a candidate runs (chunks 2 to 4):**
  - Setup target `psx-hw-emulator-setup`, modelled on `psx-emulator-setup`. It runs `psx-net-check` first, pins every source in `tools/psx_sources.txt`, installs into its own directory under `third_party/` (git-ignored, with a stamp file), exits 0 at once on a complete install, and stays outside `make test-all`.
  - Capture target `psx-hw-capture RENDERER=<hardware|software> SCALE=<n>`. It boots the GPU replay disc and writes one capture per title-screen frame and per entry of `psx/gpu_captures.txt`, at `320*n x 240*n` (the renderer's output, not a VRAM readback). The capture format is the existing `psx-vram <w> <h> <depth>` header followed by raw pixels, with the width and height carrying the scale. When the emulator has no Lua host to drive the scratchpad capture handshake, the frames are chosen by a build-time capture schedule: the GPU replay holds each listed frame for a fixed number of vsyncs and prints one TTY line per held frame, and the target dumps every vsync and keeps the middle dump of each hold, in list order. The title screen is captured on the game disc after a fixed number of vsyncs, where it is static.
  - The comparator learns a scale. Decoding accepts `320*n x 240*n` captures for an integer n of at least 1 and judges uniform blocks of `2n x 2n`. The `offpalette` and `nonuniform` metrics keep their meaning, and n = 1 behaves exactly as today, so `psx-check` is unchanged.
  - Check target `psx-hw-check`. It runs the capture under the hardware renderer with SCALE above 1, then under the same emulator's software renderer as the control, and prints one line per condition: `hw-control` (every software capture has `offpalette 0`, `nonuniform 0`), `hw-title` and `hw-<label>` (offpalette 0 and nonuniform 0 under the hardware renderer). It exits 1 if any condition fails. On today's disc it must fail at least `hw-title` and pass `hw-control`. This is recorded in the note with the failing counts. It stays a separate target outside `make test-all` and outside `make psx-check`, as step 4 requires. The `differ` ceilings stay `psx-check`'s business.

## Implementation Chunks

1. **Feasibility probe and step note (always).** Confirm the DuckStation blocker without cloning: no regtest asset in the releases, plus the license and owner-instruction clauses as quoted above. Re-run the two PCSX-Redux commands with a throwaway HOME and a timeout, keeping exit codes, the stderr line and the gdb top frames. Write the § 6 "Step 1" note and the § 5 one-line answer. Scratch files go in `tmp/` or the session scratchpad, never in the repository. **If both candidates are exhausted, the step ends here:** commit the note and skip chunks 2 to 4. A docs-only commit is expected, and `make test-all` and the e2e hashes are unchanged.
2. **Install target (only if a candidate ran in chunk 1).** `psx-hw-emulator-setup`, pinned sources, git-ignored install directory, outside `make test-all`.
3. **Scaled capture and comparator (only if chunk 2 exists).** The comparator's scale (unit-tested first), then `psx-hw-capture` for the title screen and the 14 listed frames, under both renderers.
4. **Check and control (only if chunk 3 exists).** `psx-hw-check`, shown red on today's disc with a green software control, and the note updated with the command and the failing counts.

## Testing Decisions

- A good test drives a module through its public interface and asserts observable output: the comparator's metrics for a given capture, a check script's condition lines and exit code. It never asserts internal buffers.
- **Chunk 1 adds no unit test.** It changes no code, and its proof is the recorded commands and errors in the step note.
- **Chunk 3 (conditional):** the comparator's scale is tested in the comparator's existing host tests. Cases: a synthetic 640x480 capture of uniform 4x4 blocks in palette colours measures `offpalette 0`, `nonuniform 0` and `differ 0` against its PC frame; one block with a 1-pixel seam counts `nonuniform 1`; a size that is not a multiple of 320x240 is refused with an error naming the size; a 320x240 capture measures exactly as before. Prior art: `tests/test_psx_capture_check.c` and the comparator in `src/devtools/capture_compare.c`.
- **Chunk 4 (conditional):** the check script gets a host test in `make psx-tools-test` with fixture captures (one exact, one with uneven blocks) proving its condition lines and exit code, following the existing `psx_capture_check.sh` tests. The real-emulator run (red on today's disc, green control) is evidence in the step note and is not a test in the suite.
- `make test` and `make e2e` must not get slower. Nothing in this step is added to them.

## Out of Scope

- Plan steps 2 to 6: drawing into the display buffer, primitives at x2, the `line` decision, re-measuring, and the human acceptance.
- Cloning, building, packaging or modifying DuckStation in any form, and asking its author for permission. Whether to seek permission is the human's call.
- Patching PCSX-Redux (EGL context, internal-resolution scaling) or touching `third_party/pcsx-redux` or `third_party/psn00bsdk`.
- Xvfb or any other X/Wayland server, and opening any window.
- Committing emulator binaries, Mesa drivers, or probe programs.
- Changing `make psx-check`, its thirty conditions, the ceilings or the 14 accepted references.
- Audio.

## Further Notes

- If chunk 1 exhausts both candidates, the plan's two later dependencies on this step resolve as follows. Step 3's two-`LINE_F2` option for `line` is inadmissible, so `line` takes the pixel list (or merged `TILE` runs). Step 4's "if step 1 produced a headless reproducer" clause is void.
- The human-side alternative stays open outside this plan. A human (not an agent) can build DuckStation's regtest for personal use and run `-renderer OpenGL -upscale 4 -dumpdir … -dumpinterval 1` on the disc. That is close to the step 6 check, and the human may choose to do it.
- Surfaceless llvmpipe works here (GL 4.5 core), so a future emulator whose license allows a build recipe and that has a surfaceless GL or Vulkan path and an internal-resolution scale would fit the conditional chunks as written.
