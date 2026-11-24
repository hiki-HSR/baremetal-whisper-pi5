#!/usr/bin/env bash
set -euo pipefail

ROOT="$HOME/whisper.cpp/ggml"
BASE="$HOME/circle-stdlib"
OUT="$PWD/ggml-build"

mkdir -p "$OUT"

COMMON=(
  -O2 -nostdinc
  -mcpu=cortex-a76 -mlittle-endian
  -D_GNU_SOURCE
  -D_POSIX_C_SOURCE=200809L
  -D_POSIX_THREADS=200809L
  -DAARCH=64
  -DRASPPI=5
  -D__circle__=510101
  -DSTDLIB_SUPPORT=4
  -U__unix__ -U__linux__
  -isystem "$BASE/install/aarch64-none-circle-libc++/include"
  -isystem "$BASE/install/aarch64-none-circle-libc++/include/c++/v1"
  -isystem "$BASE/install/aarch64-none-circle-libc++-threading/include"
  -isystem "$HOME/toolchains/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/15.2.1/include"
  -isystem "$BASE/install/aarch64-none-circle/include"
  -I "$BASE/include"
  -I "$BASE/libs/circle/include"
  -I "$ROOT/include"
  -I "$ROOT/src"
  -I "$ROOT/src/ggml-cpu"
  -I "$OUT"
)

echo "=== ggml-cpu.c ==="

aarch64-none-elf-gcc \
  -std=c11 \
  -include stdint.h \
  -include "$HOME/toolchains/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/15.2.1/include/stdatomic.h" \
  "${COMMON[@]}" \
  -c "$ROOT/src/ggml-cpu/ggml-cpu.c" \
  -o "$OUT/ggml-cpu-c.o"

echo "=== ggml-cpu.cpp ==="

aarch64-none-elf-g++ \
  -std=c++17 "${COMMON[@]}" \
  -c "$ROOT/src/ggml-cpu/ggml-cpu.cpp" \
  -o "$OUT/ggml-cpu-cpp.o"

echo "GGML CPU BASIC BUILD PASSED"
