"""Pack pico8/gfx.json into a PS1-native 4bpp CLUT sprite atlas.

Reads pico8/gfx.json directly (no PNG decoding -- gfx.json is already
fully-decoded, structured pixel-index data; see docs/prd/triage/
psx-sprites-audio-extraction.md, Implementation Decisions). Emits
psx-slipways/sprite_atlas.h/.c:

  - g_sprite_atlas_4bpp[128*128/2]: the full 128x128 sprite sheet packed
    2 pixels/byte, row-major, matching the PS1 GPU's native 4bpp CLUT
    texture format.
  - g_pico8_clut[16]: PICO-8's real, fixed default system palette,
    converted to PS1 16-bit BGR555, with index 0 forced to raw 0x0000
    (PICO-8's own default sprite-transparency convention, reproduced by
    the PS1 GPU's own "raw 0x0000 texel is transparent" hardware rule).
  - The 3 named SPRITE_* constants this chunk wires, and
    sprite_atlas_cel_rect(), a pure/host-testable helper that computes a
    sprite id's pixel rectangle in the atlas.

This is this codebase's first "tool reads source data, emits a compiled-in
C data module" case for a texture/CLUT asset (see tools/ttf_to_pixelfont.py
for the closest existing precedent -- it bakes a font into a PNG for
LOVE2D, not a C header/source pair, so the output shape here is new, not
copied). Mirrors tools/gen_pcsx_*.py's convention of writing plain,
readable Python with no third-party dependencies (gfx.json is read with
the standard library's json module only -- no PIL, unlike
ttf_to_pixelfont.py, since there is no image to rasterize here).
"""
import argparse
import json

ATLAS_WIDTH = 128
ATLAS_HEIGHT = 128
CELS_PER_ROW = 16
CEL_SIZE = 8

PICO8_PALETTE_HEX = [
    "000000",
    "1D2B53",
    "7E2553",
    "008751",
    "AB5236",
    "5F574F",
    "C2C3C7",
    "FFF1E8",
    "FF004D",
    "FFA300",
    "FFEC27",
    "00E436",
    "29ADFF",
    "83769C",
    "FF77A8",
    "FFCCAA",
]


def load_gfx_json(path):
    with open(path) as f:
        data = json.load(f)

    width = data["width"]
    height = data["height"]
    pixels = data["pixels"]

    if width != ATLAS_WIDTH or height != ATLAS_HEIGHT:
        raise ValueError(
            "%s: expected a %dx%d sprite sheet, got %dx%d"
            % (path, ATLAS_WIDTH, ATLAS_HEIGHT, width, height)
        )
    if len(pixels) != width * height:
        raise ValueError(
            "%s: expected %d pixels (width*height), got %d"
            % (path, width * height, len(pixels))
        )
    for v in pixels:
        if not isinstance(v, int) or v < 0 or v > 15:
            raise ValueError("%s: pixel value %r out of range [0,15]" % (path, v))

    return pixels


def pack_4bpp(pixels):
    """Packs a row-major, one-index-per-pixel 128x128 array into the PS1
    GPU's native 4bpp CLUT texture format: 2 pixels/byte, row-major.

    Nibble order: this codebase has zero existing CLUT/texture precedent
    to match (confirmed by grep -- see the PRD's own "Extraction pipeline"
    section), so this is a genuine first-use decision, made here and
    documented in the generated header: for each horizontally-adjacent
    pixel pair (px_even at x, px_odd at x+1), the packed byte is
    `(px_odd << 4) | px_even` -- the low nibble holds the lower-x/first
    pixel of the pair, the high nibble holds the higher-x/second pixel.
    This is the standard convention for packed low-to-high nibble order
    matching increasing x, and is exactly what host_tests/
    sprite_atlas_test.c's unpack_4bpp_pixel() helper un-packs with.
    """
    packed = bytearray(ATLAS_WIDTH * ATLAS_HEIGHT // 2)
    for y in range(ATLAS_HEIGHT):
        row_base = y * ATLAS_WIDTH
        out_base = y * (ATLAS_WIDTH // 2)
        for x in range(0, ATLAS_WIDTH, 2):
            px_even = pixels[row_base + x]
            px_odd = pixels[row_base + x + 1]
            packed[out_base + x // 2] = (px_odd << 4) | px_even
    return bytes(packed)


def rgb_to_bgr555(hex_rgb):
    """Converts an 8-bit-per-channel RRGGBB hex string to PS1 16-bit
    BGR555 (bit layout bbbbbgggggrrrrr, 5 bits each of blue/green/red,
    blue in the high bits -- the standard PS1 GPU CLUT color format).
    Each 8-bit channel is reduced to 5 bits by the standard right-shift-
    by-3 truncation (no rounding), matching the standard convention this
    codebase has no prior color-conversion precedent to instead match
    (confirmed by grep -- see the PRD's Implementation Decisions,
    "Extraction pipeline and output format")."""
    r8 = int(hex_rgb[0:2], 16)
    g8 = int(hex_rgb[2:4], 16)
    b8 = int(hex_rgb[4:6], 16)
    r5 = r8 >> 3
    g5 = g8 >> 3
    b5 = b8 >> 3
    return (b5 << 10) | (g5 << 5) | r5


def build_clut():
    clut = [rgb_to_bgr555(h) for h in PICO8_PALETTE_HEX]
    clut[0] = 0x0000
    return clut


def format_bytes_c_array(data, indent="    "):
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        lines.append(indent + ", ".join("0x%02x" % b for b in chunk) + ",")
    return "\n".join(lines)


def format_uint16_c_array(values, indent="    "):
    lines = []
    for i in range(0, len(values), 8):
        chunk = values[i:i + 8]
        lines.append(indent + ", ".join("0x%04x" % v for v in chunk) + ",")
    return "\n".join(lines)


HEADER_TEMPLATE = """#ifndef SPRITE_ATLAS_H
#define SPRITE_ATLAS_H

#include <stdint.h>

/* GENERATED FILE -- do not edit by hand. Produced by
 * tools/gen_psx_sprite_atlas.py from pico8/gfx.json. Re-run `make
 * sprite-atlas` (or `make test-sprite-atlas`/`make psx-slipways`, which
 * both depend on it) to regenerate. See docs/prd/triage/
 * psx-sprites-audio-extraction.md, "Extraction pipeline and output
 * format" / Implementation Chunks chunk 1. */

/* The full 128x128 PICO-8 sprite sheet (pico8/gfx.json), packed 2
 * pixels/byte, row-major, matching the PS1 GPU's native 4bpp CLUT texture
 * format: 128*128/2 = 8192 bytes.
 *
 * Nibble order (a genuine first-use decision -- this codebase had zero
 * existing CLUT/texture precedent to instead match, confirmed by grep):
 * for each horizontally-adjacent pixel pair (px_even at x, px_odd at
 * x+1), the packed byte is (px_odd << 4) | px_even -- i.e. the low nibble
 * holds the lower-x/first/"even" pixel of the pair, the high nibble holds
 * the higher-x/second/"odd" pixel. See gen_psx_sprite_atlas.py's
 * pack_4bpp() for the packing logic and host_tests/sprite_atlas_test.c's
 * unpack_4bpp_pixel() for the matching unpack logic this is proven
 * round-trip-correct against.
 *
 * Declared here (extern, non-static) rather than this codebase's usual
 * "static const table + accessor function" convention (see
 * rules_tech.c/rules_planets.c) because both this chunk's host test and a
 * later chunk's sprite_atlas_init() (LoadImage/VRAM upload) need direct
 * read access to the raw packed blob itself, not just individual decoded
 * values. */
extern const uint8_t g_sprite_atlas_4bpp[128 * 128 / 2];

/* PICO-8's fixed, well-known 16-color default system palette (see
 * gen_psx_sprite_atlas.py's PICO8_PALETTE_HEX for the source hex values
 * and citations) converted to PS1 16-bit BGR555 (bit layout
 * bbbbbgggggrrrrr, 5 bits each of blue/green/red, blue in the high bits
 * -- the standard PS1 GPU CLUT color format; each 8-bit channel reduced
 * to 5 bits by truncating right-shift-by-3, the standard convention,
 * since this codebase has no prior color-conversion precedent to instead
 * match). This is a hardware/system constant, not a
 * slipways-mechanics.md-traceable game-design value (mirroring
 * docs/prd/psx-galaxy-rendering.md's own precedent for labeling
 * rendering/benchmark constants as such).
 *
 * Index 0 is deliberately forced to raw 0x0000 regardless of what PICO-8
 * color 0 (black, RGB (0,0,0), which also converts to 0x0000 here, so
 * this is a deliberate encoding of intent, not an incidental match) would
 * otherwise convert to: on real PS1 GPU hardware, a texel/CLUT color of
 * exactly raw 0x0000 is drawn fully transparent, reproducing PICO-8's own
 * default "sprite color 0 is transparent" convention (jo:kj's
 * palt(0,true), ui:ev's/dl()'s/lq:mn()'s all-implicit reliance on that
 * same default) with zero extra blend-mode code. See the PRD's
 * "Transparency" section. */
extern const uint16_t g_pico8_clut[16];

/* Sprites this chunk wires (Implementation Chunks chunk 1,
 * docs/prd/triage/psx-sprites-audio-extraction.md). The remaining
 * traced-but-unwired roster (11 planet types, 4 mood icons, 8 structure
 * icons, 2 unreachable cursor variants) is chunk 3's job, not this
 * chunk's. */
#define SPRITE_PLANET_UNKNOWN 43           /* jo:kj's `self.s or 43` fallback, pico8/lua.lua:541-551 */
#define SPRITE_CURSOR_DEFAULT 58           /* ui's prototype default ql=58, pico8/lua.lua:213 */
#define SPRITE_LIAISON_INVALID_TARGET 31   /* dl()'s spr(31,...), pico8/lua.lua:564, reachable via jc:ev */

/* Computes sprite id's pixel rectangle within the 128x128 atlas. PICO-8's
 * sprite sheet is a fixed 16x16 grid of 8x8-pixel cels, so a sprite id's
 * top-left cel is at cel_x = id % 16, cel_y = id / 16, giving pixel
 * coordinates out_x = cel_x*8, out_y = cel_y*8. cel_w/cel_h let a caller
 * request a multi-cel sprite (e.g. 2x2 for planets, jo:kj's
 * spr(...,2,2) call) -- pure, host-testable, no PSX dependency. */
void sprite_atlas_cel_rect(int sprite_id, int cel_w, int cel_h, int *out_x, int *out_y, int *out_w, int *out_h);

#endif
"""

C_TEMPLATE = """#include "sprite_atlas.h"

/* GENERATED FILE -- do not edit by hand. Produced by
 * tools/gen_psx_sprite_atlas.py from pico8/gfx.json. See sprite_atlas.h
 * for the packing/CLUT-conversion decisions this data encodes. */

const uint8_t g_sprite_atlas_4bpp[128 * 128 / 2] = {
%(atlas_bytes)s
};

const uint16_t g_pico8_clut[16] = {
%(clut_values)s
};

/* Pure, host-testable: PICO-8's sprite sheet is a fixed 16x16 grid of
 * 8x8-pixel cels (see sprite_atlas.h's own doc comment for the formula).
 * No PSX dependency -- unlike a texture-upload function (which does not
 * exist yet in this chunk), this needs no psxgpu.h at all. */
void sprite_atlas_cel_rect(int sprite_id, int cel_w, int cel_h, int *out_x, int *out_y, int *out_w, int *out_h) {
    int cel_x = sprite_id %% 16;
    int cel_y = sprite_id / 16;

    *out_x = cel_x * 8;
    *out_y = cel_y * 8;
    *out_w = cel_w * 8;
    *out_h = cel_h * 8;
}
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "gfx_json", nargs="?", default="pico8/gfx.json",
        help="Path to pico8/gfx.json (default: pico8/gfx.json)",
    )
    parser.add_argument(
        "--out-header", default="psx-slipways/sprite_atlas.h",
        help="Output header path (default: psx-slipways/sprite_atlas.h)",
    )
    parser.add_argument(
        "--out-c", default="psx-slipways/sprite_atlas.c",
        help="Output C source path (default: psx-slipways/sprite_atlas.c)",
    )
    args = parser.parse_args()

    pixels = load_gfx_json(args.gfx_json)
    atlas_bytes = pack_4bpp(pixels)
    clut = build_clut()

    with open(args.out_header, "w") as f:
        f.write(HEADER_TEMPLATE)

    with open(args.out_c, "w") as f:
        f.write(C_TEMPLATE % {
            "atlas_bytes": format_bytes_c_array(atlas_bytes),
            "clut_values": format_uint16_c_array(clut),
        })

    print("Wrote %s (%d bytes atlas) and %s" % (args.out_header, len(atlas_bytes), args.out_c))


if __name__ == "__main__":
    main()
