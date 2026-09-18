#!/usr/bin/env python3
"""Generate a minimal Skyrim SE light plugin (ESL) with AdventureXP_Percent.

The ESL is newly written. It only exposes a float global so HUD / other
mods can read current level progress. No third-party assets.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

SSE_FORM_VERSION = 44
ESL_FLAG = 0x00000200
GLOB_FORM_ID = 0x00000800  # light-plugin local range 0x800–0xFFF
HEDR_VERSION = 1.70


def _record_header(rectype: bytes, data: bytes, flags: int, form_id: int) -> bytes:
    # SSE/AE record header is 24 bytes.
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


def build_esl() -> bytes:
    glob_data = b"".join(
        [
            _sub(b"EDID", _zstr("AdventureXP_Percent")),
            _sub(b"FNAM", struct.pack("<B", ord("f"))),
            _sub(b"FLTV", struct.pack("<f", 0.0)),
        ]
    )
    glob_record = _record_header(b"GLOB", glob_data, 0, GLOB_FORM_ID)

    grup_label = b"GLOB"
    grup_data = glob_record
    # GRUP header: type(4) + size(4, includes 24-byte header) + label(4)
    # + groupType(4) + stamp(4) + version(2) + unknown(2)
    grup = (
        b"GRUP"
        + struct.pack("<I", 24 + len(grup_data))
        + grup_label
        + struct.pack("<I", 0)  # group type 0 = top
        + struct.pack("<I", 0)
        + struct.pack("<H", SSE_FORM_VERSION)
        + struct.pack("<H", 0)
        + grup_data
    )

    tes4_data = b"".join(
        [
            _sub(b"HEDR", struct.pack("<fII", HEDR_VERSION, 1, GLOB_FORM_ID + 1)),
            _sub(b"CNAM", _zstr("Jo")),
            _sub(
                b"SNAM",
                _zstr("AdventureXP 4.0.0 light plugin — progress global. Newly written."),
            ),
            _sub(b"MAST", _zstr("Skyrim.esm")),
            _sub(b"DATA", struct.pack("<Q", 0)),
        ]
    )
    tes4 = _record_header(b"TES4", tes4_data, ESL_FLAG, 0)
    return tes4 + grup


def write_esl(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(build_esl())


def parse_records(blob: bytes) -> list[tuple[str, int, int]]:
    """Return (type, flags, form_id) for top-level TES4 + GRUP children."""
    out: list[tuple[str, int, int]] = []
    i = 0
    # TES4
    rtype = blob[i : i + 4].decode("ascii")
    size, flags, form_id = struct.unpack_from("<III", blob, i + 4)
    out.append((rtype, flags, form_id))
    i += 24 + size
    # GRUP
    if blob[i : i + 4] != b"GRUP":
        raise ValueError("expected GRUP after TES4")
    grup_size = struct.unpack_from("<I", blob, i + 4)[0]
    body = blob[i + 24 : i + grup_size]
    j = 0
    while j < len(body):
        rtype = body[j : j + 4].decode("ascii")
        size, flags, form_id = struct.unpack_from("<III", body, j + 4)
        out.append((rtype, flags, form_id))
        j += 24 + size
    return out


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
    for rec in records:
        print(f"  {rec[0]} flags=0x{rec[1]:08X} form=0x{rec[2]:08X}")


if __name__ == "__main__":
    main()
