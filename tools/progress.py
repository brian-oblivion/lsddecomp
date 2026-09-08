#!/usr/bin/env python3
"""Project progress: functions matched / queued / stalled / uncarved.

    python3 tools/progress.py
    python3 tools/progress.py --json

COLUMNS, and the distinctions matter because runner assignment is made from
them (docs/PARALLEL-RUNS.md, Gate 1):

  matched   defined as real C in a src/ unit. The build is byte-verified, so
            "defined in C" == "matched" for as long as build-and-verify.sh is
            green -- which is the only reason this can be counted statically.
  queued    INCLUDE_ASM entries: carved into a C unit, awaiting decompilation.
  stalled   queued AND has a docs/match-reports/ file, i.e. someone already
            attempted it and wrote down why it did not go. NOT fresh ground.
  banked    queued, unattempted, in a unit whose header says DELIBERATELY
            UNWORKED -- carved to fix a boundary or bank the ground, but
            classified as senior work rather than cold-runner work.
  reopened  queued AND has a report, but that report carries one of the two
            EXACT line-anchored markers below. Counted as FRESH, not as a
            stall. Both exist for the same reason: this tool decides stall-vs-
            fresh purely by whether a report FILE exists, so any report that is
            not a stall verdict silently deletes matchable ground from every
            future round.

              REOPENED -- ASSIGNABLE
                  The report's stall verdict has been INVALIDATED, almost
                  always because the blocker it blamed was resolved. A report
                  written while a blocker was live outlives the blocker, and a
                  function everybody believes is blocked is one nobody
                  re-measures. `addiu_at` resolving in round 21 left five such
                  reports.

              DERIVATION ONLY -- ASSIGNABLE
                  No C was ever written or compiled: the report is a partial
                  derivation of a body too large to attempt in one bounded
                  session, which the project asks for rather than a heroic
                  single attempt. There is no stall verdict to invalidate,
                  because there was never an attempt. Added round 25, when
                  runner bravo filed exactly this for func_80032D34 (274w) and
                  wrote in its own prose "deliberately NOT filed as a STALL"
                  -- prose this tool cannot read, so the function left `fresh`
                  anyway. Do NOT use it for a body that WAS built and scored;
                  that is a real stall however low the score.

            Both are the counterpart of DELIBERATELY UNWORKED: an exact phrase,
            keyed on by this tool, that lets a report stay on disk (its
            derivation is still worth reading) without still claiming the
            function is worked.
  fresh     queued - stalled - banked. THE ONLY COLUMN TO ASSIGN FROM. Raw
            `queued` includes documented stalls, and staffing a runner onto
            one means paying again to re-derive what someone already recorded.
  uncarved  still inside a monolithic `asm` segment; needs a carve first.
  library   Sony Psy-Q SDK code compiled into the executable. Excluded from the
            game-code denominator -- it is not this game's source and matching
            it proves nothing about the game.

Byte metrics need a linked build/lsdde.elf, so run ./build-and-verify.sh
first. Sizes weight a 400-instruction dispatcher above a 6-word leaf, which is
the honest way to read "how much is left".
"""
import json
import re
import subprocess
import sys
from pathlib import Path

import srcpath

ROOT = Path(__file__).resolve().parent.parent
NM = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
ELF = ROOT / "build/lsdde.elf"
YAML = ROOT / "config/splat.slps01556.lsdde.yaml"
REPORTS = ROOT / "docs/match-reports"
REOPENED_RE = re.compile(
    r"^[\s>*_#-]*(?:REOPENED|DERIVATION ONLY) -- ASSIGNABLE\b", re.M)

VRAM_BASE = 0x80010000
FILE_BASE = 0x800

INCLUDE_RE = re.compile(r'INCLUDE_ASM\("[^"]+",\s*(\w+)\)')
GLABEL_RE = re.compile(r"^glabel (\w+)$", re.M)
# splat emits these alongside real functions; they are segment boundaries and
# data labels, not code we could ever decompile.
NOT_A_FUNCTION = re.compile(
    r"(_TEXT_START|_TEXT_END|_VRAM|_RODATA|_DATA|_BSS|_SDATA|_SBSS"
    r"|\.NON_MATCHING|^D_[0-9A-Fa-f]{8}$|^jtbl_|^gcc2_compiled|^__gnu_compiled)")


def strip_dead_code(text):
    """Remove preserved bodies and comments before counting anything.

    A stalled function's best-known body is routinely kept in the unit inside
    `#if 0 ... #endif` so a later session can resume it, with the INCLUDE_ASM
    restored right after. NEITHER form is compiled, so neither is matched --
    and counting a preserved derivation as progress means good practice
    inflates the headline. The same strip catches a commented-out INCLUDE_ASM,
    which would otherwise be counted as a queued function that does not exist.

    Return the stripped text SEPARATELY: the caller still needs the original,
    because the DELIBERATELY UNWORKED marker lives in a header comment and
    stripping comments in place silently zeroes the banked column.
    """
    code = re.sub(r"^#if 0\b.*?^#endif\b[^\n]*\n", "", text, flags=re.M | re.S)
    return re.sub(r"/\*.*?\*/", "", code, flags=re.S)


def library_ranges():
    """vram ranges of the Psy-Q SDK blocks, read out of the splat config.

    DERIVED, NOT HARDCODED, and that is the point. A hand-maintained list of
    library addresses in a tool goes stale the first time someone carves a new
    `psyq_` segment, and it goes stale SILENTLY -- the functions simply migrate
    into the game-code denominator one carve at a time and the headline
    percentage drifts for a reason that has nothing to do with the code. The
    yaml already records the fact; read it there.

    The naming convention is the contract: a subsegment whose name starts with
    `psyq_` is SDK code. Name a new one that way and it counts as library
    everywhere, automatically.
    """
    text = YAML.read_text()
    # Every subsegment start, in order, so a block's end is the next start.
    entries = []
    for m in re.finditer(r"^\s+- \[\s*(0x[0-9A-Fa-f]+)\s*,\s*([.\w]+)"
                         r"(?:\s*,\s*(\w+))?\s*\]", text, re.M):
        off, kind, name = int(m.group(1), 16), m.group(2), m.group(3)
        entries.append((off, kind, name))
    entries.sort()
    ranges = []
    for i, (off, kind, name) in enumerate(entries):
        if not (name or "").startswith("psyq_"):
            continue
        end = entries[i + 1][0] if i + 1 < len(entries) else None
        if end is None:
            continue
        ranges.append((off - FILE_BASE + VRAM_BASE, end - FILE_BASE + VRAM_BASE))
    return ranges


LIBRARY_RANGES = library_ranges()


def is_library(addr):
    return any(lo <= addr < hi for lo, hi in LIBRARY_RANGES)


def text_symbols():
    """name -> (vram, byte size) for every plausible function in the ELF.

    Size is the gap to the next symbol address. The ELF is the arbiter of
    whether a name is a real ROM function at all: a `static inline` helper the
    compiler folded away emits no symbol, so it cannot be miscounted as a
    match, and one the compiler did NOT fold would occupy real bytes and break
    the build anyway.
    """
    if not (NM.exists() and ELF.exists()):
        return {}
    out = subprocess.run([str(NM), "-n", str(ELF)],
                         capture_output=True, text=True, check=True).stdout
    syms = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) != 3 or parts[1] not in ("T", "t"):
            continue
        syms.append((int(parts[0], 16), parts[2]))
    info = {}
    for i, (addr, name) in enumerate(syms):
        if NOT_A_FUNCTION.search(name):
            continue
        nxt = next((a for a, _ in syms[i + 1:] if a > addr), addr)
        info[name] = (addr, nxt - addr)
    return info


def main():
    info = text_symbols()
    sizes = {n: s for n, (_, s) in info.items()}
    # A report marked REOPENED -- ASSIGNABLE or DERIVATION ONLY -- ASSIGNABLE
    # is deliberately NOT a stall: see the `reopened` entry in the module
    # docstring. The phrase must stand on its own line so that a report
    # *discussing* the convention (this file's own docs, a learnings entry
    # quoting it) cannot trip the marker.
    all_reports = list(REPORTS.glob("*.md")) if REPORTS.exists() else []
    reopened = {p.stem for p in all_reports
                if REOPENED_RE.search(p.read_text())}
    reports = {p.stem for p in all_reports} - reopened

    per_unit = {}
    matched = queued = stalled = banked = 0
    matched_b = queued_b = 0
    lib_matched, lib_queued = [], []

    for c in srcpath.src_files():
        text = c.read_text()
        code = strip_dead_code(text)
        inc = INCLUDE_RE.findall(code)
        defs = re.findall(r"^\w[^;=]*?\b(\w+)\s*\([^;{]*\)\s*\{", code, re.M)
        defs = [d for d in defs
                if d not in ("if", "while", "for", "switch", "do", "return")]
        if info:
            defs = [d for d in defs if d in info]

        stall = [f for f in inc if f in reports]
        is_banked = "DELIBERATELY UNWORKED" in text
        unattempted = len(inc) - len(stall)

        per_unit[c.stem] = {
            "matched": len(defs), "queued": len(inc), "stalled": len(stall),
            "banked": unattempted if is_banked else 0,
            "fresh": 0 if is_banked else unattempted,
        }
        matched += len(defs)
        queued += len(inc)
        stalled += len(stall)
        if is_banked:
            banked += unattempted
        matched_b += sum(sizes.get(f, 0) for f in defs)
        queued_b += sum(sizes.get(f, 0) for f in inc)

        # A LIBRARY FUNCTION IS STILL LIBRARY CODE ONCE IT IS CARVED INTO A C
        # UNIT. Counting src/ definitions without an address check would let
        # SDK functions migrate into "game code" one at a time and inflate the
        # headline for work that is not this game's source. The same applies to
        # the unmatched ones -- both buckets fed by src/ need the check, not
        # just the one that happens to move first.
        for f in defs:
            a = info.get(f, (None, 0))[0]
            if a is not None and is_library(a):
                lib_matched.append((f, sizes.get(f, 0)))
        for f in inc:
            a = info.get(f, (None, 0))[0]
            if a is not None and is_library(a):
                lib_queued.append((f, sizes.get(f, 0)))

    library_matched, library_matched_b = len(lib_matched), sum(s for _, s in lib_matched)
    library_queued, library_queued_b = len(lib_queued), sum(s for _, s in lib_queued)
    matched -= library_matched
    matched_b -= library_matched_b
    queued -= library_queued
    queued_b -= library_queued_b

    # STALE-FILE GUARD. splat never deletes what it stops generating, and asm/
    # is gitignored, so orphaned .s files accumulate in a working checkout and
    # every glabel in them gets counted a second time. `make extract` wipes
    # asm/nonmatchings for exactly this reason; the top-level asm/*.s files it
    # cannot wipe safely, so cross-check them against the yaml instead. Warn
    # rather than die: a stale file is a working-tree artifact, not a repo
    # defect, and the fix is `rm` plus an extract.
    yaml_text = YAML.read_text()
    declared = set(re.findall(
        r"^\s+- \[\s*0x[0-9A-Fa-f]+\s*,\s*h?asm\s*,\s*(\w+)\s*\]",
        yaml_text, re.M))
    hasm = set(re.findall(
        r"^\s+- \[\s*0x[0-9A-Fa-f]+\s*,\s*hasm\s*,\s*(\w+)\s*\]",
        yaml_text, re.M))

    uncarved = library = handwritten = 0
    uncarved_b = library_b = handwritten_b = 0
    stale = []
    for s in sorted(ROOT.glob("asm/*.s")):
        if s.stem not in declared and s.stem not in ("header",):
            stale.append(s)
            continue
        for f in GLABEL_RE.findall(s.read_text()):
            addr, size = info.get(f, (None, 0))
            if addr is not None and is_library(addr):
                library += 1
                library_b += size
            elif s.stem in hasm:
                # Hand-written assembly: never was C in the retail build, so
                # there is no C form to decompile it to. FINISHED, not pending.
                handwritten += 1
                handwritten_b += size
            else:
                uncarved += 1
                uncarved_b += size

    live_inc = set()
    for c in srcpath.src_files():
        live_inc.update(INCLUDE_RE.findall(strip_dead_code(c.read_text())))
    reopened_live = len(reopened & live_inc)
    stale_nm = [p for p in srcpath.nm_all() if p.stem not in live_inc]

    library += library_matched + library_queued
    library_b += library_matched_b + library_queued_b
    total = matched + queued + uncarved + library + handwritten
    game = total - library
    fresh = queued - stalled - banked
    total_b = matched_b + queued_b + uncarved_b + library_b + handwritten_b
    game_b = total_b - library_b

    if "--json" in sys.argv:
        print(json.dumps({
            "matched": matched, "queued": queued, "stalled": stalled,
            "banked": banked, "fresh": fresh, "reopened": reopened_live,
            "uncarved": uncarved,
            "library": library, "handwritten": handwritten,
            "total": total, "game": game,
            "bytes": {"matched": matched_b, "queued": queued_b,
                      "uncarved": uncarved_b, "library": library_b,
                      "handwritten": handwritten_b,
                      "total": total_b, "game": game_b},
            "library_ranges": [[hex(a), hex(b)] for a, b in LIBRARY_RANGES],
            "units": per_unit}, indent=2))
        return

    if stale:
        print("WARNING: ignoring stale asm/*.s not declared in the splat config —")
        for s in stale:
            n = len(GLABEL_RE.findall(s.read_text()))
            print(f"         {s.relative_to(ROOT)}  ({n} glabels)")
        print("         Delete them and re-run `make extract`; splat leaves "
              "them behind on a rename or split.\n")
    if stale_nm:
        print(f"WARNING: {len(stale_nm)} stale asm/nonmatchings/**/*.s with no "
              f"live INCLUDE_ASM ({len(live_inc)} live).")
        print("         They do NOT affect the counts below, but they DO "
              "corrupt every tool that treats")
        print("         that tree as ground truth. Fix: make extract\n")

    print("LSD: Dream Emulator (PSX) decomp progress")
    if total:
        print(f"  matched (C, byte-verified): {matched:5d}"
              f"  ({100 * matched / total:.2f}% of all,"
              f" {100 * matched / game:.2f}% of game code)")
    print(f"  queued (INCLUDE_ASM):       {queued:5d}")
    print(f"    stalled (has report):     {stalled:5d}")
    print(f"    banked (unit unworked):   {banked:5d}")
    print(f"    fresh (runner-workable):  {fresh:5d}   <- ASSIGN FROM THIS")
    if reopened_live:
        print(f"      of which reopened:      {reopened_live:5d}   "
              f"(report kept, marked REOPENED/DERIVATION ONLY -- ASSIGNABLE)")
    print(f"  uncarved game code:         {uncarved:5d}")
    if handwritten:
        print(f"  hand-written asm (DONE):    {handwritten:5d}"
              f"  (never was C — can never be matched)")
    print(f"  library (Psy-Q SDK):        {library:5d}  (excluded from game %)"
          f"  [{library_matched} matched, {library - library_matched} to go]")
    print(f"  total functions:            {total:5d}  ({game} game)")
    if handwritten and game:
        # Keep hand-written asm inside the game denominator and report the
        # ceiling instead of shrinking the denominator -- excluding it would
        # move the headline UP for a purely definitional reason.
        print(f"  ACHIEVABLE CEILING:               "
              f"{100 * (game - handwritten) / game:.2f}% of game code")

    print()
    if total_b:
        print(f"  matched bytes:  {matched_b:8d} / {total_b} code"
              f"  ({100 * matched_b / total_b:.2f}% of all,"
              f" {100 * matched_b / game_b:.2f}% of {game_b} game bytes)")
    else:
        print("  (no build/lsdde.elf — run ./build-and-verify.sh for byte "
              "metrics and the library split)")

    print()
    print(f"  {'unit':<16} {'matched':>8} {'queued':>8} {'stalled':>8}"
          f" {'banked':>8} {'fresh':>8}")
    for name, u in per_unit.items():
        print(f"  {name:<16} {u['matched']:>8} {u['queued']:>8}"
              f" {u['stalled']:>8} {u['banked']:>8} {u['fresh']:>8}")


if __name__ == "__main__":
    main()
