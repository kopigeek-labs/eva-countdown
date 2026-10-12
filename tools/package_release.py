#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Package a locally built candidate; never commit, tag, push or publish."""

import argparse
import hashlib
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIRS = ("main", "tools", "assets", "licenses", "docs", "preview", ".github")
SOURCE_FILES = (
    "README.md",
    "LICENSE",
    "NOTICE",
    "THIRD_PARTY.md",
    "CHANGELOG.md",
    "CONTRIBUTING.md",
    "VERSION",
    "CMakeLists.txt",
    "sdkconfig.defaults",
    ".gitignore",
    ".editorconfig",
    ".clang-format",
)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "dist")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    version = (ROOT / "VERSION").read_text().strip()
    desc = json.loads((build / "project_description.json").read_text())
    if desc.get("project_version") != version or desc.get("target") != "esp32s3":
        raise SystemExit(
            "Build version/target mismatch; rebuild this source before packaging."
        )
    flash = json.loads((build / "flasher_args.json").read_text())
    if flash["extra_esptool_args"]["chip"] != "esp32s3":
        raise SystemExit("Unexpected flash target")
    contents = {}
    manifest = {
        "version": version,
        "board": "Waveshare ESP32-S3-RLCD-4.2",
        "target": "esp32s3",
        "idf_version": desc["git_revision"],
        "flash_settings": flash["flash_settings"],
        "images": [],
    }
    for offset, relative in sorted(
        flash["flash_files"].items(), key=lambda v: int(v[0], 0)
    ):
        path = (build / relative).resolve()
        if not path.is_relative_to(build):
            raise SystemExit("Flash file lies outside build directory")
        data = path.read_bytes()
        if path.name in contents:
            raise SystemExit("Duplicate flash filename")
        contents[path.name] = data
        manifest["images"].append(
            {
                "offset": offset,
                "file": path.name,
                "bytes": len(data),
                "sha256": digest(data),
            }
        )
    if {x["offset"] for x in manifest["images"]} != {"0x0", "0x8000", "0x10000"}:
        raise SystemExit(
            "Unexpected partitions; update flashing documentation before packaging."
        )
    source_paths = [ROOT / name for name in SOURCE_FILES]
    for directory in SOURCE_DIRS:
        source_paths.extend(
            p
            for p in (ROOT / directory).rglob("*")
            if p.is_file() and "__pycache__" not in p.parts
        )
    for path in sorted(source_paths):
        contents["source/" + str(path.relative_to(ROOT))] = path.read_bytes()
    # Ship source and each asset's license alongside the compiled adaptations.
    idf = Path(desc["idf_path"])
    for name, path in {
        "ESP-IDF.txt": idf / "LICENSE",
        "FreeRTOS.txt": idf / "components/freertos/FreeRTOS-Kernel/LICENSE.md",
        "Newlib.txt": idf / "components/newlib/COPYING.NEWLIB",
    }.items():
        contents["sdk-licenses/" + name] = path.read_bytes()
    compiler = Path(desc["c_compiler"]).resolve()
    gcc_licenses = compiler.parent.parent / "share/licenses/gcc"
    for name in ("COPYING3", "COPYING3.LIB", "COPYING.RUNTIME"):
        contents["sdk-licenses/gcc/" + name] = (gcc_licenses / name).read_bytes()
    contents["manifest.json"] = (json.dumps(manifest, indent=2) + "\n").encode()
    contents["FLASHING.md"] = (ROOT / "docs/flashing.md").read_bytes()
    checksums = "".join(
        f"{digest(data)}  {name}\n" for name, data in sorted(contents.items())
    )
    contents["SHA256SUMS"] = checksums.encode()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output = args.output_dir / f"eva-countdown-v{version}-esp32s3-rlcd-4.2.zip"
    if output.exists():
        raise SystemExit(
            f"Refusing to overwrite {output}; choose another output directory."
        )
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(contents.items()):
            entry = zipfile.ZipInfo(name, (2026, 1, 1, 0, 0, 0))
            entry.compress_type = zipfile.ZIP_DEFLATED
            entry.external_attr = (0o100755 if name.endswith(".sh") else 0o100644) << 16
            archive.writestr(entry, data)
    output.with_suffix(".zip.sha256").write_text(
        f"{digest(output.read_bytes())}  {output.name}\n"
    )
    print(output)


if __name__ == "__main__":
    main()
