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


_PP_RE = re.compile(r"^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif)\b[ \t]*(.*?)[ \t]*$")


def _pp_cond(kw, arg):
    """True = compiled, False = dead, None = unknown (keep both arms)."""
    arg = re.sub(r"/\*.*?\*/", "", arg).strip()
    if kw == "if":
        return {"0": False, "1": True}.get(arg)
    if arg == "NON_MATCHING":
        return kw == "ifndef"     # `#ifdef NON_MATCHING` is dead, `#ifndef` live
    return None


def strip_dead_code(text):
    """Remove preserved bodies and comments before counting anything.

    A stalled function's best-known body is routinely kept in the unit inside
    `#if 0 ... #endif` so a later session can resume it, with the INCLUDE_ASM
    restored right after; a readable body kept under `#ifdef NON_MATCHING`
    (FINISHING-PLAN.md, track 1b) is compiled only by `make nonmatching`, with
    the live INCLUDE_ASM in its `#else`. NEITHER form is compiled by the
    verified build, so neither is matched -- and counting a preserved
    derivation as progress means good practice inflates the headline. The same
    strip catches a commented-out INCLUDE_ASM, which would otherwise be counted
    as a queued function that does not exist.

    This is a small preprocessor walk rather than three regexes (round 64):
    the regex form dropped a `#if 0` block's `#else` arm along with the dead
    one, and could not see that the INCLUDE_ASM in the `#else` of an
    `#if 1 ... #else` is DEAD -- funcdiff then warned "still INCLUDE_ASM" on a
    genuine match, inverting its signal exactly when a function closes. Only
    `#if 0`, `#if 1`, `#ifdef/#ifndef NON_MATCHING` and their `#else`/`#elif`
    are decided; any other conditional keeps both arms. Directive lines
    themselves are dropped.

    Return the stripped text SEPARATELY: the caller still needs the original,
    because the DELIBERATELY UNWORKED marker lives in a header comment and
    stripping comments in place silently zeroes the banked column.
    """
    out = []
    stack = []   # per open conditional: [this arm live (True/False/None), an arm already taken]
    for line in text.split("\n"):
        m = _PP_RE.match(line)
        if m:
            kw, arg = m.group(1), m.group(2)
            if kw in ("if", "ifdef", "ifndef"):
                c = _pp_cond(kw, arg)
                stack.append([c, c is True])
            elif kw in ("elif", "else") and stack:
                prev, taken = stack[-1]
                if prev is None:
                    c = None
                elif taken:
                    c = False
                else:
                    c = True if kw == "else" else _pp_cond("if", arg)
                stack[-1] = [c, taken or c is True]
            elif kw == "endif" and stack:
                stack.pop()
            continue
        if all(c is not False for c, _ in stack):
            out.append(line)
    code = "\n".join(out)
    return re.sub(r"/\*.*?\*/", "", code, flags=re.S)


def library_ranges():
    """vram ranges of the Psy-Q SDK blocks, read out of the splat config.

    DERIVED, NOT HARDCODED, and that is the point. A hand-maintained list of
    library addresses in a tool goes stale the first time someone carves a new
    `psyq_` segment, and it goes stale SILENTLY -- the functions simply migrate
    into the game-code denominator one carve at a time and the headline
    percentage drifts for a reason that has nothing to do with the code. The
    yaml already records the fact; read it there.

    Two things count as library, and both are read from the yaml:

    - a subsegment whose name starts with `psyq_` -- SDK code still carried as
      disassembly. Name a new one that way and it counts as library
      everywhere, automatically;
    - any subsegment of type `o` -- a prebuilt Sony object linked straight
      from the SDK (see config/psyq-objects.txt). `o` segments are library
      BY DEFINITION, whatever they are named, because the only objects the
      build links that it did not compile are Sony's.
    """
    text = YAML.read_text()
    # Every subsegment start, in order, so a block's end is the next start.
    entries = []
    for m in re.finditer(r"^\s+- \[\s*(0x[0-9A-Fa-f]+)\s*,\s*([.\w]+)"
                         r"(?:\s*,\s*([\w/]+))?\s*\]", text, re.M):
        off, kind, name = int(m.group(1), 16), m.group(2), m.group(3)
        entries.append((off, kind, name))
    entries.sort()
    ranges = []
    for i, (off, kind, name) in enumerate(entries):
        if kind != "o" and not (name or "").startswith("psyq_"):
            continue
        end = entries[i + 1][0] if i + 1 < len(entries) else None
        if end is None:
            continue
        ranges.append((off - FILE_BASE + VRAM_BASE, end - FILE_BASE + VRAM_BASE))
    return ranges


LIBRARY_RANGES = library_ranges()
SDK_IN_GAME = ROOT / "config/sdk-in-game.txt"
SYMBOLS = ROOT / "config/symbols.slps01556.lsdde.txt"


def sony_in_game():
    """Start addresses of Sony functions that sit inside GAME segments.

    Segment names decide `library` everywhere else, and round 69 measured that
    they are wrong at function grain: `code_179d8_k` is libsnd/seqread, 16 of
    its 18 functions exact against the 3.3 disc's object, which never placed
    as a whole because retail's SetControlChange is a different build. Two
    sources, both cheap to read:
      - config/sdk-in-game.txt, GENERATED by `sdkname.py --game --write`:
        every game-segment function with an EXACT fingerprint;
      - a symbols-file entry whose comment block says `identified` (track 2's
        record for a name proven by position + header, no disc build to
        fingerprint: libapi/counter's StartRCnt, libcd's getintr, ...);
      - an external config/psyq-objects.ld pins: a linked Sony object calls it
        BY NAME and the pin is the address retail's code calls, so a relocation
        names it (func_80036528, matched in round 70, is Snd_crescendo).
    """
    out = set(ld_pinned_externals())
    if SDK_IN_GAME.exists():
        for line in SDK_IN_GAME.read_text().splitlines():
            m = re.match(r"^(0x[0-9A-Fa-f]{8})\s", line)
            if m:
                out.add(int(m.group(1), 16))
    if SYMBOLS.exists():
        block = []
        for line in SYMBOLS.read_text().splitlines():
            s = line.strip()
            if s.startswith("//"):
                block.append(s)
                continue
            m = re.match(r"^\w+\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)$", s)
            if m:
                cmt = " ".join(block) + " " + m.group(2)
                if re.search(r"(?<!un)identified\b", cmt):
                    a = int(m.group(1), 16)
                    if not in_library_segment(a):
                        out.add(a)
            block = []
    return out


LDFRAG = ROOT / "config/psyq-objects.ld"


def ld_pinned_externals():
    """{vram: name} of the externals config/psyq-objects.ld pins in GAME
    segments (its bss pins are data addresses and never a function start)."""
    out = {}
    if LDFRAG.exists():
        for m in re.finditer(r"^(\w+) = (0x[0-9A-Fa-f]{8});", LDFRAG.read_text(), re.M):
            a = int(m.group(2), 16)
            if not in_library_segment(a):
                out[a] = m.group(1)
    return out


def in_library_segment(addr):
    return any(lo <= addr < hi for lo, hi in LIBRARY_RANGES)


SONY_IN_GAME = sony_in_game()


def is_library(addr):
    """Library by segment (psyq_* or an `o` object) or, at function grain,
    one of SONY_IN_GAME. Called with a function's START address."""
    return in_library_segment(addr) or addr in SONY_IN_GAME


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
    lib_stalled = 0

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
                if f in reports:
                    lib_stalled += 1

    library_matched, library_matched_b = len(lib_matched), sum(s for _, s in lib_matched)
    library_queued, library_queued_b = len(lib_queued), sum(s for _, s in lib_queued)
    matched -= library_matched
    matched_b -= library_matched_b
    queued -= library_queued
    stalled -= lib_stalled
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
    # NOT A GOAL. The SDK is linked from Sony's objects where a disc has
    # them and otherwise left as disassembly; matching it proves nothing
    # about this game (FINISHING-PLAN.md). What the plan does want from it
    # is NAMES for the functions game code calls, which tools/plan.py counts.
    print(f"  library (Psy-Q SDK):        {library:5d}  in the image as asm or C"
          f"  (excluded from game %; NOT a matching goal, see FINISHING-PLAN.md)")
    if library_matched or library_queued:
        print(f"    of which in C units:      {library_matched + library_queued:5d}  ({library_matched} written as C,"
              f" {library_queued} INCLUDE_ASM; Sony code in game segments: config/sdk-in-game.txt"
              f" and `identified` symbols)")
    # Functions inside `o` segments have no asm and are not counted above: the
    # linker takes them straight from Sony's objects. Report the objects.
    o_text = len(re.findall(r"^\s+- \[\s*0x[0-9A-Fa-f]+\s*,\s*o\s*,\s*[\w/]+\s*\]", YAML.read_text(), re.M))
    if o_text:
        print(f"    linked from SDK objects:  {o_text:5d}  objects (lib/, per config/psyq-objects.txt;"
              f" not in any count above)")
    # What the remaining psyq_* disassembly could become. sdk/work/*/match.txt
    # is the discovery corpus `tools/psyq_sdk.py match` writes; absent on a
    # checkout without the SDK discs, in which case this line is skipped.
    placed = []
    for mfile in (ROOT / "sdk/work").glob("*/match.txt"):
        for line in mfile.read_text().splitlines():
            m = re.match(r"\S+\s+text=0x([0-9a-f]+)\s+fileoff=0x([0-9a-f]+)", line)
            if m:
                off = int(m.group(2), 16)
                placed.append((off - FILE_BASE + VRAM_BASE, off + int(m.group(1), 16) - FILE_BASE + VRAM_BASE))
    if placed and library:
        owned = 0
        for f in sorted(ROOT.glob("asm/psyq_*.s")):
            for fm in re.finditer(r"^glabel \w+\n\s+/\* [0-9A-F]+ ([0-9A-F]{8}) ", f.read_text(), re.M):
                a = int(fm.group(1), 16)
                if any(lo <= a < hi for lo, hi in placed):
                    owned += 1
        print(f"      convertible now:        {owned:5d}  still-asm SDK functions a placed object owns"
              f"  ({library - library_matched - library_queued - owned} more in psyq_* segments stay as disassembly)")
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
