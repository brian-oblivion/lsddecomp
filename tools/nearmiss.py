#!/usr/bin/env python3
"""Gate 1b: build the near-miss queue -- the screened, ranked list of live
INCLUDE_ASM functions worth assigning.

WHY THIS EXISTS.  docs/PARALLEL-RUNS.md's Gate 1b says the near-miss corpus is
a queue and must be screened FROM THE ASM, not from report prose.  Every head
has rebuilt that by hand, and the doc records the same two failures recurring:

  1. The nop_mflo_mfhi screen gets RE-IMPLEMENTED BACKWARDS.  Rounds 15 and 16
     both wrote it as "mflo/mfhi within two instructions AFTER a mult/div" --
     the inverted direction -- and manufactured false blockers, one of which
     was matched at 65/65 within the hour of the correction.  A false blocker
     is strictly worse than a missed one: it becomes a stub report, which
     progress.py counts as a documented stall, which removes the function from
     `fresh` PERMANENTLY.  Nobody re-measures a function everyone believes is
     blocked.
     -> So this tool SHELLS OUT to the canonical grep form from the doc.  It
        does not re-express the window.  `grep -A2` prints the two FOLLOWING
        lines, which is what makes the direction right by construction.

  2. Scores get PARSED OUT OF REPORT BODIES.  Rounds 18, 19 and 20 each hit
     this.  A report body is full of numbers describing variants that were
     tried and thrown away, and nothing distinguishes them lexically from the
     one that stands -- round 19's head pulled "0/14" out of an ordinary
     attempt narrative where the real residue was 7/14.
     -> So this tool reads the TITLE/VERDICT REGION ONLY (the H1 plus the
        first few lines), and prints it verbatim rather than extracting a
        figure.  Ranking is the head's judgement; the tool's job is to put
        honest text in front of it.

The fourth screen (BIOS trampolines) is here because the other three are blind
to it and fail in the flattering direction: a trampoline has no gp_rel, no
addiu $at and no mflo/mfhi, so a trampoline-dense segment reads as the
CLEANEST ground left while being the least matchable.  Round 17 measured
class_3bb8c_h at 15-of-17 "clean" when it was 13 BIOS trampolines.

Usage:
    python3 tools/nearmiss.py                # whole live queue, clean first
    python3 tools/nearmiss.py --all          # include blocked functions
    python3 tools/nearmiss.py --unit code_55dd4
"""

import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

INCLUDE_ASM_RE = re.compile(r'^INCLUDE_ASM\("[^"]*",\s*(\w+)\)', re.M)
SIZE_RE = re.compile(r'nonmatching \w+, 0x([0-9A-Fa-f]+)')


def screens(path):
    """Return the list of blockers hitting `path`.

    Each screen is the canonical shell form from docs/PARALLEL-RUNS.md and
    CLAUDE.md, run as-is.  Do not "simplify" these into Python regexes -- the
    third one in particular is correct BY CONSTRUCTION because grep -A2 prints
    the two FOLLOWING lines, and every re-expression of it so far has inverted
    the direction.
    """
    hits = []

    if subprocess.run(["grep", "-q", "gp_rel", path]).returncode == 0:
        hits.append("gp_rel")

    if subprocess.run(
        ["grep", "-qE", r"addiu *\$at, *\$at, *%lo", path]
    ).returncode == 0:
        hits.append("addiu_at")

    # THE ORDER MATTERS: an mflo/mfhi FOLLOWED WITHIN TWO INSTRUCTIONS BY a
    # mult/div.  Piped exactly as the doc writes it.
    if subprocess.run(
        f"grep -A2 -nE '\\b(mflo|mfhi)\\b' {path!r} | grep -qE '\\b(mult|multu|div|divu)\\b'",
        shell=True,
    ).returncode == 0:
        hits.append("nop_mflo_mfhi")

    if subprocess.run(["grep", "-qE", r"jr *\$t2", path]).returncode == 0:
        hits.append("trampoline")

    return hits


def verdict(func):
    """The report's title/verdict region, verbatim. Never a figure from the body."""
    path = os.path.join(ROOT, "docs", "match-reports", f"{func}.md")
    if not os.path.exists(path):
        return "(NO REPORT -- counts as FRESH ground to progress.py)"
    with open(path, encoding="utf-8", errors="replace") as fh:
        head = [next(fh, "") for _ in range(8)]
    text = " ".join(l.strip() for l in head if l.strip())
    text = text.lstrip("# ").replace("**", "")
    return re.sub(r"\s+", " ", text)[:110]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true",
                    help="include blocker-hit functions (default: clean only)")
    ap.add_argument("--unit", help="restrict to one unit")
    args = ap.parse_args()

    os.chdir(ROOT)
    rows = []
    src = sorted(f for f in os.listdir("src") if f.endswith(".c"))
    for cfile in src:
        unit = cfile[:-2]
        if unit.startswith("psyq_"):
            continue
        if args.unit and unit != args.unit:
            continue
        body = open(os.path.join("src", cfile), encoding="utf-8",
                    errors="replace").read()
        for func in INCLUDE_ASM_RE.findall(body):
            spath = os.path.join("asm", "nonmatchings", unit, f"{func}.s")
            if not os.path.exists(spath):
                rows.append((0, unit, func, ["NO-ASM"], "(no .s -- re-extract?)"))
                continue
            text = open(spath, encoding="utf-8", errors="replace").read()
            m = SIZE_RE.search(text)
            words = int(m.group(1), 16) // 4 if m else 0
            rows.append((words, unit, func, screens(spath), verdict(func)))

    clean = [r for r in rows if not r[3]]
    blocked = [r for r in rows if r[3]]

    print(f"live INCLUDE_ASM queue: {len(rows)}   "
          f"blocker-clean: {len(clean)}   blocked: {len(blocked)}")
    if blocked:
        tally = {}
        for r in blocked:
            tally[",".join(r[3])] = tally.get(",".join(r[3]), 0) + 1
        print("  blocked by: " + "  ".join(
            f"{k}={v}" for k, v in sorted(tally.items(), key=lambda x: -x[1])))
    print()
    print("ASSIGN FROM HERE -- blocker-clean, smallest first.")
    print("Rank by reading the verdict text. Do NOT trust a figure you did not rebuild;")
    print("a title line is only as good as the last person who rebuilt it.")
    print()
    for words, unit, func, _, v in sorted(clean):
        print(f"{words:5d}w  {unit:<16} {func:<16} {v}")

    if args.all and blocked:
        print()
        print("BLOCKED -- do not assign; each needs a stub report only.")
        for words, unit, func, b, v in sorted(blocked):
            print(f"{words:5d}w  {unit:<16} {func:<16} [{','.join(b)}] {v}")


if __name__ == "__main__":
    sys.exit(main())
