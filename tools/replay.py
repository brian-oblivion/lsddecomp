#!/usr/bin/env python3
"""Replay a merged branch's renames on the merged tree (FINISHING-PLAN §3).

    python3 tools/replay.py                 # during a merge: both sides since the merge base
    python3 tools/replay.py R [R ...]       # named ranges, e.g. after it: HEAD^2..HEAD^1 HEAD^1..HEAD^2
    python3 tools/replay.py ... --dry-run   # list what is left, change nothing

Every rename commit's first line is its exact command (`python3
tools/rename.py OLD NEW`, `renametype.py`, `unitfile.py rename|merge|header`). Once
the branch is merged its renames are already in the tree, so re-running a
command is refused (NEW exists, OLD is gone: round 90). What is still wrong is
each side's OLD tokens in the OTHER side's text: a conflicted hunk resolved to
one side, lines that never conflicted at all (git cannot flag them), and a
file one side edited and the other renamed (modify/delete: keep the renamed
file). This tool re-applies each command's TOKEN REWRITE, main's then the
branch's, each in its own order, to those leftovers, over the files the
original tool rewrites; during a merge it first rebuilds the ledger by a
three-way JSON merge (a hand-resolved ledger is replaced); then `make extract` and
`./build-and-verify.sh`. What it rewrote is staged (round 94: a merge commit
missed replay's unstaged edits).

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
    for line in git("log", "--reverse", "--format=%H %s", rng).splitlines():
        sha, _, subj = line.partition(" ")
        w = shlex.split(subj) if subj.startswith("python3 tools/") else []
        flags = tuple(x for x in w if x.startswith("--"))
        w = [x for x in w if not x.startswith("--")]
        if len(w) == 4 and w[1] in ("tools/rename.py", "tools/renametype.py"):
            out.append((w[1][6:-3], w[2], w[3], flags, sha))
        elif len(w) == 5 and w[1] == "tools/unitfile.py" and w[2] in ("rename", "merge", "header"):
            out.append((f"unitfile {w[2]}", w[3], w[4], flags, sha))
    return out


def moved(sha):
    """{old path: new path} for the files commit `sha` renamed."""
    out = {}
    for l in git("show", "--format=", "--name-status", "-M", sha).splitlines():
        f = l.split("\t")
        if f[0].startswith("R") and len(f) == 3:
            out[f[1]] = f[2]
    return out


def mapping(kind, a, b, flags=(), sha=None):
    """The token map of one command, from what is LEFT and the paths its
    commit `sha` moved."""
    if kind == "rename":
        return {a: b}
    if kind == "renametype":
        found, lower_rx, upper_rx = renametype.occurrences(a, a.upper())
        return {t: (lower_rx.sub(b, t) if lower_rx.search(t) else upper_rx.sub(b.upper(), t))
                for t in found}
    if kind in ("unitfile rename", "unitfile header"):
        new = unitfile.unit_stem(b)
        mv = moved(sha) if sha else {}
        if kind == "unitfile header":
            return unitfile.rename_map(a, new, None, None, True, True)
        cs = [(o, n) for o, n in mv.items() if o.endswith(f"/{a}.c") and o.startswith("src/")]
        # no move in the command's own commit (squashed, or staged into
        # another): the tree says where the file went, and a stem change
        # keeps its directory
        new_c = cs[0][1] if cs else f"src/{b}.c"
        old_c = cs[0][0] if cs else f"{new_c.rsplit('/', 1)[0]}/{a}.c"
        hdr = f"include/{a}.h" in mv or (not cs and (ROOT / f"include/{new}.h").exists()
                                          and not (ROOT / f"include/{a}.h").exists())
        # paths only when the stem is still a type in code (unitfile.cmd_rename)
        return unitfile.rename_map(a, new, old_c, new_c, hdr, bool(unitfile.code_identifier(a)))
    if "--as-b" in flags:
        return {a: b}
    return {b: a}              # unitfile merge A B: B's name became A


_TIP_LINES = {}


def own_lines(tip, p):
    """The lines of `p` as the side that ran a command left them at its tip.
    Such a line kept its OLD token on purpose: the side's own tool skipped it
    (a report's `> Renamed from OLD`, a quoted command, a history table), so
    replay must not rewrite it. Only the OTHER side's text is replayed onto
    (round 91: the charlie and bravo merges rewrote their own reports'
    history lines)."""
    if tip is None:
        return set()
    key = (tip, p)
    if key not in _TIP_LINES:
        r = subprocess.run(["git", "show", f"{tip}:{p.relative_to(ROOT).as_posix()}"], cwd=ROOT,
                           capture_output=True, text=True, errors="replace")
        _TIP_LINES[key] = set(r.stdout.split("\n")) if r.returncode == 0 else set()
    return _TIP_LINES[key]


def rewrite(m, tip, dry, unit=False):
    """Re-apply one command's token map as its own tool would: code whole,
    prose by rename.sub_prose (a report's history sections and a line
    already naming NEW stay as written), and never a line the side that ran
    the command left as it is (own_lines). Returns [(path, line)] changed,
    writing unless dry."""
    # a unit name uses unitfile's match, which leaves `OLD.h` alone while that
    # header exists (round 101: class_3bb8c.h's includes, six lines)
    rx = unitfile.token_rx if unit else (lambda o: re.compile(rf"(?<![A-Za-z0-9_]){re.escape(o)}(?![A-Za-z0-9_])"))
    rxs = [(rx(o), n) for o, n in sorted(m.items(), key=lambda kv: -len(kv[0]))]
    hits = []
    for p in unitfile.text_files() if unit else [q for q in rename.text_files() if q.exists()] + [unitfile.WARNINGS]:
        if not p.exists():
            continue
        text = p.read_text(errors="replace")
        if not any(rx.search(text) for rx, _ in rxs):
            continue
        mine = own_lines(tip, p)
        out = text
        if rename.is_code(p):
            for rx, n in rxs:
                out = "\n".join(l if l in mine else rx.sub(n, l) for l in out.split("\n"))
        elif unit:
            # a unit's keys are judged per line at once, as unitfile.py does
            out = unitfile.sub_prose_all(rxs, out, p, keep_lines=mine)
        else:
            for rx, n in rxs:
                out = rename.sub_prose(rx, n, out, p, keep_lines=mine)
        if out != text:
            a, b = text.split("\n"), out.split("\n")
            rel = p.relative_to(ROOT).as_posix()
            hits += [(rel, i + 1) for i in range(len(a)) if a[i] != b[i]]
            if not dry:
                p.write_text(out)
    return hits


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
    # Each command runs with the tip of the side that ran it (`A..B`: B).
    cmds = [(c, r.split("..")[-1]) for r in rngs for c in commands(r)]
    print(f"replay {' '.join(rngs)}: {len(cmds)} rename command(s)")
    total, touched = 0, set()
    for (kind, x, y, flags, sha), tip in cmds:
        m = mapping(kind, x, y, flags, sha)
        hits = rewrite(m, tip, a.dry_run, unit=kind.startswith("unitfile")) if m else []
        total += len(hits)
        touched |= {h for h, _ in hits}
        print(f"  {kind} {x} {y}{''.join(' ' + f for f in flags)}: {len(hits)} leftover(s)" + "".join(f"\n      {h}:{n}" for h, n in hits[:12]))
        if m and not a.dry_run:
            plan.ledger_rename(m, drop_duplicates=kind == "unitfile merge")
            if kind == "unitfile rename" and x not in m:        # paths only
                plan.ledger_rename_unit(x, unitfile.unit_stem(y))
                unitfile.warnings_prefix(x, unitfile.unit_stem(y))
    if not a.dry_run:
        # staged, so the merge commit holds them (round 94: the first merge
        # commit held the pre-replay ObjM.h)
        subprocess.run(["git", "add", "--", plan.STATE.relative_to(ROOT).as_posix(), *sorted(touched)],
                       cwd=ROOT, check=True)
    if a.dry_run or not total:
        print("nothing to rewrite" if not total else "dry run: nothing changed")
        return 0
    return unitfile.build(a.no_build)


if __name__ == "__main__":
    sys.exit(main())
