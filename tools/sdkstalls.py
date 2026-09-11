#!/usr/bin/env python3
"""Cross the live INCLUDE_ASM stall queue against Sony objects already placed
inside GAME segments.

Why this exists
---------------
`psyq_sdk.py coverage` has always printed a section headed "Placed objects that
fall inside GAME-code segments (SDK code miscounted as game)". It is telling
the truth and nobody crossed it against the STALL QUEUE, so functions Sony
owns kept attracting decompilation attempts as if they were game code.

That is expensive in the direction the project cares about. A function inside a
placed object cannot be matched by writing C -- the bytes come from Sony's
object, not from anything cc1 will produce from a game source file -- so every
attempt is spent for certain. Round 20 lost a sitting to `func_8003FC70`
(libgs/gs_108.o) this way; round 32 found fourteen more, carrying several
thousand lines of accumulated derivation between them.

CLAUDE.md already states the rule ("Never write C for a function a Sony object
owns") and already names the command to check it. The gap was never the rule,
it was that checking meant reading a 60-line object list against a 200-line
queue by hand, per round, and noticing an overlap in hex. This does that.

Usage
-----
    python3 tools/sdkstalls.py            # the overlaps, worst first
    python3 tools/sdkstalls.py --all      # plus every placed object in a game segment

A FULLY-contained hit is conclusive: convert it per docs/SDK-OBJECTS-GUIDE.md
rather than decompiling it. A PARTIAL hit means the function straddles an
object boundary, which usually means a mis-carve or a mis-placed object -- read
it before acting, and do not assume either half.
"""
import argparse, glob, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
PY = str(ROOT / ".venv/bin/python3")


def placed_objects():
    """Objects psyq_sdk.py reports as sitting inside GAME-code segments."""
    out = subprocess.run([PY, str(ROOT / "tools/psyq_sdk.py"), "coverage"],
                         capture_output=True, text=True, cwd=ROOT).stdout
    pat = re.compile(r"^\s+(\S+\.o)\s+size=(0x[0-9a-f]+)\s+vram=(0x[0-9a-f]+)"
                     r"\s+(\S+)\s+\((.*?)\)", re.M)
    return [(m.group(1), int(m.group(2), 16), int(m.group(3), 16),
             m.group(4), m.group(5)) for m in pat.finditer(out)]


def live_functions():
    """Every live INCLUDE_ASM symbol with its vram extent.

    Anchored on the function's own `glabel`/`endlabel` and NOT on the whole
    file: several .s files carry trailing data or jump-table sections whose
    addresses are nowhere near the code, and a naive min/max over the file
    reports extents of tens of thousands of words. That bug produced a
    143-hit census on its first run, against a true 14.
    """
    insn = re.compile(r"\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/")
    out = []
    for c in sorted(glob.glob(str(ROOT / "src/*.c"))):
        unit = pathlib.Path(c).stem
        src = pathlib.Path(c).read_text()
        for fn in re.findall(r'^INCLUDE_ASM\("[^"]*",\s*(\w+)\)', src, re.M):
            s = ROOT / f"asm/nonmatchings/{unit}/{fn}.s"
            if not s.exists():
                continue
            lines = s.read_text().splitlines()
            try:
                i = next(k for k, l in enumerate(lines)
                         if l.startswith(f"glabel {fn}"))
            except StopIteration:
                continue
            vs = []
            for l in lines[i + 1:]:
                if l.startswith("endlabel"):
                    break
                m = insn.match(l)
                if m:
                    vs.append(int(m.group(1), 16))
            if vs:
                out.append((fn, unit, min(vs), max(vs) + 4))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true",
                    help="also list every placed object inside a game segment")
    args = ap.parse_args()

    objs, live = placed_objects(), live_functions()
    hits = []
    for fn, unit, lo, hi in live:
        for name, size, vram, seg, disc in objs:
            if lo < vram + size and hi > vram:
                hits.append((fn, unit, lo, (hi - lo) // 4, name, disc,
                             lo >= vram and hi <= vram + size))
    hits.sort(key=lambda h: -h[3])

    print(f"live INCLUDE_ASM queue: {len(live)}   "
          f"placed objects inside game segments: {len(objs)}")
    if not hits:
        print("\nNo live stalled function overlaps a placed Sony object.")
        return 0

    words = sum(h[3] for h in hits)
    print(f"\n*** {len(hits)} LIVE STALLED FUNCTIONS ARE SONY LIBRARY CODE "
          f"({words} words) ***")
    print("These cannot be matched by writing C. Convert them per "
          "docs/SDK-OBJECTS-GUIDE.md;\ndo NOT staff a runner onto them. "
          "Report length is the derivation already spent.\n")
    for fn, unit, lo, sz, name, disc, full in hits:
        rep = ROOT / f"docs/match-reports/{fn}.md"
        n = len(rep.read_text().splitlines()) if rep.exists() else 0
        print(f"  {'FULLY' if full else 'PARTIAL'} {fn:22} {unit:16} "
              f"{lo:#010x} {sz:>4}w  <- {name:22} ({disc})  report={n}L")

    if args.all:
        print("\nAll placed objects inside game segments:")
        for name, size, vram, seg, disc in sorted(objs, key=lambda o: o[2]):
            print(f"  {name:24} {vram:#010x} size={size:#x:>8} {seg:16} ({disc})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
