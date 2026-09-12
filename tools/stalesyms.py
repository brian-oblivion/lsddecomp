#!/usr/bin/env python3
"""Find match reports whose PRESERVED BODY references a symbol since RENAMED.

WHY THIS EXISTS (round 35). A preserved near-miss body in a match report is
source you are meant to splice back in and rebuild -- CLAUDE.md requires it be
inlined "with every declaration it needs, positioned where it would compile".
It stops being that the moment a symbol it calls is renamed underneath it,
which is exactly what an SDK-object round does, wholesale, when it identifies a
placeholder `func_XXXXXXXX` as a real Sony symbol (`func_80025900` -> `VSync`).

The body still LOOKS right. It is internally consistent, it reads correctly,
and nothing static exposes the problem: it simply would not LINK. Round 33
found the first instance by hand (`func_800375E8`, really `SpuSetNoiseVoice`).
Round 35 found two more INDEPENDENTLY and in the same sitting -- echo hit four
renames in one unit, and delta found `func_8004109C`'s recorded 42/56 had never
been measured at all (funcdiff's staleness guard fired). Three instances across
three rounds is a class, not a coincidence, and the corpus census below is why
it needed a tool rather than another warning.

The check is mechanical because a placeholder name ENCODES its own address:
`func_80025900` claims 0x80025900. If the symbol table now gives that address a
different name, a body still calling `func_80025900` cannot link.

TWO THINGS THIS TOOL DOES NOT CLAIM, both measured rather than assumed:

- **A stale name does not invalidate the recorded RESIDUE.** Echo rebuilt all
  three of its bodies with corrected names and reproduced the recorded scores
  exactly. What is unverified is whether anyone ever BUILT the body -- which is
  the whole point of Gate 1b's "build the inherited body once" rule. Treat a
  hit as "this figure is unverified until someone compiles it", not as "this
  figure is wrong".
- **A hit does not mean nobody noticed.** Some flagged reports already document
  the rename in prose and still leave the BODY on the old names -- the runner
  corrected the names in its working tree to measure, and the durable artifact
  kept the un-linkable version. Those are the cheapest to fix and the easiest
  to miss, because the report reads as though it were already handled.
- **A hit on an ALREADY-MATCHED function is archival, not actionable**, and
  round 36 measured the split: of the flagged reports, **120 belong to
  functions already matched** and only **33 are still `INCLUDE_ASM`**. For a
  matched function `src/` is the source of truth and the report's code fence is
  a historical record -- its stale name gates nothing and rebuilding it buys
  nothing. For a live one the body IS the next runner's starting point and its
  recorded figure is unverified until someone compiles it. Round 35's headline
  ("most recorded near-miss figures in this corpus are attached to bodies that
  will not link") was right about the mechanism and four times too large about
  the queue, because the tool did not know which half it was looking at. It
  does now: read the LIVE section, and treat ARCHIVAL as cleanup.
- **THE TOOL USED TO FLAG ITS OWN REPAIRS, and that is the failure mode that
  matters most here.** Round 36 shipped three corrected bodies and the tool
  re-flagged all three the same afternoon, by three routes: a prose sentence
  writing "`#if 0`" in backticks opened a bogus region running to the next
  real `#endif`; a corrected body's own `/* ... (was func_XXXXXXXX) ... */`
  note, which the project REQUIRES it to carry, was scanned as code; and a
  round-18 "here is what I tried" ```c fence was read as a resume-from body.
  All three are fixed above (line-anchored `#if 0`, comments stripped, `#if 0`
  authoritative when present). The general shape is worth keeping in view: a
  screen whose hit SURVIVES the fix never converges, the count never falls,
  and the next round re-staffs work already done -- the same expensive
  direction as a false blocker, arriving through documentation rather than
  through a grep.

Scanning whole reports instead of just the preserved regions over-reports by
roughly an order of magnitude: a prose mention of an old name is harmless and
usually historically accurate. Only `#if 0 ... #endif` blocks and ```c fences
are scanned, which is where linkage actually matters.

Usage:  python3 tools/stalesyms.py [--reports DIR] [--quiet] [--all]
        --all also lists the ARCHIVAL (already-matched) reports in full.
Exit 1 if any stale reference is found, so it can gate a round.
"""
import os, re, sys

SYMS = "config/symbols.slps01556.lsdde.txt"
REPORTS = "docs/match-reports"

def load_symbols(path):
    """address -> name, for every `name = 0xADDR;` line."""
    table = {}
    pat = re.compile(r'^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;')
    with open(path) as fh:
        for line in fh:
            m = pat.match(line)
            if m:
                table[int(m.group(2), 16)] = m.group(1)
    return table

def live_include_asm(srcdir="src"):
    """Symbols still carried as INCLUDE_ASM -- the ones whose BODY still matters.

    A report's preserved body is only load-bearing while the function is
    unmatched: it is what the next runner splices back in, and the figure
    recorded next to it is unverified until that body compiles. Once the
    function is MATCHED, `src/` is the truth and the report is a record, so a
    stale name in it costs nothing and fixing it proves nothing.
    """
    pat = re.compile(r'INCLUDE_ASM\("[^"]*",\s*(\w+)\)')
    live = set()
    for name in os.listdir(srcdir):
        if name.endswith(".c"):
            with open(os.path.join(srcdir, name), errors="replace") as fh:
                live.update(pat.findall(fh.read()))
    return live


def preserved_regions(text):
    """Only the parts of a report that are SOURCE meant to be spliced back in.

    A prose mention of `func_8001A564` is harmless and often historically
    accurate -- the name was real when it was written. What breaks a rebuild is
    a stale name inside a body someone is meant to compile. Scanning the whole
    report instead of just those regions over-reports by roughly an order of
    magnitude (measured round 35: 463 hits whole-file vs the preserved-body
    subset), which is how a screen becomes noise the next reader learns to
    ignore.

    Two region kinds count: `#if 0 ... #endif` preservation blocks (the
    project's mandated form) and fenced ```c code blocks.
    """
    out = []   # list of (1-based start line, source text)
    # `#if 0` must open a LINE. A prose sentence that mentions "`#if 0`" in
    # backticks -- which is exactly how a report explains that it corrected a
    # stale body -- otherwise opens a bogus region that runs to the next real
    # `#endif`, swallowing the surrounding prose and every old name in it.
    # Measured round 36 on func_80050B28.md, whose round-36 fix paragraph was
    # captured this way and re-flagged the report it had just repaired.
    for m in re.finditer(r'^[ \t]*#if\s+0\b(.*?)^[ \t]*#endif', text, re.S | re.M):
        out.append((text.count("\n", 0, m.start()) + 1, m.group(1)))
    # A `#if 0` block is the project's MANDATED preservation form (CLAUDE.md:
    # "Preserve a stalled body in `#if 0 ... #endif`, never in a `/* */` block
    # comment"), so when a report has one it IS the resume-from body and the
    # ```c fences around it are attempt history -- "what I tried in round 18",
    # written with the names that were current in round 18. Those are accurate
    # records, nobody splices them back in, and scanning them keeps a fully
    # repaired report on the list forever.
    #
    # Only when a report has NO `#if 0` block at all do the fences become the
    # best available candidate for a resume-from body. That case is itself a
    # finding -- the report is not in the mandated form -- so main() counts it
    # separately rather than quietly folding it in.
    for m in re.finditer(r'```+\s*c\b(.*?)```+', text, re.S):
        out.append((text.count("\n", 0, m.start()) + 1, m.group(1)))
    return [(ln, strip_comments(t)) for ln, t in sorted(out)]


def has_if0(text):
    return bool(re.search(r'^[ \t]*#if\s+0\b.*?^[ \t]*#endif', text, re.S | re.M))


def strip_comments(src):
    """Drop C comments before scanning. A NAME IN A COMMENT CANNOT FAIL TO LINK.

    This is not a nicety, it is the difference between a screen that converges
    and one that never does. The project requires a corrected body to say what
    it corrected, and runners write exactly that:

        /* CdSearchFile/printf (was func_8002B640/func_80012C20): Sony's ... */
        extern s32 CdSearchFile(StatBuf179D8H *statBuf, char *path);

    That body links. Scanning its comment re-flags it anyway, so the report
    stays on the list for every future round no matter how many times it is
    fixed, and the count never falls. Round 36 hit this on all three of one
    runner's reports the same afternoon it shipped the corrections -- the tool
    was flagging the repair.

    A false hit here is the expensive direction, the same shape as a false
    blocker: it sends the next round to redo finished work, and the report
    reads as untouched while being correct.
    """
    src = re.sub(r'/\*.*?\*/', ' ', src, flags=re.S)
    src = re.sub(r'//[^\n]*', ' ', src)
    return src

def main():
    args = sys.argv[1:]
    quiet = "--quiet" in args
    reports = REPORTS
    if "--reports" in args:
        reports = args[args.index("--reports") + 1]

    syms = load_symbols(SYMS)
    ref = re.compile(r'\bfunc_([0-9A-Fa-f]{8})\b')
    findings = {}
    blockinfo = {}

    for name in sorted(os.listdir(reports)):
        if not name.endswith(".md"):
            continue
        path = os.path.join(reports, name)
        with open(path, errors="replace") as fh:
            text = fh.read()
        regions = preserved_regions(text)
        nblocks = len(regions)
        stale = {}
        where = {}
        for lineno, body in regions:
            for hexaddr in set(ref.findall(body)):
                addr = int(hexaddr, 16)
                current = syms.get(addr)
                # Only a RENAME is stale. An address absent from the table is
                # not evidence of anything -- plenty of names are never listed.
                if current and current.lower() != ("func_" + hexaddr).lower():
                    stale["func_" + hexaddr] = current
                    where.setdefault("func_" + hexaddr, []).append(lineno)
        blockinfo[name] = (nblocks, where)
        # NO "THIS REPORT LOOKS REPAIRED" FILTER. One was written in round 36
        # and REMOVED the same afternoon, because it produced a false clearance
        # on the first report it was tested against -- which is the expensive
        # direction, the same shape as a false blocker.
        #
        # The idea was: a repaired report keeps its old bodies (correctly --
        # deleting attempt history to satisfy a grep would be worse), so clear
        # it once the NEW name also appears in some preserved region. On
        # func_80031A44.md that cleared a report whose live body is still
        # stale. That report holds TWO preserved bodies: a superseded 87/88 one
        # in the `#if 0` block, and the round-31 HEAD SALVAGE body -- the
        # authoritative 84/88 one, the figure in the title -- in a ```c fence
        # six sections further down, under a heading that says "THIS SUPERSEDES
        # THE TITLE FIGURES ABOVE". Correcting the superseded block cleared the
        # whole report while the body anyone would actually resume from stayed
        # un-linkable.
        #
        # Positional rules fail the same way and were also tried: one round-36
        # runner put its corrected snapshot immediately after the title,
        # another appended it at the end, and here the live body is neither
        # first nor last. Which body supersedes which is stated in PROSE, and
        # no lexical rule follows prose.
        #
        # So the tool does not choose. It reports every preserved block that is
        # stale, WITH ITS LINE NUMBER, and the reader decides which one they
        # mean to resume from. A report with one clean and one stale block is a
        # real state that deserves to be seen, not resolved away.
        if stale:
            findings[name] = stale

    if not findings:
        if not quiet:
            print("No match report references a renamed symbol.")
        return 0

    live = live_include_asm()
    is_live = lambda n: n[:-3] in live      # report file is <func>.md
    live_hits = {k: v for k, v in findings.items() if is_live(k)}
    arch_hits = {k: v for k, v in findings.items() if not is_live(k)}

    total = sum(len(v) for v in findings.values())
    ltotal = sum(len(v) for v in live_hits.values())
    print(f"STALE SYMBOL REFERENCES: {total} in {len(findings)} report(s).")
    print(f"  LIVE (function still INCLUDE_ASM -- the body gates a figure): "
          f"{ltotal} in {len(live_hits)}")
    print(f"  ARCHIVAL (function already matched -- src/ is the truth):     "
          f"{total - ltotal} in {len(arch_hits)}")
    print()
    print("A preserved body using these names WILL NOT LINK as written.")
    print("Correct the names and REBUILD before trusting any score attached")
    print("to that body -- the name is stale, the residue usually is not.")
    print()
    print("Work the LIVE list. An ARCHIVAL hit is a record of how a matched")
    print("function was written before a rename; it gates nothing, and")
    print("rebuilding it proves nothing. --all lists those too.")
    print()

    nofmt = sorted(k for k in live_hits if not has_if0(
        open(os.path.join(reports, k), errors="replace").read()))
    if nofmt:
        print(f"NOTE: {len(nofmt)} of the LIVE reports have NO `#if 0` block, so the")
        print("figure above was read from their ```c fences instead -- a weaker")
        print("candidate for a resume-from body, and a finding in its own right:")
        print("the preservation form CLAUDE.md mandates is missing. Rewrite the")
        print("body into `#if 0 ... #endif` while you are correcting the names.")
        for k in nofmt:
            print(f"    {k}")
        print()

    print(f"LIVE -- {len(live_hits)} report(s), correct and rebuild these:")
    for name in sorted(live_hits):
        print(f"  {name}")
        nb, where = blockinfo[name]
        for old, new in sorted(live_hits[name].items()):
            at = ", ".join(f"L{n}" for n in where.get(old, []))
            print(f"      {old}  ->  {new}   (in preserved block at {at}"
                  f" of {nb})")

    if arch_hits:
        print(f"\nARCHIVAL -- {len(arch_hits)} report(s) for matched functions.")
        if "--all" in args:
            for name in sorted(arch_hits):
                print(f"  {name}")
                for old, new in sorted(arch_hits[name].items()):
                    print(f"      {old}  ->  {new}")
        else:
            print("  (pass --all to list them)")
    return 1

if __name__ == "__main__":
    sys.exit(main())
