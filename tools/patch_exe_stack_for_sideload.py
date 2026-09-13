"""Patch a PSn00bSDK PS-EXE's stack fields (s_addr/s_size, offsets 0x30/0x34)
for Mednafen's -psx.dbg_exe_cdpath sideload hook. PSn00bSDK leaves those
header fields zeroed (it expects SYSTEM.CNF's STACK= to set the stack via a
normal BIOS boot); Mednafen's sideload path reads them directly from the
header instead, so a zeroed exe crashes with SP=0. See
docs/psx-hello-world.md for how this fits into the boot-test flow."""
import argparse
import struct


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input")
    parser.add_argument("-o", "--output", required=True)
    parser.add_argument("--stack-addr", default="0x801ffff0", type=lambda s: int(s, 0))
    args = parser.parse_args()

    with open(args.input, "rb") as f:
        data = bytearray(f.read())

    struct.pack_into("<I", data, 0x30, args.stack_addr)
    struct.pack_into("<I", data, 0x34, 0)

    with open(args.output, "wb") as f:
        f.write(data)


if __name__ == "__main__":
    main()
