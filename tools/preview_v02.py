#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Render the single README overview from the native C++ firmware renderer."""
import argparse
import io
from pathlib import Path
import subprocess
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("renderer")
    args = parser.parse_args()
    output = ROOT / "preview/v0.2/modes.png"
    output.parent.mkdir(parents=True, exist_ok=True)
    font = ImageFont.truetype(str(ROOT / "assets/fonts/BarlowCondensed-Regular.ttf"), 24)
    modes = Image.new("RGB", (1260, 370), "#e9e9e5")
    draw = ImageDraw.Draw(modes)
    scenes = [
        ("normal", "NORMAL / USB READY", 0),
        ("clock", "DESK CLOCK", 17000000),
        ("pomodoro", "POMODORO / 25 MINUTES", 0),
    ]
    for i, (scene, title, time) in enumerate(scenes):
        raw = subprocess.check_output([args.renderer, scene, str(time)])
        frame = Image.open(io.BytesIO(raw)).convert("RGB")
        x, y = 10 + i * 420, 10
        draw.text((x + 4, y), title, font=font, fill="#151819")
        modes.paste(frame, (x, y + 35))
    modes.save(output)
    print("Updated preview/v0.2/modes.png")


if __name__ == "__main__":
    main()
