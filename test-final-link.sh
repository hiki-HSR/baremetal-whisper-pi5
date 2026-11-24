#!/usr/bin/env bash
set -uo pipefail

cd "$HOME/circle-stdlib/samples/10-whisper-init"

cp Makefile Makefile.before-whisper-link

restore_makefile() {
    cp Makefile.before-whisper-link Makefile
}

trap restore_makefile EXIT

python3 - <<'PY'
from pathlib import Path
import re

p = Path("Makefile")
s = p.read_text()

old = r"(?m)^(OBJS\s*=\s*main\.o\s+kernel\.o)\s*$"

new = (
    "OBJS = main.o kernel.o "
    "ggml-build/whisper-combined.o"
)

s, count = re.subn(old, new, s, count=1)

if count != 1:
    raise SystemExit(
        "Makefile의 OBJS 항목을 찾지 못했습니다."
    )

p.write_text(s)
print("Whisper 오브젝트 임시 연결 완료")
PY

if [ $? -ne 0 ]; then
    exit 1
fi

echo "=== Circle + Whisper final link ==="

make -j1 > ggml-build/final-link.log 2>&1

RESULT=$?

echo "Final link exit code: $RESULT"

echo "=== First linker errors ==="

grep -m 30 -E \
'undefined reference|multiple definition|cannot find|error:|failed' \
ggml-build/final-link.log || true

echo "=== Last 15 log lines ==="

tail -15 ggml-build/final-link.log

exit "$RESULT"
