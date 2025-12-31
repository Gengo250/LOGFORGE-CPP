#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

python3 scripts/gen_synth_log.py --out out/synthetic.log --lines 200000 --start "2025-01-01 00:00:00"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/logforge --in out/synthetic.log --out out/report --bench
