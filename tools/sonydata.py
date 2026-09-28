#!/usr/bin/env python3
"""Game-style names on data only Sony code reads, and Sony's name for each.

    python3 tools/sonydata.py            # list them, with Sony's name where a disc gives one
    python3 tools/sonydata.py --check    # exit 1 if any (plan.py's track 2 reads this)

WHY. FINISHING-PLAN's rule is that nothing Sony owns gets a game name.
rename.py enforces it for variables config/psyq-objects.ld pins, which covers
only objects that PLACED. An object that never placed (libsnd/vmanager.o: its
functions were matched one at a time as C) has no pins, so its bss took game
names: round 86 found gVoiceEnv*/gVoiceFade* (fields of `_svm_voice`),
gVoiceActivityRing/Idx (`_svm_envx_hist`/`_ptr`) and more.

WHAT. A data symbol in the symbols file whose name follows the game convention
(`gName`/`sName`) and whose every accessor, read from the relocations in the
BUILT objects, is a function progress.py counts as library. A symbol with no
accessor found (no build, or referenced only by linked Sony objects) is not
listed: unknown is not Sony's.

SONY'S NAME. Each accessor is aligned, opcode by opcode, against the function
of the same name in every SDK object (sdk/work/*/elf); where our instruction
relocates against the flagged symbol, the aligned Sony instruction's
relocation names Sony's symbol (+addend when inside a table). A `.bss`/`.data`
target that stays UNRESOLVED means the variable is STATIC in Sony's object: no
disc carries its name, so the honest spelling is the placeholder, with an
`identified` comment saying whose it is. A section target is first resolved by its addend
against the object's own symbols, because these converted objects reach even
their named weak bss section-relative. Candidates are printed per disc.

EVERY ANSWER IS A LEAD. The ratio compares mnemonics and ignores addresses,
so a build can align 1.00 and still lay its data out differently (round 86:
3.3's SsSetTickMode aligns 1.00 and puts `_snd_seq_tick_mode` beside
`_snd_openflag`, 0x8008EA04, while retail reads 0x8006DCA4). A name becomes a
verdict when the object's layout lands the address: an anchor symbol already
pinned or identified in the same object at the right distance (vmanager.o's
bss from `_svm_sreg_buf`), or the object's initialised bytes found at the
address in retail (libapi/counter.o's .data at 0x8006DCAC), or the object's
variables found in retail in their declaration order (ssinit.o's at
0x8006DC8C..A8). Weak bss Sony's linker scattered (vm_g.o) has no layout to
land; there the verdict is every aligned access on every disc naming the same
symbol (`_svm_auto_kof_mode`, discs 3.0 to 3.6). Record the anchor in the
symbols file's `identified` comment.
"""
import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

SYMBOLS = ROOT / "config/symbols.slps01556.lsdde.txt"
OBJDUMP = ROOT / "tools/binutils/bin/mipsel-linux-gnu-objdump"
GAME_STYLE = re.compile(r"^[gs][A-Z]")


def data_names():
    """Game-style DATA names in the symbols file (func entries excluded)."""
    out = []
    for line in SYMBOLS.read_text().splitlines():
        m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)", line)
        if m and "type:func" not in m.group(3) and GAME_STYLE.match(m.group(1)):
            out.append((m.group(1), int(m.group(2), 16)))
    return out


def flagged():
    """[(name, addr, sorted accessor functions)] for game-named Sony data."""
    import progress
    import typeviews
    names = data_names()
    acc = typeviews.global_accessors([n for n, _ in names])
    syms = progress.text_symbols()
    return [(n, a, sorted(acc[n])) for n, a in names
            if acc[n] and all(f in syms and progress.is_library(syms[f][0]) for f in acc[n])]


def disasm(obj):
    """function -> [[mnemonic, reloc target or None], ...] from objdump -dr."""
    out = subprocess.run([str(OBJDUMP), "-dr", str(obj)], capture_output=True, text=True).stdout
    d, fn = {}, None
    for line in out.splitlines():
        m = re.match(r"^[0-9a-f]+ <(\S+)>:", line)
        if m:
            fn = m.group(1)
            d[fn] = []
            continue
        if fn is None:
            continue
        m = re.match(r"^\s+[0-9a-f]+:\s+([0-9a-f]{8})\s+(\S+)", line)
        if m:
            d[fn].append([m.group(2), None, int(m.group(1), 16)])
            continue
        m = re.search(r"R_MIPS_(HI16|LO16|GPREL16|26|32)\s+(\S+)", line)
        if m and d[fn]:
            tgt = m.group(2)
            if tgt.startswith(".") and m.group(1) in ("LO16", "GPREL16"):
                # REL: the addend is the instruction's own immediate. A
                # section-relative target is how these converted objects
                # reach even their NAMED weak bss (ssinit.o: `.bss+0x10` is
                # `_snd_video_mode`), so it is resolved below, never read as
                # "static" on sight.
                imm = d[fn][-1][2] & 0xFFFF
                tgt = f"{tgt}+{imm - 0x10000 if imm & 0x8000 else imm:#x}"
            d[fn][-1][1] = tgt
    return d


def section_names(obj):
    """{(section, offset): name} for the object's named data symbols."""
    out = subprocess.run([str(OBJDUMP), "-t", str(obj)], capture_output=True, text=True).stdout
    names = {}
    for line in out.splitlines():
        m = re.match(r"^([0-9a-f]{8}) .{7} (\.\w+)\s+[0-9a-f]{8} (\S+)$", line)
        if m and m.group(3) != m.group(2):
            names[(m.group(2), int(m.group(1), 16))] = m.group(3)
    return names


def resolve(tgt, names):
    """`.bss+0x10` -> the object's own name for it, `name+0x8` when it lies
    inside a named symbol that another named symbol bounds from above (these
    objects give weak bss no size: vmanager.o's `.bss+0x6c0` is
    `_ss_spu_vm_rec+0x8`), else unchanged: past the last name is unknown."""
    m = re.match(r"^(\.\w+)\+(-?0x[0-9a-f]+)$", tgt)
    if not m:
        return tgt
    sec, off = m.group(1), int(m.group(2), 16)
    starts = sorted(o for s_, o in names if s_ == sec)
    below = [o for o in starts if o <= off]
    above = [o for o in starts if o > off]
    if not below or not above:
        return tgt
    base = below[-1]
    return names[(sec, base)] + (f"+{off - base:#x}" if off != base else "")


def sony_names(rows):
    """name -> {(sony target, disc, object, function, ratio)}."""
    want = {f for _, _, fs in rows for f in fs}
    ours = {}
    for o in sorted((ROOT / "build/src").rglob("*.c.o")):
        ours.update({k: v for k, v in disasm(o).items() if k in want})
    sony = {}
    for o in sorted((ROOT / "sdk/work").glob("*/elf/*/*.o")):
        hits = {fn: body for fn, body in disasm(o).items() if fn in want}
        if hits:
            names = section_names(o)
            for fn, body in hits.items():
                for ins in body:
                    if ins[1]:
                        ins[1] = resolve(ins[1], names)
                sony.setdefault(fn, []).append((o, body))
    res = {}
    for name, _, fs in rows:
        for fn in fs:
            a = ours.get(fn)
            for o, b in sony.get(fn, []):
                if not a:
                    continue
                sm = difflib.SequenceMatcher(None, [x[0] for x in a], [x[0] for x in b], autojunk=False)
                for i, j, n in sm.get_matching_blocks():
                    for k in range(n):
                        ra, rb = a[i + k][1], b[j + k][1]
                        if ra and rb and ra.split("+")[0] == name:
                            disc = o.relative_to(ROOT / "sdk/work").parts[0]
                            obj = f"{o.parent.name}/{o.stem}"
                            res.setdefault(name, set()).add((rb, disc, obj, fn, round(sm.ratio(), 2)))
    return res


def verdict(cands):
    """The reading of the BEST-ALIGNED build (highest ratio; the retail build
    is the one that shape-matches best), a named target preferred on a tie. A
    lower-ratio disc that names a static is a different build and only a lead
    (round 86: counter.o's GetRCnt aligns 1.00 on 3.0 to an unnamed .data)."""
    if not cands:
        return "no aligned Sony function found"
    best = max(cands, key=lambda c: (c[4], not c[0].startswith(".")))
    if not best[0].startswith("."):
        return (f"lead: Sony `{best[0]}` ({best[2]} disc {best[1]}, {best[3]} {best[4]}); "
                f"ANCHOR before renaming")
    lead = [c for c in cands if not c[0].startswith(".")]
    tail = ""
    if lead:
        l = max(lead, key=lambda c: c[4])
        tail = f"; lead only: `{l[0]}` on disc {l[1]} at {l[4]}"
    return (f"lead: STATIC in {best[2]} (disc {best[1]}, {best[3]} {best[4]}; that object "
            f"names nothing there: placeholder + identified comment{tail})")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--check", action="store_true", help="exit 1 if any game-named Sony data remains")
    ap.add_argument("-v", "--verbose", action="store_true", help="every per-disc candidate")
    a = ap.parse_args()
    rows = flagged()
    if a.check:
        for n, addr, fs in rows:
            print(f"{n} {addr:#x}")
        print(f"{len(rows)} game-named Sony data symbol(s)")
        sys.exit(1 if rows else 0)
    res = sony_names(rows)
    for n, addr, fs in rows:
        print(f"{n:26s} {addr:#010x}  read by {', '.join(fs)}")
        print(f"{'':26s} -> {verdict(res.get(n, set()))}")
        if a.verbose:
            for c in sorted(res.get(n, set()), key=lambda c: (-c[4], c[1])):
                print(f"{'':30s}{c[0]:28s} disc {c[1]:4s} {c[2]:20s} {c[3]} {c[4]}")
    print(f"\n{len(rows)} game-named Sony data symbol(s)"
          + ("" if rows else ": nothing Sony owns carries a game name"))


if __name__ == "__main__":
    main()
