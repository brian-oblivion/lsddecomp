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
import subprocess
import sys
from pathlib import Path

import progress
import srcpath

ROOT = Path(__file__).resolve().parent.parent
RETAIL = ROOT / "disk/SLPS_015.56"
BUILT = ROOT / "build/SLPS_015.56"
ELF = ROOT / "build/lsdde.elf"
NM = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"

VRAM_BASE = 0x80010000
FILE_BASE = 0x800

# /* FILEOFS VRAM WORD */
INSN_RE = re.compile(r"/\* ([0-9A-Fa-f]+) [0-9A-Fa-f]{8} [0-9A-Fa-f]{8} \*/")


def range_from_elf(name):
    """(start, end) file offsets from the linked ELF's symbol table, or None.

    THE ONLY SOURCE THAT WORKS FOR AN ALREADY-MATCHED FUNCTION. Once a function
    is real C its `asm/nonmatchings/` file stops being generated, so the two
    disassembly sources below cannot see it at all -- and funcdiff would answer
    "not found" for precisely the functions someone is trying to RE-verify.
    That matters: the head agent's protocol spot-checks every claimed match
    after a merge, and a tool that cannot score a match is useless for it.

    Size is the gap to the next symbol, which is how the linker lays functions
    out. It over-reads when the next symbol is not a function, so this is the
    fallback rather than the primary source.
    """
    if not (NM.exists() and ELF.exists()):
        return None
    out = subprocess.run([str(NM), "-n", str(ELF)],
                         capture_output=True, text=True).stdout
    syms = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in ("T", "t"):
            syms.append((int(parts[0], 16), parts[2]))
    for i, (addr, sym) in enumerate(syms):
        if sym != name:
            continue
        nxt = next((a for a, _ in syms[i + 1:] if a > addr), None)
        if nxt is None:
            return None
        return (addr - VRAM_BASE + FILE_BASE, nxt - VRAM_BASE + FILE_BASE)
    return None


def find_range(name):
    """(start, end) file offsets for `name`, or None.

    Three sources, in descending order of precision:
      1. asm/nonmatchings/**/<name>.s -- one function per file, exact.
      2. the monolithic `asm/*.s` segments, where a function is delimited by
         `glabel <name>` ... `endlabel <name>`.
      3. the linked ELF's symbol table, which is the only one that still knows
         about a function after it has been MATCHED.
    """
    for p in srcpath.nm_find(name):
        text = p.read_text()
        # DELIMIT BY glabel/endlabel, exactly as source 2 below does -- do NOT
        # min/max over the whole file.  A function that owns a jump table has
        # its rodata emitted into its OWN `.s` as a leading
        # `.section .rodata` block, and those data words carry the same
        # `/* fileofs vram word */` comment shape as instructions.  Because the
        # rodata slot sits at a much LOWER file offset than the text, a
        # min/max over the file returned a window spanning the whole image:
        # func_800513D0 measured `65533/65533 words match (file 0x1E28-0x41E1C)`
        # against a real size of 147 words.
        #
        # That was worse than a cosmetic wrong denominator.  It also DISABLED
        # THE DRIFT GUARD -- with a ~262KB window almost nothing is "outside
        # this range", so the out-of-range byte count can no longer fire.  And
        # for a near-miss it prints a precise-looking ratio in which 3 words
        # off and 30 words off are indistinguishable.
        m = re.search(rf"^glabel {re.escape(name)}$\n(.*?)^endlabel "
                      rf"{re.escape(name)}$", text, re.S | re.M)
        body = m.group(1) if m else text
        offs = [int(x, 16) for x in INSN_RE.findall(body)]
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

    return range_from_elf(name)


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
        # A preserved body, a commented-out line, or the `#else` arm of an
        # `#if 1` a runner iterates under is not compiled and must not count
        # as a live INCLUDE_ASM (round 64: the regex form warned "still
        # INCLUDE_ASM" on a genuine match). progress.py owns the walk.
        code = progress.strip_dead_code(c.read_text())
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
        sys.exit(f"function {name} not found in asm/ or in build/lsdde.elf.\n"
                 f"  Is it spelled right? Has `make extract` run? Has "
                 f"./build-and-verify.sh run at least once\n"
                 f"  (the ELF is the only source that knows about an "
                 f"already-matched function)?")
    start, end = rng

    if not RETAIL.exists():
        sys.exit(f"{RETAIL} missing — see README.md, 'Building it'")
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

    # Insertions / deletions at the OPCODE level, so Gate 3's check 3 (does the
    # permuter scaffold's signature AGREE with the real build) can be run at
    # all: round 58 found it had been inferred from "length exact, so 0/0",
    # and equal length is exactly where an insertion and a deletion cancel
    # (measured 3/3 and 14/14). Same skeleton as tools/sdkname.py: the
    # instruction with its immediate field zeroed, so a register or offset
    # change counts as a REPLACEMENT and only a genuinely added or dropped
    # instruction counts here. A pure register-identity residue reads 0/0.
    import difflib
    def skel(w):
        v = int.from_bytes(w, "little")
        op = v >> 26
        if op == 0:
            return v & 0xFFFFF83F
        if op in (2, 3):
            return v & 0xFC000000
        return v & 0xFFFF0000
    ra = [skel(a[i * 4:i * 4 + 4]) for i in range(len(a) // 4)]
    rb = [skel(b[i * 4:i * 4 + 4]) for i in range(len(b) // 4)]
    # Positional skeleton mismatches. If ZERO at equal length, no instruction
    # was inserted or dropped, whatever the sequence matcher says: on a loop
    # nest's repeating skeleton alphabet it can report N/N where nothing moved
    # (round 63 measured 26/26 and 22/22 on synthetic zero-insertion inputs).
    positional = sum(1 for x, y in zip(ra, rb) if x != y) if len(ra) == len(rb) else None
    ins = dele = 0
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, ra, rb, autojunk=False).get_opcodes():
        if tag == "insert":
            ins += j2 - j1
        elif tag == "delete":
            dele += i2 - i1
        elif tag == "replace":
            d = (j2 - j1) - (i2 - i1)
            ins += max(d, 0)
            dele += max(-d, 0)
    if positional == 0:
        ins = dele = 0
    note = ""
    if positional is not None and ins and ins == dele:
        note = (f"; NOTE equal length with {positional} positional skeleton diff(s): an N/N figure "
                f"here is a pointer to READ the diff, not a verdict (a repeating skeleton alphabet "
                f"can align falsely)")
    print(f"{name}: insertions {ins} / deletions {dele} (opcode-level, built vs retail; "
          f"positional skeleton diffs {positional if positional is not None else 'n/a'}; "
          f"Gate 3 check 3 compares these with the scaffold's --stack-diffs){note}")

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
