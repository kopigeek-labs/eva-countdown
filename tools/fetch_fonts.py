#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Download the pinned original fonts for graphics regeneration, never for flashing."""
import hashlib
from pathlib import Path
import urllib.error
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
SOURCES = {
    "assets/fonts/BarlowCondensed-Regular.ttf": "https://raw.githubusercontent.com/google/fonts/89f5431ff0db41bd2fe3f7ba21a723a01622428b/ofl/barlowcondensed/BarlowCondensed-Regular.ttf",
    "assets/fonts/ZenAntique-Regular.ttf": "https://raw.githubusercontent.com/google/fonts/30a4532c801f2650897aa9949ceab031a6401037/ofl/zenantique/ZenAntique-Regular.ttf",
    "assets/fonts/true-lcd-nge.ttf": "https://raw.githubusercontent.com/scottykwok/eva-timer/42e1b12361aa1f569bea8026f31c5cd3d4096da7/src/fonts/true-lcd-nge/true-lcd-nge.ttf",
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    checksums = {}
    for line in (ROOT / "assets/SHA256SUMS").read_text().splitlines():
        expected, name = line.split("  ", 1)
        checksums[name] = expected
    if checksums.keys() != SOURCES.keys():
        raise SystemExit("Font source list and SHA256SUMS do not match.")
    for name, url in SOURCES.items():
        path = ROOT / name
        expected = checksums[name]
        if path.exists():
            if digest(path.read_bytes()) != expected:
                raise SystemExit(f"Modified font cache: {name}. Move it aside before retrying.")
            print(f"Verified cached {path.name}")
            continue
        request = urllib.request.Request(url, headers={"User-Agent": "EVA-Countdown-font-setup"})
        with urllib.request.urlopen(request, timeout=30) as response:
            data = response.read(16 * 1024 * 1024 + 1)
        if digest(data) != expected:
            raise SystemExit(f"Downloaded font checksum mismatch: {name}; nothing was written.")
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        print(f"Downloaded and verified {path.name}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, urllib.error.URLError) as exc:
        raise SystemExit(f"Could not fetch fonts: {exc}") from exc
