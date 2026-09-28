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

import srcpath

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

INCLUDE_ASM_RE = re.compile(r'^INCLUDE_ASM\("[^"]*",\s*(\w+)\)', re.M)
SIZE_RE = re.compile(r'nonmatching \w+, 0x([0-9A-Fa-f]+)')


_INS_RE = re.compile(r'^\s*/\* [0-9A-F]+ [0-9A-F]{8} [0-9A-F]{8} \*/\s+(\S+)\s*(.*)$')
_LOAD_RE = re.compile(r'^l(?:b|bu|h|hu|w|wl|wr)$')
_STORE_RE = re.compile(r'^s(?:b|h|w|wl|wr)$')


def _nop_at_expansion_sites(path):
    """Count `load $r,off($b); nop; lui $at,...; s? $r,...($at)` in a .s."""
    lines = []
    with open(path, encoding="utf-8", errors="replace") as fh:
        for l in fh:
            m = _INS_RE.match(l)
            if m:
                lines.append((m.group(1), m.group(2)))
    n = 0
    for i in range(len(lines) - 3):
        op, args = lines[i]
        if not _LOAD_RE.match(op) or "(" not in args or "%" in args:
            continue
        r = args.split(",")[0].strip()
        if lines[i + 1][0] != "nop":
            continue
        if lines[i + 2][0] != "lui" or not lines[i + 2][1].startswith("$at"):
            continue
        op2, args2 = lines[i + 3]
        if _STORE_RE.match(op2) and args2.split(",")[0].strip() == r and "($at)" in args2:
            n += 1
    return n


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
        hits.append("gp_rel(RESOLVED-not-a-blocker)")

    # Load-delay nop before a store-to-symbol macro of the loaded register:
    # RESOLVED round 63 by maspsx --nop-at-expansion; reported so the reports
    # that blame it (func_80030E90, func_8003149C, ...) read as pre-fix.
    # docs/research/load-delay-nop-blocker.md.
    if _nop_at_expansion_sites(path):
        hits.append("nop_at_expansion(RESOLVED-not-a-blocker)")

    # addiu_at is NO LONGER A BLOCKER as of round 21 (2026-09-06).  maspsx
    # gained a --addiu-at flag (tools/patches/maspsx-addiu-at.patch, applied
    # by tools/setup.sh, passed by the Makefile) that sets addiu_at ALONE,
    # decoupled from the sub-2.30 version bump that used to drag three
    # nop-insertion rules along with it.  The pipeline now emits retail's
    # unfolded four-instruction indexed form directly.
    #
    # The screen is kept, reported, and NOT counted as a blocker, because the
    # construct still marks functions whose earlier stall reports blamed it --
    # see docs/research/addiu-at-blocker.md.  Delete this block only once no
    # report references the class.
    if subprocess.run(
        ["grep", "-qE", r"addiu *\$at, *\$at, *%lo", path]
    ).returncode == 0:
        hits.append("addiu_at(RESOLVED-not-a-blocker)")

    # THE ORDER MATTERS: an mflo/mfhi FOLLOWED WITHIN TWO INSTRUCTIONS BY a
    # mult/div.  Piped exactly as the doc writes it.
    if subprocess.run(
        f"grep -A2 -nE '\\b(mflo|mfhi)\\b' {path!r} | grep -qE '\\b(mult|multu|div|divu)\\b'",
        shell=True,
    ).returncode == 0:
        hits.append("nop_mflo_mfhi(RESOLVED-not-a-blocker)")

    if subprocess.run(["grep", "-qE", r"jr *\$t2", path]).returncode == 0:
        hits.append("trampoline")

    return hits


# A TITLE carries a figure if it states a length verdict or a word-match.  Round
# 23 made three figures mandatory in a STALL title precisely because Gate 1b
# ranks from title lines and from nothing else -- but that rule is not
# retroactive, and a third of the queue predates it.  Flagging those is the
# point: a head ranking from titles must be able to tell "this is ranked" from
# "this could not be ranked", or it silently mis-ranks a third of the corpus
# and reaches for the report BODY, which is the failure that burned rounds 18,
# 19 and 20.
_FIGURE = re.compile(
    r"[0-9]+\s*(?:/|of)\s*[0-9]+"          # 58/63, 58 of 63
    r"|[0-9]+\s+words?\s+(?:short|long)"   # 5 words short
    r"|[0-9]+\s+instructions?\b", re.I)   # 53/53 instructions

# "exact" counts as a length verdict ONLY alongside a number.  Bare /\bexact\b/i
# was tried and rejected: it matches "instruction-exact" (arguably fine) but
# equally "the exact cause", and that is the FLATTERING direction -- it tells a
# head a title is rankable when it is not, which is the mis-ranking this flag
# exists to prevent.  A flag like this should over-report, never under-report.
_EXACT = re.compile(r"\bexact\b", re.I)
_DIGIT = re.compile(r"[0-9]")


def title_has_figure(func):
    """False when the report exists but its TITLE states no length/word figure."""
    path = os.path.join(ROOT, "docs", "match-reports", f"{func}.md")
    if not os.path.exists(path):
        return True                      # no report at all is a different state
    with open(path, encoding="utf-8", errors="replace") as fh:
        title = next(fh, "")
    if _FIGURE.search(title):
        return True
    return bool(_EXACT.search(title) and _DIGIT.search(title))


def verdict(func):
    """The report's title/verdict region, verbatim. Never a figure from the body."""
    path = os.path.join(ROOT, "docs", "match-reports", f"{func}.md")
    if not os.path.exists(path):
        return "(NO REPORT -- counts as FRESH ground to progress.py)"
    with open(path, encoding="utf-8", errors="replace") as fh:
        head = [next(fh, "") for _ in range(8)]
    text = " ".join(l.strip() for l in head if l.strip())
    text = text.lstrip("# ").replace("**", "")
    out = re.sub(r"\s+", " ", text)[:110]
    if not title_has_figure(func):
        out = "[UNRANKABLE-TITLE] " + out
    return out


def sdk_owned():
    """Functions a placed Sony object owns -- they can never match as C.

    Delegated to tools/sdkstalls.py rather than re-expressed here, for the
    same reason this file shells out to the canonical blocker greps: every
    re-implementation of a screen in this project so far has got it wrong
    (the nop_mflo_mfhi window was inverted three times, and sdkstalls' own
    extent calculation was wrong on its first run).  One implementation, one
    place to fix.

    Returns an empty mapping if the SDK tooling cannot run -- lib/ is generated
    and gitignored, so a fresh clone legitimately has no objects placed yet.
    Failing open is right here: it degrades to the pre-round-32 behaviour
    instead of hiding assignable ground.
    """
    try:
        out = subprocess.run([sys.executable, os.path.join(ROOT, "tools",
                                                           "sdkstalls.py")],
                             capture_output=True, text=True, cwd=ROOT,
                             timeout=180)
    except Exception:
        return {}
    owned = {}
    for m in re.finditer(r"^\s+(?:FULLY|PARTIAL)\s+(\S+)\s+\S+\s+\S+\s+"
                         r"\d+w\s+<- (\S+)", out.stdout, re.M):
        owned[m.group(1)] = m.group(2)
    return owned


def not_game_code():
    """Functions a REPORT certifies are Sony's, with no placed object to prove it.

    The fourth honesty mechanism, and it closes the one gap the other three
    cannot see.  `sdk_owned()` crosses the queue against PLACED objects, so it
    answers "is this owned by an object we HAVE" -- it cannot answer "is this
    Sony's".  progress.py reports hundreds of SDK functions with no object on
    any disc in sdk/, and a function in that gap passes every screen the
    project owns (no gp_rel, no nop_mflo_mfhi, not a trampoline, no
    placed-object overlap) while being unmatchable by construction.

    Round 39 found one the expensive way: func_80050B28, the SMALLEST function
    in the queue and therefore FIRST in this list for four rounds, carrying 634
    lines of derivation across four of them.  It is Psy-Q libcard.  Round 39
    proved it and wrote it in the report title, and this list went on ranking
    it first anyway, because nothing read the title.

    The marker is the exact phrase NOT GAME CODE in the report's TITLE/VERDICT
    REGION -- the same first-8-lines window verdict() reads, and for the same
    reason: the body is full of claims that were tried and thrown away, and
    only the title is safe to key on.  A function so marked keeps its report
    and stays a documented stall to progress.py (it IS documented); it is only
    removed from ASSIGN FROM HERE, which is the one place the error was paid.

    Marking one is a HEAD decision backed by the two mechanical detectors in
    Gate 1b's eighth screen (segment topology, and the addiu/ori assembler
    fingerprint), never a runner's judgement call.
    """
    marked = {}
    d = os.path.join(ROOT, "docs", "match-reports")
    if not os.path.isdir(d):
        return marked
    for name in os.listdir(d):
        if not name.endswith(".md"):
            continue
        try:
            with open(os.path.join(d, name), encoding="utf-8",
                      errors="replace") as fh:
                head = " ".join(next(fh, "") for _ in range(8))
        except OSError:
            continue
        if "NOT GAME CODE" in head:
            marked[name[:-3]] = True
    return marked


def sony_by_address(rows):
    """Queued functions whose START address progress.py counts as Sony code
    inside a game segment (config/sdk-in-game.txt, `identified` symbols)."""
    sys.path.insert(0, os.path.join(ROOT, "tools"))
    import progress
    insn = re.compile(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/", re.M)
    out = set()
    for _, unit, func, _, _ in rows:
        spath = str(srcpath.nm_dir(unit) / f"{func}.s")
        if not os.path.exists(spath):
            continue
        m = insn.search(open(spath, encoding="utf-8", errors="replace").read())
        if m and int(m.group(1), 16) in progress.SONY_IN_GAME:
            out.add(func)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true",
                    help="include blocker-hit functions (default: clean only)")
    ap.add_argument("--unit", help="restrict to one unit")
    args = ap.parse_args()

    os.chdir(ROOT)
    rows = []
    for cpath in srcpath.src_files():
        unit = cpath.stem
        if unit.startswith("psyq_"):
            continue
        if args.unit and unit != args.unit:
            continue
        body = open(cpath, encoding="utf-8",
                    errors="replace").read()
        for func in INCLUDE_ASM_RE.findall(body):
            spath = str(srcpath.nm_dir(unit) / f"{func}.s")
            if not os.path.exists(spath):
                rows.append((0, unit, func, ["NO-ASM"], "(no .s -- re-extract?)"))
                continue
            text = open(spath, encoding="utf-8", errors="replace").read()
            m = SIZE_RE.search(text)
            words = int(m.group(1), 16) // 4 if m else 0
            rows.append((words, unit, func, screens(spath), verdict(func)))

    # A screen hit is only a BLOCKER if the pinned pipeline still cannot emit
    # retail's bytes for it.  addiu_at is reported but no longer blocks (round
    # 21) -- partitioning on "any hit" would keep 76 assignable functions
    # hidden in the blocked column, which is the same
    # false-blocker-deletes-matchable-ground failure the nop_mflo_mfhi
    # inversions caused in rounds 15 and 16.
    def real_blockers(hits):
        return [h for h in hits if not h.endswith("(RESOLVED-not-a-blocker)")]

    # Sony library code is a THIRD partition, and it must come out before
    # "blocker-clean" is computed.  A function inside a placed SDK object
    # passes every blocker screen -- there is no gp_rel, no mflo/mfhi hazard,
    # nothing -- so it reads as the cleanest possible ground while being
    # unmatchable by construction.  Exactly the failure mode the BIOS
    # trampoline screen was added to Gate 2 for, arriving in Gate 1b.
    owned = sdk_owned()
    sdk = [r for r in rows if r[2] in owned]
    rest = [r for r in rows if r[2] not in owned]

    # ...and a FOURTH partition, for Sony code with no object to prove it.
    # See not_game_code() for why this cannot be folded into the screen above.
    # progress.SONY_IN_GAME joins it (plan revision 13): an exact per-function
    # fingerprint (config/sdk-in-game.txt) or an `identified` symbols entry
    # is the same verdict, reached by a tool instead of a report title.
    nogame = not_game_code()
    sony = sony_by_address(rows)
    notgame = [r for r in rest if r[2] in nogame or r[2] in sony]
    rest = [r for r in rest if r[2] not in nogame and r[2] not in sony]

    clean = [r for r in rest if not real_blockers(r[3])]
    blocked = [r for r in rest if real_blockers(r[3])]

    print(f"live INCLUDE_ASM queue: {len(rows)}   "
          f"blocker-clean: {len(clean)}   blocked: {len(blocked)}   "
          f"Sony library code: {len(sdk)}   marked NOT GAME CODE: {len(notgame)}")
    if sdk:
        print(f"  {len(sdk)} function(s) lie inside a placed Sony object and can NEVER")
        print("  match as C -- excluded from the assignable list below. Convert them")
        print("  per docs/SDK-OBJECTS-GUIDE.md; see `python3 tools/sdkstalls.py`.")
    if notgame:
        print(f"  {len(notgame)} function(s) are NOT GAME CODE, by their own report or by")
        print("  config/sdk-in-game.txt / an `identified` symbols entry --")
        print("  Sony library code no placed object owns. Matching it is not a goal")
        print("  (FINISHING-PLAN section 1); naming it is track 2's. Excluded from the")
        print("  assignable list below. See Gate 1b's eighth screen.")
        for words, unit, func, _, v in sorted(notgame):
            print(f"    {words:5d}w  {unit:<16} {func:<16} {v[:70]}")
    if blocked:
        tally = {}
        for r in blocked:
            k = ",".join(real_blockers(r[3]))
            tally[k] = tally.get(k, 0) + 1
        print("  blocked by: " + "  ".join(
            f"{k}={v}" for k, v in sorted(tally.items(), key=lambda x: -x[1])))
    print()
    print("ASSIGN FROM HERE -- blocker-clean, smallest first.")
    print("Rank by reading the verdict text. Do NOT trust a figure you did not rebuild;")
    print("a title line is only as good as the last person who rebuilt it.")
    unrankable = [r[2] for r in clean if not title_has_figure(r[2])]
    if unrankable:
        print()
        print(f"[UNRANKABLE-TITLE] marks {len(unrankable)} of {len(clean)} whose report "
              "TITLE states no length or")
        print("word figure, so they CANNOT be ranked from the title. Round 23 made three")
        print("figures mandatory in a STALL title; the rule is not retroactive. Do NOT")
        print("recover a figure from the report BODY -- it is full of numbers describing")
        print("variants that were thrown away (rounds 18, 19 and 20 each got caught).")
        print("Re-measure the function instead, and rebuild its title while you are there.")
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
