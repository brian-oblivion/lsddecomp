#!/usr/bin/env python3
"""Which units would fight over a header if you staffed them in one round.

WHY THIS EXISTS. A runner owns exactly one `src/<unit>.c` (PARALLEL-RUNS
collision rule 1), and that rule does NOT partition `include/`. Two runners
whose units include the same project header will both edit it, and the head
pays for it at merge time. Measured across three rounds, and it scales with how
many runners share a header rather than with how many runners there are:

  round 8   2 runners, adjacent slices of one carve -> the round's only conflict
  round 13  3 shared-header collisions, one of which git auto-merged with NO
            conflict marker and only the build caught, on `conflicting types`
  round 15  5 runners all on class_3bb8c_* slices -> 6 of 7 merges conflicted,
            4 hard prototype/definition collisions, one latent across two
            merges before it broke a unit that never touched the declaration

None of those were unresolvable and none cost a match. What they cost was HEAD
ATTENTION, which PARALLEL-RUNS names as the binding constraint on round size --
round 15 left three runners un-resent with fresh ground still in their units
because the merge queue, not runner capacity, had become the bottleneck.

THE TRAP THIS TOOL EXISTS TO CATCH: contention is a property of the HEADER SET,
not of the unit-name prefix. `class_3bb8c_k` looks like an ordinary
`class_3bb8c_*` slice, but it also includes `class_39e08.h` -- and that is
exactly the edge that broke round 15: a prototype another runner put in
`class_3bb8c.h` collided with the canonical declaration in `class_39e08.h`, in
the one unit that sees both. Grouping by name prefix would not have shown it.

`common.h` and `types.h` are excluded: every unit includes them, nobody adds
declarations to them, and counting them would make every unit contend with
every other and say nothing.

USAGE

    python3 tools/headercontention.py
        The whole map, most-shared header first. Read it when deciding which
        blocks to carve into next.

    python3 tools/headercontention.py <unit> <unit> ...
        Verdict on a proposed assignment: which of those units contend, over
        what, and the largest mutually-independent subset. This is the Gate 1
        call.

Exit status is 0 either way -- contention is a cost to price in, not an error.
A concentrated round is sometimes the right call (it is often the only ground
available); the point is to choose it knowingly and to budget the merges.
"""
import re
import sys
from collections import defaultdict
from itertools import combinations

import srcpath

UBIQUITOUS = {"common.h", "types.h"}


def unit_headers(unit):
    """Project headers a unit includes, minus the ones every unit includes."""
    p = srcpath.unit_src(unit)
    if p is None:
        return set()
    return {h for h in re.findall(r'#include\s+"([^"]+)"', p.read_text())
            if h not in UBIQUITOUS}


def build_map():
    return {u: unit_headers(u) for u in srcpath.units()}


def print_full_map(hdrs):
    by_header = defaultdict(list)
    for unit, hs in hdrs.items():
        for h in hs:
            by_header[h].append(unit)
    print("Header contention map -- most-shared first.")
    print("A header shared by N units is N runners who may all edit it.\n")
    for h, us in sorted(by_header.items(), key=lambda kv: (-len(kv[1]), kv[0])):
        flag = "  <-- pick at most one of these per round" if len(us) > 1 else ""
        print(f"  {h:24} {len(us):>2} unit(s){flag}")
        if len(us) > 1:
            print(f"    {' '.join(sorted(us))}")
    solo = sorted(u for u, hs in hdrs.items()
                  if all(len(by_header[h]) == 1 for h in hs) or not hs)
    print(f"\n{len(solo)} unit(s) share no header with any other unit -- "
          "these are free to staff alongside anything:")
    for u in solo:
        print(f"  {u}")


def print_verdict(hdrs, chosen):
    unknown = [u for u in chosen if srcpath.unit_src(u) is None]
    if unknown:
        print(f"NOT A UNIT: {' '.join(unknown)}")
        print("(a unit is a src/<unit>.c basename; see python3 tools/srcpath.py)\n")
    chosen = [u for u in chosen if u not in unknown]
    if not chosen:
        return

    print(f"Proposed assignment: {' '.join(chosen)}\n")
    for u in chosen:
        hs = sorted(hdrs.get(u, ()))
        print(f"  {u:20} includes {' '.join(hs) if hs else '(no project header)'}")

    clashes = []
    for a, b in combinations(chosen, 2):
        shared = hdrs.get(a, set()) & hdrs.get(b, set())
        if shared:
            clashes.append((a, b, sorted(shared)))

    print()
    if not clashes:
        print("NO CONTENTION. No two of these units share a project header, so "
              "no two runners\nshould touch the same file. Expect clean merges.")
        return

    print(f"CONTENTION: {len(clashes)} pair(s) share a header.\n")
    for a, b, shared in clashes:
        print(f"  {a} <-> {b}   via {' '.join(shared)}")

    # Greedy largest-independent-set: drop the unit involved in the most
    # clashes until none remain. Greedy is fine -- the input is <= 5 units.
    keep = list(chosen)
    while True:
        deg = defaultdict(int)
        for a, b in combinations(keep, 2):
            if hdrs.get(a, set()) & hdrs.get(b, set()):
                deg[a] += 1
                deg[b] += 1
        if not deg:
            break
        keep.remove(max(deg, key=lambda u: (deg[u], u)))

    print(f"\nA mutually-independent subset ({len(keep)} of {len(chosen)}): "
          f"{' '.join(keep) if keep else '(none)'}")
    print("""
This is NOT an instruction to shrink the round. Concentrating runners on one
block is often the only ground available, and it is a legitimate trade. If you
take it, price it in:

  - expect a conflict on nearly every merge, and expect them to be
    COMPLEMENTARY (runners naming different slots of one struct from different
    call sites) -- union them and unify any field given two type names;
  - tell every contending runner: header edits strictly ADDITIVE, placed next
    to related declarations, and STATE any change to an EXISTING declaration in
    the final summary;
  - forbid cross-unit prototypes in the shared header -- they belong in the
    calling unit's own .c. This is the failure git does not mark, and it can
    stay latent for several merges before breaking a unit that never touched
    the declaration;
  - after resolving, re-verify EVERY match from EVERY contending runner
    individually, and grep the tree for each symbol the resolution declares;
  - budget head merge time explicitly, and expect to re-send fewer runners.""")


def main(argv):
    hdrs = build_map()
    if argv:
        print_verdict(hdrs, argv)
    else:
        print_full_map(hdrs)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
