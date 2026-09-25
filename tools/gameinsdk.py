#!/usr/bin/env python3
"""Game code inside `psyq_*` segments: the inverse of config/sdk-in-game.txt.

    .venv/bin/python3 tools/gameinsdk.py            # every psyq_* function, classified, as runs
    .venv/bin/python3 tools/gameinsdk.py --write    # regenerate config/game-in-sdk.txt
    .venv/bin/python3 tools/gameinsdk.py --check    # exit 1 if it is stale

WHY THIS EXISTS (FINISHING-PLAN revision 18). `progress.py` counts a
`psyq_*` segment as library BY NAME, and the names were given by position:
a gap between two placed Sony objects was called "the game's own libspu
build, which no disc has". Measured 2026-09-25, ten of those segments held
game code instead -- methods listed in BasicClass-derived method tables and
functions that `jal` the game's framework -- and progress.py reported "100%
of game code matched" with over two hundred game functions never carved.
Revision 13 fixed the mirror image (Sony code in game-named segments, now
config/sdk-in-game.txt); this is the other direction.

EVIDENCE. A psyq_* function is GAME when any of these holds, iterated to a
fixed point:

  call    it `jal`s or `j`s a function that is game code. Sony's libraries
          never call the game by name; a linked Sony object that calls game
          code does it through a psyq-objects.ld pin, and those targets are
          library (progress.is_library), so they are not evidence.
  table   it sits in a run of function pointers in data (a method table)
          together with a game function.
  bracket it lies in a run of unclassified functions whose neighbours on
          both sides, inside the same segment, are game. An object file is
          contiguous in the link.
  edge    it lies in such a run that touches game code on ONE side, and
          every referrer of the run (caller or `lui`/`addiu` address-taker)
          from outside it is game, with at least one. The adjacency is what
          makes this sound: "all its callers are game" alone describes every
          Sony API the game calls (DrawSync, ClearImage), so it is never used
          on its own.

GUARD. Whatever the evidence, a function stays SONY when it carries a Sony
name the SDK corpus knows, a psyq-objects.ld pin, an EXACT relocation-masked
fingerprint of at least sdkname.GAME_MIN_WORDS words whose candidates agree on
one name or one module, or a shape lead (>= sdkname.LEAD_SHAPE at >=
sdkname.LEAD_MIN_WORDS words). Game evidence on a guarded function is printed
as a CONFLICT and needs a person. Tiny exact matches (a 4-word `jr $ra`
stub matches dozens of Sony stubs) guard nothing, as in sdkname.py --game.

THE FILE. config/game-in-sdk.txt lists every GAME function still inside a
psyq_* segment. progress.py counts them as game (so they show as uncarved
ground, and uncarved.py lists them), which is the honest state until they are
carved into C units (PARALLEL-RUNS Gate 2). After a complete carve it lists
none, and `--check` keeps it that way: a new psyq_* segment that holds game
code makes it stale.
"""
import argparse
import bisect
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import progress  # noqa: E402

ROOT = progress.ROOT
EXE = ROOT / "disk/SLPS_015.56"
OUT = ROOT / "config/game-in-sdk.txt"
LDFRAG = ROOT / "config/psyq-objects.ld"
VRAM, HDR = 0x80010000, 0x800
BASIC_CLASS = (0x80017EB0, 0x80018100)   # BasicClass's own methods (include/BasicClass.h)


def segments():
    """[(vram, kind, name)] of every subsegment, sorted."""
    out = []
    for m in re.finditer(r"^\s+- \[\s*(0x[0-9A-Fa-f]+)\s*,\s*([.\w]+)(?:\s*,\s*([\w/]+))?",
                         progress.YAML.read_text(), re.M):
        out.append((int(m.group(1), 16) - HDR + VRAM, m.group(2), m.group(3) or ""))
    return sorted(out)


class Image:
    def __init__(self):
        self.exe = EXE.read_bytes()
        info = progress.text_symbols()
        import classtable
        self.funcs = sorted((a, n, s) for n, (a, s) in info.items()
                            if s > 0 and classtable.TEXT[0] <= a < classtable.TEXT[1])
        self.name = {a: n for a, n, _ in self.funcs}
        self.size = {a: s for a, _, s in self.funcs}
        self.segs = segments()
        self._segstart = [s[0] for s in self.segs]
        self.refs = {a: set() for a, _, _ in self.funcs}    # target -> referrers
        self.calls = {a: set() for a, _, _ in self.funcs}   # caller -> targets
        for a, _, s in self.funcs:
            hi = {}
            for k in range(0, s, 4):
                x = self.word(a + k)
                op = x >> 26
                if op in (2, 3):
                    t = ((a + k) & 0xF0000000) | ((x & 0x3FFFFFF) << 2)
                    if t in self.refs and t != a:
                        self.calls[a].add(t)
                        self.refs[t].add(a)
                elif op == 15:                               # lui
                    hi[(x >> 16) & 31] = (x & 0xFFFF) << 16
                elif op == 9 and (x >> 21) & 31 in hi:       # addiu off a lui
                    t = (hi[(x >> 21) & 31] + ((x & 0xFFFF ^ 0x8000) - 0x8000)) & 0xFFFFFFFF
                    if t in self.refs and t != a:
                        self.refs[t].add(a)

    def word(self, vram):
        return struct.unpack_from("<I", self.exe, vram - VRAM + HDR)[0]

    def seg(self, vram):
        return self.segs[bisect.bisect_right(self._segstart, vram) - 1]

    def tables(self):
        """[[function vram, ...]]: runs of function pointers (or nulls) in
        .data, the method tables classtable.py --scan shows and shorter ones."""
        import classtable
        v, end, out = classtable.DATA[0], VRAM + len(self.exe) - HDR, []
        while v < end:
            if self.word(v) in self.size:
                run = []
                while v < end and (self.word(v) in self.size or self.word(v) == 0):
                    if self.word(v):
                        run.append(self.word(v))
                    v += 4
                if len(run) >= 2:
                    out.append(run)
            else:
                v += 4
        return out


def ld_pins():
    return {int(m.group(2), 16) for m in re.finditer(
        r"^(\w+) = (0x[0-9A-Fa-f]{8});", LDFRAG.read_text(), re.M)} if LDFRAG.exists() else set()


def sony_guard(img, psy):
    """{vram: reason} for psyq_* functions with Sony evidence."""
    import sdkname
    corp = sdkname.corpus()
    sony_names = {k[1] for k in corp}
    pins = ld_pins()
    out = {}
    for a in psy:
        n = img.size[a] // 4
        if img.name[a] in sony_names:
            out[a] = f"name {img.name[a]}"
            continue
        if a in pins:
            out[a] = "psyq-objects.ld pin"
            continue
        if n < sdkname.GAME_MIN_WORDS:
            continue
        rows = sdkname.rank([img.word(a + 4 * i) for i in range(n)], corp, top=10 ** 6)
        ex = [r for r in rows if r[0]]
        if ex and (len({r[4] for r in ex}) == 1 or len({r[3] for r in ex}) == 1):
            out[a] = f"exact {ex[0][3]}:{ex[0][4]}"
            continue
        best = max(rows, key=lambda r: r[2]) if rows else None
        if best and best[2] >= sdkname.LEAD_SHAPE and n >= sdkname.LEAD_MIN_WORDS:
            out[a] = f"shape {best[2]:.2f} {best[3]}:{best[4]}"
    return out


def classify():
    img = Image()
    psy_ranges = []
    for i, (v, kind, nm) in enumerate(img.segs):
        if kind != "o" and nm.startswith("psyq_") and i + 1 < len(img.segs):
            psy_ranges.append((v, img.segs[i + 1][0], nm))

    def psyseg(a):
        return next((nm for lo, hi, nm in psy_ranges if lo <= a < hi), None)

    psy = [a for a, _, _ in img.funcs if psyseg(a)]
    guard = sony_guard(img, psy)
    game, why = set(), {}

    def is_game(a):
        if a in game:
            return True
        return not progress.is_library(a) and not psyseg(a)

    tables = img.tables()
    changed = True
    while changed:
        changed = False
        for t in tables:
            if any(is_game(x) or BASIC_CLASS[0] <= x < BASIC_CLASS[1] for x in t):
                for x in t:
                    if psyseg(x) and x not in game and x not in guard:
                        game.add(x); why[x] = "table"; changed = True
        for a in psy:
            if a in game or a in guard:
                continue
            if any(is_game(t) for t in img.calls[a]):
                game.add(a); why[a] = "call"; changed = True
        # bracket and edge, per segment
        for lo, hi, nm in psy_ranges:
            fs = [a for a in psy if lo <= a < hi]
            known = lambda b: b in game or b in guard  # noqa: E731
            i = 0
            while i < len(fs):
                if known(fs[i]):
                    i += 1
                    continue
                j = i
                while j < len(fs) and not known(fs[j]):
                    j += 1
                run = fs[i:j]                     # a maximal unclassified run
                left = fs[i - 1] if i > 0 else None
                right = fs[j] if j < len(fs) else None
                refs = set().union(*(img.refs[a] for a in run)) - set(run)
                if left in game and right in game:
                    tag = "bracket"
                elif (left in game or right in game) and refs and all(is_game(x) for x in refs):
                    tag = "edge"
                else:
                    tag = None
                if tag:
                    for a in run:
                        game.add(a); why[a] = tag
                    changed = True
                i = j
    conflicts = []
    for a in psy:
        if a in guard and (any(is_game(t) for t in img.calls[a])):
            conflicts.append((a, guard[a], "calls game code"))
    return img, psy, psyseg, game, why, guard, conflicts


def body(img, game, why, psyseg):
    lines = ["# GENERATED by `.venv/bin/python3 tools/gameinsdk.py --write`. Do not edit.",
             "# GAME functions still inside `psyq_*` segments (the inverse of",
             "# config/sdk-in-game.txt; evidence rules in the tool's docstring).",
             "# progress.py counts them as game code, uncarved, until a carve moves",
             "# them into a C unit. Keyed by vram, so a rename never makes this stale.",
             "# vram        words  evidence  segment"]
    for a in sorted(game):
        lines.append(f"0x{a:08X}  {img.size[a] // 4:5d}  {why[a]:<8}  {psyseg(a)}")
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--check", action="store_true")
    a = ap.parse_args()
    if not EXE.exists() or not progress.ELF.exists():
        sys.exit("FATAL: needs disk/SLPS_015.56 and build/lsdde.elf (./build-and-verify.sh)")
    img, psy, psyseg, game, why, guard, conflicts = classify()
    text = body(img, game, why, psyseg)
    if a.check:
        cur = OUT.read_text() if OUT.exists() else ""
        if cur != text:
            sys.exit(f"{OUT.relative_to(ROOT)} is stale -- run tools/gameinsdk.py --write")
        print(f"OK: {OUT.relative_to(ROOT)} current ({len(game)} game functions in psyq_* segments)")
        return
    if a.write:
        OUT.write_text(text)
        print(f"wrote {OUT.relative_to(ROOT)} ({len(game)} functions)", file=sys.stderr)
    # the report: runs per segment
    cur = None
    for s in psy:
        nm = psyseg(s)
        if nm != cur:
            if cur is not None:
                print(f"    {run_desc(run)}")
            print(f"{nm}")
            cur, run = nm, []
        tag = "GAME" if s in game else ("SONY" if s in guard else "?")
        if run and run[-1][0] != tag:
            print(f"    {run_desc(run)}")
            run = []
        run.append((tag, s))
    if cur is not None:
        print(f"    {run_desc(run)}")
    for c, g, r in conflicts:
        print(f"CONFLICT 0x{c:08X} {img.name[c]}: Sony by {g}, but {r}")
    print(f"{len(game)} game function(s) in psyq_* segments; {len(guard)} Sony-guarded; "
          f"{len(psy) - len(game) - len(guard)} unclassified; {len(conflicts)} conflict(s).")


def run_desc(run):
    tag, a0 = run[0]
    a1 = run[-1][1]
    fo = lambda v: v - VRAM + HDR  # noqa: E731
    return f"{tag:4} {len(run):4d}  0x{a0:08X}..0x{a1:08X}  (file 0x{fo(a0):X}..)"


if __name__ == "__main__":
    main()
