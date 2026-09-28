#!/usr/bin/env python3
"""Declaration census for game code (track 10 `prototypes`).

    .venv/bin/python3 tools/declcheck.py [--verbose] [NAME...]

Written by round 103's prototypes runner; it is the measurement behind
FINISHING-PLAN track 10's `prototypes` item.

Preprocesses every unit under src/ with the PINNED cpp and the Makefile's
CPP_FLAGS (plus -DM2CTX, so INCLUDE_ASM expands to nothing), blanks __asm__
blocks, parses the result with pycparser, and records every declaration of a
function or object with its file:line (from cpp's line markers), whether it
is a definition, and its type (parameter names dropped, `struct T` read as
`T`, K&R definitions typed from their parameter declarations). Every
identifier a unit references is recorded too.

A declaration SITE is a file:line; a header line counts once however many
units include it. Game sites are those outside include/psyq/ and src/psyq/.

Reports (each exits non-zero when non-empty):
  MULTI     a name with more than one declaration site, at least one a game
            site
  LOCAL     a non-static declaration in a game .c of a name that is
            (a) defined in another C unit, (b) declared by any header, or
            (c) referenced by another unit
  TYPE      a declaration whose type differs from the definition's
  IMPLICIT  a function a game unit calls with no declaration in that unit

A TYPE or MULTI hit whose declaration line, or the unbroken (no blank line)
block of lines above it, carries a `MATCHING:` or `arity-ok` note is a deliberate view (a dead argument, a
packed pair) and is listed under DELIBERATE without failing.
"""
import copy
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

from pycparser import c_ast, c_generator, c_parser

REPO = Path(__file__).resolve().parent.parent
if len(sys.argv) > 1 and sys.argv[1].startswith("--repo="):
    REPO = Path(sys.argv.pop(1).split("=", 1)[1]).resolve()


def cpp_flags():
    mk = (REPO / "Makefile").read_text()
    out = []
    for m in re.finditer(r"^CPP_FLAGS\s*[:+]?=\s*(.*)$", mk, re.M):
        out += m.group(1).split()
    return out


def strip_asm(text):
    out, i, n = [], 0, len(text)
    pat = re.compile(r"\b__asm__\b(\s*volatile\b)?\s*\(")
    while True:
        m = pat.search(text, i)
        if not m:
            out.append(text[i:])
            break
        out.append(text[i:m.start()])
        j, depth = m.end(), 1
        while j < n and depth:
            c = text[j]
            if c == '"':
                j += 1
                while text[j] != '"':
                    j += 2 if text[j] == "\\" else 1
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            j += 1
        out.append("\n" * text[m.start():j].count("\n"))
        i = j
    return "".join(out)


def preprocess(path):
    cmd = [str(REPO / "tools/gcc263/cpp")] + cpp_flags() + ["-DM2CTX", str(path)]
    r = subprocess.run(cmd, cwd=REPO, capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"cpp failed on {path}:\n{r.stderr}")
    text = strip_asm(r.stdout)
    return re.sub(r"\b__inline__\b|\binline\b|\b__extension__\b", "", text)


GEN = c_generator.CGenerator()


def typestr(node):
    t = copy.deepcopy(node)

    def strip_name(n):
        while not isinstance(n, c_ast.TypeDecl):
            if isinstance(n, (c_ast.IdentifierType, c_ast.Struct, c_ast.Union, c_ast.Enum)):
                return
            n = n.type
        n.declname = None

    def drop(n):
        if isinstance(n, c_ast.FuncDecl) and n.args:
            for p in n.args.params:
                if isinstance(p, c_ast.Decl):
                    p.name = None
                    strip_name(p.type)
        for _, c in n.children():
            drop(c)

    drop(t)
    strip_name(t)
    s = GEN.visit(t)
    s = re.sub(r"\bstruct\s+", "", s)
    return re.sub(r"\s+", " ", s).strip()


class Decl:
    def __init__(self, unit, name, file, line, is_def, typ, kind, storage):
        self.unit, self.name, self.file, self.line = unit, name, file, line
        self.is_def, self.typ, self.kind, self.storage = is_def, typ, kind, storage

    @property
    def where(self):
        return f"{self.file}:{self.line}"


def rel(f):
    try:
        return str(Path(f).resolve().relative_to(REPO))
    except ValueError:
        return f


def collect(unit, text):
    decls, refs, calls, declared = [], set(), set(), set()
    ast = c_parser.CParser().parse(text, filename=str(unit))

    def add(d, is_def):
        if not isinstance(d, c_ast.Decl) or d.name is None:
            return
        declared.add(d.name)
        if "typedef" in (d.storage or []):
            return
        kind = "func" if isinstance(d.type, c_ast.FuncDecl) else "obj"
        decls.append(Decl(unit, d.name, rel(d.coord.file), d.coord.line, is_def,
                          typestr(d.type), kind, d.storage or []))

    class Body(c_ast.NodeVisitor):
        def visit_Decl(self, node):
            declared.add(node.name)
            if "extern" in (node.storage or []) or isinstance(node.type, c_ast.FuncDecl):
                add(node, False)
            for _, c in node.children():
                self.visit(c)

        def visit_ID(self, node):
            refs.add(node.name)

        def visit_FuncCall(self, node):
            if isinstance(node.name, c_ast.ID):
                calls.add(node.name.name)
            self.generic_visit(node)

    for ext in ast.ext:
        if isinstance(ext, c_ast.FuncDef):
            d = ext.decl
            if ext.param_decls:
                d = copy.deepcopy(d)
                byname = {p.name: p for p in ext.param_decls}
                d.type.args.params = [copy.deepcopy(byname.get(getattr(p, "name", None), p))
                                      for p in d.type.args.params]
            add(d, True)
            for p in (ext.decl.type.args.params if ext.decl.type.args else []):
                if getattr(p, "name", None):
                    declared.add(p.name)
            Body().visit(ext.body)
        elif isinstance(ext, c_ast.Decl):
            fn = isinstance(ext.type, c_ast.FuncDecl)
            is_def = ext.init is not None or (not fn and "extern" not in (ext.storage or []))
            add(ext, is_def)
            if ext.init is not None:
                Body().visit(ext.init)
    return decls, refs, calls, declared


_LINES = {}


def deliberate(d):
    """The declaration's line or the one above names why it differs."""
    if d.file not in _LINES:
        f = REPO / d.file
        _LINES[d.file] = f.read_text(errors="replace").split("\n") if f.exists() else []
    lines = _LINES[d.file]
    near = [lines[d.line - 1]] if 0 < d.line <= len(lines) else []
    i = d.line - 2
    while i >= 0 and lines[i].strip() and len(near) < 12:
        near.append(lines[i])
        i -= 1
    return any("MATCHING" in x or "arity-ok" in x for x in near)


def is_game(path):
    return not (path.startswith("include/psyq/") or path.startswith("src/psyq/"))


def main():
    args = sys.argv[1:]
    verbose = "--verbose" in args
    only = {a for a in args if not a.startswith("-")}
    decls, refs_by_unit, implicit = [], {}, []
    for u in sorted((REPO / "src").rglob("*.c")):
        ur = rel(str(u))
        ds, refs, calls, declared = collect(ur, preprocess(u))
        decls += ds
        refs_by_unit[ur] = refs
        if is_game(ur):
            for c in sorted(calls - declared):
                implicit.append(f"{c}: called in {ur} with no declaration")

    defs = {}
    for d in decls:
        if d.is_def and d.file.endswith(".c") and "static" not in d.storage:
            defs.setdefault(d.name, d)
    sites = defaultdict(dict)
    for d in decls:
        if not d.is_def:
            sites[d.name][d.where] = d
    users = defaultdict(set)
    for u, refs in refs_by_unit.items():
        for r in refs:
            users[r].add(u)

    problems = defaultdict(list)
    for name, s in sorted(sites.items()):
        if only and name not in only:
            continue
        game = {w: d for w, d in s.items() if is_game(w)}
        if not game:
            continue
        df = defs.get(name)
        if len(s) > 1:
            key = "DELIBERATE" if any(deliberate(d) for d in s.values()) else "MULTI"
            problems[key].append(f"{name}: " + ", ".join(sorted(s)))
        hdr = sorted(w for w in s if w.startswith("include/"))
        for w, d in game.items():
            if not d.file.endswith(".c") or "static" in d.storage:
                continue
            others = sorted(users[name] - {d.file})
            why = []
            if df and df.file != d.file:
                why.append(f"defined {df.where}")
            if hdr:
                why.append("header " + ", ".join(hdr))
            if others:
                why.append("used by " + ", ".join(others))
            if why:
                problems["LOCAL"].append(f"{name}: {w} ({'; '.join(why)})")
        if df:
            for w, d in s.items():
                if d.typ != df.typ:
                    problems["DELIBERATE" if deliberate(d) else "TYPE"].append(f"{name}: {w} `{d.typ}` vs definition {df.where} `{df.typ}`")
    problems["IMPLICIT"] = [x for x in implicit if not only or x.split(":")[0] in only]
    total = 0
    for k in ("MULTI", "LOCAL", "TYPE", "IMPLICIT", "DELIBERATE"):
        v = problems[k]
        total += len(v) if k != "DELIBERATE" else 0
        print(f"== {k}: {len(v)}")
        if verbose or only:
            for line in v:
                print("  " + line)
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
