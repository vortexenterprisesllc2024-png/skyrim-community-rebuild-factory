#!/usr/bin/env python3
"""Packed zip is a Vortex Data-root layout that includes the MCM."""

from __future__ import annotations

import sys
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import generate_esl as esl  # noqa: E402
import pack  # noqa: E402


REQUIRED = {
    "AdventureXP.esl",
    "AdventureXP.seq",
    "SKSE/Plugins/AdventureXP.ini",
    "Scripts/Source/AdventureXP.psc",
    "Scripts/Source/AdventureXPMCM.psc",
}

FORBIDDEN = (
    "Experience.dll",
    "Experience.ini",
    "Experience.pex",
    "ExperienceMCM.pex",
    "ExperienceMCM.psc",
    "HUDHooks.swf",
    "statsmenu.swf",
)


def test_layout_zip() -> None:
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        esl_path = tmp_path / "AdventureXP.esl"
        esl.write_esl(esl_path)
        out = tmp_path / "AdventureXP-4.0.1.zip"
        rc = pack.pack_layout(
            dll=None,
            ini=ROOT / "dist" / "SKSE" / "Plugins" / "AdventureXP.ini",
            esl=esl_path,
            seq=esl_path.with_suffix(".seq"),
            output=out,
            allow_missing_dll=True,
            publish_artifacts=False,
        )
        assert rc == 0, rc
        with zipfile.ZipFile(out) as zf:
            names = set(zf.namelist())
        missing = REQUIRED - names
        assert not missing, missing
        for banned in FORBIDDEN:
            assert banned not in names
        assert "SKSE/Plugins/AdventureXP.dll" not in names


if __name__ == "__main__":
    test_layout_zip()
    print("test_pack_layout: ok")
