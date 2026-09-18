#!/usr/bin/env python3
"""Validate the generated AdventureXP.esl is a light plugin with the percent global."""

from __future__ import annotations

import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from generate_esl import ESL_FLAG, GLOB_FORM_ID, parse_records, write_esl  # noqa: E402


def main() -> int:
    path = ROOT / "dist" / "AdventureXP.esl"
    if not path.is_file():
        write_esl(path)

    blob = path.read_bytes()
    if len(blob) < 48:
        print("FAIL: ESL too small")
        return 1

    records = parse_records(blob)
    tes4 = records[0]
    if tes4[0] != "TES4":
        print("FAIL: first record is not TES4")
        return 1
    if tes4[1] & ESL_FLAG == 0:
        print(f"FAIL: TES4 missing ESL flag, flags=0x{tes4[1]:08X}")
        return 1

    globs = [r for r in records if r[0] == "GLOB"]
    if len(globs) != 1:
        print(f"FAIL: expected 1 GLOB, got {len(globs)}")
        return 1
    if globs[0][2] != GLOB_FORM_ID:
        print(f"FAIL: GLOB form id 0x{globs[0][2]:08X}")
        return 1
    if b"AdventureXP_Percent\x00" not in blob:
        print("FAIL: EDID AdventureXP_Percent missing")
        return 1
    if b"Skyrim.esm\x00" not in blob:
        print("FAIL: master Skyrim.esm missing")
        return 1

    # HEDR version 1.70
    hed_off = blob.find(b"HEDR")
    version = struct.unpack_from("<f", blob, hed_off + 6)[0]
    if abs(version - 1.70) > 0.001:
        print(f"FAIL: HEDR version {version}")
        return 1

    print("test_esl: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
