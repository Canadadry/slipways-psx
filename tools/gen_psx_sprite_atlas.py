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

/* The 3 sprites live-wired into game_draw.c (Implementation Chunks
 * chunks 1/4/5/6, docs/prd/triage/psx-sprites-audio-extraction.md). The
 * remaining traced-but-unwired roster (11 planet types, 4 mood icons, 8
 * structure icon variants, 2 unreachable cursor variants), added by
 * chunk 3 below, is real, tested data with zero live caller -- see that
 * roster's own doc comments for citations. */
#define SPRITE_PLANET_UNKNOWN 43           /* jo:kj's `self.s or 43` fallback, pico8/lua.lua:541-551 */
#define SPRITE_CURSOR_DEFAULT 58           /* ui's prototype default ql=58, pico8/lua.lua:213 */
#define SPRITE_LIAISON_INVALID_TARGET 31   /* dl()'s spr(31,...), pico8/lua.lua:564, reachable via jc:ev */

/* Chunk 3 (docs/prd/triage/psx-sprites-audio-extraction.md, Implementation
 * Chunks item 3): the full traced-but-currently-unreachable roster (11
 * planet types, 4 mood-status icons, 8 structure-purchase icon variants,
 * 2 unreachable cursor variants). Real, source-traced sprite ids, backed
 * by a real, tested atlas -- but with zero live caller in this port, the
 * same "traced but unwired" shape rules_planets.c's own 10-type roster
 * and rules_tech.c's own 19-tech table already have (Implementation
 * Decisions, "Real sprite inventory," items 1/2/6; Out of Scope). None of
 * these are read by game_draw.c or any rules_*.c file yet. */

/* 11 planet-type sprites -- df (pico8/lua.lua:1377-1389), the same
 * planet-type table docs/prd/psx-rules-planets.md already traced for
 * name/rarity but explicitly did NOT port the `s` (sprite) field
 * (Implementation Decisions item 1). Drawn via jo:kj's `spr(self.nk and
 * self.nk.s or self.s or 43,p.x-4,p.y-4,2,2)` (pico8/lua.lua:541-551,
 * hence always 2x2 cels) once a planet has a discovered/transformed
 * type -- not reachable in this port yet since no discovery-commit
 * mechanism exists (see SPRITE_PLANET_UNKNOWN above, which IS wired). */
#define SPRITE_PLANET_EMPTY 45              /* df.ll's s=45 (empty/unknown-roll sentinel), pico8/lua.lua:1377-1389 -- distinct from the `43` pre-discovery fallback above */
#define SPRITE_PLANET_EARTHLIKE 160         /* df.e's s=160 ("earth-like"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_FORGEWORLD 128        /* df.f's s=128 ("forgeworld"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_MINERAL 134           /* df.m's s=134 ("mineral"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_OCEAN 130             /* df.o's s=130 ("ocean"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_REMNANT 140           /* df.r's s=140 ("remnant"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_XENO 162              /* df.x's s=162 ("xeno"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_JUNGLE 136            /* df.j's s=136 ("jungle"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_ICEBALL 138           /* df.i's s=138 ("iceball"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_BARREN 142            /* df.s's s=142 ("barren"), pico8/lua.lua:1377-1389 */
#define SPRITE_PLANET_GASGIANT 96           /* df.g's s=96 ("gas giant"), pico8/lua.lua:1377-1389 */

/* 4 mood-status icons -- jo:kj's `spr(self.mi+1,p.x-3,p.y-6)`
 * (pico8/lua.lua:541-551), gated on self:gg() (a colonized-job flag).
 * self.mi indexes jo.dz's 4 mood entries 1-based, so self.mi+1 ranges
 * 2-5, in order unhappy -> content -> prosperous -> rich. Not reachable
 * in this port yet (same gap as the planet-type roster above -- no live
 * colonization-job state exists). */
#define SPRITE_MOOD_UNHAPPY 2                /* jo.dz[1] ("unhappy"), self.mi+1 where self.mi=1, pico8/lua.lua:541-551 */
#define SPRITE_MOOD_CONTENT 3                /* jo.dz[2] ("content"), self.mi+1 where self.mi=2, pico8/lua.lua:541-551 */
#define SPRITE_MOOD_PROSPEROUS 4             /* jo.dz[3] ("prosperous"), self.mi+1 where self.mi=3, pico8/lua.lua:541-551 */
#define SPRITE_MOOD_RICH 5                   /* jo.dz[4] ("rich"), self.mi+1 where self.mi=4, pico8/lua.lua:541-551 */

/* 8 structures' icon sprite ids -- ie:kj (pico8/lua.lua:936-939). Not one
 * of this step's six named draw-call placeholders (no structure-purchase
 * UI exists anywhere in game_draw.c, docs/prd/psx-structures-and-
 * transforms.md's own Out of Scope) -- captured as data since the ids
 * were sitting right there in the same spr()/sspr() trace (Implementation
 * Decisions item 6). Infragate is the one structure whose icon changes
 * with live connection count (0-3): its source table is
 * `hz=o(78,78,78,76)` (pico8/lua.lua:936-939) -- the first 3 variants
 * genuinely share sprite id 78, only the 4th differs to 76; ported
 * faithfully here as 4 distinct named constants, not collapsed/
 * deduplicated, even though 3 of the 4 values are identical. */
#define SPRITE_STRUCTURE_LABORATORY 174           /* Laboratory s=174, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_ASCENSION_GATE 166       /* Ascension Gate s=166, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_PROTOSTAR 98             /* Protostar s=98, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_SYNTHESIZER 66           /* Synthesizer s=66, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_TRADING_HUB 68           /* Trading Hub s=68, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_FOOD_PROCESSOR 100       /* Food processor s=100, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_SLIPGATE 170             /* Slipgate s=170, ie:kj, pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_INFRAGATE_0 78           /* Infragate hz[1]=78 (0 live connections), ie:kj's hz=o(78,78,78,76), pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_INFRAGATE_1 78           /* Infragate hz[2]=78 (1 live connection) -- same sprite id as INFRAGATE_0, ie:kj's hz=o(78,78,78,76), pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_INFRAGATE_2 78           /* Infragate hz[3]=78 (2 live connections) -- same sprite id as INFRAGATE_0/1, ie:kj's hz=o(78,78,78,76), pico8/lua.lua:936-939 */
#define SPRITE_STRUCTURE_INFRAGATE_3 76           /* Infragate hz[4]=76 (3 live connections) -- the one variant that actually differs, ie:kj's hz=o(78,78,78,76), pico8/lua.lua:936-939 */

/* 2 unreachable cursor variants -- ui:ev, pico8/lua.lua:225:
 * `spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql, ...)`.
 * Both are real sprite ids, both confirmed dead/unreachable in the
 * original source (Implementation Decisions item 2 / Further Notes) --
 * not wired to this port's own click/drag state, since doing so would
 * invent behavior the source itself never exercises. */
#define SPRITE_CURSOR_HELD 9                  /* the self.rheld branch -- self.rheld is never assigned anywhere in pico8/lua.lua (confirmed by exhaustive grep), so this is confirmed dead code in the original game, not a bug. ui:ev, pico8/lua.lua:225 */
#define SPRITE_CURSOR_HOVER_OVERRIDE 11       /* the self.o_.ql per-hovered-object override -- self.o_.ql has exactly one other assignment in the whole source (ql=11, pico8/lua.lua:1158), on an entirely unwired UI entity, so also unreachable in this port currently. ui:ev, pico8/lua.lua:225 */

/* Computes sprite id's pixel rectangle within the 128x128 atlas. PICO-8's
 * sprite sheet is a fixed 16x16 grid of 8x8-pixel cels, so a sprite id's
 * top-left cel is at cel_x = id % 16, cel_y = id / 16, giving pixel
 * coordinates out_x = cel_x*8, out_y = cel_y*8. cel_w/cel_h let a caller
 * request a multi-cel sprite (e.g. 2x2 for planets, jo:kj's
 * spr(...,2,2) call) -- pure, host-testable, no PSX dependency. */
void sprite_atlas_cel_rect(int sprite_id, int cel_w, int cel_h, int *out_x, int *out_y, int *out_w, int *out_h);

/* PSX-only (Implementation Chunks chunk 4, docs/prd/triage/
 * psx-sprites-audio-extraction.md): uploads g_sprite_atlas_4bpp/
 * g_pico8_clut to VRAM once at boot (LoadImage) and records the texpage/
 * CLUT values later draw calls need. Declared here unconditionally, like
 * sprite_atlas_cel_rect() above, so any PSX-side caller (game_update.c's
 * game_init()) sees one prototype regardless of build target -- but the
 * *implementation* (sprite_atlas.c's own #ifdef __mips__ block, this
 * template's C_TEMPLATE below) is psxgpu.h-dependent and compiled only
 * for the real PSn00bSDK cross build, not the host gcc test build
 * (test-sprite-atlas has no psxgpu.h on its include path and never calls
 * any of these three). */
void sprite_atlas_init(void);
uint16_t sprite_atlas_get_tpage(void);
uint16_t sprite_atlas_get_clut(void);

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

#ifdef __mips__
/* PSX-only half of this otherwise fully-generated file (Implementation
 * Chunks chunk 4, docs/prd/triage/psx-sprites-audio-extraction.md) -- the
 * first hand-written, psxgpu.h-dependent addition here. Lives in this
 * Python template (rather than a hand-maintained sibling .c file) because
 * `make sprite-atlas` fully regenerates sprite_atlas.c on every build
 * (see this file's own header comment above); a sibling file would never
 * be re-touched by that regeneration but could silently drift out of sync
 * with it instead.
 *
 * Guarded by __mips__, GCC's own standard predefined macro for any MIPS
 * target (confirmed defined by the installed mipsel-none-elf-gcc), so
 * this half compiles only for the real PSn00bSDK cross build and is
 * skipped entirely for the host gcc test build (test-sprite-atlas) --
 * which has no psxgpu.h on its include path (-I psx-slipways only, see
 * the Makefile's SPRITE_ATLAS_TEST_HOST_SRCS recipe) and never calls any
 * of these three functions. Mirrors world.c/screen_map.c's existing
 * pure/impure split for one concern, just enforced by a preprocessor
 * guard here instead of a second file, for the reason above. */
#include <psxgpu.h>

/* VRAM placement for the uploaded atlas texture + CLUT. main.c's
 * init_context() (SetDefDispEnv/SetDefDrawEnv) places this port's two
 * double-buffered 320x240 framebuffers at VRAM (0,0)-(320,240) and
 * (320,0)-(640,240) -- together occupying the full x:[0,640), y:[0,240)
 * rectangle. VRAM is 1024x512 total (the PS1 GPU's fixed framebuffer
 * size), so anything placed entirely at x >= 640 is safe regardless of y,
 * with 384 columns x 512 rows of headroom to spare -- far more than this
 * atlas (32 columns) or CLUT (16 columns) need. Placed at the left edge
 * of that safe region (x=640, y=0) purely for readability; no other
 * constraint picks a more specific offset. x=640 is also exactly
 * texpage-column-aligned (640 = 10*64, texpages are 64px wide in X, see
 * getTPage's own bit-packing below) and CLUT-column-aligned (640 = 40*16,
 * getClut's x is truncated to 16px units), so neither upload loses any
 * precision snapping to hardware granularity.
 *
 * SPRITE_ATLAS_VRAM_W is in 16-bit VRAM "pixel" units, the PS1 GPU's
 * native VRAM addressing granularity: a 4bpp texture packs 4 texels into
 * each 16-bit VRAM pixel, so the 128px-wide atlas occupies 128/4=32 VRAM
 * columns (matching g_sprite_atlas_4bpp's own 2-texels-per-byte packing,
 * sprite_atlas.h's own doc comment) by 128 rows -- entirely inside one
 * 64x256 texture page, so the texture never straddles a page boundary.
 * The CLUT (16 uint16_t BGR555 entries) occupies 16 VRAM columns by 1
 * row, placed directly below the atlas (y = SPRITE_ATLAS_VRAM_H) so the
 * two uploads can never overlap. */
#define SPRITE_ATLAS_VRAM_X 640
#define SPRITE_ATLAS_VRAM_Y 0
#define SPRITE_ATLAS_VRAM_W (128 / 4)
#define SPRITE_ATLAS_VRAM_H 128
#define SPRITE_ATLAS_CLUT_VRAM_X SPRITE_ATLAS_VRAM_X
#define SPRITE_ATLAS_CLUT_VRAM_Y (SPRITE_ATLAS_VRAM_Y + SPRITE_ATLAS_VRAM_H)

static uint16_t g_sprite_atlas_tpage;
static uint16_t g_sprite_atlas_clut_id;

void sprite_atlas_init(void) {
    RECT atlas_rect;
    setRECT(&atlas_rect, SPRITE_ATLAS_VRAM_X, SPRITE_ATLAS_VRAM_Y, SPRITE_ATLAS_VRAM_W, SPRITE_ATLAS_VRAM_H);
    /* g_sprite_atlas_4bpp is declared uint8_t[8192] (sprite_atlas.h) but
     * LoadImage's real signature (verified directly against the installed
     * psxgpu.h -- int LoadImage(const RECT *rect, const uint32_t *data) --
     * not assumed) takes const uint32_t*: the PS1 GPU's VRAM DMA transfers
     * data as 32-bit words. This relies on the compiler's default
     * alignment of a file-scope 8192-byte array (mipsel-none-elf-gcc
     * aligns data objects this size to at least a 4-byte boundary by
     * default); no explicit alignment attribute is added here since that
     * would mean hand-editing the generated array declaration itself,
     * out of scope for this chunk. */
    LoadImage(&atlas_rect, (const uint32_t *) g_sprite_atlas_4bpp);

    RECT clut_rect;
    setRECT(&clut_rect, SPRITE_ATLAS_CLUT_VRAM_X, SPRITE_ATLAS_CLUT_VRAM_Y, 16, 1);
    /* Same LoadImage entry point uploads the CLUT: PS1 CLUTs are just an
     * ordinary small VRAM pixel region (16 entries x 1 row here), not a
     * separate upload mechanism -- confirmed against the header, which
     * declares no CLUT-specific transfer function. */
    LoadImage(&clut_rect, (const uint32_t *) g_pico8_clut);

    /* LoadImage's transfer is GPU-DMA-driven; DrawSync(0) blocks until it
     * (and anything queued before it) finishes, so no draw call can ever
     * race ahead of the upload -- called once here, at boot, before any
     * primitive referencing this texture/CLUT is ever drawn. */
    DrawSync(0);

    /* getTPage(tp, abr, x, y) (psxgpu.h's own helper, used here instead of
     * hand-rolling the bit-packing): tp=0 selects the GPU's 4bpp CLUT
     * texture page color mode -- the atlas's real format, per
     * g_sprite_atlas_4bpp's own doc comment; the PS1 GPU's 3 tp values are
     * 0=4bit CLUT, 1=8bit CLUT, 2=15bit direct. abr=0: no semi-
     * transparency blending (none of this chunk's sprites need it). x/y
     * are this texture's raw VRAM pixel coordinates -- getTPage's own
     * bit-packing (verified against the installed header) masks these to
     * page granularity itself, so the raw VRAM x/y is passed directly,
     * not pre-divided. getClut(x, y) is the CLUT-upload's equivalent
     * helper. */
    g_sprite_atlas_tpage = getTPage(0, 0, SPRITE_ATLAS_VRAM_X, SPRITE_ATLAS_VRAM_Y);
    g_sprite_atlas_clut_id = getClut(SPRITE_ATLAS_CLUT_VRAM_X, SPRITE_ATLAS_CLUT_VRAM_Y);
}

uint16_t sprite_atlas_get_tpage(void) {
    return g_sprite_atlas_tpage;
}

uint16_t sprite_atlas_get_clut(void) {
    return g_sprite_atlas_clut_id;
}
#endif
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
