#!/usr/bin/env python3
"""Confirm layout + source zips and that they stay AdventureXP-named."""

from __future__ import annotations

import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN_FRAGMENTS = (
    "experience.dll",
    "experience.ini",
    "experience.pex",
    "experience.h",
    "experience.cpp",
    "hudhooks.swf",
    "statsmenu.swf",
    "trainingmenu.swf",
    "experiencemcm",
)


def _forbidden(names: list[str]) -> list[str]:
    hits = []
    for name in names:
        lowered = name.lower()
        if any(frag in lowered for frag in FORBIDDEN_FRAGMENTS):
            hits.append(name)
    return hits


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "AdventureXP-test.zip"
        rc = subprocess.call(
            [
                sys.executable,
                str(ROOT / "scripts" / "pack.py"),
                "--allow-missing-dll",
                "--no-artifacts",
                "-o",
                str(out),
            ],
            cwd=ROOT,
        )
        if rc != 0:
            print(f"FAIL: pack.py exited {rc}")
            return 1
        names = zipfile.ZipFile(out).namelist()
        expected = {
            "AdventureXP.esl",
            "SKSE/Plugins/AdventureXP.ini",
        }
        missing = expected - set(names)
        if missing:
            print(f"FAIL: zip missing {missing}, had {names}")
            return 1
        if "SKSE/Plugins/AdventureXP.dll" in names:
            print("FAIL: layout-only zip should not contain a DLL")
            return 1
        bad = _forbidden(names)
        if bad:
            print(f"FAIL: layout zip has forbidden names {bad}")
            return 1

    rc = subprocess.call(
        [
            sys.executable,
            str(ROOT / "scripts" / "pack.py"),
            "--allow-missing-dll",
            "--source",
        ],
        cwd=ROOT,
    )
    layout = ROOT / "dist" / "packed" / "AdventureXP-4.0.0.zip"
    source = ROOT / "dist" / "packed" / "AdventureXP-4.0.0-SOURCE.zip"
    if rc != 0 or not layout.is_file() or not source.is_file():
        print("FAIL: expected AdventureXP-4.0.0.zip and AdventureXP-4.0.0-SOURCE.zip")
        return 1

    source_names = zipfile.ZipFile(source).namelist()
    if not any(name.endswith("src/PlayerXp.cpp") for name in source_names):
        print("FAIL: source zip missing src/PlayerXp.cpp")
        return 1
    if not any(name.endswith("CLEANROOM.md") for name in source_names):
        print("FAIL: source zip missing CLEANROOM.md")
        return 1
    bad = _forbidden(source_names)
    if bad:
        print(f"FAIL: source zip has forbidden names {bad}")
        return 1
    print("test_pack_layout: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
