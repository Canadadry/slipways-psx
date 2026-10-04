# Plan 9 — PSX toolchain and a first `.bin/.cue`

Draft: run it through `plan-converge` before executing anything. Third of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

In this sandbox: `psx-hello` compiled, packed into a `.bin/.cue`, and booted in a windowless emulator, with proof an agent can read.

## 2. What exists

- `psx-hello/` and `tools/install_psn00bsdk_toolchain.sh`, never run (`docs/psx-hello-world.md`).
- The known `elf2x` trap, and its workaround.
- The sandbox network currently blocks the GNU mirrors and the emulators.

## 3. Steps

1. **Network.** List the hosts needed (toolchain, PSn00bSDK, CD image tool, emulator, BIOS) and have the human open them. Prefer prebuilt binaries over compiling GCC.
2. **Toolchain.** `make psn00bsdk-toolchain-setup` installs everything in a fresh sandbox, then `make psx-hello` produces an `.exe`.
3. **Windowless emulator.** Boot the `.exe` headless and get proof out of it: a VRAM capture to a file, which plan 10 reuses, plus the TTY output if it helps. Windows stay forbidden, as with raylib. If no emulator fits, stop and hand back to the human: plans 10 and 11 depend on it.
4. **CD image.** A `make` target produces `psx-hello.bin/.cue` (with a tool like `mkpsxiso`), including the license and `SYSTEM.CNF`, booted as in step 3.
5. **Check target.** A `make` target, outside `make test-all`, builds the image, boots it, and fails if the proof differs from its reference (a capture sha256, like the e2e).
6. **Docs.** `docs/psx-hello-world.md` rewritten to describe what actually works.

## 4. Open question

The emulator is chosen in step 3: it needs a windowless mode that boots an `.exe`, then a CD image, and produces a usable trace.
