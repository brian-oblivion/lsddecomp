#!/usr/bin/env python3
"""Compare the built executable against retail over one function's byte range.

    python3 tools/funcdiff.py <func_name>          # score, plus differing words
    python3 tools/funcdiff.py <func_name> --context  # every word, matched too

Exit status is the answer: 0 = byte-identical, 1 = differs, 2 = THE NUMBER
CANNOT BE TRUSTED (see the two guards below).

`./build-and-verify.sh` is the project's only oracle. This tool exists to say
*where* a function is still wrong once you already know it is wrong, and to
give a score you can watch move. It is not a substitute for the full check.

Addresses come from splat's own instruction comments, which carry the file
offset directly: `/* 39CD8 800494D8 E8FFBD27 */`, i.e. FILEOFS VRAM WORD. For
this executable file offset = vram - 0x80010000 + 0x800.
"""
import re
import sys
from pathlib import Path

import srcpath

ROOT = Path(__file__).resolve().parent.parent
RETAIL = ROOT / "disk/SLPS_015.56"
BUILT = ROOT / "build/SLPS_015.56"

# /* FILEOFS VRAM WORD */
INSN_RE = re.compile(r"/\* ([0-9A-Fa-f]+) [0-9A-Fa-f]{8} [0-9A-Fa-f]{8} \*/")


def find_range(name):
    """(start, end) file offsets for `name`, or None.

    Looks in asm/nonmatchings/**/<name>.s first (one function per file), then
    falls back to scanning the monolithic `asm/*.s` segments, where a function
    is delimited by `glabel <name>` ... `endlabel <name>`.
    """
    for p in srcpath.nm_find(name):
        offs = [int(x, 16) for x in INSN_RE.findall(p.read_text())]
        if offs:
            return min(offs), max(offs) + 4

    for p in sorted(ROOT.glob("asm/*.s")):
        text = p.read_text()
        m = re.search(rf"^glabel {re.escape(name)}$\n(.*?)^endlabel "
                      rf"{re.escape(name)}$", text, re.S | re.M)
        if not m:
            continue
        offs = [int(x, 16) for x in INSN_RE.findall(m.group(1))]
        if offs:
            return min(offs), max(offs) + 4
    return None


def staleness_check():
    """Warn if any build input is NEWER than the built executable.

    THE ORACLE TRAP THIS PROJECT WILL HIT MOST OFTEN, made mechanical instead
    of procedural. This tool reads the BUILT FILE. A failed compile or a failed
    LINK leaves the PREVIOUS build in place, so every number below comes from
    the last build that succeeded -- and when that build had the function as
    INCLUDE_ASM, the comparison is retail against retail and the score reads as
    a FULL MATCH with no diagnostic anywhere.

    The reason a written rule ("always check the build's exit status") is not
    enough: a stale number can be PLAUSIBLE AND SELF-CONSISTENT. It will often
    equal a real score you measured minutes earlier, so nothing about the
    number itself looks wrong.

    Comparing mtimes is a heuristic in one direction only -- it cannot catch a
    stale build whose sources were not touched -- but it never fires falsely,
    and it catches the sequence that actually recurs: edit, build fails, read
    the score.
    """
    try:
        built_mtime = BUILT.stat().st_mtime
    except OSError:
        return "no build/SLPS_015.56 — run ./build-and-verify.sh first."
    newest, newest_path = 0.0, None
    for pat in ("src/**/*.c", "include/**/*.h", "include/*.inc",
                "asm/*.s", "asm/nonmatchings/**/*.s"):
        for p in ROOT.glob(pat):
            try:
                m = p.stat().st_mtime
            except OSError:
                continue
            if m > newest:
                newest, newest_path = m, p
    if newest > built_mtime:
        return (f"STALE BUILD: {newest_path.relative_to(ROOT)} is NEWER than "
                f"build/SLPS_015.56.\n"
                f"         The build does not reflect your latest edit, which "
                f"means it almost certainly\n"
                f"         FAILED (compile or link). EVERY NUMBER ABOVE IS "
                f"FROM THE PREVIOUS BUILD.\n"
                f"         Re-run ./build-and-verify.sh and read its exit "
                f"status before believing anything here.")
    return None


def include_asm_check(name):
    """Warn if `name` is still INCLUDE_ASM, i.e. this is retail vs retail.

    A DIFFERENT TRAP IN KIND from staleness: the build SUCCEEDS, the output is
    FRESH, and the score is still a full match -- because an INCLUDE_ASM
    function contributes retail's own assembled bytes. Nothing fails and there
    is no message to grep for.

    It fires most often when spot-checking someone else's stall claim after a
    merge, because merging restores the INCLUDE_ASM. To re-measure a stall you
    have to re-enable its body first.
    """
    inc = re.compile(r"INCLUDE_ASM\([^,]+,\s*" + re.escape(name) + r"\s*\)")
    for c in srcpath.src_files():
        text = c.read_text()
        # A preserved body or a commented-out line is not compiled and must
        # not count as a live INCLUDE_ASM.
        code = re.sub(r"^#if 0\b.*?^#endif\b[^\n]*\n", "", text,
                      flags=re.M | re.S)
        code = re.sub(r"/\*.*?\*/", "", code, flags=re.S)
        if inc.search(code):
            return (f"{name} is still INCLUDE_ASM in {c.name}.\n"
                    f"         The build therefore contains RETAIL'S OWN BYTES "
                    f"for it, so this compares\n"
                    f"         retail against retail and a full match means "
                    f"NOTHING. Re-enable the C\n"
                    f"         body before reading a score.")
    return None


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    name = sys.argv[1]

    rng = find_range(name)
    if not rng:
        sys.exit(f"function {name} not found in asm/ — is it spelled right, "
                 f"and has `make extract` run?")
    start, end = rng

    if not RETAIL.exists():
        sys.exit(f"{RETAIL} missing — see disk/README.md")
    if not BUILT.exists():
        sys.exit(f"{BUILT} missing — run ./build-and-verify.sh")

    retail = RETAIL.read_bytes()
    built = BUILT.read_bytes()

    stale_warning = staleness_check()
    inc_warning = include_asm_check(name)

    a, b = retail[start:end], built[start:end]
    outside = sum(1 for i in range(min(len(retail), len(built)))
                  if (i < start or i >= end) and retail[i] != built[i])

    n = len(a) // 4
    bad = 0
    for i in range(n):
        # Little-endian: print the word as the disassembler shows it, so a
        # value here can be grepped straight out of a .s comment.
        x, y = a[i * 4:i * 4 + 4], b[i * 4:i * 4 + 4]
        if x != y:
            bad += 1
        if x != y or "--context" in sys.argv:
            mark = "OK  " if x == y else "DIFF"
            print(f"{i:3d} off=0x{start + i * 4:06X} vram=0x{0x80010000 + start + i * 4 - 0x800:08X} "
                  f"{mark} retail={x.hex()} built={y.hex()}")

    print(f"{name}: {n - bad}/{n} words match "
          f"(file 0x{start:X}-0x{end:X})")

    if outside:
        print(f"WARNING: the build differs OUTSIDE this range too ({outside} "
              f"bytes) — a size change may have\n"
              f"         shifted linked addresses, so this per-function read "
              f"is NOT trustworthy.\n"
              f"         ./build-and-verify.sh is the oracle.")
    # Print the warnings LAST as well as exiting non-zero: a full match is
    # exactly the case where the reader stops reading, and a false full match
    # is the failure these guard against.
    if inc_warning:
        print(f"WARNING: {inc_warning}")
    if stale_warning:
        print(f"WARNING: {stale_warning}")

    if stale_warning or inc_warning:
        sys.exit(2)
    sys.exit(1 if bad or outside else 0)


if __name__ == "__main__":
    main()
