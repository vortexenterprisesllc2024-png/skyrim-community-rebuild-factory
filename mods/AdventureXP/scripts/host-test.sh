#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
cmake --preset host-tests
cmake --build --preset host-tests
ctest --preset host-tests --output-on-failure
python3 scripts/generate_esl.py
python3 tests/test_esl.py
python3 tests/test_pack_layout.py
python3 scripts/assert-cleanroom.py
python3 scripts/pack.py --allow-missing-dll --source
echo "host tests passed"
