#!/usr/bin/env python3
"""Print a .data table from the retail image as a C initializer.

    python3 tools/datatable.py sEntityMoodTable --rows 130 \\
        --layout "s8[2] s8 s8 u8 s8 s8 s8 s8 s8 s8 s8 ptr"
    python3 tools/datatable.py 0x80089EA4 --rows 4 --layout "u32 u32 u32 ptr"

Each row prints as `/* i */ {...},` with its fields in `--layout` order.
Field types: s8 u8 s16 u16 s32 u32, and ptr (a word printed as the symbol
at that address from config/symbols.slps01556.lsdde.txt, NULL for 0, or hex
if no symbol is there). `T[n]` is an array field, printed as a nested brace.
Unsigned 16- and 32-bit values above 0xFF print in hex. A layout's fields
are packed with C's alignment rules, so it must be the struct's fields in
order, padding included (spell padding as a u8[n]).

Before the table the tool lists every symbol the symbols file places inside
the table's range. Each one is a label splat cut into the table (usually a
column read on its own): it has to go from the symbols file, and its
readers have to read the row field instead (CLEANUP.md track 14, on archive/process).
"""
import argparse
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RETAIL = ROOT / "disk/SLPS_015.56"
SYMBOLS = ROOT / "config/symbols.slps01556.lsdde.txt"
BASE = 0x80010000
HEADER = 0x800

FMT = {"s8": "b", "u8": "B", "s16": "h", "u16": "H", "s32": "i", "u32": "I", "ptr": "I"}
SIZE = {"s8": 1, "u8": 1, "s16": 2, "u16": 2, "s32": 4, "u32": 4, "ptr": 4}


def load_symbols():
    by_addr, by_name = {}, {}
    for line in SYMBOLS.read_text().splitlines():
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+);", line)
        if m:
            addr = int(m.group(2), 16)
            by_addr.setdefault(addr, m.group(1))
            by_name[m.group(1)] = addr
    return by_addr, by_name


def parse_layout(text):
    fields = []
    for tok in text.split():
        m = re.fullmatch(r"(\w+)(?:\[(\d+)\])?", tok)
        if not m or m.group(1) not in FMT:
            sys.exit(f"datatable: bad field {tok!r} (types: {' '.join(FMT)})")
        fields.append((m.group(1), int(m.group(2)) if m.group(2) else None))
    return fields


def row_size(fields):
    off, align = 0, 1
    for t, n in fields:
        a = SIZE[t]
        align = max(align, a)
        off = (off + a - 1) // a * a + a * (n or 1)
    return (off + align - 1) // align * align


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("start", help="symbol name or 0x address")
    ap.add_argument("--rows", type=int, required=True)
    ap.add_argument("--layout", required=True)
    args = ap.parse_args()

    by_addr, by_name = load_symbols()
    start = int(args.start, 16) if args.start.startswith("0x") else by_name.get(args.start)
    if start is None:
        sys.exit(f"datatable: no symbol {args.start!r}")
    fields = parse_layout(args.layout)
    size = row_size(fields)
    end = start + size * args.rows
    image = RETAIL.read_bytes()

    inside = sorted((a, s) for a, s in by_addr.items() if start < a < end)
    print(f"/* {start:08X}..{end:08X}: {args.rows} rows of {size:#x} bytes, file offset "
          f"{start - BASE + HEADER:#x}..{end - BASE + HEADER:#x} */")
    for a, s in inside:
        print(f"/* label inside the table: {s} at {a:08X} (row {(a - start) // size}, "
              f"+{(a - start) % size:#x}) */")

    def fmt(t, v):
        if t == "ptr":
            return "NULL" if v == 0 else by_addr.get(v, f"(void *){v:#010x}")
        if t in ("u16", "u32") and v > 0xFF:
            return f"{v:#x}"
        return str(v)

    for r in range(args.rows):
        off = start - BASE + HEADER + r * size
        pos, cells = 0, []
        for t, n in fields:
            a = SIZE[t]
            pos = (pos + a - 1) // a * a
            vals = [struct.unpack_from("<" + FMT[t], image, off + pos + i * a)[0] for i in range(n or 1)]
            pos += a * (n or 1)
            text = [fmt(t, v) for v in vals]
            cells.append("{" + ", ".join(text) + "}" if n else text[0])
        print(f"    /* {r:3d} */ {{" + ", ".join(cells) + "},")


if __name__ == "__main__":
    main()
