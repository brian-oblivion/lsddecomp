#!/usr/bin/env python3
"""Where the original source files began and ended, from the rodata.

    python3 tools/research/tuboundary.py              # validation, then every unit edge that is not free
    python3 tools/research/tuboundary.py --units      # every C unit: its edges and what lies inside it
    python3 tools/research/tuboundary.py --unit <u>   # one unit, gap by gap
    python3 tools/research/tuboundary.py --json

FINISHING-PLAN track 7 wants one `.c` per ORIGINAL translation unit, named
for what it holds. The units in src/ were carved for parallel matching (the
`_b`, `_c` slices are twenty-function cuts), so their edges say nothing about
the original files. This tool measures what the binary does say. Two facts
about the pinned toolchain, both measured through cc1 (plan revision 27):

  1. cc1 emits a function's rodata (its string literals at `.align 2`, its
     jump tables at `.align 3`) into the object's `.rdata` in FUNCTION
     order, and the linker lays each section out in OBJECT order. So a file
     boundary between text functions i and i+1 is POSSIBLE only if every
     file-private rodata item referenced at or before i lies below every one
     referenced after i. A crossing makes the boundary IMPOSSIBLE: those two
     functions were in one file.
  2. A jump table is 8-aligned relative to its object's rodata, and the
     objects' rodata is NOT 8-aligned (retail has tables at 4 mod 8). So two
     address-consecutive jump tables whose addresses differ mod 8 lie in
     DIFFERENT files: a boundary is FORCED somewhere between their functions.
     (parasite-eve-2-decomp's rodata_cut.py rests on the same rule.)

File-private items are jump tables (words pointing into the referencing
function) and strings (NUL-terminated text). Other const data can be an
extern table shared across files, so it is reported but never decides.
References are recovered from the retail image itself (lui/addiu, lui/load,
the indexed `addu` of a table walk), so the tool needs no source and reads
Sony code the same way as game code.

VALIDATION comes first in the output: every edge between two placed Sony
objects is a real file boundary, so the rule must call each one POSSIBLE, and
no forced boundary may fall wholly inside one Sony object. If either fails the
model is wrong and the rest of the output is not evidence; the tool says so.

What it cannot see: a boundary between two files with no private rodata
between them is merely POSSIBLE, and most gaps are. An impossible edge is a
measurement (merge); a forced interval is a measurement (split somewhere in
it); a possible edge is only the absence of a contradiction. Content (a
class's methods together, the header banners) decides the rest, and that is
the runner's judgement recorded in the report.
"""
import argparse
import bisect
import json
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import progress  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent.parent
MAP = ROOT / "build/lsdde.map"
IMAGE = ROOT / "build/SLPS_015.56"
VRAM, FILEOFF = 0x80010000, 0x800
GP = 0x8008A808

LOADS_STORES = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A, 0x2B, 0x2E, 0x32, 0x3A}


def map_sections():
    """[(start, size, object path)] for .text, .rodata and the data sections, from the link map."""
    text, rodata, data = [], [], []
    rx = re.compile(r"^ (\.text|\.rodata|\.data|\.sdata|\.sbss|\.bss)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)")
    for line in MAP.read_text(errors="replace").splitlines():
        m = rx.match(line)
        if not m:
            continue
        a, n = int(m.group(2), 16), int(m.group(3), 16)
        if a < VRAM or n == 0:
            continue
        if m.group(1) in (".text", ".rodata"):
            (text if m.group(1) == ".text" else rodata).append((a, n, m.group(4)))
        else:
            data.append((a, n, m.group(4), m.group(1)))
    return sorted(text), sorted(rodata), sorted(data)


def unit_of(path):
    p = Path(path)
    name = p.name
    for sfx in (".c.o", ".s.o", ".o"):
        if name.endswith(sfx):
            name = name[: -len(sfx)]
            break
    if "/lib/" in path:
        return "sony:" + "/".join(p.parts[-2:])[: -len(".o")]
    return name


class Image:
    def __init__(self):
        self.b = IMAGE.read_bytes()

    def word(self, a):
        o = a - VRAM + FILEOFF
        return struct.unpack_from("<I", self.b, o)[0] if 0 <= o <= len(self.b) - 4 else None

    def byte(self, a):
        o = a - VRAM + FILEOFF
        return self.b[o] if 0 <= o < len(self.b) else None


def refs_of(img, start, size):
    """Absolute addresses a function forms with lui + (addiu | load/store | indexed addu)."""
    hi = {}          # reg -> value from lui (possibly + an index, still resolvable with %lo)
    out = set()
    for a in range(start, start + size, 4):
        w = img.word(a)
        if w is None:
            break
        op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if rs == 28 and (op == 0x09 or op in LOADS_STORES):   # %gp_rel
            out.add((GP + simm) & 0xFFFFFFFF)
            if op < 0x28 and op != 0x32:
                hi.pop(rt, None)
            continue
        if op == 0x0F:                      # lui
            hi[rt] = imm << 16
            continue
        if op == 0x09 and rs in hi:         # addiu rt, rs, lo
            out.add((hi[rs] + simm) & 0xFFFFFFFF)
            hi.pop(rt, None)
            continue
        if op in LOADS_STORES and rs in hi:
            out.add((hi[rs] + simm) & 0xFFFFFFFF)
            if op < 0x28 and op != 0x32:
                hi.pop(rt, None)
            continue
        if op == 0 and (w & 0x3F) == 0x21 and rd in (rs, rt) and (rs in hi) != (rt in hi):
            hi[rd] = hi[rs] if rs in hi else hi[rt]   # table walk: base + index keeps %hi
            continue
        # anything else that writes a register kills its %hi
        if op == 0 and rd:
            hi.pop(rd, None)
        elif op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E) or (0x20 <= op <= 0x26):
            hi.pop(rt, None)
        elif op == 0x03:                    # jal: caller-saved registers die
            for r in list(hi):
                if 1 <= r <= 15 or r in (24, 25, 31):
                    hi.pop(r)
    return out


def classify(img, addr, funcs_by_start, fstarts):
    """'jtbl' / 'string' / 'data' for a rodata item at addr."""
    w = img.word(addr)
    if w is not None and VRAM <= w < 0x80200000:
        i = bisect.bisect_right(fstarts, w) - 1
        if i >= 0:
            fa, fn, fsz = funcs_by_start[i]
            if fa <= w < fa + fsz:
                return "jtbl:" + fn
    n, a = 0, addr
    while n < 256:
        c = img.byte(a)
        if c is None:
            break
        if c == 0:
            break
        if not (c >= 0x20 and c != 0x7F or c in (9, 10, 13)):
            n = -1
            break
        n += 1
        a += 1
    if n >= 2:
        return "string"
    return "data"


def crossing_gaps(rows, key):
    """For each gap i (between rows[i] and rows[i+1]): True when no item of kind
    `key` referenced at or before i lies at or above one referenced after i."""
    n = len(rows)
    INF = 1 << 40
    pre, suf = [0] * n, [INF] * n
    m = -1
    for i, r in enumerate(rows):
        for x in r[key]:
            m = max(m, x)
        pre[i] = m
    m = INF
    for i in range(n - 1, -1, -1):
        for x in rows[i][key]:
            m = min(m, x)
        suf[i] = m
    return [(pre[i] < suf[i + 1], pre[i], suf[i + 1]) for i in range(n - 1)]


def analyse():
    info = progress.text_symbols()
    if not info or not MAP.exists() or not IMAGE.exists():
        sys.exit("needs a built tree: run ./build-and-verify.sh first")
    text, rodata, data = map_sections()
    ro_lo = min(a for a, _, _ in rodata)
    ro_hi = max(a + n for a, n, _ in rodata)
    dstarts = [a for a, _, _, _ in data]

    def data_kind(x):
        i = bisect.bisect_right(dstarts, x) - 1
        return data[i][3] if i >= 0 and x < data[i][0] + data[i][1] else None
    tstarts = [a for a, _, _ in text]

    def text_obj(a):
        i = bisect.bisect_right(tstarts, a) - 1
        return text[i] if i >= 0 and a < text[i][0] + text[i][1] else None

    # functions only: text_symbols() also returns labels splat typed as code
    # that lie in rodata (strings a splat pass called functions)
    funcs = sorted((a, n, s) for n, (a, s) in info.items() if s > 0 and text_obj(a))
    fstarts = [a for a, _, _ in funcs]
    img = Image()

    rows = []
    for a, name, size in funcs:
        unit = unit_of(text_obj(a)[2])
        priv, kinds, dat = [], [], []
        for r in sorted(refs_of(img, a, size)):
            if ro_lo <= r < ro_hi:
                kind = classify(img, r, funcs, fstarts)
                if kind == "jtbl:" + name:
                    priv.append(r)
                    kinds.append((r, "jtbl"))
                elif kind == "string":
                    priv.append(r)
                    kinds.append((r, "string"))
            elif data_kind(r):
                dat.append((data_kind(r), r))
        rows.append({"name": name, "addr": a, "unit": unit, "priv": priv, "kinds": kinds, "data": dat})

    # soft evidence: a data address referenced by exactly ONE function is
    # probably that file's own (a static, or a global only it touches)
    users = {}
    for i, r in enumerate(rows):
        for x in r["data"]:
            users.setdefault(x, set()).add(i)
    # bss is not evidence: Sony's linker scattered library bss and COMMON is
    # allocated by the linker, not per object (config/psyq-objects.ld pins it)
    sections = sorted({k for k, _ in users} - {".bss", ".sbss"})
    for i, r in enumerate(rows):
        for sec in sections:
            r["solo" + sec] = [x for k, x in r["data"] if k == sec and len(users[(k, x)]) == 1]

    # each output section is laid out in object order on its own, so the test
    # runs per section; the soft verdict is the first section that crosses
    hard = crossing_gaps(rows, "priv")
    per = {sec: crossing_gaps(rows, "solo" + sec) for sec in sections}
    gaps = []
    for i, h in enumerate(hard):
        g = {"possible": h[0], "before": h[1], "after": h[2], "soft": True,
             "soft_before": 0, "soft_after": 0, "soft_sec": None}
        for sec in sections:
            ok, b, a_ = per[sec][i]
            if not ok:
                g.update(soft=False, soft_before=b, soft_after=a_, soft_sec=sec)
                break
        gaps.append(g)

    # forced boundaries from jump-table parity
    tables = sorted((x, i) for i, r in enumerate(rows) for x, k in r["kinds"] if k == "jtbl")
    forced, contradictions = [], []
    for (x1, i1), (x2, i2) in zip(tables, tables[1:]):
        if (x1 & 7) == (x2 & 7):
            continue
        if i1 >= i2:
            contradictions.append((rows[i1]["name"], hex(x1), rows[i2]["name"], hex(x2)))
            continue
        forced.append((i1, i2 - 1, hex(x1), hex(x2)))

    # validation against placed Sony objects: each edge between two of them
    # is a real file boundary
    val = {"sony_edges": 0, "bad_edges": [], "soft_bad_edges": [], "bad_forced": [],
           "tables": len(tables), "odd_tables": sum(1 for x, _ in tables if x & 7)}
    for i in range(len(rows) - 1):
        u1, u2 = rows[i]["unit"], rows[i + 1]["unit"]
        if u1 != u2 and u1.startswith("sony:") and u2.startswith("sony:"):
            val["sony_edges"] += 1
            if not gaps[i]["possible"]:
                val["bad_edges"].append((u1, u2))
            if not gaps[i]["soft"]:
                val["soft_bad_edges"].append((u1, u2))
    for lo, hi, x1, x2 in forced:
        us = {rows[j]["unit"] for j in range(lo, hi + 2)}
        if len(us) == 1 and next(iter(us)).startswith("sony:"):
            val["bad_forced"].append((next(iter(us)), x1, x2))
    return rows, gaps, forced, contradictions, val


def unit_report(rows, gaps, forced):
    """One entry per RUN of consecutive functions in the same unit."""
    runs = []
    for i, r in enumerate(rows):
        if runs and runs[-1]["unit"] == r["unit"]:
            runs[-1]["last"] = i
        else:
            runs.append({"unit": r["unit"], "first": i, "last": i})
    out = []
    for k, d in enumerate(runs):
        f, l = d["first"], d["last"]
        start = gaps[f - 1] if f > 0 else None
        inner = range(f, l)
        splits = [(lo, hi, x1, x2) for lo, hi, x1, x2 in forced if lo >= f and hi <= l - 1]
        out.append({
            "unit": d["unit"], "prev": runs[k - 1]["unit"] if k else None,
            "functions": [rows[j]["name"] for j in range(f, l + 1)],
            "start_possible": None if start is None else start["possible"],
            "start_soft": None if start is None else start["soft"],
            "start_detail": None if start is None else (hex(start["before"]), hex(start["after"])),
            "start_soft_detail": None if start is None else (hex(start["soft_before"]), hex(start["soft_after"])),
            "inner_possible": sum(1 for g in inner if gaps[g]["possible"]),
            "inner_soft": sum(1 for g in inner if gaps[g]["possible"] and gaps[g]["soft"]),
            "inner_gaps": len(inner),
            "forced_inside": [(rows[lo]["name"], rows[hi + 1]["name"], x1, x2) for lo, hi, x1, x2 in splits],
            "first": f, "last": l,
        })
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--units", action="store_true")
    ap.add_argument("--unit")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    rows, gaps, forced, contra, val = analyse()
    rep = unit_report(rows, gaps, forced)
    if a.json:
        print(json.dumps({"validation": val, "units": rep, "contradictions": contra}, indent=1, default=str))
        return
    ok = not val["bad_edges"] and not val["bad_forced"] and not contra
    print(f"VALIDATION (hard, rodata): {val['sony_edges']} edge(s) between placed Sony objects, "
          f"{len(val['bad_edges'])} called impossible; {len(val['bad_forced'])} forced boundary(ies) inside one "
          f"Sony object; {len(contra)} parity contradiction(s); {val['tables']} jump tables "
          f"({val['odd_tables']} at 4 mod 8)")
    print(f"VALIDATION (soft, single-user data): {len(val['soft_bad_edges'])} of the same "
          f"{val['sony_edges']} edge(s) called unlikely")
    for e in val["bad_edges"][:10]:
        print(f"  BAD EDGE {e[0]} | {e[1]}")
    for e in val["soft_bad_edges"][:10]:
        print(f"  soft-bad edge {e[0]} | {e[1]}")
    for e in val["bad_forced"][:10]:
        print(f"  BAD FORCED inside {e[0]} ({e[1]} / {e[2]})")
    for c in contra[:10]:
        print(f"  CONTRADICTION {c}")
    if not ok:
        print("  => the hard model does not hold here; nothing below is evidence until this is understood.")
    print()
    if a.unit:
        r = next((x for x in rep if x["unit"] == a.unit), None)
        if r is None:
            sys.exit(f"no unit {a.unit}")
        print(f"{r['unit']} (after {r['prev']}): start edge "
              f"{'possible' if r['start_possible'] else 'IMPOSSIBLE ' + str(r['start_detail'])}"
              f"{'' if r['start_soft'] else ', soft-unlikely ' + str(r['start_soft_detail'])}")
        for j in range(r["first"], r["last"] + 1):
            row = rows[j]
            p = ", ".join(f"{k}@{x:08X}" for x, k in row["kinds"])
            print(f"  {row['addr']:08X} {row['name']:<44} {p}")
            if j < r["last"]:
                g = gaps[j]
                mark = ("   ==== same file (rodata)" if not g["possible"] else
                        "   ~~~~ boundary unlikely (single-user data)" if not g["soft"] else
                        "   ---- boundary possible")
                inside = [f for f in forced if f[0] <= j <= f[1]]
                if inside:
                    mark += f"   (a forced boundary lies in this stretch: tables {inside[0][2]} / {inside[0][3]})"
                print(mark)
        return
    print("Game units, edge to the previous run and what lies inside ('!' = decided by rodata, "
          "'~' = soft evidence only):")
    print()
    for r in rep:
        if r["unit"].startswith("sony:"):
            continue
        notes = []
        if r["start_possible"] is False:
            notes.append(f"! one file with {r['prev']} (rodata {r['start_detail'][0]} >= {r['start_detail'][1]})")
        elif r["start_soft"] is False:
            notes.append(f"~ probably one file with {r['prev']} (single-user data "
                         f"{r['start_soft_detail'][0]} >= {r['start_soft_detail'][1]})")
        for s_ in r["forced_inside"]:
            notes.append(f"! SPLIT: a file boundary lies between {s_[0]} and {s_[1]} (tables {s_[2]} / {s_[3]})")
        if notes or a.units:
            print(f"  {r['unit']:<18} {len(r['functions']):>3} fn; inner gaps possible "
                  f"{r['inner_possible']}/{r['inner_gaps']} (and soft-clean {r['inner_soft']})")
            for s_ in notes:
                print(f"      {s_}")


if __name__ == "__main__":
    main()
