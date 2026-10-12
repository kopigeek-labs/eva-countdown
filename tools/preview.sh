#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/eva-preview.XXXXXX")
trap 'rm -f "$binary"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I "$root/main" "$root/tools/preview.cpp" -o "$binary"
"$binary" 300000000 0 0 > "$root/preview/ready.pbm"
"$binary" 234560000 1 0 > "$root/preview/running.pbm"
"$binary" 12340000 1 500000 > "$root/preview/emergency.pbm"
"$binary" 0 0 0 > "$root/preview/ended.pbm"

"${PYTHON:-python3}" "$root/tools/export_previews.py"
