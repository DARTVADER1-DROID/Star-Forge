#!/usr/bin/env sh
set -eu

if ! command -v clang-format >/dev/null 2>&1; then
    printf '%s\n' "clang-format is required to format the C sources." >&2
    exit 1
fi

find src tests -type f \( -name '*.c' -o -name '*.h' \) -print0 |
    xargs -0 clang-format -i

printf '%s\n' "Formatted all C source and header files."
