#!/usr/bin/env python3
"""Replay a merged branch's renames on the merged tree (FINISHING-PLAN §3).

    python3 tools/replay.py                 # during a merge: both sides since the merge base
    python3 tools/replay.py R [R ...]       # named ranges, e.g. after it: HEAD^2..HEAD^1 HEAD^1..HEAD^2
    python3 tools/replay.py ... --dry-run   # list what is left, change nothing

Every rename commit's first line is its exact command (`python3
tools/rename.py OLD NEW`, `renametype.py`, `unitfile.py rename|merge`). Once
the branch is merged its renames are already in the tree, so re-running a
command is refused (NEW exists, OLD is gone: round 90). What is still wrong is
each side's OLD tokens in the OTHER side's text: a conflicted hunk resolved to
one side, lines that never conflicted at all (git cannot flag them), and a
file one side edited and the other renamed (modify/delete: keep the renamed
file). This tool re-applies each command's TOKEN REWRITE, main's then the
branch's, each in its own order, to those leftovers, over the files the
original tool rewrites; during a merge it first rebuilds the ledger by a
three-way JSON merge (a hand-resolved ledger is replaced); then `make extract` and
`./build-and-verify.sh`.

At a conflict: take main's side of each hunk that differs only by a rename,
run this, then the oracle. A rename changes zero bytes; red means a hunk was
resolved wrong. Commit with this command as the message's first line.
"""
import argparse
import json
import re
import shlex
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import rename      # noqa: E402
import renametype  # noqa: E402
import unitfile    # noqa: E402
import plan        # noqa: E402

ROOT = Path(__file__).resolve().parent.parent


def git(*args):
    return subprocess.run(["git", *args], cwd=ROOT, capture_output=True, text=True, check=True).stdout


def commands(rng):
    out = []
    for subj in git("log", "--reverse", "--format=%s", rng).splitlines():
        w = shlex.split(subj) if subj.startswith("python3 tools/") else []
        w = [x for x in w if not x.startswith("--")]
        if len(w) == 4 and w[1] in ("tools/rename.py", "tools/renametype.py"):
            out.append((w[1][6:-3], w[2], w[3]))
        elif len(w) == 5 and w[1] == "tools/unitfile.py" and w[2] in ("rename", "merge"):
            out.append((f"unitfile {w[2]}", w[3], w[4]))
    return out


def mapping(kind, a, b):
    """(token map, unitfile semantics?) of one command, from what is LEFT."""
    if kind == "rename":
        return {a: b}, False
    if kind == "renametype":
        found, lower_rx, upper_rx = renametype.occurrences(a, a.upper())
        return {t: (lower_rx.sub(b, t) if lower_rx.search(t) else upper_rx.sub(b.upper(), t))
                for t in found}, False
    if kind == "unitfile rename":
        new = unitfile.unit_stem(b)
        return {a: new, f"{a.upper()}_H": f"{new.upper()}_H"}, True
    return {b: a}, True              # unitfile merge A B: B's name became A


def leftovers(m, unit):
    rxs = [re.compile(rf"(?<![A-Za-z0-9_]){re.escape(o)}(?![A-Za-z0-9_])") for o in m]
    hits = []
    for p in [q for q in rename.text_files() if q.exists()] + [unitfile.WARNINGS]:
        text = p.read_text(errors="replace")
        keep = unitfile.history_spans(text) if unit and p.parent.name == "match-reports" else []
        for rx in rxs:
            for x in rx.finditer(text):
                if not any(s <= x.start() < e for s, e in keep):
                    hits.append(f"{p.relative_to(ROOT)}:{text.count(chr(10), 0, x.start()) + 1}")
    return hits


def rewrite_symbols(m):
    """rename.py's rewrite: code whole, prose line by line, leaving a line that
    already names NEW (it is ABOUT the rename, round 71)."""
    for p in [q for q in rename.text_files() if q.exists()] + [unitfile.WARNINGS]:
        t = p.read_text(errors="replace")
        o = t
        prose = not p.relative_to(ROOT).as_posix().startswith(("src/", "include/", "config/"))
        for old, new in sorted(m.items(), key=lambda kv: -len(kv[0])):
            rx = re.compile(rf"(?<![A-Za-z0-9_]){re.escape(old)}(?![A-Za-z0-9_])")
            nx = re.compile(rf"(?<![A-Za-z0-9_]){re.escape(new)}(?![A-Za-z0-9_])")
            o = "\n".join(l if prose and nx.search(l) else rx.sub(new, l) for l in o.split("\n"))
        if o != t:
            p.write_text(o)


def merge3(b, o, t, path, clashes):
    """Three-way merge of the ledger's JSON, key by key. A rename is a key
    deleted plus a key added; plan.save_state sorts keys, so the two land in
    different hunks and "main's side" of the second loses the entry (round
    90's merge, replayed). Structurally it is just a delete and an add."""
    if o == t:
        return o
    if o == b:
        return t
    if t == b:
        return o
    if isinstance(o, dict) and isinstance(t, dict):
        b = b if isinstance(b, dict) else {}
        out = {}
        for k in sorted(set(o) | set(t)):
            if k in o and k in t:
                out[k] = merge3(b.get(k), o[k], t[k], f"{path}.{k}", clashes)
            elif k in o:
                if k not in b or b[k] != o[k]:      # main added or changed it
                    out[k] = o[k]
            elif k not in b or b[k] != t[k]:        # the branch added or changed it
                out[k] = t[k]
        return out
    if isinstance(o, list) and isinstance(t, list) and isinstance(b, list) \
            and o[:len(b)] == b and t[:len(b)] == b:
        return o + [x for x in t[len(b):] if x not in o]
    clashes.append(path)
    return o


def merge_ledger():
    rel = plan.STATE.relative_to(ROOT).as_posix()
    base = git("merge-base", "HEAD", "MERGE_HEAD").strip()
    b, o, t = (json.loads(git("show", f"{c}:{rel}")) for c in (base, "HEAD", "MERGE_HEAD"))
    clashes = []
    plan.save_state(merge3(b, o, t, "", clashes))
    print(f"  ledger: three-way merged from {base[:8]}, HEAD, MERGE_HEAD"
          + "".join(f"\n      BOTH SIDES CHANGED {c}: kept main's, decide by hand" for c in clashes))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("ranges", nargs="*")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    a = ap.parse_args()
    rngs = a.ranges
    if not rngs:
        if subprocess.run(["git", "rev-parse", "-q", "--verify", "MERGE_HEAD"], cwd=ROOT,
                          capture_output=True).returncode:
            sys.exit("FATAL: no merge in progress; name the ranges (e.g. HEAD^2..HEAD^1 HEAD^1..HEAD^2)")
        rngs = ["MERGE_HEAD..HEAD", "HEAD..MERGE_HEAD"]
        if not a.dry_run:
            merge_ledger()
    if subprocess.run(["git", "grep", "-q", "-I", "-e", "^<<<<<<< ", "--", "."], cwd=ROOT).returncode == 0:
        sys.exit("FATAL: conflict markers in the tree; resolve each hunk (main's side) first")
    cmds = [c for r in rngs for c in commands(r)]
    print(f"replay {' '.join(rngs)}: {len(cmds)} rename command(s)")
    total = 0
    for kind, x, y in cmds:
        m, unit = mapping(kind, x, y)
        hits = leftovers(m, unit) if m else []
        total += len(hits)
        print(f"  {kind} {x} {y}: {len(hits)} leftover(s)" + "".join(f"\n      {h}" for h in hits[:12]))
        if hits and not a.dry_run:
            if unit:
                unitfile.rewrite_tokens(m)
            else:
                rewrite_symbols(m)
        if m and not a.dry_run:
            plan.ledger_rename(m, drop_duplicates=kind == "unitfile merge")
    if a.dry_run or not total:
        print("nothing to rewrite" if not total else "dry run: nothing changed")
        return 0
    return unitfile.build(a.no_build)


if __name__ == "__main__":
    sys.exit(main())
