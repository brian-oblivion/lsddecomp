#!/usr/bin/env python3
"""Pull SLPS_015.56 out of a PlayStation disc image into disk/.

    python3 tools/extract_exe.py "LSD - Dream Emulator (Japan).bin"
    python3 tools/extract_exe.py disc.iso --list

WHY THIS IS A REPO TOOL AND NOT A ONE-LINER. A PSX disc is CD-ROM XA, so a raw
`.bin` rip has **2352-byte sectors**: 16 bytes of sync/header, 8 bytes of Mode 2
subheader, 2048 bytes of user data, then 280 bytes of EDC/ECC. `dd` and every
"just copy the file out" recipe silently give you the ECC bytes interleaved
into your executable. The user data has to be sieved out sector by sector, and
the ISO 9660 directory that says WHERE the executable lives is itself stored in
those same sectors.

A 2048-byte-per-sector `.iso` needs none of that, so both layouts are detected
from the CD001 magic at LBA 16 rather than from the file extension -- extensions
lie constantly in ripped-disc collections.
"""
import argparse
import hashlib
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

EXPECTED_NAME = "SLPS_015.56"
EXPECTED_SHA1 = "76322eeade5ebb22dca57fdeac7d68c30f06308d"
EXPECTED_SIZE = 505856

# (sector size, offset of user data within the sector). Mode 2 Form 1 puts the
# 2048 data bytes at +24; a plain .iso has no sector framing at all.
LAYOUTS = [(2352, 24), (2048, 0)]


class Disc:
    def __init__(self, path, sector_size, data_offset):
        self.f = open(path, "rb")
        self.sector_size = sector_size
        self.data_offset = data_offset

    def sector(self, lba):
        self.f.seek(lba * self.sector_size + self.data_offset)
        return self.f.read(2048)

    def read(self, lba, size):
        out = bytearray()
        for i in range((size + 2047) // 2048):
            out += self.sector(lba + i)
        return bytes(out[:size])


def open_disc(path):
    """Detect the sector layout by looking for the ISO 9660 PVD at LBA 16."""
    for sector_size, data_offset in LAYOUTS:
        disc = Disc(path, sector_size, data_offset)
        pvd = disc.sector(16)
        if len(pvd) == 2048 and pvd[1:6] == b"CD001":
            return disc, sector_size
        disc.f.close()
    sys.exit(f"{path}: no ISO 9660 volume descriptor at LBA 16.\n"
             "  Not a disc image, or a layout this tool does not know "
             "(only 2352-byte raw and 2048-byte iso are handled).")


def root_entries(disc):
    """Every entry in the root directory: (name, lba, size)."""
    pvd = disc.sector(16)
    root = pvd[156:156 + 34]
    lba = struct.unpack("<I", root[2:6])[0]
    length = struct.unpack("<I", root[10:14])[0]
    data = disc.read(lba, length)
    out, i = [], 0
    while i < len(data):
        rec_len = data[i]
        if rec_len == 0:
            # Directory records never straddle a logical block; a zero length
            # means "rest of this block is padding".
            i = (i // 2048 + 1) * 2048
            if i >= len(data):
                break
            continue
        rec = data[i:i + rec_len]
        name_len = rec[32]
        name = rec[33:33 + name_len].decode("ascii", "replace")
        # ISO 9660 suffixes a file version: "SLPS_015.56;1".
        name = name.split(";")[0]
        out.append((name, struct.unpack("<I", rec[2:6])[0],
                    struct.unpack("<I", rec[10:14])[0]))
        i += rec_len
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("image", help="disc image (.bin raw or .iso)")
    ap.add_argument("--list", action="store_true",
                    help="list the root directory and exit")
    ap.add_argument("--name", default=EXPECTED_NAME,
                    help=f"file to extract (default {EXPECTED_NAME})")
    ap.add_argument("-o", "--out", default=str(ROOT / "disk" / EXPECTED_NAME))
    args = ap.parse_args()

    disc, sector_size = open_disc(args.image)
    entries = root_entries(disc)

    if args.list:
        print(f"{args.image}: {sector_size}-byte sectors")
        for name, lba, size in entries:
            print(f"  {name:<20} lba={lba:<8} {size:>10} bytes")
        return 0

    match = [e for e in entries if e[0].upper() == args.name.upper()]
    if not match:
        print(f"{args.name} is not in this disc's root directory. It holds:",
              file=sys.stderr)
        for name, _, size in entries:
            print(f"  {name} ({size} bytes)", file=sys.stderr)
        return 1
    name, lba, size = match[0]

    data = disc.read(lba, size)
    if data[:8] != b"PS-X EXE":
        print(f"WARNING: {name} does not start with the PS-X EXE magic "
              f"(got {data[:8]!r}). Extracting anyway.", file=sys.stderr)

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(data)

    digest = hashlib.sha1(data).hexdigest()
    print(f"wrote {out} — {len(data)} bytes, sha1 {digest}")
    if digest == EXPECTED_SHA1:
        print("MATCHES the expected SLPS-01556 dump.")
        return 0
    print(f"\nWARNING: this is NOT the dump this project targets.\n"
          f"  expected sha1 {EXPECTED_SHA1} ({EXPECTED_SIZE} bytes)\n"
          f"  got      sha1 {digest} ({len(data)} bytes)\n"
          f"Every address in config/ is wrong for a different revision.",
          file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
