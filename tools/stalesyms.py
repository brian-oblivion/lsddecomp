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
  round 36 measured the split: of 158 flagged reports, **121 belong to
  functions already matched** and only **37 are still `INCLUDE_ASM`**. For a
  matched function `src/` is the source of truth and the report's code fence is
  a historical record -- its stale name gates nothing and rebuilding it buys
  nothing. For a live one the body IS the next runner's starting point and its
  recorded figure is unverified until someone compiles it. Round 35's headline
  ("most recorded near-miss figures in this corpus are attached to bodies that
  will not link") was right about the mechanism and four times too large about
  the queue, because the tool did not know which half it was looking at. It
  does now: read the LIVE section, and treat ARCHIVAL as cleanup.

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
    out = []
    for m in re.finditer(r'#if\s+0\b(.*?)#endif', text, re.S):
        out.append(m.group(1))
    for m in re.finditer(r'```+\s*c\b(.*?)```+', text, re.S):
        out.append(m.group(1))
    return "\n".join(out)

def main():
    args = sys.argv[1:]
    quiet = "--quiet" in args
    reports = REPORTS
    if "--reports" in args:
        reports = args[args.index("--reports") + 1]

    syms = load_symbols(SYMS)
    ref = re.compile(r'\bfunc_([0-9A-Fa-f]{8})\b')
    findings = {}

    for name in sorted(os.listdir(reports)):
        if not name.endswith(".md"):
            continue
        path = os.path.join(reports, name)
        with open(path, errors="replace") as fh:
            text = fh.read()
        text = preserved_regions(text)
        stale = {}
        for hexaddr in set(ref.findall(text)):
            addr = int(hexaddr, 16)
            current = syms.get(addr)
            # Only a RENAME is stale. An address absent from the table is not
            # evidence of anything -- plenty of names are never listed.
            if current and current.lower() != ("func_" + hexaddr).lower():
                stale["func_" + hexaddr] = current
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

    print(f"LIVE -- {len(live_hits)} report(s), correct and rebuild these:")
    for name in sorted(live_hits):
        print(f"  {name}")
        for old, new in sorted(live_hits[name].items()):
            print(f"      {old}  ->  {new}")

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
