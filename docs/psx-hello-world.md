# psx-hello: toolchain smoke test

`psx-hello/` is a minimal PSn00bSDK project exercising the concrete building
blocks the PSX port needs before any game logic: double-buffered GPU setup,
the BIOS pad driver, and a first controller mapping that emulates the PICO-8
mouse (Cross toggles mouse mode, D-pad moves a pixel cursor while it's on,
Circle simulates a click).

None of the steps below has been run yet: the cloud sandbox blocks the GNU
mirrors and the emulators, so neither the toolchain nor a boot test was
possible there.

## Building it (on a machine with normal network access)

1. `make psn00bsdk-toolchain-setup` (`tools/install_psn00bsdk_toolchain.sh`)
   — builds the `mipsel-none-elf` GCC 12.2.0 + binutils 2.40 toolchain from
   source per `doc/toolchain.md` in the PSn00bSDK repo, then PSn00bSDK
   itself. Needs outbound access to `ftpmirror.gnu.org` and `github.com`,
   and about half an hour of CPU for GCC. Every build step is wrapped in a
   45-minute `timeout`: a GCC/binutils bootstrap sometimes hangs instead of
   failing, and a timeout is the signal to go look.
2. `make psx-hello` — produces `psx-hello/build/psx-hello.exe`.
3. Boot it in an emulator with debugging capability (PCSX-Redux,
   DuckStation, no$psx — the PSn00bSDK docs warn off ePSXe and MAME-based
   emulators as inaccurate), e.g. `pcsx-redux --no-ui --loadexe
   psx-hello/build/psx-hello.exe --run --test-mode`, and confirm: pad status
   text renders, Cross toggles "MOUSE MODE", D-pad moves the cursor tile
   while it's on, Circle flashes it yellow for `CLICK_FLASH_FRAMES` frames.

## Known trap: `elf2x`

PSn00bSDK's ELF→PS-EXE converter (`tools/util/elf2x.c`) crashed on an ELF
linked by binutils 2.42: it sizes its buffer from the highest segment's
address and the *last* program header's file size, an out-of-bounds
`fread()` at `elf2x.c:194`. If it recurs with the real toolchain, a
permanent substitute is `objcopy -O binary` plus a hand-written 2048-byte
PS-EXE header (`PS-X EXE` magic, `pc0`, `t_addr`, `t_size`, `sp_addr`, the
rest zeroed).
