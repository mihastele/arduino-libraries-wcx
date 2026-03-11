#!/usr/bin/env bash

set -euo pipefail

CC_BIN="${CC:-gcc}"
OUT="tests/test_wcx"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror -pedantic -Isrc tests/test_wcx.c src/*.c -o "$OUT" -lm
"./$OUT"