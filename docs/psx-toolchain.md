# PSX toolchain, emulator and capture

The pinned toolchain that builds PS-EXEs and CD images, and the pinned
emulator that boots them headless into a capture an agent can check.
Everything below runs in the cloud sandbox, without a window. The game's own
build and checks are in `docs/psx-game.md`; `make help` and the usage header
of each `tools/psx_*.sh` script hold the per-option detail.

## Sources and network

`tools/psx_sources.txt` is the single source of truth for every URL and pin
(commits, sha256s, apt series). Nothing is downloaded from anywhere else.

| Component | What is used |
|---|---|
| Emulator | PCSX-Redux built from source at its pinned commit, plus the upstream headless fix and the repository's `tools/pcsx-redux-*.patch` applied on top |
| BIOS | OpenBIOS from the same PCSX-Redux tree; no Sony BIOS |
| Toolchain, SDK, CD tool | Linux: the PSn00bSDK 0.24 release (prebuilt `mipsel-none-elf` GCC 12.3.0, binutils 2.40, `elf2x`, libpsn00b, mkpsxiso 2.03). macOS: the prebuilt arm64 `mipsel-none-elf` GCC 14.2.0 of alextrevisan/mipsel-none-elf (sha256-pinned), PSn00bSDK 0.24 built from its commit with it |
| CD license data | none: PSn00bSDK ships none and OpenBIOS does not check it |
| Emulator test media | a ps1-tests PS-EXE and the BuzzyBee homebrew CD image, used by `psx-emulator-check` only |

Hosts reached: `github.com` (git clones and release downloads, the latter
redirected through `release-assets.githubusercontent.com`), `psx.arthus.net`,
the Ubuntu noble archive and security hosts, `ftp.gnu.org` and `gcc.gnu.org`
(the macOS toolchain build). `make psx-net-check` probes
every one of them without downloading and names each unreachable host, in
seconds; both setups run it first.

GitHub web and API pages and the PCSX-Redux prebuilt host are unreachable from
the sandbox, which is why the emulator is built from source. A new host is
added to `tools/psx_sources.txt` and so to the probe, never reached ad hoc.

## Setup

Run once each, in either order, before any build or check:

- `make psx-emulator-setup` builds PCSX-Redux and OpenBIOS into
  `PSX_EMULATOR_DIR` (default `third_party/pcsx-redux`), ~10 min. The
  emulator finds OpenBIOS next to its own binary: no BIOS to place.
- `make psn00bsdk-toolchain-setup` installs the sha256-checked PSn00bSDK
  release into `PSN00BSDK_DIR` (default `third_party/psn00bsdk`), a few
  seconds plus the download. On macOS, which has no release, it unpacks the
  prebuilt arm64 GCC 14.2.0 and builds PSn00bSDK with it into the same
  layout (`tools/build_psn00bsdk.sh`), a few minutes. With
  `PSN00BSDK_FROM_SOURCE` set it builds binutils 2.40 and GCC 12.3.0 from
  source instead, 30 to 60 min; on Linux that SDK gives byte-identical
  PS-EXEs to the release's.

On Linux both install their build dependencies with `sudo apt` unless
`PSX_EMULATOR_SKIP_APT` or `PSN00BSDK_SKIP_APT` is set. On macOS nothing
needs sudo: the toolchain build needs only the Command Line Tools, `cmake`,
`git` and `curl`; the emulator takes its libraries from the user's Homebrew
and builds OpenBIOS with the `mipsel-none-elf` GCC of `PSN00BSDK_DIR`, so the
toolchain setup comes first. That OpenBIOS differs from the Linux one, built
with `mipsel-linux-gnu`, yet gives the same captures (`psx-check` green
with it). Both exit 0 at once
when the directory holds the stamp of a complete install for the same pins,
and refuse, deleting nothing, a non-empty directory without that stamp: look
at what is there before removing it.

No build or check target ever runs a setup; a missing install fails with a
message naming the setup target.

## Boot and capture

`make psx-boot PSX_EXE=<exe>` or `make psx-boot PSX_ISO=<cue>` (or neither,
for an empty boot) boots that input headless: `-cli`, software GPU,
interpreter, a throwaway HOME per boot. It runs `PSX_FRAMES` vsyncs (default
600, ~11 s at real-time pace) and writes:

- `PSX_OUT.vram`, the capture (default `PSX_OUT` is `tmp/psx/boot`);
- `PSX_OUT.tty.txt`, the emulator's stdout, which holds the program's TTY;
- `PSX_OUT.log`, its stderr.

A capture is one text line `psx-vram <w> <h> <depth>` (depth 16 or 24), then
the raw bytes of the displayed frame, as `PCSX.GPU.takeScreenShot()` returns
them (`tools/psx_capture.lua`). A boot that exits non-zero, writes no capture
or outlasts `PSX_BOOT_TIMEOUT` seconds (default 120) fails naming its input.
Any later PSX binary boots the same way; the until-exit mode for programs
that leave the emulator themselves is in `docs/psx-game.md`.

## Checks

| Target | What it proves | Time |
|---|---|---|
| `make psx-emulator-check` | the emulator itself: the pinned test media boot deterministically, the test TTY line appears, the CD capture differs from an empty boot | ~1 min |

None of these targets is in `make test-all`, and none runs a setup.

## Traps

- Never hash the `.bin`: mkpsxiso stamps the build time into it, so it changes
  on every build; the capture is what stays stable.
- Never compare TTY files whole: they also hold emulator log lines with
  per-boot temp paths. Grep them for a known line.
- A `.cue` naming a missing `.bin` does not fail the boot: the emulator lands
  in the OpenBIOS shell and captures it deterministically. Only the `cue` and
  `tty-line` conditions catch it; a deterministic capture is not a successful
  boot.
- The emulator pin needs the upstream headless fix applied on top: without it
  `-cli` segfaults on a disc boot. A pin bump must keep it or include it.
- The emulator pin is the last commit before PCSX-Redux moved to SDL3, which
  Ubuntu noble does not ship.
- `tools/pcsx-redux-event-listener-stack.patch` stops each Lua event listener
  from leaving two slots on its Lua thread per event: unpatched, any boot
  with a `GPU::Vsync` listener aborts at about 32,700 vsyncs (LuaJIT stack
  overflow, `An unknown exception occured`). Upstream still has the bug.
  Every patch is in the install stamp, so changing one rebuilds.
