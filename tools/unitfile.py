#!/usr/bin/env python3
"""Rename or merge source units (files), keeping every reference in step.

    python3 tools/unitfile.py rename OLD NEW [--dry-run] [--no-build]
    python3 tools/unitfile.py merge A B [--as-b] [--dry-run] [--no-build]    # B's functions join A
    python3 tools/unitfile.py header OLD NEW [--dry-run] [--no-build]        # include/OLD.h, no unit
    python3 tools/unitfile.py check                                          # game files not snake_case

A unit's NAME lives in the splat yaml (`- [0xD294, c, code_d294]` and its
`.rodata` line), in its file names (src/OLD.c, the same-stem header
include/OLD.h and its guard), in the INCLUDE_ASM paths inside it, in the
warnings baseline, and in prose in the
reports and live docs. FINISHING-PLAN track 8 renames and merges units, so
this is one command, like rename.py for symbols.

NEW may carry a directory (`sound/SoundDriver`): splat then writes
src/sound/SoundDriver.c. The UNIT name everywhere else (tools,
reports) is the last component, which must stay unique (tools/srcpath.py).

rename: the yaml line(s) (text, `.rodata` and `.data`), `git mv` of src/OLD.c (and include/OLD.h when it
exists), whole-token rewrite of OLD / OLD_H guard in code and docs (not
PROGRESS.md or the archive, nor a report's history sections, nor a prose
line already naming NEW, as rename.py) and the warnings
baseline; deletes build/src/OLD.c.o; `make extract`; `./build-and-verify.sh`.
`OLD.h` is never rewritten while include/OLD.h exists.

PATHS ONLY for a type-named file (track 11): when OLD is also an identifier
in code (`Entity`, `SceneNode`), a token rewrite would rename the type, so
only file references move: `src/<dir>/OLD.c`, `asm/nonmatchings/<dir>/OLD`,
`OLD.c`, `OLD.h` (and include/OLD.h itself) and the guard `OLD_H` become
NEW's, and so does the warnings baseline's `OLD:` prefix. Prose that names the TYPE is left
alone. `rename Entity world/entity` is one such run; a move into a
directory that keeps the stem is the same rule with nothing but a path to
change.

header: include/OLD.h -> include/NEW.h for a header no unit owns (a class
header whose methods live in a unit named otherwise, `ObjM.h`): `OLD.h`
references and the guard. Paths only, always.

check: every game file in src/ and include/ whose stem is not snake_case
(FINISHING-PLAN track 11; Sony's src/psyq/ and include/psyq/ excluded).

merge: B must be the text subsegment IMMEDIATELY after A in the yaml, and if
both own a `.rodata` line, B's must immediately follow A's (else the merged
file's rodata would have to absorb what lies between: the tool refuses and
names it). The same holds for their `.data` lines, since a unit's C data is
one run. B's `#include`s join A's; B's body is appended after A's last
line under a `/* ---- merged from B ---- */` marker; B's yaml lines go; B's
file (and header, whose declarations you move by hand) go. With --as-b the
merged file takes B's name instead, and A's is the one rewritten (a file named
for a class that starts second). When only B owns rodata, its `.rodata` line
becomes the merged file's. The build usually
goes RED on the first try: two units' local views of the same thing now meet
in one file (`redefinition of`, `conflicting types`). Those are the job: keep
one declaration, delete the other, rebuild, until the oracle is byte-identical.
A merge changes zero bytes when it is right. Only the TU evidence
(tools/research/tuboundary.py) says it is right to do at all.
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


def find_data(rows, unit):
    """The unit's `.data` line(s) in the data list (track 14: its C tables)."""
    return [r for r in rows if r[2] == ".data" and unit_stem(r[3]) == unit]


def token_rx(old):
    """OLD as a whole token, except in `OLD.h` while include/OLD.h exists: a
    unit stem that is also a live header's stem (a merge keeps B's header;
    round 101: replay rewrote `#include "class_3bb8c.h"`, six lines)."""
    keep_h = r"(?!\.h\b)" if (ROOT / f"include/{old}.h").exists() else ""
    return re.compile(rf"(?<![A-Za-z0-9_/.]){re.escape(old)}(?![A-Za-z0-9_]){keep_h}"
                      if "/" in old else rf"(?<![A-Za-z0-9_]){re.escape(old)}(?![A-Za-z0-9_]){keep_h}")


def text_files():
    """rename.py's files, the README (its code map names paths) and the
    warnings baseline."""
    return [q for q in rename.text_files() if q.exists()] + [ROOT / "README.md", WARNINGS]


def rename_map(old, new, old_c, new_c, header_moved, paths_only):
    """The text map of one `rename` or `header` run. old_c/new_c are the .c
    paths relative to ROOT (None for `header`)."""
    m = {}
    if old_c and old_c != new_c:
        m[old_c] = new_c
        strip = lambda c: c[len("src/"):-len(".c")]
        m[f"asm/nonmatchings/{strip(old_c)}"] = f"asm/nonmatchings/{strip(new_c)}"
    if new != old:
        if old_c:
            m[f"{old}.c"] = f"{new}.c"
        if header_moved:
            m[f"{old}.h"] = f"{new}.h"
        m[f"{old.upper()}_H"] = f"{new.upper()}_H"
        if not paths_only:
            m[old] = new
    return {o: n for o, n in m.items() if o != n}      # SceneNode's SCENENODE_H moves; Entity's ENTITY_H does not


def sub_prose_all(pairs, text, path, keep_lines=()):
    """rename.sub_prose for several keys at once. Each line is judged ONCE,
    on its original text, and then takes every key: applied one key at a
    time, the first key's rewrite makes the line "already name NEW" for the
    next, and `src/world/Entity.c` became `src/world/entity.c` beside a bare
    `Entity.c` left behind."""
    nxs = [re.compile(rf"(?<![A-Za-z0-9_]){re.escape(n)}(?![A-Za-z0-9_])") for _, n in pairs]
    frozen = rename.frozen_spans(text) if Path(path).parent.name == "match-reports" else []
    out, pos = [], 0
    for line in text.split("\n"):
        at, pos = pos, pos + len(line) + 1
        keep = (any(nx.search(line) for nx in nxs) or line in keep_lines
                or line.startswith(rename.RENAMED_NOTE) or any(s <= at < e for s, e in frozen))
        if not keep:
            for rx, n in pairs:
                line = rx.sub(n, line)
        out.append(line)
    return "\n".join(out)


def warnings_prefix(old, new, dry=False):
    """The warnings baseline keys a line by unit name (`Entity: warning:`)."""
    if old == new or not WARNINGS.exists():
        return 0
    text = WARNINGS.read_text()
    out, n = re.subn(rf"^{re.escape(old)}:", f"{new}:", text, flags=re.M)
    if n and not dry:
        WARNINGS.write_text(out)
    return n


def rewrite_tokens(mapping, skip=()):
    rxs = [(token_rx(o), n) for o, n in sorted(mapping.items(), key=lambda kv: -len(kv[0]))]
    touched = []
    for p in text_files():
        if p in skip or not p.exists():
            continue
        text = p.read_text(errors="replace")
        out = text
        if rename.is_code(p):
            for rx, n in rxs:
                out = rx.sub(n, out)
        else:
            out = sub_prose_all(rxs, out, p)
        if out != text:
            p.write_text(out)
            touched.append(p.relative_to(ROOT).as_posix())
    return touched


def rule_doc_mentions(mapping):
    """RULE docs are never rewritten (rename.py): list their mentions for the head."""
    rxs = [token_rx(o) for o in mapping]
    for rel in sorted(rename.RULE_DOCS):
        rp = ROOT / rel
        if rp.exists():
            n = sum(len(rx.findall(rp.read_text(errors="replace"))) for rx in rxs)
            if n:
                print(f"  rule doc, NOT rewritten: {rel} ({n} mention(s)); the head updates it")


SNAKE = re.compile(r"^[a-z][a-z0-9_]*$")
NOT_GAME = {"include_asm.h"}          # splat regenerates it on every make extract


def not_snake():
    """Game files whose stem is not snake_case (track 11's measurement)."""
    files = [p for p in srcpath.src_files() if "psyq" not in p.relative_to(ROOT / "src").parts]
    files += [p for p in sorted((ROOT / "include").glob("*.h")) if p.name not in NOT_GAME]
    return [p.relative_to(ROOT).as_posix() for p in files if not SNAKE.match(p.stem)]


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
    ro = ro + find_data(rows, old)
    # A type-named file (the stem is also an identifier in code) moves by
    # paths only: a token rewrite would rename the type (track 11).
    hit = code_identifier(old) if new != old else None
    src_old = srcpath.unit_src(old)
    old_path = src_old.relative_to(ROOT / "src").with_suffix("").as_posix()
    src_new = ROOT / "src" / f"{new_path}.c"
    hdr_old, hdr_new = ROOT / f"include/{old}.h", ROOT / f"include/{new}.h"
    moving_hdr = hdr_old.exists() and new != old
    if moving_hdr and hdr_new.exists() and hdr_new.resolve() != hdr_old.resolve():
        sys.exit(f"FATAL: include/{new}.h already exists")
    print(f"unitfile rename {old} -> {new_path}" + (f"  (paths only: {old} is a type, {hit})" if hit else ""))
    print(f"  yaml: {1 + len(ro)} line(s); src: {src_old.relative_to(ROOT)} -> {src_new.relative_to(ROOT)}")
    if moving_hdr:
        print(f"  header: include/{old}.h -> include/{new}.h")
    mapping = rename_map(old, new, f"src/{old_path}.c", f"src/{new_path}.c", moving_hdr, bool(hit))
    rule_doc_mentions(mapping)
    if a.dry_run:
        return 0
    for i, _, _, name in [c] + ro:
        m = SEG.match(lines[i])
        lines[i] = m.group(1) + m.group(3) + m.group(4) + new_path + m.group(6)
    YAML.write_text("\n".join(lines))
    src_new.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["git", "mv", str(src_old), str(src_new)], check=True)
    if moving_hdr:
        subprocess.run(["git", "mv", str(hdr_old), str(hdr_new)], check=True)
    text = src_new.read_text()
    text = text.replace(f'"asm/nonmatchings/{old_path}"', f'"asm/nonmatchings/{new_path}"')
    src_new.write_text(text)
    touched = rewrite_tokens(mapping)
    warnings_prefix(old, new) if hit else None
    obj = ROOT / "build/src" / f"{old_path}.c.o"
    if obj.exists():
        obj.unlink()
    print(f"  rewrote {len(touched)} file(s)")
    return build(a.no_build)


def cmd_header(a):
    old, new = a.old, a.new
    if not SNAKE.match(new) and not re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", new):
        sys.exit(f"FATAL: {new!r} is not a header stem")
    hdr_old, hdr_new = ROOT / f"include/{old}.h", ROOT / f"include/{new}.h"
    if not hdr_old.exists():
        sys.exit(f"FATAL: include/{old}.h does not exist")
    if hdr_new.exists() and hdr_new.resolve() != hdr_old.resolve():
        sys.exit(f"FATAL: include/{new}.h already exists")
    if old in srcpath.units():
        print(f"  note: unit {old} keeps its name; `rename {old} ...` would have moved both")
    mapping = rename_map(old, new, None, None, True, True)
    print(f"unitfile header include/{old}.h -> include/{new}.h")
    rule_doc_mentions(mapping)
    if a.dry_run:
        return 0
    subprocess.run(["git", "mv", str(hdr_old), str(hdr_new)], check=True)
    touched = rewrite_tokens(mapping)
    print(f"  rewrote {len(touched)} file(s)")
    return build(a.no_build)


def cmd_check(a):
    left = not_snake()
    for f in left:
        print(f"  {f}")
    print(f"{len(left)} game file(s) not snake_case (FINISHING-PLAN track 11)")
    return 0


def cmd_merge(a):
    ua, ub = a.a, a.b
    # the merged file keeps A's name, or with --as-b B's (A comes first in ROM,
    # but B is the class the file is named for: round 101, code_179d8_o+CdDriver)
    gone, keep = (ua, ub) if a.as_b else (ub, ua)
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
    da, db = find_data(rows, ua), find_data(rows, ub)
    if len(da) > 1 or len(db) > 1:
        sys.exit("FATAL: a unit with more than one .data line; merge by hand")
    if da and db:
        nxt_d = next((r for r in rows if r[1] > da[0][1]), None)
        if nxt_d is None or nxt_d[0] != db[0][0]:
            sys.exit(f"FATAL: {ub}'s .data line does not follow {ua}'s; the merged file's data "
                     f"would not be one run (CLEANUP.md track 14, archive/process). Decide that by hand.")
    if roa and rob:
        between = [r for r in rows if roa[0][1] < r[1] < rob[0][1] and r[2] not in TEXT_TYPES]
        if between:
            sys.exit(f"FATAL: rodata between {ua}'s and {ub}'s .rodata lines "
                     f"({', '.join(f'{r[2]} 0x{r[1]:X}' for r in between)}); the merged file would have to "
                     f"own it. Decide that by hand (FINISHING-PLAN track 8).")
    hit = code_identifier(gone)
    if hit:
        sys.exit(f"FATAL: {gone} is also an identifier in code ({hit}); a token rewrite would rename it. "
                 + ("Merge by hand." if a.as_b else f"Keep its name instead: merge {ua} {ub} --as-b"))
    sa, sb = srcpath.unit_src(ua), srcpath.unit_src(ub)
    dest = sb if a.as_b else sa
    print(f"unitfile merge {ua} + {ub} as {keep}: {sb.relative_to(ROOT)} appended to "
          f"{sa.relative_to(ROOT)}, written to {dest.relative_to(ROOT)}")
    # The merged file's rodata is the first unit's line (the second's, when
    # the first owns none: rounds 100 and 101 renamed it by hand), named KEEP.
    ro_keep = (roa or rob)[0][0] if (roa or rob) else None
    if rob and not roa:
        print(f"  {ua} owns no rodata: {ub}'s .rodata line becomes the merged file's")
    hg = ROOT / f"include/{gone}.h"
    if hg.exists():
        print(f"  include/{gone}.h stays: move what it declares into the surviving header by hand, then delete it")
    if a.dry_run:
        return 0
    # the surviving text line is A's (it holds the start address); the
    # surviving .rodata line is ro_keep; both are named KEEP
    # likewise the data: A's `.data` line (B's when A has none) holds the run
    d_keep = (da or db)[0][0] if (da or db) else None
    drop = {cb[0]} | {r[0] for r in roa + rob if r[0] != ro_keep} | {r[0] for r in da + db if r[0] != d_keep}
    keep_path = dest.relative_to(ROOT / "src").with_suffix("").as_posix()
    out = []
    for i, l in enumerate(lines):
        if i in drop:
            continue
        if i in (ca[0], ro_keep, d_keep):
            m = SEG.match(l)
            l = m.group(1) + m.group(3) + m.group(4) + keep_path + m.group(6)
        out.append(l)
    YAML.write_text("\n".join(out))
    ta, tb = sa.read_text(), sb.read_text()
    inc_a = set(re.findall(r'^#include\s+["<][^">]+[">]', ta, re.M))
    new_inc = [i for i in re.findall(r'^#include\s+["<][^">]+[">]', tb, re.M) if i not in inc_a]
    tb = re.sub(r'^#include\s+["<][^">]+[">][ \t]*\n', "", tb, flags=re.M)
    if new_inc:
        last = list(re.finditer(r'^#include\s+["<][^">]+[">][ \t]*\n', ta, re.M))
        pos = last[-1].end() if last else 0
        ta = ta[:pos] + "".join(i + "\n" for i in new_inc) + ta[pos:]
    body = ta.rstrip("\n") + f"\n\n/* ---- merged from {ub} ---- */\n\n" + tb.lstrip("\n")
    body = body.replace(f'"asm/nonmatchings/{gone}"', f'"asm/nonmatchings/{keep}"')
    dest.write_text(body)
    subprocess.run(["git", "rm", "-qf", str(sb if dest == sa else sa)], check=True)
    subprocess.run(["git", "add", str(dest)], check=True)
    rewrite_tokens({gone: keep}, skip={dest})
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
    m.add_argument("--as-b", action="store_true", help="the merged file takes B's name (A's is rewritten)")
    h = sub.add_parser("header")
    h.add_argument("old")
    h.add_argument("new")
    sub.add_parser("check")
    for p in (r, m, h):
        p.add_argument("--dry-run", action="store_true")
        p.add_argument("--no-build", action="store_true")
    a = ap.parse_args()
    os.chdir(ROOT)
    sys.exit({"rename": cmd_rename, "merge": cmd_merge, "header": cmd_header, "check": cmd_check}[a.cmd](a))


if __name__ == "__main__":
    main()
