#!/usr/bin/env python3
"""Fail if Experience / zax filenames or symbols appear in the clean-room tree."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SKIP_DIRS = {
    ".git",
    "uploads",
    "agent-tools",
    "terminals",
    "__pycache__",
    "build",
    "tools",
}

# Legal / test files may name the forbidden packages to say we do not ship them.
SKIP_CONTENT = {
    "README.md",
    "CLEANROOM.md",
    "assert-cleanroom.py",
    "test_mcm_scripts.py",
    "test_pack_layout.py",
    "test_esl.py",
}

FORBIDDEN_NAMES = {
    "experience.dll",
    "experience.ini",
    "experience.pex",
    "experience.psc",
    "experiencemcm.pex",
    "experiencemcm.psc",
    "experiencemcm_skillxp.pex",
    "experiencemcm_skillxp.psc",
    "experiencemcm_reset.pex",
    "experiencemcm_reset.psc",
    "experiencemcm_save.pex",
    "experiencemcm_save.psc",
    "hudhooks.swf",
    "statsmenu.swf",
    "trainingmenu.swf",
}

FORBIDDEN_SUBSTRINGS = (
    "experiencemcm",
    "experiencemcmquest",
)

CONTENT_TOKENS = (
    re.compile(r"\bExperienceMCM\b"),
    re.compile(r"\biXPDisc"),
    re.compile(r"\biXPClear"),
    re.compile(r"\biXPQuest"),
    re.compile(r"\bSetSettingInt\b"),
    re.compile(r"\bHUDHooks\b"),
    re.compile(r"\bzax\b", re.I),
)

SCAN_SUFFIXES = {".h", ".cpp", ".hpp", ".psc", ".ini", ".cmake"}


def main() -> int:
    bad: list[str] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        rel = path.relative_to(ROOT)
        if any(part in SKIP_DIRS for part in rel.parts):
            continue
        name = path.name.lower()
        if name in FORBIDDEN_NAMES:
            bad.append(str(rel))
            continue
        if any(token in name for token in FORBIDDEN_SUBSTRINGS):
            bad.append(str(rel))
            continue
        if path.name in SKIP_CONTENT:
            continue
        if path.suffix.lower() not in SCAN_SUFFIXES:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for token in CONTENT_TOKENS:
            if token.search(text):
                bad.append(f"{rel} ({token.pattern})")
    if bad:
        print("clean-room violation: forbidden names or symbols:", file=sys.stderr)
        for item in bad:
            print(f"  {item}", file=sys.stderr)
        return 1
    print("clean-room: no Experience/zax filenames or symbols")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
