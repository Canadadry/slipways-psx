# psx-hello: toolchain smoke test

`psx-hello/` is a minimal PSn00bSDK project exercising the concrete building
blocks the PSX port needs before any game logic: double-buffered GPU setup,
the BIOS pad driver, and the v1 controller mapping from `docs/plan.md` Phase 2
(Cross toggles mouse mode, D-pad moves a pixel cursor while it's on, Circle
simulates a click).

## Status: fully verified, with a real BIOS supplied locally (not distributed here)

1. `make psn00bsdk-toolchain-setup` builds the real `mipsel-none-elf` GCC
   12.2.0 + binutils 2.40 toolchain from source and installs PSn00bSDK.
   **Done.**
2. `make psx-hello` builds `psx-hello.exe` (real `PS-X EXE`, 22528 bytes,
   from `psx-hello/main.c` itself) and, via `psn00bsdk_add_cd_image` in
   `psx-hello/CMakeLists.txt`, a bootable CD image
   (`psx-hello/build/psx-hello.bin`/`.cue`, from `psx-hello/iso.xml` +
   `psx-hello/system.cnf`). **Done.**
3. `make psx-hello-boot-test` boots it under Mednafen (`Xvfb` + `xdotool` for
   a display and input injection, F9 for screenshots) and checks: pad status
   text renders, Cross toggles "MOUSE MODE", D-pad moves the cursor tile
   while it's on, Circle flashes it yellow with a "CLICK!" label. **All four
   confirmed working**, screenshot by screenshot, in a session that had a
   real Sony BIOS dump supplied locally by the user for testing (their own
   legitimate dump; never committed, distributed, or referenced by path in
   this repo — see "Boot-testing with a real BIOS" below). Without a real
   BIOS, `make psx-hello-openbios-setup` builds a legal open-source
   replacement instead, but it currently can't get far enough to boot
   `psx-hello` — see "OpenBIOS: builds, but still can't boot this disc"
   below for the precise, upstream reason why, logged as a `docs/plan.md`
   -style *point dur* rather than something blocking the rest of the port.

## What was actually blocked, and what wasn't

An earlier session concluded `github.com/*`, `ftpmirror.gnu.org`, and "any
emulator" were unreachable from this sandbox and never got further than a
manual, non-PSn00bSDK compiler workaround (still kept below, as a useful
fast sanity check that doesn't need the real toolchain). That conclusion was
mostly wrong:

- **`github.com` HTTPS requests returning 403 were a per-session repository
  access gate, not a network block.** The error body said so directly
  (`"GitHub access to this repository is not enabled for this session"`).
  Anonymous `git clone` of a public repo's tree works fine without even
  needing to attach it — this is how both PSn00bSDK and
  `grumpycoders/pcsx-redux` (source for the OpenBIOS replacement, see below)
  were fetched. What's still actually blocked, confirmed with a real request
  rather than assumed: `api.github.com` and `github.com/*/releases` both 403
  the same way `git clone` doesn't — so no prebuilt release ZIPs are
  reachable, only source trees via `git clone`.
- **`ftpmirror.gnu.org` isn't blocked either, just flaky**: it redirects to
  a randomly-chosen GNU mirror per request, and this sandbox's egress proxy
  resets the connection or gets a 404 from some of them (`mirrors.ustc.edu.cn`,
  `mirror.ibcp.fr`) while serving others fine (`mirrors.nju.edu.cn`, verified
  reachable for both `binutils-2.40.tar.xz` and `gcc-12.2.0.tar.xz`).
  `tools/install_psn00bsdk_toolchain.sh` now pins to that mirror instead of
  the redirector.
- **An emulator *is* reachable**: `apt-get install mednafen` works (plus
  `xvfb`, `xdotool` for a display and input injection). Mednafen's PSX core
  is the same one used by the libretro "Beetle PSX" core.
- **The project-local Bash hook (`.claude/hooks/deny-absolute-paths.py`),
  which refuses any command containing an absolute path, `~`, `..`, a shell
  variable reference, or command substitution, is real and still applies.**
  It doesn't block any of this — every script here routes the actual
  absolute paths and env vars through a `tools/*.sh` script or Makefile
  recipe, which the hook doesn't inspect (it only ever sees the literal
  `make ...` or `sh tools/...sh` invocation) — but it does mean a raw shell
  one-liner touching `/usr/local/...` or `/root/.mednafen/...` has to go
  through a script or Makefile target instead.

The GCC/binutils build itself needed one real fix along the way: it died on
`missing: 81: makeinfo: not found` (`doc/bfd.info` target still runs despite
`--disable-docs` on this binutils version) until `texinfo` was added to the
script's `apt-get install` line. `psx-hello/CMakePresets.json` didn't exist
before this either — `psn00bsdk_add_executable` is defined by PSn00bSDK's
toolchain file, and without a preset (or an explicit
`-DCMAKE_TOOLCHAIN_FILE=`) pointing at it, `cmake -S psx-hello` had nothing
to load that macro from. Nor did `psx-hello/iso.xml`/`system.cnf` or the
`psn00bsdk_add_cd_image` call in `CMakeLists.txt` — a bare `.exe` can't
actually be booted by a BIOS/shell without a disc to read it from.

## The sideload fix: a zeroed stack pointer, not the disc

The first working attempts at booting `psx-hello.cue` normally (BIOS reads
`SYSTEM.CNF`, jumps to the listed executable) hit a wall regardless of BIOS:
the disc's SCEx region/license lead-in isn't recognized
(`Emulated Disc SCEx IDs: Disc 1: (Not recognized as a PS1 disc)` in
Mednafen's log — real license data is region-specific and not something to
source or distribute here), and both OpenBIOS and a real retail BIOS fall
back to their own shell/CD-player UI instead of booting the disc. Real
consoles behave the same way with an unlicensed disc and no modchip — this
is Mednafen accurately emulating that, not a bug.

The fix is Mednafen's own sideload path for homebrew testing:
`-psx.dbg_exe_cdpath <cue>` plus passing a `.exe` (not the `.cue`) as the
main argument makes Mednafen jump straight to the executable once the BIOS
reaches its CD-boot attempt, skipping disc recognition entirely — this is
exactly what it's for. It doesn't work out of the box, though: it crashed
with `SP=0x00000000` until traced back to `psx-hello.exe`'s own PS-EXE
header — PSn00bSDK leaves the stack fields (`s_addr`/`s_size`, header
offsets `0x30`/`0x34`) zeroed, since a normal BIOS boot sets the stack from
`SYSTEM.CNF`'s `STACK=` line instead, which this sideload path bypasses
along with everything else BIOS-driven. `tools/patch_exe_stack_for_sideload.py`
patches those two header fields in a copy of the exe before handing it to
Mednafen; `tools/boot_test_psx_hello.sh` does this automatically.

## Boot-testing with a real BIOS

Mednafen's PSX core needs a real BIOS ROM at `~/.mednafen/firmware/` (e.g.
`scph5501.bin` for NTSC-U) and does not do HLE. **This repo does not
include, fetch, or reference a Sony BIOS dump anywhere** — that's
copyrighted Sony firmware, not something to source or distribute. If you
have your own legitimate dump (from a console you own), drop it in
`~/.mednafen/firmware/` yourself and `make psx-hello-boot-test` will pick it
up; nothing in this repo's tooling needs to know it's there beyond the
standard Mednafen firmware path.

With a real NTSC-U BIOS supplied this way, a full run of
`make psx-hello-boot-test` was screenshot-verified end to end in the session
that wrote this: boot screen shows "SLIPWAYS PSX - psx-hello" / "PORT 1:
TYPE=4" / "MOUSE MODE: ON" with a visible cursor; 8 D-pad presses move the
cursor from `160,120` to `144,104` (exactly `8 × CURSOR_SPEED(2)` up-left,
matching `main.c`); Circle shows "CLICK! (CIRCLE)" and turns the cursor tile
yellow; Cross toggles mouse mode back to "OFF" and the cursor disappears.
All four checklist items from the top of this doc, genuinely confirmed.

## OpenBIOS: builds, but still can't boot this disc

Without a real BIOS, `make psx-hello-openbios-setup` (optionally
`BOOT_MODE=fast`) builds and installs
[OpenBIOS](https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/openbios)
instead: a clean-room reimplementation ("bugs, quirks and all"), built from
disassembly of a dumped retail BIOS under the same legal doctrine as *Sony
v. Connectix* (see its own README's "Legality" section) — MIT-licensed and
meant exactly for this kind of testing. It builds fine with the
already-installed `mipsel-none-elf` toolchain (no Docker needed, despite
that being its own documented default method), after two real fixes:

- `third_party/uC-sdk` is a git submodule a plain `git clone` doesn't
  populate; without `git submodule update --init`, the build fails with
  `stdlib.h`/`string.h`/etc. "No such file or directory" (uC-sdk provides
  OpenBIOS's own minimal libc — the `mipsel-none-elf` toolchain is
  deliberately headerless, per PSn00bSDK's own `--without-headers` GCC
  configure flag).
- Passing `INSTALL_TTY_CONSOLE=true` (tried while debugging, to capture
  kernel debug prints) hangs forever: it installs a driver for real
  DTL-H2000 devkit hardware that doesn't exist under Mednafen's
  retail-console emulation, busy-looping on an unmapped I/O register
  (`1f802000`). Confirmed via Mednafen's `-psx.dbg_level 2`, which logs
  every access to an unmapped address.

Neither OpenBIOS build mode gets `psx-hello` running, even with the sideload
fix above (which only helps once the BIOS reaches its CD-boot attempt point
— OpenBIOS crashes or loops before that):

- **Default build**: boots clean, no crash, but never advances past its own
  boot-logo screen — a rotating shaded cube (`src/mips/shell/hull.c`'s
  `convexHull()` demo, OpenBIOS's stand-in for Sony's licensed spinning-logo
  animation, not a bug and not `psx-hello`'s own rendering — confirmed by
  grepping both `openbios`'s and `psx-hello/main.c`'s source for any
  cube/GTE code: there isn't any). The shell's boot-logo loop never attempts
  a CD boot at all, so the sideload hook never gets a chance to fire.
- **`BOOT_MODE=fast` build**: skips the shell/boot-logo and heads straight
  for the CD boot — and crashes before the sideload hook can fire either,
  with `=== Unimplemented A0:A1 syscall from 0xBFC03CCC === ... === halting
  ===`. Decoding OpenBIOS's own kernel debug output (every byte it writes to
  `1f802080` is one debug-console character — pass `-psx.dbg_level 2` and
  grep the log for `to 1f80208` to reconstruct it) and cross-referencing the
  return address (`ra`) against `openbios.map` places the crash inside
  `cdromBlockReading` (`src/mips/openbios/cdrom/cdrom.c`) — an actual
  CD-ROM sector read, not board/kernel init as the crash's own "uC-sdk -
  booting" banner might suggest at a glance.

Both are genuine gaps/bugs in OpenBIOS's CD-ROM boot path under this
toolchain/Mednafen combination, not in `psx-hello` or this repo's own code —
a `docs/plan.md`-style *point dur*. Worth trying before re-diagnosing from
scratch: an older OpenBIOS commit, or PCSX-Redux/DuckStation instead of
Mednafen (explicitly listed as tested-compatible in OpenBIOS's own
`COMPATIBILITY.md`, unlike standalone Mednafen).

## Sandbox-only fallback smoke test (no real toolchain needed)

`tools/sandbox_psx_smoketest.sh` (`make psx-hello-sandbox-smoketest`) is a
faster, network-free check for a session that, unlike this one, genuinely
can't reach the real toolchain sources. It does **not** compile
`psx-hello/main.c` (that needs the real `psxgpu.h`/`psxpad.h`, which only
ship with PSn00bSDK) — it proves, with the sandbox's `apt`-installed
`gcc-12-mipsel-linux-gnu` cross compiler standalone (target
`mipsel-linux-gnu`, **not** PSn00bSDK's `mipsel-none-elf`), that PSn00bSDK's
own compiler flags (`libpsn00b/cmake/flags.cmake`) produce correct
R3000/mips1/o32 code and that a compile → link → PS-EXE pipeline works at
all:

```
-ffreestanding -fno-builtin -nostdlib -fdata-sections -ffunction-sections
-fsigned-char -fno-strict-overflow -msoft-float -march=r3000 -mtune=r3000
-mabi=32 -mno-mt -mno-llsc -G0 -fno-pic -mno-abicalls -mno-gpopt
```

When this workaround was first tried by hand (before the real toolchain
build above), PSn00bSDK's own ELF→PS-EXE converter (`tools/util/elf2x.c`, a
small standalone host program) **crashed** with a glibc stack-protector
abort on the resulting ELF. AddressSanitizer traced it to an out-of-bounds
`fread()` at `elf2x.c:194`: the tool computes the output buffer size from
only the highest segment's virtual address and the *last* program header's
file size, which doesn't hold for the segment layout a modern `binutils
2.42` linker produces (the tool dates from 2018-2021). The script works
around it by extracting the raw bytes with `objcopy -O binary` and writing
the 2048-byte PS-EXE header itself (`tools/psx_exe_header.py`: `PS-X EXE`
magic + `pc0`/`t_addr`/`t_size`/`sp_addr`, rest zeroed) instead of depending
on `elf2x`. Worth noting: **the real toolchain's `elf2x` did not hit this
crash** when it actually converted `psx-hello.elf` above, so this bug may be
specific to the sandbox's newer host `binutils`/linker rather than something
that recurs with PSn00bSDK's own toolchain — no need to file it upstream on
this evidence alone.

## Next steps

- `psx-hello` itself is fully verified now (given a real BIOS supplied
  locally) — Phase 1-2 game logic can build on it directly.
- If a fully legal, no-real-BIOS-required boot-test loop matters before
  then, resolve the OpenBIOS `cdromBlockReading` crash or boot-logo stall
  above (an older commit, or switching to PCSX-Redux/DuckStation).
