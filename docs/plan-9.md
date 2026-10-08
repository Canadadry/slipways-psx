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
2. **CD image.** A `make` target produces `psx-hello.bin/.cue` (with a tool like `mkpsxiso`), including the license and `SYSTEM.CNF`. Verified: two boots give byte-identical captures, as in step 1.2, with the same proof that `psx-hello` ran as in step 2.1. The proof is step 2.1's check script, extended to take the CD image as well as the `.exe`, not a second script.
3. **Check target.** A `make` target, outside `make test-all`, builds the image, boots it, and fails if the capture's sha256 differs from its committed reference, like the e2e. The reference is committed only from a boot that passed step 2.2's proof: the target runs that same check on the image before comparing the sha256. Verified: it passes, and fails on a deliberately altered reference.
4. **Docs.** `docs/psx-hello-world.md` rewritten to describe what actually works.

## 4. Done when

- From a clean checkout, with the toolchain and emulator setup targets run into empty install directories, the step 2.3 target passes.
- `make test-all` passes and its timing is unchanged.
- `docs/psx-hello-world.md` describes the working toolchain, emulator and capture.

## 5. Open questions

None.

## 6. Step notes

### Step 1.1

Done: `tools/psx_sources.txt` (single list of URLs and pins) and `make psx-net-check` (`tools/psx_net_check.sh`), outside `make test-all`. Run on 2026-10-08: exit 0, all 9 lines reachable; failure path checked by hand (one `unreachable` line per bad host, every line still probed, exit 1).

| Component | Source | Pin |
|---|---|---|
| Emulator | PCSX-Redux, git clone of `grumpycoders/pcsx-redux` (35 submodules, all on github.com) | `9eb5441` (last before SDL3, absent from noble) + upstream fix `307177b` applied on top: without it `-cli` segfaults on a disc boot (`src/gpu/soft/gpu.cc:87`) |
| Build deps | Ubuntu noble apt (capstone, freetype, glfw3, ffmpeg libs, curl, libuv, zlib) | noble series only (-updates/-security drop old versions) |
| BIOS | OpenBIOS from the same tree, built with noble's `gcc-mipsel-linux-gnu` 12.2.0; no Sony BIOS | same commit |
| Toolchain, PSn00bSDK, mkpsxiso | `PSn00bSDK-0.24-Linux.zip` release: prebuilt `mipsel-none-elf` GCC 12.3.0, binutils, `elf2x`, mkpsxiso 2.03; no GCC compiled | sha256 `5ada7c94…` |
| CD license data | none: PSn00bSDK ships none, OpenBIOS does not check it | — |
| Test PS-EXE | ps1-tests `build-158` `gte/test-all/test-all.exe`; TTY text `Auto test all GTE valid opcodes/registers` | zip sha256 `5de0e828…` |
| Test CD | BuzzyBee v1.1 `buzzy.bin/.cue` from `psx.arthus.net` (no license data) | sha256 in the sources list |

Hosts: `github.com`, `release-assets.githubusercontent.com`, `psx.arthus.net`, `archive.ubuntu.com`, `security.ubuntu.com`.

Deviations: the network was never opened by the human. GitHub git clones and release downloads (via `release-assets.githubusercontent.com`) work through the session proxy; GitHub web/API pages and `distrib.app.pcsx-redux.consoledev.net` (PCSX-Redux prebuilts) do not, so the emulator is built from source (~8.5 min on 4 cores in the PRD's trial). "Prebuilt" holds for GCC. Rejected: DuckStation (many hosts, no headless pad scripting), Mednafen and MAME (need a Sony BIOS), PCSX-Reloaded (needs a display). The PRD's trial already saw headless TTY output, identical CD captures and a different no-disc capture; scripted pad input (Lua `setOverride`) is untried. No `guideline` decisions.

### Step 1.2

Done: `make psx-emulator-setup` (`tools/install_psx_emulator.sh`), `make psx-boot` (`tools/psx_boot.sh`, `tools/psx_capture.lua`), `make psx-emulator-check` (`tools/psx_emulator_check.sh`), all outside `make test-all`.

- Setup into an empty directory: exit 0 in 610 s, stamp written; re-run exits 0 at once. The emulator finds `src/mips/openbios/openbios.bin` by its own fallback (next to the binary), so the BIOS needs no placement.
- Boot: `-cli -stdout -softgpu -interpreter -run` plus `-loadexe`/`-iso` and a Lua `-dofile` that writes the capture at frame `PSX_FRAMES` (600) and quits; each boot in a throwaway HOME. Capture format: `psx-vram <w> <h> <16|24>\n` then the raw `PCSX.GPU.takeScreenShot()` bytes. A 600-frame boot takes ~11 s (real-time pace).
- The TTY file also holds emulator log lines with per-boot temp paths: never compare TTY files whole, only grep them.
- Check: exit 0 in ~55 s. exe captures `c9b5ccda…` ×2 (test-all draws nothing: all-zero but deterministic); known line in the exe TTY; BuzzyBee 320×240 `23cd1099…` ×2; empty boot 640×478 `d5b36d08…`, different.
- Scripted pad input works headless and deterministically: `PCSX.SIO0.slots[1].pads[1].setOverride(PCSX.CONSTS.PAD.BUTTON.CROSS)` / `clearOverride` from the `GPU::Vsync` listener changed BuzzyBee's screen, same capture over two runs. Trial only, nothing committed.
- Deviations: the check also prints one `ok` line per passing condition. No `guideline` decisions.

### Step 2.1

Done: `make psn00bsdk-toolchain-setup` rewritten (`tools/install_psn00bsdk_toolchain.sh`: the pinned PSn00bSDK 0.24 zip, sha256-checked, into `PSN00BSDK_DIR`, default `third_party/psn00bsdk`; ~7 s; GCC no longer built, `ftpmirror.gnu.org` gone), `make psx-hello` (SDK `sdk.cmake`, `cmake --fresh`, fails without the SDK stamp), the TTY line `psx-hello: main loop started`, and `make psx-hello-check` (`tools/psx_hello_check.sh <install> --exe <file> <line> <frames> <out-dir>`), all outside `make test-all`.

- Setup into an empty `tmp/psn00bsdk`: stamp written; re-run exits 0 at once. `elf2x` did not crash; `.exe` sha256 `b79ccaeb…`, identical across build directories.
- Check against the step 1.2 emulator: exit 0 in ~32 s; five conditions hold (`magic`, `deterministic`, `differs-from-empty`, `not-uniform`, `tty-line`). Captures 320×240×16 `b4934260…` ×2; empty boot `d5b36d08…` (same as step 1.2).
- A pad is present on port 1 headless: the capture shows `PORT 1: TYPE=4`.
- Failure paths hand-tested: wrong TTY line, missing emulator install with a fake `.exe`, uniform synthetic capture, missing SDK, non-empty dir without stamp, wrong sha256.
- Deviations: the check also prints one `ok` line per condition; it calls `tools/psx_boot.sh` directly. The verification reused the chunk 1 toolchain install and the step 1.2 emulator install (both created from empty directories). No `guideline` decisions.

### Step 2.2

Done: `psx-hello/system.cnf` (`BOOT=cdrom:\PSXHELLO.EXE;1`), `psx-hello/iso.xml` (trimmed SDK template), `psn00bsdk_add_cd_image` in `psx-hello/CMakeLists.txt`, `make psx-hello-cd` (`psx-hello/build/psx-hello.bin/.cue`, `PSX_HELLO_CUE`), `--iso <cue>` in `tools/psx_hello_check.sh` (a `cue` condition replaces `magic`), `make psx-hello-cd-check`; all outside `make test-all`.

- Image: 2 490 768-byte `.bin` (1059 × 2352, `MODE2/2352`). The `.bin` changes on every build (mkpsxiso stamps the time): never hash it. The `.exe` stays `b79ccaeb…`.
- `make psx-hello-cd-check`: exit 0 in ~32 s, five conditions hold. CD captures 320×240×16 `b4934260…` ×2, equal to the `.exe` capture, and stable across builds. Empty boot `d5b36d08…`. The TTY shows `SYSTEM.CNF` read, `PSXHELLO.EXE` booted, then the known line.
- Deviation from the step's wording: no license data on the disc, per step 1.1's decision (PSn00bSDK ships none, OpenBIOS does not check it).
- Deviation from the PRD: a `.cue` naming a missing `.bin` does not fail the boot; the emulator drops to the OpenBIOS shell (deterministic 640×478 capture `aff8b611…`). Only `cue` and `tty-line` catch it, which is enough for exit 1.
- Verification reused the step 2.1 SDK install and the step 1.2 emulator install. No `guideline` decisions.

### Step 2.3

Done: `make psx-hello-e2e` and `make psx-hello-e2e-baseline` (both depend on `psx-hello-cd-check`, i.e. a fresh image plus step 2.2's proof, and on a `psx-hello-frames-guard` refusing `PSX_FRAMES` ≠ 600), `tools/psx_reference_check.sh <reference> <capture>`, and the committed reference `psx-hello/psx-hello-cd.vram.sha256`; all outside `make test-all`.

- Reference `b4934260…` (320×240×16 CD capture, header included), written by the baseline target after the five conditions held; equals the step 2.1/2.2 captures.
- `make psx-hello-e2e`: exit 0 in ~32 s. On an altered copy of the reference (`PSX_HELLO_REFERENCE=tmp/…`): exit non-zero, naming expected and actual hashes and the baseline target. The committed file was never hand-edited.
- Hand-tested: missing/malformed reference, missing/empty capture, wrong argument count; a failing proof stops make before any comparison; `PSX_FRAMES=10` fails in 0.4 s with no boot.
- `make test` and `make e2e` pass, no `tests/e2e/*.sha256` changed.
- Deviation: the frame guard is a separate prerequisite target (a recipe line ran after the proof had already failed). Verification reused the existing installs. No `guideline` decisions.
