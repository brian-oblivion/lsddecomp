#!/usr/bin/env python3
"""Locate prebuilt Psy-Q library objects inside the retail executable.

For every ELF object under DIR (as produced by psyq-obj-parser from the SDK's
.LIB members), take its .text bytes, mask out every byte the linker rewrites
(the relocation targets), and search the executable for positions where all
the remaining bytes agree. A hit therefore means "this object, as shipped in
this SDK version, is what the game linked" -- exactly, not approximately.

Approach follows parasite-eve-2-decomp's tools/match_obj.py (CC0), which
slides each object over the image; that one scores by edit distance, this one
requires a masked exact match so the answer is a yes/no per object.

Usage: match_obj.py EXE DIR [--vram 0x80010000] [--header 0x800] [--min 8]
Prints one line per matched object, then a per-library tally.
"""
import argparse
import sys
from collections import defaultdict
from pathlib import Path

from elftools.elf.elffile import ELFFile

# MIPS relocation types (ELF) and how many low bytes of the target word they own.
R_MIPS_16, R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 1, 2, 4, 5, 6, 7
R_MIPS_LITERAL, R_MIPS_GOT16, R_MIPS_PC16 = 8, 9, 10


def masked_text(path: Path):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        text = elf.get_section_by_name(".text")
        if text is None or text.data_size == 0:
            return None, None
        data = bytearray(text.data())
        mask = bytearray(b"\xff" * len(data))  # 0xff = compare, 0x00 = ignore
        rel = elf.get_section_by_name(".rel.text")
        if rel is not None:
            for r in rel.iter_relocations():
                off, typ = r["r_offset"], r["r_info_type"]
                if typ == R_MIPS_32:
                    mask[off:off + 4] = b"\0\0\0\0"
                elif typ == R_MIPS_26:
                    mask[off:off + 3] = b"\0\0\0"
                    mask[off + 3] &= 0xFC          # low 2 bits of byte 3 are target bits
                elif typ in (R_MIPS_16, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16,
                             R_MIPS_LITERAL, R_MIPS_GOT16, R_MIPS_PC16):
                    mask[off:off + 2] = b"\0\0"
                else:
                    mask[off:off + 4] = b"\0\0\0\0"
        return bytes(data), bytes(mask)


def find_all(exe: bytes, data: bytes, mask: bytes, min_anchor: int):
    """Yield every offset in exe where data matches under mask (4-byte aligned)."""
    n = len(data)
    # longest run of fully-compared bytes -> the needle for bytes.find
    best_s = best_len = 0
    s = None
    for i in range(n + 1):
        if i < n and mask[i] == 0xFF:
            if s is None:
                s = i
        else:
            if s is not None and i - s > best_len:
                best_s, best_len = s, i - s
            s = None
    if best_len < min_anchor:
        return
    needle = data[best_s:best_s + best_len]
    pos = exe.find(needle)
    while pos != -1:
        start = pos - best_s
        if start >= 0 and start % 4 == 0 and start + n <= len(exe):
            window = exe[start:start + n]
            if all((window[i] & mask[i]) == (data[i] & mask[i]) for i in range(n)):
                yield start
        pos = exe.find(needle, pos + 1)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("exe", type=Path)
    ap.add_argument("dir", type=Path)
    ap.add_argument("--vram", type=lambda s: int(s, 0), default=0x80010000)
    ap.add_argument("--header", type=lambda s: int(s, 0), default=0x800)
    ap.add_argument("--min", type=int, default=8, help="minimum fully-compared anchor bytes (default 8)")
    ap.add_argument("--unmatched", action="store_true", help="also list objects with no hit")
    args = ap.parse_args()

    exe = args.exe.read_bytes()
    tally = defaultdict(lambda: [0, 0, 0])  # lib -> [matched, ambiguous, total]
    rows = []
    for o in sorted(args.dir.rglob("*.o")):
        lib = o.parent.name
        data, mask = masked_text(o)
        if data is None:
            continue
        tally[lib][2] += 1
        hits = list(find_all(exe, data, mask, args.min))
        if len(hits) == 1:
            tally[lib][0] += 1
            vram = hits[0] - args.header + args.vram
            rows.append((hits[0], f"{o.relative_to(args.dir)}  text=0x{len(data):x}  fileoff=0x{hits[0]:x}  vram=0x{vram:08x}"))
        elif len(hits) > 1:
            tally[lib][1] += 1
            rows.append((hits[0], f"{o.relative_to(args.dir)}  text=0x{len(data):x}  AMBIGUOUS x{len(hits)}: " + " ".join(f"0x{h:x}" for h in hits[:6])))
        elif args.unmatched:
            rows.append((1 << 40, f"{o.relative_to(args.dir)}  text=0x{len(data):x}  no match"))
    for _, line in sorted(rows):
        print(line)
    print()
    print(f"{'library':<10} {'matched':>7} {'ambig':>5} {'total':>5}")
    tm = ta = tt = 0
    for lib in sorted(tally):
        m, a, t = tally[lib]
        tm += m; ta += a; tt += t
        print(f"{lib:<10} {m:>7} {a:>5} {t:>5}")
    print(f"{'ALL':<10} {tm:>7} {ta:>5} {tt:>5}")


if __name__ == "__main__":
    main()
