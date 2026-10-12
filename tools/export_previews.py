#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Export the shared C++ renderer's PBMs into README-friendly PNGs."""

from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1] / "preview"
canvas = Image.new("RGB", (824, 652), "#808080")
draw = ImageDraw.Draw(canvas)
for i, name in enumerate(("ready", "running", "emergency", "ended")):
    with Image.open(root / f"{name}.pbm") as image:
        image.save(root / f"{name}.png")
        x, y = 8 + i % 2 * 408, 24 + i // 2 * 324
        canvas.paste(image, (x, y))
        draw.text((x, y - 17), name.upper(), fill="white")
with Image.open(root / "ready.png") as image:
    image.resize((1200, 900), Image.Resampling.NEAREST).save(root / "ready-3x.png")
canvas.save(root / "states.png")
