#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
g++ -std=c++20 -I include tests/host_types.cpp -o /tmp/adventurexp_host_types
/tmp/adventurexp_host_types
g++ -std=c++20 -I include tests/host_config.cpp src/Config.cpp src/Awards.cpp -o /tmp/adventurexp_host_config
/tmp/adventurexp_host_config
g++ -std=c++20 -I include tests/host_presets.cpp src/Config.cpp src/Awards.cpp -o /tmp/adventurexp_host_presets
/tmp/adventurexp_host_presets
python3 scripts/generate_esl.py
python3 tests/test_esl.py
python3 tests/test_mcm_scripts.py
python3 tests/test_schema.py
python3 tests/test_presets.py
python3 tests/test_pack_layout.py
python3 scripts/assert-cleanroom.py
python3 scripts/pack.py --allow-missing-dll --source
echo "host-test: ok"
