#!/usr/bin/env python3
"""Rename a type, or a whole class family, everywhere it lives; then verify.

    python3 tools/renametype.py OLD NEW              # do it
    python3 tools/renametype.py OLD NEW --dry-run    # print every token it would change
    python3 tools/renametype.py OLD NEW --no-build   # edit only (batching)

WHY THIS EXISTS. A placeholder class name is not one identifier. `Class6B5CC`
lives in the object type, `Class6B5CCMethods`, `Class6B5CCSub14`, the macros
`CLASS6B5CC_FIELDS`/`CLASS6B5CC_SLOTS`, the header's guard `CLASS6B5CC_H`,
the header's FILE NAME, every method symbol `Class6B5CC__AddChild` (symbols
file + match report file name), `New_Class6B5CC`, the table
`gClass6B5CCMethods`, the track-4 ledger, and the warnings baseline. By hand
one of those is always missed. FINISHING-PLAN track 6 is dozens of these.

WHAT IT DOES, in order:
  1. collects every TOKEN in src/, include/, the symbols file and the docs
     rename.py rewrites that contains OLD (and OLD upper-cased, as a macro or
     guard prefix), and the new spelling of each; refuses when OLD is not a
     placeholder type name (tools/readability.py's patterns) unless
     --any-stem, when OLD also occurs inside a longer unrelated stem
     (`Class86` inside `Class86AA0`), or when any new token already exists;
  2. renames every token that is a SYMBOL (a symbols-file entry: methods,
     table, constructor) with tools/rename.py --no-build, one at a time, so
     the symbols file, the report file names and rename.py's Sony guards all
     apply exactly as for a single rename;
  3. rewrites the remaining tokens (types, macros, guards) as whole tokens in
     code and in the docs rename.py rewrites, as rename.py does (not
     PROGRESS.md, the archive, the rule docs it lists, a report's history
     sections, or a prose line already naming the new token);
  4. `git mv include/OLD.h include/NEW.h` when that header exists;
  5. rewrites the ledger (config/plan-state.json, through plan.py) and the
     warnings baseline (config/typeviews-warnings.txt);
  6. `make extract`, `./build-and-verify.sh`. A rename changes zero bytes: red
     means the rename is wrong, and the tool prints how to revert.

A commit made from this tool is REPLAYABLE (FINISHING-PLAN §3, "Renames
replay"): put the exact command in the commit message, and a hunk that
conflicts at merge is resolved by taking main's side and `tools/replay.py`.

It does not judge the name: the evidence goes in the class header's banner
and the reports (FINISHING-PLAN track 6).
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import rename      # noqa: E402
import readability  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
TOKEN = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
WARNINGS = ROOT / "config/typeviews-warnings.txt"


def files():
    return [p for p in rename.text_files() if p.exists()] + [WARNINGS]


def occurrences(stem, upper):
    """{token: new token} for every token holding OLD at a stem boundary."""
    # OLD is followed by a non-lowercase character (Methods, Sub14, __, end)
    # and preceded by start, `_`, or a lower-case prefix letter (gOLD, sOLD, New_OLD).
    lower_rx = re.compile(rf"(?:^|(?<=[_a-z])){re.escape(stem)}(?![a-z])|(?:^|(?<=[_a-z])){re.escape(stem)}(?=s$)")
    upper_rx = re.compile(rf"(?:^|(?<=_)){re.escape(upper)}(?=_|$)")
    found = {}
    for p in files():
        for tok in set(TOKEN.findall(p.read_text(errors="replace"))):
            if stem in tok and lower_rx.search(tok):
                found.setdefault(tok, set()).add(p)
            elif upper in tok and upper_rx.search(tok):
                found.setdefault(tok, set()).add(p)
    return found, lower_rx, upper_rx


def is_symbol(tok):
    addr, line = rename.symbol_address(tok)
    return addr is not None and line is not None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("old")
    ap.add_argument("new")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--any-stem", action="store_true",
                    help="allow an OLD that is not a placeholder type name")
    a = ap.parse_args()
    os.chdir(ROOT)
    old, new = a.old, a.new
    if not (IDENT.match(old) and IDENT.match(new)) or old == new:
        sys.exit("FATAL: OLD and NEW must be two different C identifiers")
    if not a.any_stem and not readability.placeholder_type_reason(old):
        sys.exit(f"FATAL: {old!r} is not a placeholder type name (tools/readability.py); "
                 f"pass --any-stem to rename a descriptive one")
    up_old, up_new = old.upper(), new.upper()
    found, lower_rx, upper_rx = occurrences(old, up_old)
    if not found:
        sys.exit(f"FATAL: no token contains {old!r}")
    mapping = {}
    for tok in sorted(found):
        t = lower_rx.sub(new, tok) if lower_rx.search(tok) else upper_rx.sub(up_new, tok)
        mapping[tok] = t
    # a longer stem that merely starts with OLD (Class86 in Class86AA0) was
    # excluded by the boundary rule; say so if any exist, so a wrong OLD shows
    longer = sorted({tok for p in files() for tok in TOKEN.findall(p.read_text(errors="replace"))
                     if old in tok and tok not in found})
    if longer:
        print(f"note: {len(longer)} token(s) contain {old!r} inside a longer stem and are NOT touched: "
              + ", ".join(longer[:8]))
    existing = {t for p in files() for t in TOKEN.findall(p.read_text(errors="replace"))}
    clash = sorted(t for t in mapping.values() if t in existing)
    if clash:
        sys.exit("FATAL: new token(s) already exist: " + ", ".join(clash[:10]))

    syms = [t for t in mapping if is_symbol(t)]
    plain = [t for t in mapping if t not in syms]
    hdr_old, hdr_new = ROOT / f"include/{old}.h", ROOT / f"include/{new}.h"
    print(f"renametype {old} -> {new}: {len(syms)} symbol(s) through rename.py, "
          f"{len(plain)} type/macro token(s)")
    for t in syms:
        print(f"  symbol  {t} -> {mapping[t]}")
    for t in plain:
        print(f"  token   {t} -> {mapping[t]}   ({len(found[t])} file(s))")
    if hdr_old.exists():
        print(f"  header  include/{old}.h -> include/{new}.h")
    for rel in sorted(rename.RULE_DOCS):
        rp = ROOT / rel
        if rp.exists() and old in rp.read_text(errors="replace"):
            print(f"  rule doc, NOT rewritten: {rel} mentions {old}")
    if a.dry_run:
        return

    for t in syms:
        r = subprocess.run([sys.executable, "tools/rename.py", t, mapping[t], "--no-build"],
                           capture_output=True, text=True)
        if r.returncode != 0:
            print(r.stdout[-1500:] + r.stderr[-1500:])
            sys.exit(f"FATAL: rename.py {t} {mapping[t]} refused; nothing after it was done. "
                     f"Revert with `git checkout -- . && git clean -fd docs/match-reports` if needed.")
    pats = [(re.compile(rf"\b{re.escape(t)}\b"), mapping[t]) for t in sorted(plain, key=len, reverse=True)]
    touched = []
    for p in files():
        rel = p.relative_to(ROOT).as_posix()
        text = p.read_text(errors="replace")
        out = text
        for rx, rep in pats:
            out = rx.sub(rep, out) if rename.is_code(p) else rename.sub_prose(rx, rep, out, p)
        if out != text:
            p.write_text(out)
            touched.append(rel)
    if hdr_old.exists():
        subprocess.run(["git", "mv", str(hdr_old), str(hdr_new)], check=True)
    import plan
    plan.ledger_rename({**mapping, f"include/{old}.h": f"include/{new}.h"})
    print(f"  rewrote {len(touched)} file(s)")
    if a.no_build:
        print("edited. You must run: make extract && ./build-and-verify.sh")
        return
    for cmd in (["make", "extract"], ["./build-and-verify.sh"]):
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            print((r.stdout + r.stderr)[-3000:])
            print("\nRENAME BROKE THE BUILD. A rename changes zero bytes, so the edit is wrong.")
            print("Revert with: git checkout -- . && git status   (then make extract && ./build-and-verify.sh)")
            sys.exit(1)
    print(f"OK: {old} -> {new}, image byte-identical. Commit with the command in the message.")


if __name__ == "__main__":
    main()
