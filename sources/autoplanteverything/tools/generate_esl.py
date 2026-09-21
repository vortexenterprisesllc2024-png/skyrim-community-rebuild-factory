#!/usr/bin/env python3
"""
Auto Plant Everything - offline ESL generator.

Reads a real Skyrim Data folder + plugins.txt, finds FLOR/TREE records that
produce an ingredient, and writes an ESL that appends those pairs to the two
Hearthfire planter FormLists.

SAFETY CONTRACT (this is the part that matters):
  This tool writes FormList OVERRIDE records. In Skyrim an override REPLACES
  the record - it does not merge. So an override containing zero entries wipes
  the planter lists for everyone downstream. This generator therefore REFUSES
  to write a file unless all of the following hold:

    1. It actually read a real Data folder and plugins.txt.
    2. It resolved the two Hearthfire FormLists and read their existing entries.
    3. The output contains AT LEAST as many entries as the winning list it is
       overriding (never shrink).
    4. It is adding at least one new pair (otherwise there is nothing to ship).
    5. Both lists come out exactly the same length.

  Any failure = no file written, non-zero exit. An empty ESL is worse than no
  ESL, so this tool will not produce one.

Usage:
  python3 generate_esl.py --data "C:/.../Skyrim Special Edition/Data" \
                          --plugins "C:/Users/<you>/AppData/Local/Skyrim Special Edition/plugins.txt" \
                          --out AutoPlantEverything_Generated.esl
"""

import argparse
import os
import struct
import sys
import zlib

PLANTABLE_BASE = 0x008247   # flPlanterPlantableItem  (HearthFires.esm)
PLANTED_BASE   = 0x008246   # flPlanterPlantedFlora   (HearthFires.esm)
HEARTHFIRES    = "HearthFires.esm"

REC_HEADER = 24
COMPRESSED_FLAG = 0x00040000


# ----------------------------------------------------------------------------
# minimal plugin reader
# ----------------------------------------------------------------------------

class Plugin:
    """One .esm/.esp/.esl, parsed just far enough for what we need."""

    def __init__(self, path):
        self.path = path
        self.name = os.path.basename(path)
        self.masters = []
        self.is_light = False
        # formid (local, as stored) -> parsed record
        self.flst = {}      # FormList records: local_formid -> [entry formids]
        self.produce = {}   # FLOR/TREE local_formid -> produce item local_formid
        self._read()

    def _read(self):
        with open(self.path, "rb") as fh:
            data = fh.read()
        if len(data) < REC_HEADER or data[:4] != b"TES4":
            raise ValueError(f"{self.name}: not a plugin file")

        size, flags = struct.unpack("<II", data[4:12])
        self.is_light = bool(flags & 0x200)

        # TES4 subrecords -> master list
        p, end = REC_HEADER, REC_HEADER + size
        while p < end - 6:
            sig, sz = struct.unpack("<4sH", data[p:p + 6])
            p += 6
            body = data[p:p + sz]
            p += sz
            if sig == b"MAST":
                self.masters.append(body.rstrip(b"\x00").decode("latin-1"))

        self._walk(data, end, len(data))

    def _walk(self, buf, off, stop):
        while off < stop - REC_HEADER:
            sig = buf[off:off + 4]
            if sig == b"GRUP":
                gsize, = struct.unpack("<I", buf[off + 4:off + 8])
                if gsize < REC_HEADER:
                    return
                self._walk(buf, off + REC_HEADER, min(off + gsize, stop))
                off += gsize
                continue

            rsize, rflags, rfid = struct.unpack("<III", buf[off + 4:off + 16])
            body = buf[off + REC_HEADER:off + REC_HEADER + rsize]
            off += REC_HEADER + rsize

            if sig not in (b"FLST", b"FLOR", b"TREE"):
                continue
            if rflags & COMPRESSED_FLAG:
                try:
                    body = zlib.decompress(body[4:])
                except zlib.error:
                    continue

            if sig == b"FLST":
                entries = []
                for ssig, val in self._subrecords(body):
                    if ssig == b"LNAM" and len(val) == 4:
                        entries.append(struct.unpack("<I", val)[0])
                self.flst[rfid] = entries
            else:
                for ssig, val in self._subrecords(body):
                    if ssig == b"PFIG" and len(val) == 4:
                        produce = struct.unpack("<I", val)[0]
                        if produce:
                            self.produce[rfid] = produce
                        break

    @staticmethod
    def _subrecords(body):
        q = 0
        while q < len(body) - 6:
            sig, sz = struct.unpack("<4sH", body[q:q + 6])
            q += 6
            yield sig, body[q:q + sz]
            q += sz

    def resolve(self, local_formid, load_order):
        """Local FormID -> (plugin_name, base_id), using this plugin's masters."""
        idx = local_formid >> 24
        base = local_formid & 0xFFFFFF
        if idx < len(self.masters):
            return self.masters[idx], base
        return self.name, base


# ----------------------------------------------------------------------------
# ESL writer
# ----------------------------------------------------------------------------

def sub(sig, body):
    return sig + struct.pack("<H", len(body)) + body


def zstr(s):
    return s.encode("latin-1") + b"\x00"


def build_esl(masters, plantable_entries, planted_entries, author, desc):
    """Return the bytes of a light plugin overriding the two planter FormLists."""
    hf_index = masters.index(HEARTHFIRES)

    def flst_record(base_id, edid, entries):
        body = sub(b"EDID", zstr(edid))
        for fid in entries:
            body += sub(b"LNAM", struct.pack("<I", fid))
        formid = (hf_index << 24) | base_id
        return b"FLST" + struct.pack("<IIIIHH", len(body), 0, formid, 0, 44, 0) + body

    recs = flst_record(PLANTABLE_BASE, "flPlanterPlantableItem", plantable_entries)
    recs += flst_record(PLANTED_BASE, "flPlanterPlantedFlora", planted_entries)
    grup = b"GRUP" + struct.pack("<I", len(recs) + REC_HEADER) + b"FLST" \
        + struct.pack("<iiHH", 0, 0, 44, 0) + recs

    hedr = sub(b"HEDR", struct.pack("<fII", 1.71, 2, 0x800))
    header_body = hedr + sub(b"CNAM", zstr(author)) + sub(b"SNAM", zstr(desc))
    for m in masters:
        header_body += sub(b"MAST", zstr(m)) + sub(b"DATA", struct.pack("<Q", 0))

    tes4 = b"TES4" + struct.pack("<IIIIHH", len(header_body), 0x200, 0, 0, 44, 0) + header_body
    return tes4 + grup


# ----------------------------------------------------------------------------
# main
# ----------------------------------------------------------------------------

def die(msg):
    print(f"REFUSED: {msg}", file=sys.stderr)
    print("No file was written. An empty or shrinking override would wipe your "
          "planter lists, so this tool stops instead.", file=sys.stderr)
    sys.exit(2)


def read_load_order(plugins_txt, data_dir):
    """Active plugins, in order. plugins.txt marks active ones with a leading *."""
    order = ["Skyrim.esm", "Update.esm", "Dawnguard.esm", "HearthFires.esm", "Dragonborn.esm"]
    seen = {n.lower() for n in order}
    with open(plugins_txt, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            active = line.startswith("*")
            name = line.lstrip("*").strip()
            if not active or name.lower() in seen:
                continue
            if os.path.exists(os.path.join(data_dir, name)):
                order.append(name)
                seen.add(name.lower())
    return order


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", required=True, help="Skyrim Special Edition/Data folder")
    ap.add_argument("--plugins", required=True, help="path to plugins.txt")
    ap.add_argument("--out", default="AutoPlantEverything_Generated.esl")
    ap.add_argument("--manifest", default=None)
    ap.add_argument("--allow-empty", action="store_true",
                    help=argparse.SUPPRESS)  # deliberately undocumented; for tests only
    args = ap.parse_args()

    if not os.path.isdir(args.data):
        die(f"Data folder not found: {args.data}")
    if not os.path.isfile(args.plugins):
        die(f"plugins.txt not found: {args.plugins}")

    order = read_load_order(args.plugins, args.data)
    print(f"[*] {len(order)} active plugins in load order")

    plugins, produce_pairs = {}, []
    winning_plantable, winning_planted = None, None
    winner_name = None

    for name in order:
        path = os.path.join(args.data, name)
        try:
            pl = Plugin(path)
        except Exception as exc:                    # a bad plugin must not abort the run
            print(f"    skipped {name}: {exc}")
            continue
        plugins[name] = pl

        for flora_fid, produce_fid in pl.produce.items():
            produce_pairs.append((pl, produce_fid, flora_fid))

        for fid, entries in pl.flst.items():
            base = fid & 0xFFFFFF
            src = pl.masters[fid >> 24] if (fid >> 24) < len(pl.masters) else pl.name
            if src != HEARTHFIRES:
                continue
            if base == PLANTABLE_BASE:
                winning_plantable, winner_name = (pl, entries), name
            elif base == PLANTED_BASE:
                winning_planted = (pl, entries)

    if winning_plantable is None or winning_planted is None:
        die("could not find the Hearthfire planter FormLists in this load order "
            "(is HearthFires.esm present?)")

    base_plantable = len(winning_plantable[1])
    base_planted = len(winning_planted[1])
    print(f"[*] current lists: plantable={base_plantable} planted={base_planted} "
          f"(winning override: {winner_name})")

    if base_plantable != base_planted:
        die(f"the lists already disagree in length ({base_plantable} vs {base_planted}); "
            "fix that first - appending would make it worse")

    # Existing entries, as (plugin_name, base_id) so they compare across plugins.
    src_pl, src_entries = winning_plantable
    have = {src_pl.resolve(f, order) for f in src_entries}
    keep_plantable = [src_pl.resolve(f, order) for f in src_entries]
    keep_planted = [winning_planted[0].resolve(f, order) for f in winning_planted[1]]

    added = []
    for pl, produce_fid, flora_fid in produce_pairs:
        item = pl.resolve(produce_fid, order)
        flora = pl.resolve(flora_fid, order)
        if item in have:
            continue
        have.add(item)
        added.append((item, flora))

    print(f"[*] new pairs found: {len(added)}")

    if not added and not args.allow_empty:
        die("nothing new to add - every harvestable already in the lists. "
            "Shipping an override here would gain you nothing and risk everything.")

    out_plantable = keep_plantable + [a[0] for a in added]
    out_planted = keep_planted + [a[1] for a in added]

    if len(out_plantable) != len(out_planted):
        die(f"internal error: lists ended uneven ({len(out_plantable)} vs {len(out_planted)})")
    if len(out_plantable) < base_plantable:
        die(f"output would SHRINK the list ({base_plantable} -> {len(out_plantable)})")

    # Build the master list: HearthFires plus every plugin we cite.
    masters, seen = [], set()
    for name in ("Skyrim.esm", "Update.esm", HEARTHFIRES):
        masters.append(name)
        seen.add(name.lower())
    for plugin_name, _ in out_plantable + out_planted:
        if plugin_name.lower() not in seen and plugin_name in plugins:
            masters.append(plugin_name)
            seen.add(plugin_name.lower())

    if len(masters) > 250:
        die(f"{len(masters)} masters needed - too many for one plugin")

    index_of = {m.lower(): i for i, m in enumerate(masters)}

    def to_local(ref):
        plugin_name, base = ref
        idx = index_of.get(plugin_name.lower())
        if idx is None:
            return None
        return (idx << 24) | base

    local_plantable, local_planted = [], []
    for a, b in zip(out_plantable, out_planted):
        la, lb = to_local(a), to_local(b)
        if la is None or lb is None:          # drop the pair, never half of it
            continue
        local_plantable.append(la)
        local_planted.append(lb)

    if len(local_plantable) < base_plantable:
        die(f"after resolving masters the list would shrink "
            f"({base_plantable} -> {len(local_plantable)})")

    blob = build_esl(masters, local_plantable, local_planted,
                     "Auto Plant Everything offline generator",
                     "Planter FormList pairs generated from this load order")

    with open(args.out, "wb") as fh:
        fh.write(blob)

    manifest = args.manifest or (os.path.splitext(args.out)[0] + ".manifest.txt")
    with open(manifest, "w", encoding="utf-8") as fh:
        fh.write(f"plantable={len(local_plantable)}\n")
        fh.write(f"planted={len(local_planted)}\n")
        fh.write(f"added_pairs={len(added)}\n")
        fh.write(f"was_before={base_plantable}\n")
        fh.write(f"masters={len(masters)}\n")

    print(f"[+] wrote {args.out} ({len(blob)} bytes)")
    print(f"    {base_plantable} existing + {len(added)} new = "
          f"{len(local_plantable)} pairs, {len(masters)} masters")
    return 0


if __name__ == "__main__":
    sys.exit(main())
