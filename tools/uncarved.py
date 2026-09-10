#!/usr/bin/env python3
"""Gate 2: census the UNCARVED monoliths, screened per function.

WHY THIS EXISTS.  Gate 2 in docs/PARALLEL-RUNS.md tells the head to carve when
the fresh queue is thin, and to pick the slice by BLOCKER DENSITY rather than
by "next".  Every head has rebuilt that census by hand, and the doc records
three separate written-down censuses going stale by being acted on -- which is
why it now says "do not work from a written list" three times and gives the
`grep -c '^glabel'` one-liner instead.  But that one-liner counts FUNCTIONS,
not WORKABLE functions, and those are wildly different numbers: `class_3bb8c_h`
counted 17 and held 4.

So this is the Gate 2 companion to tools/nearmiss.py, built the same way and
for the same reason.  It answers the question Gate 2 actually asks -- "is there
still ground out there worth carving, and where?" -- as a measurement instead
of a paragraph.

It runs the SAME FOUR SCREENS as nearmiss.py, and it shells out to the
canonical grep forms from the doc rather than re-expressing them.  That matters
most for `nop_mflo_mfhi`: the window runs FORWARD (an mflo/mfhi FOLLOWED WITHIN
TWO INSTRUCTIONS BY a mult/div), `grep -A2` prints the two FOLLOWING lines, and
every re-implementation so far has inverted it (rounds 15, 16) or added a false
qualifier (round 24).  Do not rewrite the window; call the grep.

`addiu_at` is NOT a blocker -- resolved round 21, maspsx `--addiu-at`.  It is
reported, tagged, and NOT counted, exactly as nearmiss.py does it, because
older notes still blame it and the tag is how you tell "this note's verdict
predates the fix" from "this is really blocked".

BIOS trampolines are the fourth screen and they are why raw glabel counts
mislead.  A trampoline (`addiu $t2,$zero,0xB0 / jr $t2 / addiu $t1,$zero,N`)
has no gp_rel, no addiu $at and no mflo/mfhi, so all the other screens PASS it
while no C compiles to it.  A trampoline-dense segment therefore reads as the
cleanest ground in the executable while being the least matchable.

Usage:
    python3 tools/uncarved.py              # per-segment summary, best first
    python3 tools/uncarved.py --functions  # every function, with its screens
    python3 tools/uncarved.py --windows 20 # rolling clean-density windows
"""
import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# splat emits three fields: FILEOFS VRAM WORD.  Anchoring `*/` after only
# two silently matches nothing, which zeroes every word count AND disables
# the per-function mflo window (an empty instruction list never loops).
INSTR = re.compile(
    r'^\s+/\* ([0-9A-Fa-f]+) ([0-9A-Fa-f]{8}) ([0-9A-Fa-f]{8}) \*/\s*(.*)$')


def uncarved_segments():
    """Top-level asm/*.s, excluding the Psy-Q SDK and the header segment."""
    out = []
    d = os.path.join(ROOT, 'asm')
    for fn in sorted(os.listdir(d)):
        if not fn.endswith('.s'):
            continue
        base = fn[:-2]
        if base.startswith('psyq_') or base == 'header':
            continue
        out.append(base)
    return out


def split_functions(seg):
    """[(name, [instruction-lines])] for one monolithic segment."""
    path = os.path.join(ROOT, 'asm', seg + '.s')
    funcs, cur = [], None
    for ln in open(path, errors='replace'):
        ln = ln.rstrip('\n')
        m = re.match(r'^glabel (\w+)', ln)
        if m:
            cur = (m.group(1), [])
            funcs.append(cur)
        elif cur is not None:
            cur[1].append(ln)
    return funcs


def canonical_mflo_screen(path):
    """The canonical FORWARD grep, shelled out verbatim from PARALLEL-RUNS.

    Do not re-express this window in Python.  `grep -A2` prints the two
    FOLLOWING lines; that is what makes the direction right by construction,
    and four attempts to improve it have broken it.
    """
    cmd = (r"grep -A2 -nE '\b(mflo|mfhi)\b' " + repr(path) +
           r" | grep -qE '\b(mult|multu|div|divu)\b'")
    return subprocess.call(cmd, shell=True, cwd=ROOT,
                           stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL) == 0


def screen(seg):
    """Per-function screens for one segment.

    Returns [(name, words, tags)].  A function is WORKABLE iff it has no tag
    other than the informational addiu_at one.
    """
    path = os.path.join(ROOT, 'asm', seg + '.s')
    # One grep per segment for the mflo window, then attribute by line number,
    # so the canonical shell form still decides the direction.
    seg_has_mflo = canonical_mflo_screen(path)

    out = []
    for name, lines in split_functions(seg):
        body = '\n'.join(lines)
        instr = [l for l in lines if INSTR.match(l)]
        tags = []
        if 'gp_rel' in body:
            tags.append('gp_rel')
        if seg_has_mflo:
            # Same window, restricted to this function's own instructions.
            # Tag ONCE however many sites hit: a function with three mflo/div
            # pairs is one blocked function, not three, and appending per site
            # both duplicates the tag and inflates the per-tag totals.
            for j, l in enumerate(instr):
                if re.search(r'\b(mflo|mfhi)\b', l):
                    if any(re.search(r'\b(mult|multu|div|divu)\b', k)
                           for k in instr[j + 1:j + 3]):
                        tags.append('nop_mflo_mfhi')
                        break
        if re.search(r'jr\s+\$t2', body):
            tags.append('TRAMPOLINE')
        if re.search(r'addiu\s+\$at,\s*\$at,\s*%lo', body):
            tags.append('addiu_at(RESOLVED-not-a-blocker)')
        out.append((name, len(instr), tags))
    return out


def is_workable(tags):
    return not [t for t in tags if not t.startswith('addiu_at(')]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--functions', action='store_true',
                    help='list every function with its screens')
    ap.add_argument('--windows', type=int, metavar='N',
                    help='rolling clean-density windows of N functions')
    ap.add_argument('--segment', help='restrict to one segment')
    args = ap.parse_args()

    segs = uncarved_segments()
    if args.segment:
        if args.segment not in segs:
            print('not an uncarved segment: %s\nuncarved: %s'
                  % (args.segment, ' '.join(segs)), file=sys.stderr)
            return 2
        segs = [args.segment]

    rows, tot_clean, tot_all = [], 0, 0
    blocked_by = {}
    for seg in segs:
        fns = screen(seg)
        clean = [f for f in fns if is_workable(f[2])]
        tot_clean += len(clean)
        tot_all += len(fns)
        for _, _, tags in fns:
            for t in tags:
                if t.startswith('addiu_at('):
                    continue
                blocked_by[t] = blocked_by.get(t, 0) + 1
        rows.append((seg, fns, clean))

    print('UNCARVED GAME CODE -- %d functions in %d segments; '
          '%d blocker-clean' % (tot_all, len(segs), tot_clean))
    print('  blocked/not-C by: ' + ('  '.join(
        '%s=%d' % (k, v) for k, v in sorted(blocked_by.items())) or '(none)')
        + '   (per-tag; a function blocked two ways counts in both)')
    print()
    print('`addiu_at` is NOT counted -- resolved round 21. TRAMPOLINE means no C')
    print('compiles to it at all, which every other screen passes silently.')
    print()

    rows.sort(key=lambda r: (-len(r[2]), -len(r[1])))
    print('%-18s %5s %6s %7s  %s' % ('segment', 'fns', 'clean', 'clean-w',
                                     'clean functions'))
    for seg, fns, clean in rows:
        cw = sum(f[1] for f in clean)
        names = ' '.join('%s(%dw)' % (f[0], f[1]) for f in clean[:4])
        if len(clean) > 4:
            names += ' +%d more' % (len(clean) - 4)
        print('%-18s %5d %6d %7d  %s' % (seg, len(fns), len(clean), cw,
                                         names or '--'))

    if args.functions:
        for seg, fns, _ in rows:
            print('\n=== %s ===' % seg)
            for i, (name, w, tags) in enumerate(fns):
                print('%4d %5dw %-18s %s' % (
                    i, w, name, ','.join(tags) if tags else 'CLEAN'))

    if args.windows:
        n = args.windows
        for seg, fns, _ in rows:
            if len(fns) < n:
                continue
            print('\n=== %s: rolling %d-function windows ===' % (seg, n))
            for i in range(0, len(fns) - n + 1, n):
                win = fns[i:i + n]
                c = [f for f in win if is_workable(f[2])]
                print('  [%3d..%3d] %2d/%d clean, %4dw'
                      % (i, i + n - 1, len(c), n, sum(f[1] for f in c)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
