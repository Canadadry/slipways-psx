"""Convert a tools/pcsx_screenshot_driver.lua raw capture into a real PNG.

Raw capture format (little-endian):
    u32 width
    u32 height
    u8  bpp (0 = 16bpp PSX-native VRAM, 1 = 24bpp truecolor)
    <width*height*(2 or 3)> bytes of pixel data, row-major, no padding.

Pure stdlib (struct + zlib) -- no Pillow/ImageMagick dependency, since this
needs to run in this repo's minimal environment.
"""
import struct
import sys
import zlib


def _unpack_16bpp(raw, width, height):
    """PSX native VRAM format: 0BBBBBGGGGGRRRRR, little-endian u16 per pixel."""
    out = bytearray(width * height * 3)
    for i in range(width * height):
        v = raw[2 * i] | (raw[2 * i + 1] << 8)
        r5 = v & 0x1F
        g5 = (v >> 5) & 0x1F
        b5 = (v >> 10) & 0x1F
        out[3 * i] = (r5 * 255 + 15) // 31
        out[3 * i + 1] = (g5 * 255 + 15) // 31
        out[3 * i + 2] = (b5 * 255 + 15) // 31
    return bytes(out)


def _write_png(path, width, height, rgb):
    def chunk(tag, data):
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)

    stride = width * 3
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        raw += rgb[y * stride:(y + 1) * stride]
    compressed = zlib.compress(bytes(raw), 9)

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", compressed))
        f.write(chunk(b"IEND", b""))


def convert(raw_path, png_path):
    with open(raw_path, "rb") as f:
        data = f.read()

    width, height, bpp = struct.unpack_from("<IIB", data, 0)
    pixels = data[9:]

    if bpp == 0:
        rgb = _unpack_16bpp(pixels, width, height)
    elif bpp == 1:
        expected = width * height * 3
        if len(pixels) < expected:
            raise ValueError("24bpp capture too short: got %d bytes, expected %d" % (len(pixels), expected))
        rgb = pixels[:expected]
    else:
        raise ValueError("unknown bpp %d in raw capture" % bpp)

    _write_png(png_path, width, height, rgb)


def main():
    if len(sys.argv) != 3:
        print("usage: pcsx_raw_to_png.py <capture.raw> <out.png>", file=sys.stderr)
        return 1
    convert(sys.argv[1], sys.argv[2])
    return 0


if __name__ == "__main__":
    sys.exit(main())
