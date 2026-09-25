#!/usr/bin/env python3
"""Identify an unnamed Psy-Q function in retail by fingerprinting it against
every function in every SDK object on every disc in sdk/.

    .venv/bin/python3 tools/sdkname.py func_8003B20C [func_...]   # rank candidates
    .venv/bin/python3 tools/sdkname.py --all                       # every game-called unnamed SDK function
    .venv/bin/python3 tools/sdkname.py --selfcheck 10              # prove the tool on placed functions
    .venv/bin/python3 tools/sdkname.py --game [--write|--check]    # Sony code inside GAME segments

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
    """{name: (vram, words)} for every glabel in asm/psyq_*.s, plus every
    still-INCLUDE_ASM game function in asm/nonmatchings/ (round 51 wanted to
    fingerprint func_80018464, a game-segment stall, to settle its ownership)."""
    out = {}
    for s in sorted(ROOT.glob("asm/psyq_*.s")) + sorted(ROOT.glob("asm/nonmatchings/**/*.s")):
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
    out = rows[:max(top, len(exact_rows))]
    # The best SHAPE is often not the best masked score (another build: every
    # immediate moved), and round 73's screen read only the top line and missed
    # a 0.99. Always carry it; print_rank flags it.
    if rows:
        best = max(rows, key=lambda r: r[2])
        if best not in out:
            out.append(best)
    return out


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
    best_shape = max((r[2] for r in rows), default=None)
    for exact, masked, shape, mod, name, cn, discs in rows:
        tag = "EXACT " if exact else ("SHAPE " if shape == best_shape and shape >= LEAD_SHAPE else "      ")
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


# --- --game: Sony code inside GAME segments ---------------------------------

SDK_IN_GAME = ROOT / "config/sdk-in-game.txt"
# Below this an exact fingerprint is not identification: Class6D4E8__StopCdService,
# a game method, matches three Sony stubs exactly at 12 words, and every 8-word
# jal-wrapper matches a dozen. Nothing unique and genuinely Sony was lost by it.
GAME_MIN_WORDS = 12


def game_segment_functions():
    """{name: (vram, words)} for every function the ELF places outside the
    psyq_*/`o` segments, matched C and INCLUDE_ASM alike."""
    import subprocess
    import progress
    nm = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
    out = subprocess.run([str(nm), "-S", str(ROOT / "build/lsdde.elf")],
                         capture_output=True, text=True, check=True).stdout
    funcs = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) != 4 or p[2] not in "Tt" or progress.NOT_A_FUNCTION.search(p[3]):
            continue
        vram, size = int(p[0], 16), int(p[1], 16)
        if size >= 8 and not progress.in_library_segment(vram):
            funcs[p[3]] = (vram, size // 4)
    return funcs


def game_screen(corp):
    """[(vram, words, name, [(module, sony_name, discs)])]: EXACT fingerprints
    of at least GAME_MIN_WORDS whose candidates agree on one name or one module.
    Exact-only, so candidates are looked up by length and masked words instead
    of scored: seconds, not the minutes a full rank over every function takes."""
    index = defaultdict(lambda: defaultdict(list))   # (len, masks) -> masked words -> cands
    for (mod, name, _, _), c in corp.items():
        if len(c["words"]) >= GAME_MIN_WORDS:
            m = tuple(c["masks"])
            index[(len(m), m)][tuple(w & k for w, k in zip(c["words"], m))].append(
                (mod, name, sorted(set(c["discs"]))))
    by_len = defaultdict(list)
    for (n, m) in index:
        by_len[n].append(m)
    import progress
    pins = progress.ld_pinned_externals()   # counted through the pin already; and a
    hits = []                                # function renamed onto its pin links as `A`
    for fname, (vram, n) in sorted(game_segment_functions().items(), key=lambda kv: kv[1][0]):
        if n < GAME_MIN_WORDS or vram in pins:   # with no size, so listing it would churn
            continue
        q = retail_words_at(vram, n)
        cands = []
        for m in by_len.get(n, ()):
            cands += index[(n, m)].get(tuple(w & k for w, k in zip(q, m)), [])
        if not cands:
            continue
        if len({c[1] for c in cands}) == 1 or len({c[0] for c in cands}) == 1:
            hits.append((vram, n, fname, sorted(set((c[0], c[1], tuple(c[2])) for c in cands))))
    return hits


# LEADS (plan revision 14). Rounds 71-73 matched libsnd functions as game C
# because no disc carries their build, so no fingerprint is EXACT. Shape finds
# them: measured 2026-09-24 over every game-segment function, the 23 at shape
# >= 0.90 and >= 40 words are all libsnd vmanager/seqread/ssinit, and below
# that cliff only stubs of 18 words or fewer, against unrelated libraries.
# A body smaller than that qualifies only when the placed objects on BOTH
# sides are the candidate's library.
LEAD_SHAPE, LEAD_MIN_WORDS = 0.90, 40


def game_leads(corp, exclude):
    """[(vram, words, name, module, sony_name, shape)] shape leads among the
    game-segment functions NOT in `exclude` (the exact hits and the ld pins:
    both static, so this list does not move when track 2 renames a lead)."""
    placed = placements()
    out = []
    for fname, (vram, n) in sorted(game_segment_functions().items(), key=lambda kv: kv[1][0]):
        if n < GAME_MIN_WORDS or vram in exclude:
            continue
        rows = rank(retail_words_at(vram, n), corp, top=10 ** 6)
        if not rows:
            continue
        best = max(rows, key=lambda r: r[2])
        lib = best[3].split("/")[0]
        b, a = neighbours(vram, placed)
        bracket = b is not None and a is not None and b[2].split("/")[0] == lib == a[2].split("/")[0]
        if best[2] >= LEAD_SHAPE and (n >= LEAD_MIN_WORDS or bracket):
            out.append((vram, n, fname, best[3], best[4], best[2]))
    return out


def game_method_tables():
    """Addresses of the method tables classtable.py finds in .data."""
    import subprocess
    out = subprocess.run([sys.executable, str(ROOT / "tools/classtable.py"), "--scan"],
                         capture_output=True, text=True, cwd=ROOT).stdout
    return {int(a, 16) for a in re.findall(r"^\s+(0x[0-9A-Fa-f]{8})\s+\d+ slots", out, re.M)}


def builds_address(words):
    """Addresses a body forms with lui rX,hi then addiu/ori/lw..(rX) lo."""
    out, hi = set(), {}
    for w in words:
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
        if op == 0x0F:                                   # lui
            hi[rt] = imm << 16
        elif rs in hi and op in (0x09, 0x0D, 0x23, 0x21, 0x25, 0x20, 0x24):
            lo = imm - 0x10000 if (imm & 0x8000 and op != 0x0D) else imm
            out.add((hi[rs] + lo) & 0xFFFFFFFF)
    return out


def adjacency_leads(corp, library):
    """Revision 16: a SMALL body (4 words or more; 2-3 only when Sony on both sides, revision 19) with an exact fingerprint too
    common to identify alone (round 78's GsSetNearClip, SsInit, SpuInitHot, all
    TINY or AMBIGUOUS) is a lead when it TOUCHES Sony code: the function or
    placed object right before or after it is already library, and one of its
    exact candidates is from the library of the placed object on that side.
    Iterated, so a run of such wrappers chains. `library` is the static set
    (exact hits, pins, shape leads). [(vram, words, name, [(module, sony)])]"""
    placed = placements()
    funcs = game_segment_functions()
    info = {v: (nm, w) for nm, (v, w) in funcs.items()}
    starts = sorted(set(info) | {pl[0] for pl in placed})
    ends = {pl[1] for pl in placed}
    tables = game_method_tables()
    cand = {}
    for vram, (nm, w) in info.items():
        if w < 2 or vram in library:
            continue
        if builds_address(retail_words_at(vram, w)) & tables:
            continue     # a Get*Methods getter: CdLastPos's shape, the game's table (func_80047900)
        rows = [r for r in rank(retail_words_at(vram, w), corp, top=10 ** 6) if r[0]]
        if rows:
            cand[vram] = rows
    found, changed = {}, True
    while changed:
        changed = False
        for vram, rows in sorted(cand.items()):
            if vram in found:
                continue
            nm, w = info[vram]
            b, a = neighbours(vram, placed)
            i = starts.index(vram)
            prev = starts[i - 1] if i else None
            nxt = starts[i + 1] if i + 1 < len(starts) else None
            side = set()
            before = b and (vram in ends or prev in library or prev in found)
            after = a and (a[0] == vram + 4 * w or nxt in library or nxt in found)
            # Revision 19: a 2-3 word stub (jr ra; nop) matches dozens of
            # bodies, so it qualifies only SANDWICHED, Sony on both sides
            # (round 79's KeyOnCheck, between two identified vmanager functions).
            if w < 4 and not (before and after):
                continue
            if before:
                side.add(b[2].split("/")[0])
            if after:
                side.add(a[2].split("/")[0])
            hit = sorted({(r[3], r[4]) for r in rows if r[3].split("/")[0] in side})
            if hit:
                found[vram] = (w, nm, hit)
                changed = True
    return [(v, w, nm, hit) for v, (w, nm, hit) in sorted(found.items())]


def game_file_body(hits, leads=(), adj=()):
    lines = ["# GENERATED by `.venv/bin/python3 tools/sdkname.py --game --write`. Do not edit.",
             "# Functions in GAME segments whose retail bytes are an EXACT relocation-masked",
             "# fingerprint of a function in an SDK object on a disc in sdk/ (at least",
             f"# {GAME_MIN_WORDS} words; candidates agree on one name or one module). progress.py",
             "# counts them as library, so they leave the game-code counts and every",
             "# matching, promotion and naming queue; plan.py offers their Sony names to",
             "# track 2. Keyed by vram, so a rename never makes this stale.",
             "# vram        words  candidates (module:function@discs)"]
    for vram, n, _, cands in hits:
        cs = " ".join(f"{m}:{s}@{','.join(d)}" for m, s, d in cands)
        lines.append(f"0x{vram:08X}  {n:5d}  {cs}")
    lines += ["#",
              f"# LEADS: no exact fingerprint, but shape >= {LEAD_SHAPE} against one SDK function at",
              f"# >= {LEAD_MIN_WORDS} words (or bracketed by placed objects of its library). Counted as",
              "# library, and so out of every matching and naming queue, until track 2",
              "# confirms one (`identified` symbols comment, rename) or rejects it with a",
              "# `// not SDK: <reason>` comment line above its symbols entry.",
              "# LEAD vram   words  shape  best candidate (module:function)"]
    for vram, n, _, mod, sony, shape in leads:
        lines.append(f"LEAD 0x{vram:08X}  {n:5d}  {shape:.2f}  {mod}:{sony}")
    lines += ["#",
              "# ADJACENCY LEADS (revision 16): a body of 4+ words whose exact fingerprint",
              "# is too common to identify alone, touching Sony code, with a candidate from",
              "# the library on that side. Same status and closure as the leads above.",
              "# LEAD vram   words  shape  candidates from that library (module:function)"]
    for vram, n, _, hit in adj:
        lines.append(f"LEAD 0x{vram:08X}  {n:5d}  1.00  " + " ".join(f"{m}:{f}" for m, f in hit))
    return "\n".join(lines) + "\n"


def game_main(corp, write, check):
    import progress
    hits = game_screen(corp)
    static = {h[0] for h in hits} | set(progress.ld_pinned_externals())
    leads = game_leads(corp, static)
    adj = adjacency_leads(corp, static | {l[0] for l in leads})
    body = game_file_body(hits, leads, adj)
    if check:
        cur = SDK_IN_GAME.read_text() if SDK_IN_GAME.exists() else ""
        if cur != body:
            sys.exit(f"{SDK_IN_GAME.relative_to(ROOT)} is stale -- run tools/sdkname.py --game --write")
        print(f"OK: {SDK_IN_GAME.relative_to(ROOT)} current ({len(hits)} exact, {len(leads)} + {len(adj)} leads)")
        return
    if write:
        SDK_IN_GAME.write_text(body)
        print(f"wrote {SDK_IN_GAME.relative_to(ROOT)} ({len(hits)} exact, {len(leads)} leads)", file=sys.stderr)
    for vram, n, fname, cands in hits:
        cs = ", ".join(f"{s} ({m}, {','.join(d)})" for m, s, d in cands)
        print(f"  0x{vram:08x} {n:4d}w  {fname:<34} {cs}")
    print(f"{len(hits)} game-segment function(s) are Sony code by exact fingerprint.")
    for vram, n, fname, mod, sony, shape in leads:
        print(f"  LEAD 0x{vram:08x} {n:4d}w  {fname:<34} shape {shape:.2f}  {sony} ({mod})")
    print(f"{len(leads)} lead(s) by shape, for track 2 to confirm or reject.")
    for vram, n, fname, hit in adj:
        print(f"  LEAD 0x{vram:08x} {n:4d}w  {fname:<34} adjacent; " + ", ".join(f"{f} ({m})" for m, f in hit))
    print(f"{len(adj)} adjacency lead(s).")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("funcs", nargs="*", help="func_XXXXXXXX names in asm/psyq_*.s")
    ap.add_argument("--game", action="store_true",
                    help="screen every GAME-segment function (matched C too) for an exact "
                         "Sony fingerprint; with --write regenerate config/sdk-in-game.txt, "
                         "with --check exit 1 if it is stale")
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--check", action="store_true")
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
    if a.game:
        game_main(corp, a.write, a.check)
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
    game = None
    import progress
    pins = progress.ld_pinned_externals()
    for fn in names:
        if fn not in retail:
            # A matched C function (Sony code in a game unit, revision 13):
            # its retail words are wherever the byte-verified ELF put it.
            game = game if game is not None else game_segment_functions()
            if fn not in game:
                print(f"{fn}: not in asm/psyq_*.s, asm/nonmatchings/ or the ELF's game segments")
                continue
            vram, n = game[fn]
            retail[fn] = (vram, retail_words_at(vram, n))
        vram, words = retail[fn]
        print_rank(fn, vram, words, rank(words, corp, a.top), placed)
        if vram in pins:
            print(f"    PINNED: config/psyq-objects.ld names 0x{vram:08x} `{pins[vram]}` -- a linked "
                  "Sony object calls it by that name (relocation evidence)")
        print()


if __name__ == "__main__":
    main()
