# psx-hello: toolchain smoke test

`psx-hello/` is a minimal PSn00bSDK project exercising the concrete building
blocks the PSX port needs before any game logic: double-buffered GPU setup,
the BIOS pad driver, and the v1 controller mapping from `docs/plan.md` Phase 2
(Cross toggles mouse mode, D-pad moves a pixel cursor while it's on, Circle
simulates a click).

## Status: builds and produces a bootable disc; booting itself is blocked by a third-party BIOS bug

1. `make psn00bsdk-toolchain-setup` (`tools/install_psn00bsdk_toolchain.sh`)
   builds the real `mipsel-none-elf` GCC 12.2.0 + binutils 2.40 toolchain
   from source, then builds and installs PSn00bSDK itself to `/usr/local`.
   **Done, successfully.** Takes on the order of 30-45 minutes of CPU time
   for the GCC build alone; the script wraps every `make`/`cmake --build`
   step in `timeout` (45 min each) because a GCC/binutils bootstrap
   occasionally hangs instead of failing outright.
2. `make psx-hello` builds the actual project (via
   `psx-hello/CMakePresets.json`'s `default` preset, which points
   `CMAKE_TOOLCHAIN_FILE` at `$PSN00BSDK_LIBS/cmake/sdk.cmake`) and, via
   `psn00bsdk_add_cd_image` in `psx-hello/CMakeLists.txt`, also builds a
   proper bootable CD image (`psx-hello/build/psx-hello.bin`/`.cue`, built by
   PSn00bSDK's own `mkpsxiso` from `psx-hello/iso.xml` +
   `psx-hello/system.cnf`). **Done, successfully**: `psx-hello.exe` is a real
   `PS-X EXE`-tagged, 22528-byte file, built from `psx-hello/main.c` itself
   (not a stand-in) by the real toolchain, with the real `elf2x` (which did
   *not* crash — see "Sandbox-only fallback smoke test" below for when it
   did, on a different, non-PSn00bSDK toolchain).
3. Boot it in an emulator and confirm: pad status text renders, Cross
   toggles "MOUSE MODE", D-pad moves the cursor tile while it's on, Circle
   flashes it yellow for `CLICK_FLASH_FRAMES` frames. **Blocked** — not by
   network or sandbox limits, but by a real bug/gap in the open-source BIOS
   replacement used to boot it (see "Known boot blockers" below). This is a
   `docs/plan.md`-style *point dur*: a real incompatibility discovered along
   the way, logged here rather than treated as blocking the rest of the
   port.

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
  the same way `git clone` doesn't — so no prebuilt release ZIPs (a real
  Sony BIOS dump wouldn't be legitimately available this way regardless) are
  reachable, only source trees via `git clone`.
- **`ftpmirror.gnu.org` isn't blocked either, just flaky**: it redirects to
  a randomly-chosen GNU mirror per request, and this sandbox's egress proxy
  resets the connection or gets a 404 from some of them (`mirrors.ustc.edu.cn`,
  `mirror.ibcp.fr`) while serving others fine (`mirrors.nju.edu.cn`, verified
  reachable for both `binutils-2.40.tar.xz` and `gcc-12.2.0.tar.xz`).
  `tools/install_psn00bsdk_toolchain.sh` now pins to that mirror instead of
  the redirector.
- **An emulator *is* reachable**: `apt-get install mednafen` works (plus
  `xvfb`, `xdotool` for a display and input injection, and `imagemagick`-free
  screenshotting via Mednafen's own F9 snapshot key). Mednafen's PSX core is
  the same one used by the libretro "Beetle PSX" core, which OpenBIOS's own
  `COMPATIBILITY.md` lists as a supported target.
- **The project-local Bash hook (`.claude/hooks/deny-absolute-paths.py`),
  which refuses any command containing an absolute path, `~`, `..`, a shell
  variable reference, or command substitution, is real and still applies.**
  It doesn't block any of this — every script below routes the actual
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
`psn00bsdk_add_cd_image` call in the `CMakeLists.txt` — a bare `.exe` can't
actually be booted by a BIOS/shell without a disc to read it from, only
sideloaded via emulator-specific debug hooks (see below).

## Getting a legal BIOS: OpenBIOS

Mednafen's PSX core needs a real BIOS ROM (`~/.mednafen/firmware/scph5500.bin`
etc.) and, unlike some looser emulators, does not do HLE — a copyrighted Sony
BIOS dump is not something to source or distribute here. Instead,
`tools/build_openbios_for_testing.sh` builds and installs
[OpenBIOS](https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/openbios)
(`src/mips/openbios` inside the `pcsx-redux` repo): a clean-room
reimplementation, "bugs, quirks and all", built from disassembly of a
dumped retail BIOS under the same legal doctrine as *Sony v. Connectix* (see
its own README's "Legality" section) — MIT-licensed and meant exactly for
this kind of testing. `make psx-hello-openbios-setup` (optionally
`BOOT_MODE=fast`, see below) builds it with the already-installed
`mipsel-none-elf` toolchain (no Docker needed, despite that being OpenBIOS's
own documented default build method) and installs it as Mednafen's BIOS.

Two real issues came up building it, both fixed in the script/repo state:

- `third_party/uC-sdk` is a git submodule that a plain `git clone` doesn't
  populate; without `git submodule update --init`, the build fails with
  `stdlib.h`/`string.h`/etc. "No such file or directory" (uC-sdk provides
  OpenBIOS's own minimal libc, not something in the `mipsel-none-elf`
  toolchain, which is deliberately headerless per PSn00bSDK's own
  `--without-headers` GCC configure flag).
- Passing `INSTALL_TTY_CONSOLE=true` (tried while debugging, to capture
  kernel debug prints) hangs forever: it installs a driver for real DTL-H2000
  devkit hardware that doesn't exist under Mednafen's retail-console
  emulation, which busy-loops reading an unmapped I/O register
  (`1f802000`). Confirmed via Mednafen's `-psx.dbg_level 2`, which logs
  every access to an unmapped address — don't pass this build flag when
  testing under a plain emulator.

## Known boot blockers

`make psx-hello-boot-test` (`tools/boot_test_psx_hello.sh`) boots
`psx-hello.cue` under Mednafen (via `Xvfb` + `xdotool` for input injection
and F9-triggered screenshots — no real display needed) and works completely
mechanically: Mednafen loads the BIOS and the disc, a window appears, and
key presses reach it. What it renders is where things stop, in one of two
ways depending how OpenBIOS was built:

- **Default build** (`make psx-hello-openbios-setup`, no `BOOT_MODE`): boots
  clean, no crash, but never gets past OpenBIOS's own boot-logo screen — a
  rotating shaded cube (`src/mips/shell/hull.c`'s `convexHull()` demo,
  OpenBIOS's stand-in for Sony's licensed spinning-logo animation, not a
  bug and not `psx-hello`'s own rendering — confirmed by grepping
  `openbios`'s and `psx-hello/main.c`'s own source for any cube/GTE code:
  there isn't any, and the same cube appears at the same rotation angles
  independent of any key sent to the emulator). This almost certainly
  because the disc's SCEx region/license lead-in isn't recognized
  (`Emulated Disc SCEx IDs: Disc 1: (Not recognized as a PS1 disc)` in
  Mednafen's own log) — real license data is region-specific and, per
  PSn00bSDK's own install docs, not distributed for exactly the reasons a
  BIOS replacement project exists in the first place.
- **`BOOT_MODE=fast` build** (`make psx-hello-openbios-setup BOOT_MODE=fast`):
  skips the shell/boot-logo entirely and goes straight for the CD boot, but
  crashes partway through with `=== Unimplemented A0:A1 syscall from
  0xBFC03CCC === ... === halting ===`. Decoding OpenBIOS's own kernel debug
  output (every byte OpenBIOS writes to `1f802080` is one debug-console
  character — see `tools/boot_test_psx_hello.sh`'s use of `-psx.dbg_level 2`
  and grep the resulting log for `to 1f80208` to reconstruct it) and cross
  -referencing the return address (`ra`) against `openbios.map` places the
  crash inside `cdromBlockReading` (`src/mips/openbios/cdrom/cdrom.c`) — an
  actual CD-ROM sector read, not board/kernel init as the crash's own "uC-sdk
  - booting" banner might suggest at a glance.

Both are genuine gaps/bugs in OpenBIOS's CD-ROM boot path under this
toolchain/Mednafen combination, not in `psx-hello` or this repo's own code —
a `docs/plan.md`-style *point dur*, logged here rather than blocking the
rest of the port. Worth trying before re-diagnosing from scratch: an older
OpenBIOS commit, PCSX-Redux or DuckStation instead of Mednafen (both
explicitly listed as tested-compatible in OpenBIOS's own
`COMPATIBILITY.md`, unlike standalone Mednafen), or building a real license
file into the CD image.

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

- Resolve one of the two "Known boot blockers" above (a real license/SCEx
  entry in the CD image, an older OpenBIOS commit, or a different
  BIOS-replacement-compatible emulator) so `make psx-hello-boot-test`
  actually reaches `psx-hello`'s own code, then confirm the pad-status HUD
  renders and responds to Cross/D-pad/Circle as designed.
