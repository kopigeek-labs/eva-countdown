#!/usr/bin/env python3
"""Rasterize licensed font sources into the exact 1-bit firmware assets (Pillow)."""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONTS = ROOT / "assets/fonts"


def text_image(text, font, size):
    f = ImageFont.truetype(str(FONTS / font), size)
    box = f.getbbox(text)
    im = Image.new("L", (max(1, box[2] - box[0]), max(1, box[3] - box[1])))
    ImageDraw.Draw(im).text((-box[0], -box[1]), text, font=f, fill=255)
    return im


def label(im, text, box, jp=False):
    x, y, w, h = box
    tile = text_image(
        text, "ZenAntique-Regular.ttf" if jp else "BarlowCondensed-Regular.ttf", 96
    )
    tile = tile.resize((w, h), Image.Resampling.LANCZOS)
    im.paste(tile, (x, y))


def binary(im):
    return im.point(lambda p: 255 if p >= 128 else 0).convert(
        "1", dither=Image.Dither.NONE
    )


def emit(name, im):
    data = binary(im).tobytes()
    return (
        f"inline constexpr unsigned char {name}[] = {{\n"
        + "\n".join(
            ",".join(f"0x{b:02x}" for b in data[i : i + 24]) + ","
            for i in range(0, len(data), 24)
        )
        + "\n};\n"
    )


im = Image.new("L", (400, 300), 0)
d = ImageDraw.Draw(im)
d.rectangle((3, 3, 396, 296), outline=255, width=2)
d.line((256, 10, 256, 287), fill=255, width=2)
for x in range(12, 390, 34):
    d.line((x, 5, x, 9), fill=255)
label(im, "活動限界まで", (12, 18, 78, 19), True)
label(im, "ACTIVE TIME REMAINING", (96, 23, 150, 12))
label(im, "あと", (12, 43, 20, 17), True)
for y0, y1, jp, en in [(16, 113, "内部", "INTERNAL"), (184, 281, "外部", "EXTERNAL")]:
    d.rectangle((264, y0, 390, y1), outline=255)
    label(im, jp, (270, y0 + 7, 86, 58), True)
    label(im, en, (270, y0 + 72, 85, 17))
    for yy in range(y0 + 3, y1 - 2):
        for xx in range(363, 388):
            if ((xx + yy) // 9) % 2 == 0:
                im.putpixel((xx, yy), 255)
d.rectangle((264, 123, 390, 174), outline=255)
label(im, "主電源供給システム", (269, 130, 116, 13), True)
label(im, "MAIN ENERGY", (275, 150, 104, 8))
label(im, "SUPPLY SYSTEM", (275, 162, 104, 8))
# English is split across two lines to remain legible at native size.
for i, name in enumerate(["STOP", "SLOW", "NORMAL", "RACING"]):
    x = 12 + i * 60
    d.rectangle((x, 252, x + 54, 283), outline=255)
    label(im, name, (x + 5, 258, 44, 10))
# Firmware fills the selected bar at y=273.
out = "// Generated bitmap data; mixed asset licenses, see THIRD_PARTY.md.\n// True LCD NGE digit adaptations: CC-BY-SA-3.0, scottykwok / hippiefabou.\n#pragma once\n"
out += emit("kBackground", im)
# Render all glyphs with common font metrics so the digit 1 stays aligned.
f = ImageFont.truetype(str(FONTS / "true-lcd-nge.ttf"), 256)
boxes = [f.getbbox(c) for c in "0123456789"]
top = min(b[1] for b in boxes)
bottom = max(b[3] for b in boxes)
width = max(b[2] for b in boxes)
for index, c in enumerate("0123456789"):
    glyph = Image.new("L", (width, bottom - top))
    ImageDraw.Draw(glyph).text((0, -top), c, font=f, fill=255)
    out += emit(f"kDigit{index}", glyph.resize((40, 174), Image.Resampling.LANCZOS))
    out += emit(
        f"kSmallDigit{index}", glyph.resize((32, 132), Image.Resampling.LANCZOS)
    )
out += (
    "inline constexpr const unsigned char* kDigits[] = {"
    + ",".join(f"kDigit{i}" for i in range(10))
    + "};\n"
)
out += (
    "inline constexpr const unsigned char* kSmallDigits[] = {"
    + ",".join(f"kSmallDigit{i}" for i in range(10))
    + "};\n"
)
(ROOT / "main/assets.h").write_text(out)
binary(im).save(ROOT / "preview/background.png")
print("Generated 400x300 background and True LCD NGE glyphs")
