#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/eva-check.XXXXXX")
trap 'rm -f "$binary"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=undefined -I "$root/main" "$root/tools/check.cpp" -o "$binary"
"$binary"
