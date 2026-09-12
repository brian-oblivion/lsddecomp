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

Scanning whole reports instead of just the preserved regions over-reports by
roughly an order of magnitude: a prose mention of an old name is harmless and
usually historically accurate. Only `#if 0 ... #endif` blocks and ```c fences
are scanned, which is where linkage actually matters.

Usage:  python3 tools/stalesyms.py [--reports DIR] [--quiet]
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

    total = sum(len(v) for v in findings.values())
    print(f"STALE SYMBOL REFERENCES: {total} in {len(findings)} report(s).")
    print("A preserved body using these names WILL NOT LINK as written.")
    print("Correct the names and REBUILD before trusting any score attached")
    print("to that body -- the name is stale, the residue usually is not.\n")
    for name in sorted(findings):
        print(f"  {name}")
        for old, new in sorted(findings[name].items()):
            print(f"      {old}  ->  {new}")
    return 1

if __name__ == "__main__":
    sys.exit(main())
