---
title: "PSX sources and network check"
description: "Plan 9 has no fixed, reachable source for the emulator, toolchain, SDK, CD tool and test media, and nothing tells a run that a host it needs is blocked."
status: needs-triage
---

## Problem Statement

Plan 9 (PSX toolchain and a first `.bin/.cue`) runs unattended in a sandbox whose network is partly blocked. Today the repository has a toolchain script that compiles GCC from `ftpmirror.gnu.org` and a doc that names three possible emulators without picking one. Nothing says which emulator, BIOS, toolchain, SDK version, CD image tool or test media the plan uses, at which exact version, or from which host. A later step that hits a blocked host fails deep inside a long setup, with an error that does not name the host. Plans 10 and 11 also need an emulator that takes scripted pad input headless, so a wrong pick now costs two plans later.

## Solution

Each component gets one concrete source at a fixed version, chosen only among sources measured as reachable from this sandbox on 2026-10-08. The choices live in one machine-readable sources list in `tools/` and in the plan's step note. A new `make psx-net-check` target probes every host the list needs and fails, naming the host, if one is unreachable. Running it passes. Later steps of plan 9 may only download from hosts in this list; a step that needs another host adds it to the list, and the probe then fails until it is reachable.

### Chosen sources (measured reachable on 2026-10-08)

| Component | Source | Fixed version |
|---|---|---|
| Emulator | PCSX-Redux, built from source: `git` clone of `https://github.com/grumpycoders/pcsx-redux` with its 35 submodules (all on `github.com`) | commit `9eb544100311010c68af9049f57bf1f718961774` (2026-05-04), the last commit before the SDL3 migration (`b4ab385`, 2026-05-06), so every build dependency is an Ubuntu 24.04 package (GLFW 3.3, not SDL3); plus upstream commit `307177ba56cdc5938f2a6f2a954753992ad13287` ("soft: Bail out of display-clear paths without a GUI", one file) applied on top, fetched from the same repository |
| Emulator build dependencies | Ubuntu 24.04 "noble" archive (`archive.ubuntu.com`, `security.ubuntu.com`): `build-essential pkg-config libcapstone-dev libfreetype-dev libglfw3-dev libavcodec-dev libavformat-dev libavutil-dev libswresample-dev libcurl4-openssl-dev libuv1-dev zlib1g-dev` | the noble series; versions measured: capstone 4.0.2, glfw 3.3.10, ffmpeg 6.1.1, libuv 1.48.0 |
| BIOS | OpenBIOS, built from the same PCSX-Redux commit (`src/mips/openbios`), with `gcc-mipsel-linux-gnu` / `g++-mipsel-linux-gnu` from the noble archive (12.2.0). No Sony BIOS: none has a legitimate source | same commit as the emulator |
| Toolchain | PSn00bSDK release `PSn00bSDK-0.24-Linux.zip` (`https://github.com/Lameguy64/PSn00bSDK/releases/download/v0.24/PSn00bSDK-0.24-Linux.zip`, redirected to `release-assets.githubusercontent.com`): it bundles a prebuilt `mipsel-none-elf` GCC 12.3.0 and binutils, so no GCC is compiled | sha256 `5ada7c9478d795b22bde96ee23d6869290c14a740bcde3d57c01c342a26ded35` |
| PSn00bSDK | the same release archive (libraries, headers, CMake setup, `elf2x`) | v0.24, tag commit `06e65bea3a778b2dae5af77a7935ae3868ddd4d3` |
| CD image tool | `mkpsxiso` 2.03, shipped in the same release archive | same sha256 |
| CD license data | none. PSn00bSDK does not ship license data, and no legitimate source exists. Recorded fact: PCSX-Redux with OpenBIOS boots a disc without Sony license data; the test CD below has none (sector 4 holds homebrew text, not the Sony license), so step 1.2 proves it | — |
| Test PS-EXE writing TTY | `gte/test-all/test-all.exe` from the ps1-tests release `build-158` (`https://github.com/JaCzekanski/ps1-tests/releases/download/build-158/tests.zip`); its reference TTY output ships beside it (`gte/test-all/psx.log`), and contains the known line `Auto test all GTE valid opcodes/registers` | zip sha256 `5de0e82854c1a511697a7ade381c2d778ec4b2fff4a7d7ac79e8098c35eb859b`, tag commit `5f73d61b27a8d51269b54dde1c456d2d2f7dc865` |
| Test CD image | BuzzyBee v1.1 homebrew, `buzzy.bin` + `buzzy.cue` from `https://psx.arthus.net/homebrew/BuzzyBee_v1.1_image/` (made with mkpsxiso, boots `BUZZYBEE.EXE` through `SYSTEM.CNF`) | `buzzy.bin` sha256 `d5d7a62937c0e4a7cfe22b5eb73d2fbc869927b8744dfe1f7301f839f7acc32d` (38,692,752 bytes), `buzzy.cue` sha256 `fe9e76a64d87e423fd18eb1b1e2458b54c498d8d78ad533e98a067d7168f7e07` |

Hosts the plan needs, and nothing else: `github.com` (git clones, release downloads), `release-assets.githubusercontent.com` (where GitHub redirects release downloads), `psx.arthus.net`, `archive.ubuntu.com`, `security.ubuntu.com`.

## User Stories

1. As the plan-pilot run, I want one fixed source per PSX component, so that every later step of plan 9 installs the same bits.
2. As the plan-pilot run, I want each source chosen among hosts measured reachable now, so that no later step discovers a blocked host halfway through a long setup.
3. As the plan-pilot run, I want `make psx-net-check` to fail when any needed host is unreachable, so that the run stops at once with a report instead of a guess.
4. As the plan-pilot run, I want the failure message to name the unreachable host, so that the report tells the human exactly which host to open.
5. As the plan-pilot run, I want the probe to check every host before concluding, so that one run reports all blocked hosts, not only the first.
6. As the plan-pilot run, I want the probe to pass in this sandbox today, so that step 1.1 is verified.
7. As the implementer of step 1.2, I want the emulator source pinned to one commit, so that the setup target into an empty directory is reproducible.
8. As the implementer of step 1.2, I want an emulator that runs without any window, so that the CLAUDE.md ban on windows holds.
9. As the implementer of step 1.2, I want an emulator that can dump VRAM from a script, so that captures are byte files an agent can compare (`takeScreenShot` from Lua at the pinned commit).
10. As the implementer of step 1.2, I want an emulator that writes TTY output to stdout, so that a capture of known text is a plain file.
11. As the implementer of step 1.2, I want a BIOS with a legitimate source, so that no copyrighted Sony BIOS is downloaded.
12. As the implementer of step 1.2, I want a prebuilt PS-EXE whose expected TTY output is published, so that I know which text to look for.
13. As the implementer of step 1.2, I want a prebuilt CD image that boots through `SYSTEM.CNF` and draws, so that the disc path is tested independently of our toolchain.
14. As the implementer of step 1.2, I want that CD image to lack Sony license data, so that its boot also proves our own discs need none.
15. As the implementer of step 2.1, I want a prebuilt toolchain, so that no GCC is compiled (the plan's rule).
16. As the implementer of step 2.1, I want the toolchain and PSn00bSDK from the same archive, so that their versions match by construction.
17. As the implementer of step 2.2, I want `mkpsxiso` from that same archive, so that the CD tool needs no extra host.
18. As the implementer of steps 1.2 to 2.3, I want each download pinned by sha256 or commit, so that a setup can refuse a changed file.
19. As the author of plans 10 and 11, I want an emulator that takes scripted pad input headless, so that the game can be driven in it.
20. As the author of plan 11, I want an emulator with a scripting API that reads emulator state per frame, so that CPU time and memory peaks can be measured.
21. As a later step that needs a new host, I want to add it to the sources list, so that the probe covers it and fails until it is reachable.
22. As a maintainer, I want the sources list in one file, so that the probe and the setup targets cannot drift apart.
23. As a maintainer, I want `make test` and `make e2e` untouched, so that the fast loop stays fast.
24. As a maintainer, I want `make psx-net-check` outside `make test-all`, so that a network outage never breaks the fast loop.
25. As a maintainer, I want the target documented in the Makefile legend, so that it is found like every other target.
26. As a human reading the plan later, I want the step note to list each choice and why, so that I can audit the picks without rerunning the probes.
27. As a human, I want the rejected candidates named with their reason, so that nobody re-investigates them.

## Implementation Decisions

- **Emulator: PCSX-Redux built from source at commit `9eb5441…`.** Reasons, from its source at that commit:
  - `-cli` / `-no-ui` selects a text UI with no window (`PCSX::TUI`); `-stdout` sends TTY to stdout; `-softgpu` selects the software renderer; `-run`, `-loadexe <file>`, `-iso <file>`, `-testmode` (quit on the PS-EXE's exit) exist.
  - Lua scripts run at start (`-dofile`, `-exec`); the Lua API exposes `PCSX.GPU.takeScreenShot()` (the displayed VRAM area, with width, height and depth: the capture step 1.2 writes to a file; the raw `PCSX.GPU.getVRAM()` only appears after this commit), and `PCSX.SIO0.slots[n].pads[m].setOverride(button)` (scripted pad input, the preference for plans 10 and 11).
  - With no BIOS configured it falls back to `openbios.bin`, which builds from its own tree.
  - Its prebuilt builds live on `distrib.app.pcsx-redux.consoledev.net`, which the egress proxy denies; so it is built from the git clone. The plan's "prefer prebuilt" rule targets GCC, which stays prebuilt.
  - The commit is the last one before SDL3 replaced GLFW (`b4ab385`, 2026-05-06): Ubuntu 24.04 ships no SDL3 package, and building SDL3 too would add a component.
  - **Without upstream commit `307177b` the pinned commit crashes headless:** in `-cli` mode the software GPU calls `glClear` with no GL context whenever the display is disabled (a disc boot hits it at once; segfault at `src/gpu/soft/gpu.cc:87`). `307177b` (2026-05-16, after the SDL3 switch) makes that path return when there is no GUI; it applies cleanly on `9eb5441`. The emulator setup (step 1.2) fetches both commits from the same repository and applies `307177b`'s change; no host is added.
  - Measured in a trial while writing this PRD (see Further Notes): the patched build runs with no display, prints the PS-EXE's TTY, and gives byte-identical captures on two disc boots. Step 1.2 still owns that check.
- **Rejected emulators:** DuckStation (needs its own dependency build script pulling from many hosts, no headless pad scripting; its `regtest` runner is source-only); Mednafen 1.29 and MAME 0.264 (in apt, but both require a Sony BIOS, which has no legitimate source; MAME is also flagged inaccurate by PSn00bSDK's docs); PCSX-Reloaded 1.9.94 (in apt, GUI plugins need a display); PCSX-Redux prebuilt (host denied).
- **Toolchain, PSn00bSDK and mkpsxiso: one release archive, PSn00bSDK v0.24 Linux.** It holds `mipsel-none-elf-gcc` 12.3.0, binutils, `elf2x`, `mkpsxiso` 2.03, the libraries and the CMake integration. The binaries were run in this sandbox (`--version`/`--help` answer on glibc 2.39). This replaces the GCC-from-source path of `tools/install_psn00bsdk_toolchain.sh` and its `ftpmirror.gnu.org` host; rewriting that script is step 2.1's job, not this step's.
- **BIOS: OpenBIOS from the PCSX-Redux tree,** built with Ubuntu's `mipsel-linux-gnu` cross GCC (the PCSX-Redux MIPS makefiles default to that prefix). It keeps the emulator setup independent from the toolchain setup of phase 2.
- **CD license data: none.** PSn00bSDK's own template says license files are not distributed and are only needed by Japanese or modchipped PAL consoles. OpenBIOS does not check them. The decision recorded: our discs carry no license data; step 1.2 proves the emulator boots such a disc with the BuzzyBee image, which carries none.
- **Test PS-EXE: ps1-tests `gte/test-all/test-all.exe`.** It prints its name, a description line, pass/fail counts and `Done.` over TTY; the published `psx.log` beside it is the expected output. The known text step 1.2 looks for is `Auto test all GTE valid opcodes/registers` (printed before any test runs, so it does not depend on emulator accuracy).
- **Test CD image: BuzzyBee v1.1.** A complete homebrew game image (data track + one audio track; audio is ignored), booted through `SYSTEM.CNF`. It is used only in the sandbox, never committed.
- **apt packages are pinned to the Ubuntu 24.04 series, not to exact versions.** The `-updates` and `-security` pockets drop superseded versions, so an exact pin would break on the next security update; the measured versions are recorded in the step note for the record.
- **Sources list, the single place for URLs and pins.** A new text file in `tools/` with one line per source: a component name, a kind, a URL, and a pin (a commit for `git`, a sha256 for `file`, the series for `apt`). A fourth kind, `host`, names a host reached only through a redirect (`release-assets.githubusercontent.com`). Comment lines start with `#`. Later steps' setup scripts read their URL and pin from this file instead of repeating them. Its lines at the end of this step: the emulator base commit and the emulator fix commit (two `git` lines on the PCSX-Redux URL); the Ubuntu noble archive and security mirrors (`apt`); the PSn00bSDK v0.24 Linux archive, the ps1-tests `build-158` archive, `buzzy.bin` and `buzzy.cue` (`file`); `release-assets.githubusercontent.com` (`host`). The emulator's submodules are not listed one by one: all 35 are on `github.com`, already covered by the emulator line, and their commits are pinned by the emulator commit itself.
- **The probe, a new script in `tools/` run by `make psx-net-check`.** For each line, by kind:
  - `git`: `git ls-remote` of the URL (the session's git proxy serves github.com for git even where its web pages answer 403).
  - `file`: an HTTP request for the first byte of the URL, following redirects, through the proxy and its CA bundle.
  - `apt`: an HTTP request for the series' `Release` file on the mirror.
  - `host`: any HTTP answer from the host counts as reachable (a 404 from the host itself is fine; a refused CONNECT, a proxy 403/502 or a timeout is not).
  - Every line is probed before concluding. Each failure prints one line naming the component and the host (`psx-net-check: <component>: <host> unreachable`), and the script exits 1 if any failed, 0 otherwise. The probe never downloads a whole file and never checks a sha256: the download and its check belong to the setup targets.
  - The sources list path is a script argument, with the `tools/` file as the Makefile's default (a `PSX_SOURCES` variable), so a run can point it at another list.
  - Every request has a short timeout, so a black-holed host fails in seconds instead of hanging.
- **Makefile:** `psx-net-check` is a phony target outside `test`, `e2e` and `test-all`, with a legend entry in the comment block at the top, in the same style as the others.
- **Step note:** the implementer appends the step 1.1 note to section 6 of `docs/plan-9.md`: the table of choices above (component, source, version or sha256), the host list, the rejected candidates with their reasons, the license-data decision, and the result of running `make psx-net-check`.
- **Failure rule.** If, when the step runs, any line of the list is unreachable, the step fails at once and the report names the missing host; it is never answered by swapping in an unmeasured source.

## Testing Decisions

- A good test here checks what the probe tells the run: pass when every host answers, fail and name the host when one does not. It does not test curl or git.
- No unit test is added to `tests/`: the probe is network-bound, and `make test`/`make e2e` must stay untouched and no slower (plan rule, CLAUDE.md). The probe is not in `make test-all`.
- The step is verified by running `make psx-net-check` in this sandbox: it exits 0.
- The failure path is verified once, by hand, during the step: a copy of the sources list in `tmp/` with one extra `host` line for an unreachable name (`https://unreachable.invalid/`) and one `file` line for a closed local port (`http://127.0.0.1:9/`), passed through `PSX_SOURCES`; the target must exit non-zero, print one line per failure naming `unreachable.invalid` and `127.0.0.1`, and still report every other line as probed (no early exit).
- Prior art for shell tools driven from the Makefile: `tools/e2e_limits.sh` (checks every file before concluding, one message per failure, exit 1).

## Out of Scope

- Installing anything: the emulator setup target is step 1.2, the toolchain setup target step 2.1. This step downloads nothing into the repository.
- Rewriting `tools/install_psn00bsdk_toolchain.sh` or `docs/psx-hello-world.md` (steps 2.1 and 2.4).
- Booting anything in the emulator, VRAM capture format, TTY file format (step 1.2).
- Verifying sha256 pins during the probe (the setup targets do it).
- Audio, the game itself, plans 10 and 11's pad replay.

## Further Notes

- Network facts measured on 2026-10-08 in this sandbox, which differ from the plan's precondition text: GitHub release downloads work (`github.com/<owner>/<repo>/releases/download/...` answers 302 to `release-assets.githubusercontent.com`, which serves the file), while `api.github.com` for these repos, `github.com` web pages and `codeload.github.com` tarballs answer 403. `objects.githubusercontent.com` is not used by any chosen source. `distrib.app.pcsx-redux.consoledev.net` is denied (502 at CONNECT). `sourceforge.net` answers 403. `ftp.gnu.org` answers but is no longer needed.
- The PSn00bSDK and ps1-tests release asset names cannot be listed (the API answers 403); the URLs above were found in the projects' own docs and CI files and downloaded successfully.
- Trial of the chosen emulator, run while writing this PRD (scratch directory, nothing kept in the repository):
  - Ubuntu packages above installed; `9eb5441` cloned with its submodules (all from `github.com` through the git proxy); `make pcsx-redux` took 8.5 minutes on 4 cores; `make -C src/mips/openbios` built `openbios.bin` (512 KiB) with Ubuntu's `mipsel-linux-gnu` GCC in about 1 second.
  - `pcsx-redux -cli -stdout -softgpu -interpreter -run -loadexe test-all.exe`, with `DISPLAY` unset: OpenBIOS found by fallback, and stdout carries `Auto test all GTE valid opcodes/registers`, `Passed tests: 1150`, `Failed tests: 0`, `Done.`. The program then loops: the run ends by `timeout` (exit 124) or by `PCSX.quit()` from Lua.
  - `-iso buzzy.cue` without `307177b`: segfault (see Implementation Decisions). With it: a Lua script (`-dofile`) on the `GPU::Vsync` event writes `PCSX.GPU.takeScreenShot().data` at vsync 600 and calls `PCSX.quit()` (exit 0). Two disc boots gave the same 320x240 capture (sha256 `d8cb7f50…`); a boot with no disc gave a different one (`7d727f2e…`). This is the recorded fact behind the license-data decision: a disc with no Sony license data boots.
  - Scripted pad input (`setOverride`) was not tried; step 1.2 records it.
