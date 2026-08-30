"""Bake a TTF/OTF into a LÖVE2D ImageFont sprite sheet (see love2d.org/wiki/ImageFontFormat)."""
import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

DEFAULT_CHARS = (
    " !\"#$%&'()*+,-./0123456789:;<=>?@"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    "abcdefghijklmnopqrstuvwxyz{|}~"
)


def hex_to_rgb(h):
    h = h.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))


def dedupe_preserve_order(s):
    seen = set()
    out = []
    for c in s:
        if c not in seen:
            seen.add(c)
            out.append(c)
    return out


def composite_over_bg(rgba_img, bg_rgb, threshold):
    if threshold is not None:
        r, g, b, a = rgba_img.split()
        a = a.point(lambda v: 255 if v >= threshold else 0)
        rgba_img = Image.merge("RGBA", (r, g, b, a))
    out = Image.new("RGB", rgba_img.size, bg_rgb)
    out.paste(rgba_img, (0, 0), rgba_img)
    return out


def trim_bg_columns(img, bg_rgb):
    w, h = img.size
    bg_row = [bg_rgb] * h
    left = 0
    while left < w and [img.getpixel((left, y)) for y in range(h)] == bg_row:
        left += 1
    right = w
    while right > left and [img.getpixel((right - 1, y)) for y in range(h)] == bg_row:
        right -= 1
    if left >= right:
        return img
    return img.crop((left, 0, right, h))


def render_glyph(draw_probe, font, char, canvas_h, color, bg_rgb, threshold, spacing):
    left, top, right, bottom = draw_probe.textbbox((0, 0), char, font=font)
    ink_w = right - left
    if char == " " or ink_w <= 0:
        adv = draw_probe.textlength(char, font=font)
        w = max(1, round(adv))
        return Image.new("RGB", (w, canvas_h), bg_rgb)

    glyph = Image.new("RGBA", (ink_w, canvas_h), (0, 0, 0, 0))
    ImageDraw.Draw(glyph).text((-left, 0), char, font=font, fill=color)
    glyph = composite_over_bg(glyph, bg_rgb, threshold)
    glyph = trim_bg_columns(glyph, bg_rgb)

    spaced = Image.new("RGB", (glyph.width + spacing, canvas_h), bg_rgb)
    spaced.paste(glyph, (0, 0))
    return spaced


def build_atlas(font_path, size, chars, color, bg_rgb, sep_color, padding, threshold, spacing):
    font = ImageFont.truetype(font_path, size)
    ascent, descent = font.getmetrics()
    canvas_h = ascent + descent

    probe = ImageDraw.Draw(Image.new("RGBA", (1, 1)))
    glyph_list = dedupe_preserve_order(chars)
    glyphs = [
        (c, render_glyph(probe, font, c, canvas_h, color, bg_rgb, threshold, spacing))
        for c in glyph_list
    ]

    total_w = padding + sum(g.width + padding for _, g in glyphs)
    atlas = Image.new("RGB", (total_w, canvas_h), bg_rgb)
    draw = ImageDraw.Draw(atlas)

    x = 0
    draw.rectangle([x, 0, x + padding - 1, canvas_h - 1], fill=sep_color)
    x += padding
    for _, glyph in glyphs:
        atlas.paste(glyph, (x, 0))
        x += glyph.width
        draw.rectangle([x, 0, x + padding - 1, canvas_h - 1], fill=sep_color)
        x += padding

    return atlas, glyph_list


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("font", help="Path to a .ttf/.otf file")
    parser.add_argument("height", type=int, help="Font pixel size passed to the rasterizer")
    parser.add_argument(
        "text", nargs="?", default=DEFAULT_CHARS,
        help="Characters to bake, in order (default: printable ASCII). Duplicates are ignored.",
    )
    parser.add_argument("-o", "--out", default="font.png", help="Output PNG path (default: font.png)")
    parser.add_argument("--color", default="ffffff", help="Glyph color, hex RRGGBB (default: ffffff)")
    parser.add_argument("--bg", default="000000", help="Opaque background color, hex RRGGBB (default: 000000)")
    parser.add_argument(
        "--separator", default="be2633",
        help="Separator column color, hex RRGGBB (default: be2633, same as love2d.org's own example). "
             "Must not appear in --color or --bg.",
    )
    parser.add_argument("--padding", type=int, default=1, help="Separator column width, in px (default: 1)")
    parser.add_argument(
        "--spacing", type=int, default=1,
        help="Blank columns kept after each glyph's ink, so love.graphics.print doesn't glue letters together (default: 1)",
    )
    parser.add_argument(
        "--threshold", type=int, default=128,
        help="Alpha cutoff 0-255 for crisp pixel edges; pass -1 to keep antialiasing (default: 128)",
    )
    parser.add_argument("--scale", type=int, default=1, help="Nearest-neighbor upscale factor after baking")
    args = parser.parse_args()

    color = hex_to_rgb(args.color)
    bg_rgb = hex_to_rgb(args.bg)
    sep_color = hex_to_rgb(args.separator)
    threshold = None if args.threshold < 0 else args.threshold

    atlas, glyph_list = build_atlas(
        args.font, args.height, args.text, color, bg_rgb, sep_color, args.padding, threshold, args.spacing,
    )
    if args.scale > 1:
        atlas = atlas.resize((atlas.width * args.scale, atlas.height * args.scale), Image.NEAREST)

    out_path = Path(args.out)
    atlas.save(out_path)
    print(f"Wrote {out_path} ({atlas.width}x{atlas.height}), {len(glyph_list)} glyphs")

    glyphs_str = "".join(glyph_list).replace("\\", "\\\\").replace('"', '\\"')
    print("\n-- Lua --")
    print(f'local font = love.graphics.newImageFont("{out_path.name}", "{glyphs_str}")')
    print("love.graphics.setFont(font)")


if __name__ == "__main__":
    main()
