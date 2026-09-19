#!/usr/bin/env python3
"""Generate AdventureXP.esl (progress global + SkyUI MCM quest).

Newly written light plugin. No third-party assets.
Form IDs stay in the ESL local range 0x800–0xFFF.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

SSE_FORM_VERSION = 44
ESL_FLAG = 0x00000200
HEDR_VERSION = 1.70

GLOB_FORM_ID = 0x00000800  # AdventureXP_Percent
QUST_FORM_ID = 0x00000801  # AdventureXPMCMQuest
NEXT_OBJECT_ID = 0x00000802

# xEdit QUST DNAM: uint16 flags, uint8 priority, uint8 formVersion,
# uint32 unknown, uint32 type. Bit 0 = Start Game Enabled. Not Run Once.
QUST_FLAG_START_GAME_ENABLED = 0x0001
QUST_PRIORITY = 0
MCM_SCRIPT = "AdventureXPMCM"
MCM_MOD_NAME = "AdventureXP"
MCM_QUEST_EDID = "AdventureXPMCMQuest"
GLOB_EDID = "AdventureXP_Percent"

VMAD_VERSION = 5
VMAD_OBJECT_FORMAT = 2


def _record_header(rectype: bytes, data: bytes, flags: int, form_id: int) -> bytes:
    return (
        rectype
        + struct.pack("<I", len(data))
        + struct.pack("<I", flags)
        + struct.pack("<I", form_id)
        + struct.pack("<I", 0)
        + struct.pack("<H", SSE_FORM_VERSION)
        + struct.pack("<H", 0)
        + data
    )


def _sub(subtype: bytes, payload: bytes) -> bytes:
    return subtype + struct.pack("<H", len(payload)) + payload


def _zstr(text: str) -> bytes:
    return text.encode("utf-8") + b"\x00"


def _wstring(text: str) -> bytes:
    encoded = text.encode("ascii")
    return struct.pack("<H", len(encoded)) + encoded


def _top_group(label: bytes, records: bytes) -> bytes:
    return (
        b"GRUP"
        + struct.pack("<I", 24 + len(records))
        + label
        + struct.pack("<I", 0)
        + struct.pack("<I", 0)
        + struct.pack("<H", SSE_FORM_VERSION)
        + struct.pack("<H", 0)
        + records
    )


def build_glob() -> bytes:
    data = b"".join(
        [
            _sub(b"EDID", _zstr(GLOB_EDID)),
            _sub(b"FNAM", struct.pack("<B", ord("f"))),
            _sub(b"FLTV", struct.pack("<f", 0.0)),
        ]
    )
    return _record_header(b"GLOB", data, 0, GLOB_FORM_ID)


def build_quest_vmad() -> bytes:
    """Attach AdventureXPMCM and set ModName so SkyUI can register the menu."""
    buf = bytearray()
    buf += struct.pack("<HHH", VMAD_VERSION, VMAD_OBJECT_FORMAT, 1)
    buf += _wstring(MCM_SCRIPT)
    buf += struct.pack("<B", 0)  # script status
    buf += struct.pack("<H", 1)  # one property: ModName
    buf += _wstring("ModName")
    buf += struct.pack("<BB", 2, 1)  # type=string, status=edited
    buf += _wstring(MCM_MOD_NAME)
    # QUST fragment trailer (empty): extra bind v2, 0 fragments, 0 aliases
    buf += struct.pack("<B", 2)
    buf += struct.pack("<H", 0)
    buf += _wstring("")
    buf += struct.pack("<H", 0)
    return bytes(buf)


def build_quest_dnam() -> bytes:
    return struct.pack(
        "<HBBII",
        QUST_FLAG_START_GAME_ENABLED,
        QUST_PRIORITY,
        0,
        0,
        0,
    )


def build_quest() -> bytes:
    data = b"".join(
        [
            _sub(b"EDID", _zstr(MCM_QUEST_EDID)),
            _sub(b"VMAD", build_quest_vmad()),
            _sub(b"FULL", _zstr(MCM_MOD_NAME)),
            _sub(b"DNAM", build_quest_dnam()),
            _sub(b"NEXT", b""),
            _sub(b"ANAM", struct.pack("<I", 0)),
        ]
    )
    return _record_header(b"QUST", data, 0, QUST_FORM_ID)


def build_tes4() -> bytes:
    tes4_data = b"".join(
        [
            _sub(b"HEDR", struct.pack("<fII", HEDR_VERSION, 2, NEXT_OBJECT_ID)),
            _sub(b"CNAM", _zstr("Jo")),
            _sub(
                b"SNAM",
                _zstr(
                    "AdventureXP 4.2.9 light plugin - progress global and SkyUI MCM quest. Newly written."
                ),
            ),
            _sub(b"MAST", _zstr("Skyrim.esm")),
            _sub(b"DATA", struct.pack("<Q", 0)),
        ]
    )
    return _record_header(b"TES4", tes4_data, ESL_FLAG, 0)


def build_esl() -> bytes:
    glob_group = _top_group(b"GLOB", build_glob())
    qust_group = _top_group(b"QUST", build_quest())
    return build_tes4() + glob_group + qust_group


def build_seq() -> bytes:
    """Start-Game-Enabled quest list so the MCM quest starts on existing saves."""
    return struct.pack("<I", QUST_FORM_ID)


def write_esl(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(build_esl())
    seq_path = path.with_suffix(".seq")
    seq_path.write_bytes(build_seq())


def iter_records(blob: bytes) -> list[tuple[str, int, int, bytes]]:
    """Return (type, flags, form_id, data) for TES4 and every GRUP child."""
    out: list[tuple[str, int, int, bytes]] = []
    i = 0
    rtype = blob[i : i + 4].decode("ascii")
    size, flags, form_id = struct.unpack_from("<III", blob, i + 4)
    out.append((rtype, flags, form_id, blob[i + 24 : i + 24 + size]))
    i += 24 + size
    while i < len(blob):
        if blob[i : i + 4] != b"GRUP":
            raise ValueError(f"expected GRUP at offset {i}")
        grup_size = struct.unpack_from("<I", blob, i + 4)[0]
        body = blob[i + 24 : i + grup_size]
        j = 0
        while j < len(body):
            rtype = body[j : j + 4].decode("ascii")
            size, flags, form_id = struct.unpack_from("<III", body, j + 4)
            out.append((rtype, flags, form_id, body[j + 24 : j + 24 + size]))
            j += 24 + size
        i += grup_size
    return out


def iter_subrecords(data: bytes) -> list[tuple[str, bytes]]:
    out: list[tuple[str, bytes]] = []
    i = 0
    while i + 6 <= len(data):
        typ = data[i : i + 4].decode("ascii")
        size = struct.unpack_from("<H", data, i + 4)[0]
        out.append((typ, data[i + 6 : i + 6 + size]))
        i += 6 + size
    return out


def parse_records(blob: bytes) -> list[tuple[str, int, int]]:
    return [(rtype, flags, form_id) for rtype, flags, form_id, _ in iter_records(blob)]


def edid_of(data: bytes) -> str | None:
    for typ, payload in iter_subrecords(data):
        if typ == "EDID":
            return payload.split(b"\x00", 1)[0].decode("ascii")
    return None


def vmad_script_names(data: bytes) -> list[str]:
    names: list[str] = []
    for typ, payload in iter_subrecords(data):
        if typ != "VMAD" or len(payload) < 6:
            continue
        version, obj_format, count = struct.unpack_from("<HHH", payload, 0)
        if version != VMAD_VERSION or obj_format != VMAD_OBJECT_FORMAT:
            continue
        offset = 6
        for _ in range(count):
            nlen = struct.unpack_from("<H", payload, offset)[0]
            offset += 2
            names.append(payload[offset : offset + nlen].decode("ascii"))
            offset += nlen
            offset += 1  # status
            nprops = struct.unpack_from("<H", payload, offset)[0]
            offset += 2
            for _p in range(nprops):
                plen = struct.unpack_from("<H", payload, offset)[0]
                offset += 2 + plen
                ptype = payload[offset]
                offset += 2  # type + status
                if ptype == 2:
                    slen = struct.unpack_from("<H", payload, offset)[0]
                    offset += 2 + slen
                elif ptype == 1:
                    offset += 8
                elif ptype in (3, 4):
                    offset += 4
                elif ptype == 5:
                    offset += 1
                else:
                    return names
    return names


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=Path("dist/AdventureXP.esl"),
        help="Output ESL path",
    )
    args = parser.parse_args()
    write_esl(args.output)
    records = parse_records(args.output.read_bytes())
    print(f"Wrote {args.output} ({args.output.stat().st_size} bytes)")
    print(f"Wrote {args.output.with_suffix('.seq')} (MCM quest SEQ)")
    for rec in records:
        print(f"  {rec[0]} flags=0x{rec[1]:08X} form=0x{rec[2]:08X}")


if __name__ == "__main__":
    main()
