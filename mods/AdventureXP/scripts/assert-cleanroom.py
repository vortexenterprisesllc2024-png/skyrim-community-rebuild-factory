#!/usr/bin/env python3
"""Fail if any zax / Experience payload filename landed in this tree."""

from __future__ import annotations

import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SKIP_DIRS = {
    ".git",
    "build",
    "extern",
    "_deps",
    "vcpkg_installed",
    ".vcpkg",
    "uploads",
    "agent-tools",
    "terminals",
}
FORBIDDEN_NAMES = {
    "experience.dll",
    "experience.ini",
    "experience.pex",
    "experience.psc",
    "experience.h",
    "experience.cpp",
    "experience.esl",
    "experience.esp",
    "experience.bsa",
    "experiencemcm.pex",
    "experiencemcm.psc",
    "experiencemcm.esl",
    "experiencemcm_skillxp.pex",
    "experiencemcm_skillxp.psc",
    "experiencemcm_reset.pex",
    "experiencemcm_reset.psc",
    "experiencemcm_save.pex",
    "experiencemcm_save.psc",
    "hudhooks.swf",
    "statsmenu.swf",
    "trainingmenu.swf",
    "experiencemcm_english.txt",
}
FORBIDDEN_DIR_FILES = {
    ("actors", "default.ini"),
    ("races", "default.ini"),
}


def main() -> int:
    hits: list[str] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if any(part in SKIP_DIRS for part in path.parts):
            continue
        name = path.name.lower()
        if name in FORBIDDEN_NAMES:
            hits.append(str(path.relative_to(ROOT)))
            continue
        parts = tuple(p.lower() for p in path.relative_to(ROOT).parts)
        if len(parts) >= 2 and (parts[-2], parts[-1]) in FORBIDDEN_DIR_FILES:
            hits.append(str(path.relative_to(ROOT)))
            continue
        if path.suffix.lower() == ".zip":
            try:
                with zipfile.ZipFile(path) as zf:
                    for inner in zf.namelist():
                        inner_name = Path(inner).name.lower()
                        if inner_name in FORBIDDEN_NAMES:
                            hits.append(f"{path.relative_to(ROOT)}::{inner}")
            except zipfile.BadZipFile:
                hits.append(f"{path.relative_to(ROOT)} (unreadable zip)")
    if hits:
        print("FAIL: forbidden Experience/zax filenames present:")
        for hit in hits:
            print(f"  {hit}")
        return 1
    print("assert-cleanroom: ok (0 Experience/zax payload files)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())