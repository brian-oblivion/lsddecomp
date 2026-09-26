#!/usr/bin/env python3
"""Rename or merge source units (files), keeping every reference in step.

    python3 tools/unitfile.py rename OLD NEW [--dry-run] [--no-build]
    python3 tools/unitfile.py merge A B [--dry-run] [--no-build]    # B's functions join A

A unit's NAME lives in the splat yaml (`- [0xD294, c, code_d294]` and its
`.rodata` line), in its file names (src/OLD.c, the same-stem header
include/OLD.h and its guard), in the INCLUDE_ASM paths inside it, in the
ledger (track 3/7 units passed), in the warnings baseline, and in prose in the
reports and live docs. FINISHING-PLAN track 8 renames and merges units, so
this is one command, like rename.py for symbols.

NEW may carry a directory (`sound/SoundDriver`): splat then writes
src/sound/SoundDriver.c. The UNIT name everywhere else (ledger, tools,
reports) is the last component, which must stay unique (tools/srcpath.py).

rename: the yaml line(s), `git mv` of src/OLD.c (and include/OLD.h when it
exists), whole-token rewrite of OLD / OLD_H guard in code and docs (not
PROGRESS.md or the archive, as rename.py), the ledger and the warnings
baseline; deletes build/src/OLD.c.o; `make extract`; `./build-and-verify.sh`.

merge: B must be the text subsegment IMMEDIATELY after A in the yaml, and if
both own a `.rodata` line, B's must immediately follow A's (else the merged
file's rodata would have to absorb what lies between: the tool refuses and
names it). B's `#include`s join A's; B's body is appended after A's last
line under a `/* ---- merged from B ---- */` marker; B's yaml lines go; B's
file (and header, whose declarations you move by hand) go. The build usually
goes RED on the first try: two units' local views of the same thing now meet
in one file (`redefinition of`, `conflicting types`). Those are the job: keep
one declaration, delete the other, rebuild, until the oracle is byte-identical.
A merge changes zero bytes when it is right. Only the TU evidence
(tools/tuboundary.py) says it is right to do at all.
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import rename   # noqa: E402
import srcpath  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
YAML = ROOT / "config/splat.slps01556.lsdde.yaml"
WARNINGS = ROOT / "config/typeviews-warnings.txt"
SEG = re.compile(r"^(\s*- \[0x([0-9A-Fa-f]+),\s*)(\.?[a-z]+)(,\s*)([\w/]+)(\s*\].*)$")
TEXT_TYPES = {"c", "asm", "hasm", "o", "pad", "bin"}


def yaml_lines():
    return YAML.read_text().split("\n")


def seg_rows(lines):
    """[(line index, offset, type, name)] for every subsegment line."""
    out = []
    for i, line in enumerate(lines):
        m = SEG.match(line)
        if m:
            out.append((i, int(m.group(2), 16), m.group(3), m.group(5)))
    return out


def unit_stem(name):
    return name.rsplit("/", 1)[-1]


def find_unit(rows, unit):
    c = [r for r in rows if r[2] == "c" and unit_stem(r[3]) == unit]
    ro = [r for r in rows if r[2] == ".rodata" and unit_stem(r[3]) == unit]
    return (c[0] if c else None), ro


def rewrite_tokens(mapping, skip=()):
    rxs = [(re.compile(rf"(?<![A-Za-z0-9_]){re.escape(o)}(?![A-Za-z0-9_])"), n)
           for o, n in sorted(mapping.items(), key=lambda kv: -len(kv[0]))]
    touched = []
    for p in [q for q in rename.text_files() if q.exists()] + [WARNINGS]:
        if p in skip:
            continue
        text = p.read_text(errors="replace")
        out = text
        for rx, n in rxs:
            out = rx.sub(n, out)
        if out != text:
            p.write_text(out)
            touched.append(p.relative_to(ROOT).as_posix())
    return touched


def code_identifier(name):
    """Is NAME also an identifier in code (a class named like its unit:
    DreamSys, Entity)? Then a token rewrite would rename the class too."""
    rx = re.compile(rf"(?<![A-Za-z0-9_]){re.escape(name)}(?![A-Za-z0-9_])")
    for p in list(srcpath.src_files()) + sorted((ROOT / "include").glob("*.h")):
        t = re.sub(r"/\*.*?\*/", " ", p.read_text(errors="replace"), flags=re.S)
        t = re.sub(r'"(?:\\.|[^"\\])*"', '""', t)
        if rx.search(t):
            return p.relative_to(ROOT).as_posix()
    return None


def build(no_build):
    if no_build:
        print("edited. You must run: make extract && ./build-and-verify.sh")
        return 0
    for cmd in (["make", "extract"], ["./build-and-verify.sh"]):
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            print((r.stdout + r.stderr)[-3000:])
            return 1
    print("OK: image byte-identical. Commit with the command in the message.")
    return 0


def cmd_rename(a):
    old, new_path = a.old, a.new
    new = unit_stem(new_path)
    if not re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", new) or not re.match(r"^[\w/]+$", new_path):
        sys.exit(f"FATAL: {new_path!r} is not a unit name")
    if new != old and new in srcpath.units():
        sys.exit(f"FATAL: a unit named {new} already exists (unit names are unique)")
    lines = yaml_lines()
    rows = seg_rows(lines)
    c, ro = find_unit(rows, old)
    if c is None:
        sys.exit(f"FATAL: no `c` subsegment named {old} in the yaml")
    hit = code_identifier(old)
    if hit:
        sys.exit(f"FATAL: {old} is also an identifier in code ({hit}); a token rewrite would rename it "
                 f"too. Rename the file by hand (yaml line, git mv, INCLUDE_ASM paths).")
    src_old = srcpath.unit_src(old)
    src_new = ROOT / "src" / f"{new_path}.c"
    hdr_old, hdr_new = ROOT / f"include/{old}.h", ROOT / f"include/{new}.h"
    if hdr_new.exists() and hdr_old.exists() and new != old:
        sys.exit(f"FATAL: include/{new}.h already exists")
    print(f"unitfile rename {old} -> {new_path}")
    print(f"  yaml: {1 + len(ro)} line(s); src: {src_old.relative_to(ROOT)} -> {src_new.relative_to(ROOT)}")
    if hdr_old.exists():
        print(f"  header: include/{old}.h -> include/{new}.h")
    if a.dry_run:
        return 0
    for i, _, _, name in [c] + ro:
        m = SEG.match(lines[i])
        lines[i] = m.group(1) + m.group(3) + m.group(4) + new_path + m.group(6)
    YAML.write_text("\n".join(lines))
    src_new.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["git", "mv", str(src_old), str(src_new)], check=True)
    if hdr_old.exists():
        subprocess.run(["git", "mv", str(hdr_old), str(hdr_new)], check=True)
    text = src_new.read_text()
    text = text.replace(f'"asm/nonmatchings/{old}"', f'"asm/nonmatchings/{new_path}"')
    src_new.write_text(text)
    touched = rewrite_tokens({old: new, f"{old.upper()}_H": f"{new.upper()}_H"})
    import plan
    plan.ledger_rename({old: new})
    obj = ROOT / "build/src" / f"{old}.c.o"
    if obj.exists():
        obj.unlink()
    print(f"  rewrote {len(touched)} file(s)")
    return build(a.no_build)


def cmd_merge(a):
    ua, ub = a.a, a.b
    lines = yaml_lines()
    rows = seg_rows(lines)
    ca, roa = find_unit(rows, ua)
    cb, rob = find_unit(rows, ub)
    if ca is None or cb is None:
        sys.exit("FATAL: both units must be `c` subsegments in the yaml")
    text_rows = [r for r in rows if r[2] in TEXT_TYPES and r[1] >= ca[1]]
    nxt = next((r for r in text_rows if r[1] > ca[1]), None)
    if nxt is None or nxt[0] != cb[0]:
        sys.exit(f"FATAL: {ub} is not the text subsegment right after {ua} "
                 f"(next is {nxt[3] if nxt else 'nothing'}); only adjacent units merge")
    if len(roa) > 1 or len(rob) > 1:
        sys.exit("FATAL: a unit with more than one .rodata line; merge by hand")
    if rob:
        if not roa:
            sys.exit(f"FATAL: {ub} owns rodata and {ua} does not; the merged file's rodata would start "
                     f"at {ub}'s line. Rename {ub}'s .rodata line to {ua} by hand if nothing lies between.")
        between = [r for r in rows if roa[0][1] < r[1] < rob[0][1] and r[2] not in TEXT_TYPES]
        if between:
            sys.exit(f"FATAL: rodata between {ua}'s and {ub}'s .rodata lines "
                     f"({', '.join(f'{r[2]} 0x{r[1]:X}' for r in between)}); the merged file would have to "
                     f"own it. Decide that by hand (FINISHING-PLAN track 8).")
    hit = code_identifier(ub)
    if hit:
        sys.exit(f"FATAL: {ub} is also an identifier in code ({hit}); merge it the other way round "
                 f"or by hand")
    sa, sb = srcpath.unit_src(ua), srcpath.unit_src(ub)
    print(f"unitfile merge {ub} into {ua}: yaml lines {cb[0] + 1}" + (f", {rob[0][0] + 1}" if rob else "")
          + f" removed; {sb.relative_to(ROOT)} appended to {sa.relative_to(ROOT)}")
    hb = ROOT / f"include/{ub}.h"
    if hb.exists():
        print(f"  include/{ub}.h stays: move what it declares into the surviving header by hand, then delete it")
    if a.dry_run:
        return 0
    drop = {cb[0]} | ({rob[0][0]} if rob else set())
    YAML.write_text("\n".join(l for i, l in enumerate(lines) if i not in drop))
    ta, tb = sa.read_text(), sb.read_text()
    inc_a = set(re.findall(r'^#include\s+["<][^">]+[">]', ta, re.M))
    new_inc = [i for i in re.findall(r'^#include\s+["<][^">]+[">]', tb, re.M) if i not in inc_a]
    tb = re.sub(r'^#include\s+["<][^">]+[">][ \t]*\n', "", tb, flags=re.M)
    tb = tb.replace(f'"asm/nonmatchings/{ub}"', f'"asm/nonmatchings/{ua}"')
    if new_inc:
        last = list(re.finditer(r'^#include\s+["<][^">]+[">][ \t]*\n', ta, re.M))
        pos = last[-1].end() if last else 0
        ta = ta[:pos] + "".join(i + "\n" for i in new_inc) + ta[pos:]
    sa.write_text(ta.rstrip("\n") + f"\n\n/* ---- merged from {ub} ---- */\n\n" + tb.lstrip("\n"))
    subprocess.run(["git", "rm", "-qf", str(sb)], check=True)
    rewrite_tokens({ub: ua}, skip={sa})
    import plan
    plan.ledger_rename({ub: ua}, drop_duplicates=True)
    for u in (ua, ub):
        obj = ROOT / "build/src" / f"{u}.c.o"
        if obj.exists():
            obj.unlink()
    rc = build(a.no_build)
    if rc:
        print("\nExpected on a first merge: resolve the duplicate declarations the compiler lists, "
              "then ./build-and-verify.sh until byte-identical. `git checkout -- . && git status` undoes it.")
    return rc


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("rename")
    r.add_argument("old")
    r.add_argument("new")
    m = sub.add_parser("merge")
    m.add_argument("a")
    m.add_argument("b")
    for p in (r, m):
        p.add_argument("--dry-run", action="store_true")
        p.add_argument("--no-build", action="store_true")
    a = ap.parse_args()
    os.chdir(ROOT)
    sys.exit(cmd_rename(a) if a.cmd == "rename" else cmd_merge(a))


if __name__ == "__main__":
    main()
