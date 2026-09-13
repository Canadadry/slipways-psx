# psx-hello: toolchain smoke test

`psx-hello/` is a minimal PSn00bSDK project exercising the concrete building
blocks the PSX port needs before any game logic: double-buffered GPU setup,
the BIOS pad driver, and the v1 controller mapping from `docs/plan.md` Phase 2
(Cross toggles mouse mode, D-pad moves a pixel cursor while it's on, Circle
simulates a click).

## Building it (on a machine with normal network access)

1. `sh tools/install_psn00bsdk_toolchain.sh` — builds the real `mipsel-none-elf`
   GCC 12.2.0 + binutils 2.40 toolchain from source per
   `doc/toolchain.md` in the PSn00bSDK repo, then builds and installs
   PSn00bSDK itself. Needs unrestricted outbound access (downloads from
   `ftpmirror.gnu.org` and `github.com`) and roughly half an hour of CPU time
   for the GCC build alone — the script wraps every `make`/`cmake --build`
   step in `timeout` (45 min each) because a GCC/binutils bootstrap
   occasionally hangs instead of failing outright, and a timed-out build is a
   clear signal to go look rather than a process left spinning unattended.
2. `make psx-hello` (or `cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build`)
   — produces `psx-hello/build/psx-hello.exe`.
3. Boot it in an emulator with debugging capability (PCSX-Redux, DuckStation,
   no$psx — the PSn00bSDK docs explicitly warn off ePSXe and MAME-based
   emulators as inaccurate) and confirm: pad status text renders, Cross
   toggles "MOUSE MODE", D-pad moves the cursor tile while it's on, Circle
   flashes it yellow for `CLICK_FLASH_FRAMES` frames.

None of this (steps 1-3) has actually been run — see below for what was and
wasn't possible to verify from inside the sandboxed session that wrote this.

## What was actually verified in this sandbox, and why the rest wasn't

The session that wrote this had two hard constraints worth recording before
anyone re-derives them the hard way:

- **Network egress policy blocks the real toolchain and any emulator.**
  `github.com/*/releases` (prebuilt PSn00bSDK packages), `ftpmirror.gnu.org`
  and `sourceware.org` (GCC/binutils source, needed to build the toolchain
  from scratch) all get a 403/connect-rejected from the sandbox's egress
  proxy. Only anonymous `git clone` of public GitHub repos was available.
  So: no real `mipsel-none-elf` toolchain, no PCSX-Redux/emulator binary, no
  way to boot-test anything, inside this sandbox.
- **A project-local Bash hook (`.claude/hooks/deny-absolute-paths.py`)
  refuses any command containing an absolute path, `~`, `..`, a shell
  variable reference, or command substitution.** This ruled out the usual
  PATH/toolchain-prefix tricks for wiring a substitute compiler into
  PSn00bSDK's CMake `find_program()`-based toolchain discovery
  (`libpsn00b/cmake/sdk.cmake`), which is why the smoke test below bypasses
  CMake entirely rather than fighting that discovery logic.

Given that, `git clone`s of `Lameguy64/PSn00bSDK` and `grumpycoders/pcsx-redux`
(source only, read-only) were used to inspect the real build system and copy
the exact compiler flags PSn00bSDK's own toolchain file uses
(`libpsn00b/cmake/flags.cmake`):

```
-ffreestanding -fno-builtin -nostdlib -fdata-sections -ffunction-sections
-fsigned-char -fno-strict-overflow -msoft-float -march=r3000 -mtune=r3000
-mabi=32 -mno-mt -mno-llsc -G0 -fno-pic -mno-abicalls -mno-gpopt
```

These are plain GCC MIPS backend flags, not something specific to a
`mipsel-none-elf`-targeted GCC build. So instead of building the real
toolchain, the sandbox's already-installed `apt` cross compiler
(`gcc-12-mipsel-linux-gnu`, target `mipsel-linux-gnu`, **not** the same
target triple PSn00bSDK expects) was used standalone, outside CMake, to prove
the compile → link → PS-EXE pipeline works at all in this environment:

1. Compiled a tiny freestanding `.c` file (an infinite loop writing to a
   fake GPU status register) with the flags above.
   `mipsel-linux-gnu-readelf -h` on the resulting object confirmed
   `Machine: MIPS R3000`, `mips1`, `o32` — i.e. `-march=r3000` was honored.
2. Linked it with a hand-written linker script placing `.text` at
   `0x80010000` (the standard PS1 RAM load address) and `_boot` as the entry
   point. The linked ELF's entry point came out as `0x80010000` as expected.
3. PSn00bSDK's own ELF→PS-EXE converter (`tools/util/elf2x.c`, a small
   standalone host program, no cross-compilation needed) **crashed** with a
   glibc stack-protector abort on this ELF. AddressSanitizer traced it to an
   out-of-bounds `fread()` at `elf2x.c:194`: the tool computes the output
   buffer size from only the highest segment's virtual address and the
   *last* program header's file size, which doesn't hold for the segment
   layout this modern `binutils 2.42` linker produced (worth flagging
   upstream if this recurs with the real toolchain — the tool dates from
   2018-2021). Worked around it for this smoke test by extracting the raw
   bytes with `objcopy -O binary` and writing the 2048-byte PS-EXE header by
   hand (`PS-X EXE` magic + `pc0`/`t_addr`/`t_size`/`sp_addr`, rest zeroed)
   instead of depending on `elf2x`.
4. End result: a structurally valid 4096-byte `.EXE` file (2048-byte header +
   2048-byte padded code) with the right magic, entry point, text address/size
   and a real compiled MIPS I prologue (`addiu $sp,$sp,-16` etc.) visible at
   the code offset.

**What this proves**: a MIPS backend with PS1-shaped flags can produce
correct R3000 code and a correctly-shaped PS-EXE container, in principle.
**What it does not prove**: that the real `mipsel-none-elf` toolchain
behaves identically (it should — same GCC flags, same instruction set — but
this wasn't the toolchain PSn00bSDK actually ships or tests against), or
that anything here actually boots (no emulator was available to try). The
`psx-hello/` project committed alongside this doc uses the real PSn00bSDK
API (`psxgpu.h`/`psxpad.h`/`InitPAD`/`StartPAD`, function signatures checked
against the cloned `libpsn00b/include` headers) and the real CMake build —
it was written for and should be built with the real toolchain from
`tools/install_psn00bsdk_toolchain.sh`, not the sandbox workaround above.

## Next steps (outside this sandbox)

- Run `tools/install_psn00bsdk_toolchain.sh` on a machine with normal network
  access and watch the two `timeout`-wrapped builds — report back if either
  one actually times out rather than completing (that's the "runaway build"
  failure mode to watch for).
- Build and boot `psx-hello` in PCSX-Redux (see `docs/plan.md` for the CLI
  flags — `--no-ui --loadexe psx-hello/build/psx-hello.exe --run
  --test-mode`) to close the loop this doc couldn't: does it actually render
  and respond to a pad?
- If `elf2x` still crashes with the real toolchain's output, that's worth
  filing upstream; the workaround in this doc (`objcopy -O binary` + a
  hand-written header) is a viable permanent substitute if not.
