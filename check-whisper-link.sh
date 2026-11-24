#!/usr/bin/env bash
set -euo pipefail

OUT="$PWD/ggml-build"

OBJECTS=(
  "$PWD/whisper-build/whisper.o"
  "$OUT/whisper-aneforge.o"
  "$OUT/circle-memory.o"
  "$OUT/circle-pthread.o"

  "$OUT/ggml-c.o"
  "$OUT/ggml.o"
  "$OUT/ggml-alloc.o"
  "$OUT/ggml-quants.o"
  "$OUT/ggml-backend.o"
  "$OUT/gguf.o"

  "$OUT/ggml-backend-meta.o"
  "$OUT/ggml-opt.o"
  "$OUT/ggml-threading.o"
  "$OUT/ggml-backend-reg-circle.o"

  "$OUT/ggml-cpu-c.o"
  "$OUT/ggml-cpu-cpp.o"
)

CPU_OBJECTS=(
  cpu-quants
  cpu-arch-arm-quants
  cpu-repack
  cpu-hbm
  cpu-traits
  cpu-binary-ops
  cpu-unary-ops
  cpu-vec
  cpu-ops
  cpu-amx-amx
  cpu-amx-mmq
  cpu-tiled-tiled
  cpu-tiled-tiled-kernel
  cpu-arch-arm-repack
)

for name in "${CPU_OBJECTS[@]}"; do
  OBJECTS+=("$OUT/$name.o")
done

for obj in "${OBJECTS[@]}"; do
  if [ ! -f "$obj" ]; then
    echo "MISSING: $obj"
    exit 1
  fi
done

echo "=== Object count: ${#OBJECTS[@]} ==="
echo "=== Partial linking ==="

aarch64-none-elf-ld \
  -r \
  -o "$OUT/whisper-combined.o" \
  "${OBJECTS[@]}"

echo "PARTIAL LINK PASSED"

echo "=== Unresolved symbol count ==="

aarch64-none-elf-nm -u "$OUT/whisper-combined.o" \
  | tee "$OUT/unresolved-symbols.txt" \
  | wc -l

echo "=== pthread unresolved symbols ==="

grep -E 'pthread_|thrd_|mtx_|cnd_' \
  "$OUT/unresolved-symbols.txt" || true

echo "=== Other unresolved symbols (first 50) ==="

grep -vE 'pthread_|thrd_|mtx_|cnd_' \
  "$OUT/unresolved-symbols.txt" \
  | head -50 || true
