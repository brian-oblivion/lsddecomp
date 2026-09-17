#!/usr/bin/env python3
"""Identify an unnamed Psy-Q function in retail by fingerprinting it against
every function in every SDK object on every disc in sdk/.

    .venv/bin/python3 tools/sdkname.py func_8003B20C [func_...]   # rank candidates
    .venv/bin/python3 tools/sdkname.py --all                       # every game-called unnamed SDK function
    .venv/bin/python3 tools/sdkname.py --selfcheck 10              # prove the tool on placed functions

WHY THIS EXISTS (FINISHING-PLAN.md, track 2). `tools/psyq_sdk.py match` places
a whole OBJECT only when its relocation-masked .text is byte-identical to
retail. The game linked some libraries from a build none of the four discs
carries (libgpu/libcd, December 1995), so those objects never place, and the
functions inside them stay `func_XXXXXXXX` even though a later build of the
same function is sitting on a disc, ninety percent identical. Game code calls
46 of them, and the goal is the NAME, not a byte match.

HOW IT SCORES. For each function symbol in each converted object
(sdk/work/<ver>/elf/**/*.o), the object's words with every relocated field
masked out (R_MIPS_26: low 26 bits; HI16/LO16/GPREL16/16: low 16 bits;
R_MIPS_32: the whole word), against the retail words with the SAME fields
masked. Two figures per candidate:

  exact   same length AND every masked word equal: the strongest evidence,
          this is the very build (the object just did not place as a whole,
          usually because a sibling function in it differs).
  masked  fraction of masked words equal at the same position, same length only.
  shape   SequenceMatcher ratio over opcode-and-register skeletons (the word
          with every immediate field zeroed), any length. This is what finds a
          function from a different build: immediates and offsets move,
          instruction order mostly does not.

A candidate is printed with the disc(s) and module it came from, so the
evidence rule in the plan (fingerprint plus one independent kind, or a
fingerprint above the self-check threshold) can be applied and recorded in the
symbols-file comment.

--selfcheck takes N functions that ARE placed (config/psyq-objects.txt), hides
their names, and asks whether the top candidate is the right one. Run it
before believing the tool on an unplaced function; the printed threshold is
the lowest winning score among the checked functions.
"""
import argparse
import difflib
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

ROOT = Path(__file__).resolve().parent.parent
EXE = ROOT / "disk/SLPS_015.56"
WORK = ROOT / "sdk/work"
HDR, VRAM = 0x800, 0x80010000

R_MIPS_16, R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 1, 2, 4, 5, 6, 7
MASK_FOR = {R_MIPS_16: 0xFFFF0000, R_MIPS_HI16: 0xFFFF0000, R_MIPS_LO16: 0xFFFF0000,
            R_MIPS_GPREL16: 0xFFFF0000, R_MIPS_26: 0xFC000000, R_MIPS_32: 0}


def skeleton(w):
    """The instruction with its immediate zeroed: opcode, registers, funct."""
    op = w >> 26
    if op == 0:                      # SPECIAL: keep everything but shamt is fine
        return w & 0xFFFFF83F
    if op in (2, 3):                 # j / jal
        return w & 0xFC000000
    return w & 0xFFFF0000            # I-type: drop the immediate


# --- the corpus: every function in every converted object -------------------

def object_functions(opath):
    """[(name, words, masks)] for each function symbol in the object's .text."""
    from elftools.elf.elffile import ELFFile
    with open(opath, "rb") as f:
        elf = ELFFile(f)
        text = elf.get_section_by_name(".text")
        symtab = elf.get_section_by_name(".symtab")
        if text is None or symtab is None or text.data_size == 0:
            return []
        tidx = [i for i, s in enumerate(elf.iter_sections()) if s.name == ".text"][0]
        data = text.data()
        n = len(data) // 4
        words = list(struct.unpack_from(f"<{n}I", data, 0))
        masks = [0xFFFFFFFF] * n
        rel = elf.get_section_by_name(".rel.text")
        if rel is not None:
            for r in rel.iter_relocations():
                off, typ = r["r_offset"], r["r_info_type"]
                if off // 4 < n and typ in MASK_FOR:
                    masks[off // 4] &= MASK_FOR[typ]
        syms = [(s["st_value"], s.name) for s in symtab.iter_symbols()
                if s["st_shndx"] == tidx and s.name and not s.name.startswith(".")
                and s["st_info"]["type"] in ("STT_FUNC", "STT_NOTYPE", "STT_OBJECT")]
        syms = sorted(set(syms))
        out = []
        for i, (val, name) in enumerate(syms):
            end = syms[i + 1][0] if i + 1 < len(syms) else len(data)
            a, b = val // 4, end // 4
            if b > a:
                out.append((name, words[a:b], masks[a:b]))
        return out


def corpus():
    """{(lib/module, func): {'words','masks','skel','discs'}} across all discs.
    Identical bodies from several discs collapse into one entry."""
    entries = {}
    for ver in sorted(p.name for p in WORK.iterdir() if (p / "elf").is_dir()):
        for o in sorted((WORK / ver / "elf").rglob("*.o")):
            rel = o.relative_to(WORK / ver / "elf").with_suffix("")
            for name, words, masks in object_functions(o):
                key = (str(rel), name, tuple(words), tuple(masks))
                e = entries.setdefault(key, {"words": words, "masks": masks,
                                             "skel": [skeleton(w) for w in words],
                                             "discs": []})
                e["discs"].append(ver)
    return entries


# --- the query: retail words for a function --------------------------------

def retail_functions_in_psyq_asm():
    """{name: (vram, words)} for every glabel in asm/psyq_*.s."""
    out = {}
    for s in sorted(ROOT.glob("asm/psyq_*.s")):
        text = s.read_text(errors="replace")
        for m in re.finditer(r"^glabel (\w+)\n(.*?)^endlabel \1$", text, re.M | re.S):
            # splat prints the word as STORED (little-endian bytes): `4404828F`
            # is 0x8F820444. Swap, or every query is nonsense (found by the
            # first --all run scoring 0.07 against DrawSync at equal length).
            words = [int.from_bytes(bytes.fromhex(w), "little") for w in re.findall(
                r"^\s*/\* [0-9A-F]+ [0-9A-F]{8} ([0-9A-F]{8}) \*/", m.group(2), re.M)]
            vm = re.search(r"/\* [0-9A-F]+ ([0-9A-F]{8}) ", m.group(2))
            if words and vm:
                out[m.group(1)] = (int(vm.group(1), 16), words)
    return out


def retail_words_at(vram, nwords):
    exe = EXE.read_bytes()
    off = vram - VRAM + HDR
    return list(struct.unpack_from(f"<{nwords}I", exe, off))


# --- scoring ----------------------------------------------------------------

def score(qwords, cand):
    cw, cm = cand["words"], cand["masks"]
    same_len = len(cw) == len(qwords)
    masked = exact = 0.0
    if same_len and cw:
        eq = sum(1 for q, w, m in zip(qwords, cw, cm) if (q & m) == (w & m))
        masked = eq / len(cw)
        exact = 1.0 if eq == len(cw) else 0.0
    qs = [skeleton(w) for w in qwords]
    sm = difflib.SequenceMatcher(None, qs, cand["skel"], autojunk=False)
    shape = sm.ratio()
    return exact, masked, shape


def rank(qwords, corp, top=5):
    rows = []
    n = len(qwords)
    for (mod, name, _, _), cand in corp.items():
        cn = len(cand["words"])
        if cn < 2 or cn < n * 0.5 or cn > n * 2.0:
            continue
        exact, masked, shape = score(qwords, cand)
        rows.append((exact, masked, shape, mod, name, cn, sorted(set(cand["discs"]))))
    rows.sort(key=lambda r: (r[0], r[1], r[2]), reverse=True)
    exact_rows = [r for r in rows if r[0]]
    return rows[:max(top, len(exact_rows))]


def placements():
    """[(vram_start, vram_end, module, ver)] of every placed object, from the
    manifest plus each object's .text size."""
    from elftools.elf.elffile import ELFFile
    out = []
    for line in (ROOT / "config/psyq-objects.txt").read_text().splitlines():
        m = re.match(r"^\s*(\S+)\s+(\S+)\s+(0x[0-9A-Fa-f]+)", line)
        if not m:
            continue
        o = ROOT / "lib" / f"{m.group(2)}.o"
        if not o.exists():
            continue
        with open(o, "rb") as f:
            t = ELFFile(f).get_section_by_name(".text")
            size = t.data_size if t else 0
        start = int(m.group(3), 16) - HDR + VRAM
        out.append((start, start + size, m.group(2), m.group(1)))
    return sorted(out)


def neighbours(vram, placed):
    before = max((p for p in placed if p[1] <= vram), default=None, key=lambda p: p[1])
    after = min((p for p in placed if p[0] > vram), default=None, key=lambda p: p[0])
    return before, after


def print_rank(label, vram, qwords, rows, placed=None):
    print(f"{label}  vram 0x{vram:08x}  {len(qwords)} words")
    if placed is not None:
        b, a = neighbours(vram, placed)
        bs = f"{b[2]} ({b[3]}) ends 0x{b[1]:08x}" if b else "none"
        as_ = f"{a[2]} ({a[3]}) starts 0x{a[0]:08x}" if a else "none"
        print(f"    position: after {bs}; before {as_}")
        print("              (SDK archives link modules in a fixed order: a function between two "
              "placed modules of ONE library is that library's module between them)")
    if not rows:
        print("    no candidate within 0.5x..2x of the length")
    exact_names = {r[4] for r in rows if r[0]}
    if len(qwords) <= 6 and len(exact_names) >= 1:
        print(f"    TINY ({len(qwords)}w): an exact match on a body this small is common to many "
              "stubs and is NOT identification on its own; position evidence is required.")
    elif len(exact_names) > 1:
        print(f"    AMBIGUOUS: {len(exact_names)} different names match exactly; position decides.")
    for exact, masked, shape, mod, name, cn, discs in rows:
        tag = "EXACT " if exact else "      "
        print(f"    {tag}masked {masked:4.2f}  shape {shape:4.2f}  {name:<28} {mod:<26} {cn:4d}w  discs {','.join(discs)}")


# --- self-check ---------------------------------------------------------------

def selfcheck(corp, n):
    """Hide the names of N placed functions; does the top candidate recover them?"""
    manifest = (ROOT / "config/psyq-objects.txt").read_text().splitlines()
    placed = []
    for line in manifest:
        m = re.match(r"^\s*(\S+)\s+(\S+)\s+(0x[0-9A-Fa-f]+)", line)
        if m:
            placed.append((m.group(1), m.group(2), int(m.group(3), 16)))
    # pick functions spread across libraries: first function of every Nth object
    picks = []
    step = max(1, len(placed) // n)
    for ver, mod, off in placed[::step]:
        o = ROOT / "lib" / f"{mod}.o"
        if not o.exists():
            continue
        funcs = object_functions(o)
        if not funcs:
            continue
        name, words, _ = max(funcs, key=lambda f: len(f[1]))   # the biggest one
        if len(words) < 6:
            continue
        # its retail address: object text offset + function offset
        foff = sum(len(f[1]) for f in funcs[:funcs.index((name, words, _))]) * 4
        vram = off - HDR + VRAM + foff
        picks.append((name, mod, vram, len(words)))
        if len(picks) >= n:
            break
    wins, floor = 0, 1.0
    for name, mod, vram, nw in picks:
        q = retail_words_at(vram, nw)
        rows = rank(q, corp, top=3)
        top = rows[0] if rows else None
        ok = top is not None and top[4] == name
        wins += ok
        if ok:
            floor = min(floor, top[1] if top[1] else top[2])
        print(f"  {'OK ' if ok else 'MISS'} {name:<26} ({mod}, {nw}w) -> "
              f"{top[4] if top else '-'} exact={int(top[0]) if top else '-'} "
              f"masked={top[1]:.2f} shape={top[2]:.2f}" if top else f"  MISS {name}: no candidates")
    print(f"\nself-check: {wins}/{len(picks)} recovered; lowest winning score {floor:.2f}")
    print("Treat a candidate below that floor as needing a second kind of evidence.")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("funcs", nargs="*", help="func_XXXXXXXX names in asm/psyq_*.s")
    ap.add_argument("--all", action="store_true",
                    help="every unnamed SDK function that game code calls (plan.py's list)")
    ap.add_argument("--selfcheck", type=int, metavar="N")
    ap.add_argument("--top", type=int, default=5)
    a = ap.parse_args()

    if not WORK.exists() or not EXE.exists():
        sys.exit("FATAL: needs sdk/work/<ver>/elf (tools/psyq_sdk.py match) and disk/SLPS_015.56")
    print("loading corpus ...", file=sys.stderr)
    corp = corpus()
    print(f"{len(corp)} distinct function bodies across "
          f"{len({k[0] for k in corp})} modules", file=sys.stderr)

    if a.selfcheck:
        selfcheck(corp, a.selfcheck)
        return

    names = list(a.funcs)
    if a.all:
        import json, subprocess
        out = subprocess.run([sys.executable, "tools/plan.py", "--json"],
                             capture_output=True, text=True, cwd=ROOT).stdout
        names += json.loads(out)["tracks"]["2"]["unnamed_list"]
    if not names:
        ap.error("give function names, --all, or --selfcheck N")

    retail = retail_functions_in_psyq_asm()
    placed = placements()
    for fn in names:
        if fn not in retail:
            print(f"{fn}: not a glabel in asm/psyq_*.s (already named, or game code)")
            continue
        vram, words = retail[fn]
        print_rank(fn, vram, words, rank(words, corp, a.top), placed)
        print()


if __name__ == "__main__":
    main()
