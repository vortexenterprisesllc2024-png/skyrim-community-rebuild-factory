#!/usr/bin/env python3
"""ESL contains the progress global and a Start-Game-Enabled MCM quest."""

from __future__ import annotations

import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import generate_esl as esl  # noqa: E402


def test_build_records() -> None:
    blob = esl.build_esl()
    records = esl.iter_records(blob)
    types = [r[0] for r in records]
    assert types == ["TES4", "GLOB", "QUST"], types

    tes4_flags = records[0][1]
    assert tes4_flags & esl.ESL_FLAG, hex(tes4_flags)

    glob = next(r for r in records if r[0] == "GLOB")
    assert glob[2] == esl.GLOB_FORM_ID
    assert esl.edid_of(glob[3]) == "AdventureXP_Percent"

    quest = next(r for r in records if r[0] == "QUST")
    assert quest[2] == esl.QUST_FORM_ID
    assert esl.edid_of(quest[3]) == "AdventureXPMCMQuest"
    assert esl.vmad_script_names(quest[3]) == ["AdventureXPMCM"]

    subs = dict(esl.iter_subrecords(quest[3]))
    assert subs["FULL"].startswith(b"AdventureXP\x00")
    flags = struct.unpack_from("<H", subs["DNAM"], 0)[0]
    assert flags & esl.QUST_FLAG_START_GAME_ENABLED
    assert flags & 0x0100 == 0  # not Run Once
    assert "Experience" not in blob.decode("latin-1")


def test_seq() -> None:
    seq = esl.build_seq()
    assert seq == struct.pack("<I", esl.QUST_FORM_ID)


def test_write(tmp_path: Path | None = None) -> None:
    dest = (tmp_path or Path("/tmp")) / "AdventureXP.esl"
    esl.write_esl(dest)
    assert dest.is_file()
    assert dest.with_suffix(".seq").read_bytes() == esl.build_seq()
    assert dest.read_bytes() == esl.build_esl()


if __name__ == "__main__":
    test_build_records()
    test_seq()
    test_write()
    print("test_esl: ok")
