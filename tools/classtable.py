#!/usr/bin/env python3
"""Dump and compare the game's hand-rolled class method tables.

    tools/classtable.py --scan                 # every method table in .data
    tools/classtable.py gDreamSysMethods       # one table, entries resolved
    tools/classtable.py 0x80087BDC             # ...by address
    tools/classtable.py gDreamSysMethods --vs 0x800878D4   # derived vs base

WHY THIS EXISTS. This game is plain C with a MANUAL class framework — proven,
not assumed; see docs/research/class-framework.md for the evidence and the
reproducer that rules out compiler-generated C++. Every object carries a
pointer to one of these tables at offset 0, and a call like

    lw $v0, 0x0($s1)      ; obj->methods
    lw $v0, 0x80($v0)     ; ->slot at +0x80
    jalr $v0

is a method call whose TARGET IS IN THE DATA, not in the instruction stream. A
runner reading such a call cannot see what it invokes without resolving the
slot by hand, and `class_39e08` alone is 415 functions of this. Resolving it by
hand is also where mistakes come from: an off-by-one slot index silently names
the wrong function.

`--vs` is the high-value mode. A derived table is a copy of its base's with
some slots replaced, so diffing them tells you exactly which methods the
subclass overrides — which is the class's actual behaviour, in one screen.
"""
import argparse
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RETAIL = ROOT / "disk/SLPS_015.56"
SYMBOLS = ROOT / "config/symbols.slps01556.lsdde.txt"
ELF = ROOT / "build/lsdde.elf"
NM = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"

VRAM = 0x80010000
HDR = 0x800
# Section bounds, from the splat config. .text runs from the first code
# subsegment to the first data subsegment.
TEXT = (0x800118DC, 0x80066870)
DATA = (0x80066870, 0x8008B800)


def file_off(vram):
    return vram - VRAM + HDR


def load_symbols():
    """vram -> name, from the symbol file and (if built) the ELF."""
    syms = {}
    for line in SYMBOLS.read_text().splitlines():
        m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", line)
        if m:
            syms[int(m.group(2), 16)] = m.group(1)
    # The ELF adds splat's auto-generated func_/D_ names, which cover
    # everything the hand-written file does not.
    if NM.exists() and ELF.exists():
        out = subprocess.run([str(NM), str(ELF)], capture_output=True,
                             text=True).stdout
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[1] in ("T", "t", "D", "d", "R", "r"):
                syms.setdefault(int(parts[0], 16), parts[2])
    return syms


def resolve(arg, syms):
    """An address from either a hex literal or a symbol name."""
    if re.fullmatch(r"0x[0-9A-Fa-f]+", arg):
        return int(arg, 16)
    for addr, name in syms.items():
        if name == arg:
            return addr
    sys.exit(f"unknown symbol: {arg}\n"
             f"  Give a 0x address, or a name from "
             f"config/symbols.slps01556.lsdde.txt.")


def words(data, vram, n):
    o = file_off(vram)
    return [struct.unpack("<I", data[o + i * 4:o + i * 4 + 4])[0]
            for i in range(n)]


def table_length(data, vram, limit=256):
    """How many slots the table has.

    Ends at the first word that is neither a .text pointer nor zero. Zero is
    included because these tables DO contain null slots mid-table — that is one
    of the things that proves they are hand-written rather than compiler
    output, since g++ fills an unimplemented slot with __pure_virtual and never
    with 0. Trailing nulls are trimmed; interior ones are real entries.
    """
    ws = words(data, vram, limit)
    n = 0
    for x in ws:
        if TEXT[0] <= x < TEXT[1] or x == 0:
            n += 1
        else:
            break
    while n and ws[n - 1] == 0:
        n -= 1
    return n


def name_of(addr, syms):
    return syms.get(addr, f"0x{addr:08X}")


def dump(data, vram, syms, label=None):
    header = words(data, vram, 1)[0]
    n = table_length(data, vram + 4)
    print(f"{label or name_of(vram, syms)}  @ 0x{vram:08X}  "
          f"(file 0x{file_off(vram):X})")
    print(f"  +0x000  0x{header:08X}   header word (not a pointer; varies per "
          f"class — id/flags)")
    for i, x in enumerate(words(data, vram + 4, n)):
        off = 4 + i * 4
        if x == 0:
            print(f"  +0x{off:03X}  {'-':<10} (null slot)")
        else:
            print(f"  +0x{off:03X}  0x{x:08X} {name_of(x, syms)}")
    print(f"  {n} slots")
    return header, words(data, vram + 4, n)


def scan(data, syms, min_slots=8):
    print(f"Method tables in .data (>= {min_slots} consecutive .text "
          f"pointers)\n")
    o, end = file_off(DATA[0]), file_off(DATA[1])
    ws = [struct.unpack("<I", data[i:i + 4])[0]
          for i in range(o, min(end, len(data) - 3), 4)]
    base = DATA[0]
    i, found = 0, 0
    while i < len(ws):
        if TEXT[0] <= ws[i] < TEXT[1]:
            j = i
            while j < len(ws) and (TEXT[0] <= ws[j] < TEXT[1] or ws[j] == 0):
                j += 1
            k = j
            while k > i and ws[k - 1] == 0:
                k -= 1
            if k - i >= min_slots:
                found += 1
                tbl = base + (i - 1) * 4      # the header word precedes slot 0
                print(f"  0x{tbl:08X}  {k - i:4d} slots  "
                      f"header 0x{ws[i - 1]:08X}  {name_of(tbl, syms)}")
            i = j
        else:
            i += 1
    print(f"\n  {found} tables")


def compare(data, a, b, syms):
    print("=" * 72)
    ha, wa = dump(data, a, syms)
    print()
    hb, wb = dump(data, b, syms)
    print("=" * 72)
    print(f"\nSLOT DIFF  ({name_of(a, syms)} vs {name_of(b, syms)})\n")
    same = 0
    for i in range(max(len(wa), len(wb))):
        x = wa[i] if i < len(wa) else None
        y = wb[i] if i < len(wb) else None
        off = 4 + i * 4
        if x == y:
            same += 1
            continue
        if x is None:
            print(f"  +0x{off:03X}  ONLY IN SECOND  {name_of(y, syms)}")
        elif y is None:
            print(f"  +0x{off:03X}  ONLY IN FIRST   {name_of(x, syms)}")
        else:
            print(f"  +0x{off:03X}  OVERRIDDEN      {name_of(y, syms)}")
            print(f"          {'':>14}  -> {name_of(x, syms)}")
    print(f"\n  {same} slots identical, "
          f"{max(len(wa), len(wb)) - same} differ.")
    if same > 4:
        print("  A long identical run is INHERITANCE: the first table is a "
              "copy of the\n  second with specific slots replaced. The "
              "differing slots are what the\n  subclass actually does.")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("table", nargs="?", help="symbol name or 0x address")
    ap.add_argument("--vs", help="second table to diff against")
    ap.add_argument("--scan", action="store_true",
                    help="list every method table in .data")
    ap.add_argument("--min-slots", type=int, default=8)
    args = ap.parse_args()

    if not RETAIL.exists():
        sys.exit(f"{RETAIL} missing — see README.md, 'Building it'")
    data = RETAIL.read_bytes()
    syms = load_symbols()

    if args.scan:
        scan(data, syms, args.min_slots)
        return 0
    if not args.table:
        ap.error("give a table (symbol or 0x address), or --scan")

    a = resolve(args.table, syms)
    if args.vs:
        compare(data, a, resolve(args.vs, syms), syms)
    else:
        dump(data, a, syms)
    return 0


if __name__ == "__main__":
    sys.exit(main())
