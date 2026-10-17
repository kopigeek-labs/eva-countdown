#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/eva-preview.XXXXXX")
trap 'rm -f "$binary"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I "$root/main" "$root/tools/preview.cpp" -o "$binary"
"${PYTHON:-python3}" "$root/tools/preview_v02.py" "$binary"
