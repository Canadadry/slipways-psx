"""Write a minimal 2048-byte PS-X EXE header around a raw code blob.

This is the by-hand workaround from docs/psx-hello-world.md, turned into a
script: PSn00bSDK's own ELF->PS-EXE converter (elf2x) crashes on ELFs
produced by a modern binutils linker (see that doc for the root cause), so
this fills in only the header fields an emulator/BIOS loader actually reads
(pc0, t_addr, t_size, sp_addr) and leaves the rest (gp0, data/bss, reserved)
zeroed.
"""
import argparse
import struct

HEADER_SIZE = 2048


def build_header(pc0, t_addr, t_size, sp_addr):
    header = bytearray(HEADER_SIZE)
    header[0:8] = b"PS-X EXE"
    struct.pack_into("<I", header, 0x10, pc0)
    struct.pack_into("<I", header, 0x18, t_addr)
    struct.pack_into("<I", header, 0x1C, t_size)
    struct.pack_into("<I", header, 0x30, sp_addr)
    return bytes(header)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pc0", required=True, type=lambda s: int(s, 0))
    parser.add_argument("--t-addr", required=True, type=lambda s: int(s, 0))
    parser.add_argument("--t-size", required=True, type=lambda s: int(s, 0))
    parser.add_argument("--sp-addr", default="0x801ffff0", type=lambda s: int(s, 0))
    parser.add_argument("-o", "--output", required=True)
    parser.add_argument("code", help="raw code blob (objcopy -O binary output)")
    args = parser.parse_args()

    with open(args.code, "rb") as f:
        code = f.read()

    padded_size = (len(code) + HEADER_SIZE - 1) // HEADER_SIZE * HEADER_SIZE
    code = code.ljust(padded_size, b"\0")

    header = build_header(args.pc0, args.t_addr, args.t_size, args.sp_addr)
    with open(args.output, "wb") as f:
        f.write(header)
        f.write(code)


if __name__ == "__main__":
    main()
