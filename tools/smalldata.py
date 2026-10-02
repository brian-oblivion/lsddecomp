#!/usr/bin/env python3
"""The small data (.sdata/.sbss) still in the disassembly, grouped into runs.

    python3 tools/smalldata.py           # one line per run: address, section, unit, symbols
    python3 tools/smalldata.py -v        # ... and every symbol in each run, with its value

A run is a stretch of consecutive labels whose users (the src/ files that
name them) are the same. A run with one user is ready to move into that
unit (SDATA and SBSS in include/common.h). The lines to read by hand are flagged:

  SHARED   more than one unit names it: the owner is the unit whose range
           it sits in (link order), and the others keep an `extern`
  NO USER  no unit names it (an unnamed `D_` label: often a string the
           code reaches through a pointer, or a variable nothing reads);
           its owner is decided by its neighbours, and it needs a name

Reads asm/data/*.sdata.s and *.sbss.s, so run `make extract` first. What a
C unit already defines is not listed: splat writes no labels for it.
"""
import glob, re, sys

LABEL = re.compile(r'^dlabel (\S+)')
ADDR = re.compile(r'/\* ([0-9A-F]+) ')


def labels():
    """[(offset, section, name, first data line)] in address order."""
    out = []
    for f in glob.glob('asm/data/*.sdata.s') + glob.glob('asm/data/*.sbss.s'):
        sec = '.sdata' if f.endswith('.sdata.s') else '.sbss'
        lines = open(f).read().splitlines()
        for i, line in enumerate(lines):
            m = LABEL.match(line)
            if m and i + 1 < len(lines):
                a = ADDR.search(lines[i + 1])
                if a:
                    body = lines[i + 1].split('*/', 1)[-1].strip()
                    out.append((int(a.group(1), 16), sec, m.group(1), body))
    return sorted(out)


def users(names):
    src = {f: open(f, errors='replace').read() for f in glob.glob('src/**/*.c', recursive=True)}
    out = {}
    for n in names:
        rx = re.compile(rf'\b{re.escape(n)}\b')
        out[n] = sorted({f[len('src/'):-len('.c')] for f, t in src.items() if rx.search(t)})
    return out


def main():
    rows = labels()
    if not rows:
        sys.exit('no sdata/sbss labels under asm/data/ -- run make extract')
    who = users([r[2] for r in rows])
    runs = []
    for off, sec, name, body in rows:
        key = (sec, tuple(who[name]))
        if runs and runs[-1][0] == key:
            runs[-1][2].append((off, name, body))
        else:
            runs.append((key, off, [(off, name, body)]))
    verbose = '-v' in sys.argv
    for (sec, us), off, syms in runs:
        flag = 'NO USER' if not us else 'SHARED ' if len(us) > 1 else '       '
        print(f'0x{off:05X}  {sec:<6}  {flag}  {",".join(us) or "-":<32} {len(syms)} symbol(s)')
        if verbose:
            for o, n, body in syms:
                print(f'           0x{o:05X}  {n:<32} {body}')
    ready = sum(len(s) for (_, us), _, s in runs if len(us) == 1)
    print(f'{len(rows)} small-data symbol(s) still in the disassembly; {ready} in single-user runs')


if __name__ == '__main__':
    main()
