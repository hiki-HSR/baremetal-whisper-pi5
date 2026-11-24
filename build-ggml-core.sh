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
  -DAARCH=64 -DRASPPI=5
  -D__circle__=510101
  -DSTDLIB_SUPPORT=4
  -U__unix__ -U__linux__
  -isystem "$BASE/install/aarch64-none-circle-libc++/include"
  -isystem "$BASE/install/aarch64-none-circle-libc++/include/c++/v1"
  -isystem "$BASE/install/aarch64-none-circle-libc++-threading/include"
  -isystem "$BASE/install/aarch64-none-circle/include"
  -isystem "$HOME/toolchains/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/15.2.1/include"
  -I "$BASE/include"
  -I "$BASE/libs/circle/include"
  -I "$ROOT/include"
  -I "$ROOT/src"
  -I "$OUT"
)

compile_c() {
  local file="$1"
  local name
  name="$(basename "$file" .c)"

  echo "=== Compiling $file ==="
  aarch64-none-elf-gcc \
    -std=c11 "${COMMON[@]}" \
    -c "$ROOT/src/$file" \
    -o "$OUT/$name.o"
}

compile_cpp() {
  local file="$1"
  local name
  name="$(basename "$file" .cpp)"

  echo "=== Compiling $file ==="
  aarch64-none-elf-g++ \
    -std=c++17 "${COMMON[@]}" \
    -c "$ROOT/src/$file" \
    -o "$OUT/$name.o"
}

compile_c ggml-alloc.c
compile_c ggml-quants.c

compile_cpp ggml.cpp
compile_cpp ggml-backend.cpp
compile_cpp gguf.cpp

echo "GGML CORE BUILD PASSED"
