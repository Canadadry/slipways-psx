# Plan 9 — PSX toolchain and a first `.bin/.cue`

Third of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

In this sandbox: `psx-hello` compiled, packed into a `.bin/.cue`, and booted in a windowless emulator, with proof an agent can read. Plans 10 and 11 reuse the emulator and its VRAM capture.

## 2. Rules

- **What exists.** `psx-hello/` and `tools/install_psn00bsdk_toolchain.sh`, never run (`docs/psx-hello-world.md`). The known `elf2x` trap, and its workaround, are in that doc.
- **Autonomous run.** The human is not involved during the plan: every choice (emulator, sources, versions) is made by the run itself, through `guideline` when needed. The only human action is a one-time precondition, before the run: opening the network in the environment settings.
- **Network precondition.** The sandbox blocks GitHub (403) and the emulators. Before the run, the human sets the environment's broadest network access level, or at least allows `github.com`, `api.github.com`, `codeload.github.com` and `objects.githubusercontent.com` (PSn00bSDK, `mkpsxiso`, emulator releases). Step 1.1 chooses only sources reachable at that moment, and fixes the full host list; no later step may reach a host outside it.
- Prefer prebuilt binaries over compiling GCC.
- Windows stay forbidden, as with raylib: the emulator runs headless.
- The game itself and audio are out of scope: only `psx-hello`.
- `make test` and `make e2e` stay untouched and no slower; every new PSX target stays outside `make test-all`.
- A step that cannot be done (host unreachable, no emulator fits) fails its check, and the run stops there with a report. It is never answered by a guess, and never by a question to the human.

## 3. Phases and steps

### Phase 1 — Access and emulator

1. **Sources and network check.** Choose the concrete source and fixed version of each component: the emulator, a BIOS if it needs one, the prebuilt toolchain, PSn00bSDK, the CD image tool, the CD license data (or the recorded fact that the chosen emulator and BIOS boot a disc without it), and a prebuilt PS-EXE (one that writes known text to TTY) and CD image to test the emulator with. Among emulators that otherwise fit, prefer one that takes scripted pad input headless: plans 10 and 11 replay input in it. A component with no reachable source fails the step at once, with a report naming the missing host. Record the choices in the step note. A `make` target probes every host they need, and fails if any is unreachable. Verified by running it: it passes. A later step that needs a host outside this list adds it to the probe and fails.
2. **Windowless emulator.** A `make` setup target, outside `make test-all`, installs the fixed version of the emulator (and a BIOS if it needs one). Boot the prebuilt PS-EXE from step 1.1 headless, writing a VRAM capture and the TTY output to files. Verified, after running the setup target into an empty install directory: two boots of the same `.exe` produce byte-identical captures, a binary that prints shows its text in the TTY file, two boots of the prebuilt CD image from step 1.1 also produce byte-identical captures, and that capture differs from a boot with no program or disc (the capture shows what is drawn). If no emulator meets this, the step fails: plans 10 and 11 depend on it (plan 10 step 1 reads the game state over TTY). The step note records whether, and how, the emulator takes scripted pad input; that alone does not fail the step.

### Phase 2 — Toolchain and image

1. **Toolchain.** `make psn00bsdk-toolchain-setup` installs the fixed versions from step 1.1, then `make psx-hello` produces an `.exe`. Verified, after running the setup target into an empty install directory (nothing left over from earlier attempts): that `.exe` boots in the step 1.2 emulator, its capture is deterministic, and it proves `psx-hello` ran: its capture differs from a boot with no program. A hang or a black screen is deterministic too. `psx-hello` also prints one known line over TTY, and that line is in the TTY file: plan 10 step 1 reads the game state the same way.
2. **CD image.** A `make` target produces `psx-hello.bin/.cue` (with a tool like `mkpsxiso`), including the license and `SYSTEM.CNF`. Verified: two boots give byte-identical captures, as in step 1.2, with the same proof that `psx-hello` ran as in step 2.1.
3. **Check target.** A `make` target, outside `make test-all`, builds the image, boots it, and fails if the capture's sha256 differs from its committed reference, like the e2e. The reference is committed only from a boot that passed step 2.2's proof. Verified: it passes, and fails on a deliberately altered reference.
4. **Docs.** `docs/psx-hello-world.md` rewritten to describe what actually works.

## 4. Done when

- From a clean checkout, with the toolchain and emulator setup targets run into empty install directories, the step 2.3 target passes.
- `make test-all` passes and its timing is unchanged.
- `docs/psx-hello-world.md` describes the working toolchain, emulator and capture.

## 5. Open questions

None.

## 6. Step notes
