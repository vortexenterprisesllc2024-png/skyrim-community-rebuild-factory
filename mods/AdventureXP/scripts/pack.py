#!/usr/bin/env python3
"""Assemble Vortex layout zip and/or full source zip for AdventureXP 4.2.7.

Layout zip (Data root):

    AdventureXP.esl
    AdventureXP.seq
    SKSE/Plugins/AdventureXP.dll   (omitted with --allow-missing-dll)
    SKSE/Plugins/AdventureXP.ini
    Scripts/Source/AdventureXP.psc
    Scripts/Source/AdventureXPMCM.psc
    Scripts/AdventureXP.pex        (if compiled)
    Scripts/AdventureXPMCM.pex     (if compiled)
"""

from __future__ import annotations

import argparse
import shutil
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARTIFACT_DIRS = [
    Path("/opt/cursor/artifacts"),
    Path("/cursor/stores/self/artifacts"),
]
SOURCE_SKIP_DIRS = {
    ".git",
    "build",
    "extern",
    "_deps",
    "vcpkg_installed",
    ".vcpkg",
    "uploads",
    "agent-tools",
    "terminals",
    "__pycache__",
    ".idea",
    ".vs",
    ".vscode",
    "tools",
}
SOURCE_SKIP_FILES = {
    ".ds_store",
    "thumbs.db",
}

PAPYRUS_SCRIPTS = (
    "AdventureXP.psc",
    "AdventureXPMCM.psc",
)


def read_version() -> str:
    version_file = ROOT / "VERSION"
    if version_file.is_file():
        text = version_file.read_text(encoding="utf-8").strip()
        if text:
            return text
    header = ROOT / "include" / "AdventureXP" / "Version.h"
    if header.is_file():
        for line in header.read_text(encoding="utf-8").splitlines():
            if "ADVENTUREXP_VERSION_STRING" in line and '"' in line:
                return line.split('"')[1]
    return "4.2.7"


VERSION = read_version()
DEFAULT_INI = ROOT / "dist" / "SKSE" / "Plugins" / "AdventureXP.ini"
DEFAULT_ESL = ROOT / "dist" / "AdventureXP.esl"
DEFAULT_SEQ = ROOT / "dist" / "AdventureXP.seq"
DEFAULT_LAYOUT = ROOT / "dist" / "packed" / f"AdventureXP-{VERSION}.zip"
DEFAULT_SOURCE = ROOT / "dist" / "packed" / f"AdventureXP-{VERSION}-SOURCE.zip"
PAPYRUS_DIR = ROOT / "papyrus"
PEX_DIR = ROOT / "dist" / "Scripts"


def find_dll(explicit: Path | None) -> Path | None:
    if explicit:
        return explicit if explicit.is_file() else None
    candidates = [
        ROOT / "build" / "windows-msvc-release" / "AdventureXP.dll",
        ROOT / "build" / "windows-msvc-release" / "Release" / "AdventureXP.dll",
        ROOT / "build" / "windows-msvc-release-vs" / "Release" / "AdventureXP.dll",
        ROOT / "build" / "Release" / "AdventureXP.dll",
        ROOT / "dist" / "SKSE" / "Plugins" / "AdventureXP.dll",
    ]
    for path in candidates:
        if path.is_file():
            return path
    return None


def copy_to_artifacts(path: Path, enabled: bool) -> None:
    if not enabled:
        return
    for folder in ARTIFACT_DIRS:
        try:
            folder.mkdir(parents=True, exist_ok=True)
        except OSError:
            continue
        dest = folder / path.name
        shutil.copy2(path, dest)
        print(f"Copied {path.name} -> {dest}")


def pack_layout(
    dll: Path | None,
    ini: Path,
    esl: Path,
    seq: Path,
    output: Path,
    allow_missing_dll: bool,
    publish_artifacts: bool,
) -> int:
    if not ini.is_file():
        print(f"error: INI not found: {ini}", file=sys.stderr)
        return 2
    if not esl.is_file():
        print(f"error: ESL not found: {esl} (run scripts/generate_esl.py)", file=sys.stderr)
        return 2
    if not seq.is_file():
        print(f"error: SEQ not found: {seq} (run scripts/generate_esl.py)", file=sys.stderr)
        return 2
    for name in PAPYRUS_SCRIPTS:
        if not (PAPYRUS_DIR / name).is_file():
            print(f"error: Papyrus source missing: {PAPYRUS_DIR / name}", file=sys.stderr)
            return 2
    if dll is None:
        if not allow_missing_dll:
            print(
                "error: AdventureXP.dll not found. Build on Windows MSVC first, "
                "or pass --dll / --allow-missing-dll.",
                file=sys.stderr,
            )
            return 3
        print("warning: packing without AdventureXP.dll (layout-only zip)")

    staging = output.parent / "_staging"
    if staging.exists():
        shutil.rmtree(staging)
    plugins = staging / "SKSE" / "Plugins"
    plugins.mkdir(parents=True)
    source_dir = staging / "Scripts" / "Source"
    source_dir.mkdir(parents=True)
    shutil.copy2(ini, plugins / "AdventureXP.ini")
    shutil.copy2(esl, staging / "AdventureXP.esl")
    shutil.copy2(seq, staging / "AdventureXP.seq")
    for name in PAPYRUS_SCRIPTS:
        shutil.copy2(PAPYRUS_DIR / name, source_dir / name)
    if PEX_DIR.is_dir():
        pex_out = staging / "Scripts"
        for pex in sorted(PEX_DIR.glob("*.pex")):
            shutil.copy2(pex, pex_out / pex.name)
    if dll:
        shutil.copy2(dll, plugins / "AdventureXP.dll")

    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        output.unlink()
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for path in sorted(staging.rglob("*")):
            if path.is_file():
                zf.write(path, path.relative_to(staging).as_posix())

    shutil.rmtree(staging)
    print(f"Wrote {output} ({output.stat().st_size} bytes)")
    with zipfile.ZipFile(output) as zf:
        for name in zf.namelist():
            print(f"  {name}")
    copy_to_artifacts(output, publish_artifacts)
    return 0


def pack_source(output: Path, publish_artifacts: bool) -> int:
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        output.unlink()
    files: list[Path] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        rel = path.relative_to(ROOT)
        if any(part in SOURCE_SKIP_DIRS for part in rel.parts):
            continue
        if path.name.lower() in SOURCE_SKIP_FILES:
            continue
        if rel.parts[:2] == ("dist", "packed"):
            continue
        if path.suffix.lower() in {".dll", ".pdb", ".obj", ".lib", ".exp"}:
            continue
        files.append(path)

    prefix = f"AdventureXP-{VERSION}"
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for path in sorted(files):
            zf.write(path, (Path(prefix) / path.relative_to(ROOT)).as_posix())

    print(f"Wrote {output} ({output.stat().st_size} bytes, {len(files)} files)")
    copy_to_artifacts(output, publish_artifacts)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dll", type=Path, default=None)
    parser.add_argument("--ini", type=Path, default=DEFAULT_INI)
    parser.add_argument("--esl", type=Path, default=DEFAULT_ESL)
    parser.add_argument("--seq", type=Path, default=DEFAULT_SEQ)
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_LAYOUT)
    parser.add_argument(
        "--allow-missing-dll",
        action="store_true",
        help="Build the layout zip even if AdventureXP.dll is not on this machine",
    )
    parser.add_argument(
        "--source",
        action="store_true",
        help=f"Also write {DEFAULT_SOURCE.name}",
    )
    parser.add_argument("--source-output", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument(
        "--source-only",
        action="store_true",
        help="Write only the source zip",
    )
    parser.add_argument(
        "--no-artifacts",
        action="store_true",
        help="Do not copy zips into /opt/cursor/artifacts",
    )
    args = parser.parse_args()
    publish = not args.no_artifacts

    rc = 0
    if not args.source_only:
        rc = pack_layout(
            find_dll(args.dll),
            args.ini,
            args.esl,
            args.seq,
            args.output,
            args.allow_missing_dll,
            publish,
        )
        if rc != 0:
            return rc
    if args.source or args.source_only:
        rc = pack_source(args.source_output, publish)
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
