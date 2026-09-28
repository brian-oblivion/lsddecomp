#!/usr/bin/env python3
"""API documentation census (FINISHING-PLAN track 12).

    python3 tools/apidoc.py [-v] [--item ITEM | PATH...]

Every game header (include/*.h; Sony's include/psyq/ is not ours to document)
is parsed from one stub unit through the PINNED cpp and pycparser, so what
counts as a prototype, a type or a field is what cc1 sees, not what a regex
guesses. Each declaration is then looked up in the raw header text, where
Doxygen reads its documentation from:

  file      the header has no `/** @file */` block
  proto     a function prototype with no `/** ... @brief ... */` block above it
  param     a documented prototype lacking `@param <name>` for a parameter,
            naming a parameter it does not have, or with an unnamed parameter
  return    a documented non-void prototype with no `@return`
  global    an `extern` object with neither a `/** */` block above nor `/**< */`
  type      a struct, union, enum or typedef defined here with no `/** */` block
  field     a named member (not `padNN`) with neither `/**< */` nor `/** */`;
            members a macro expands (`*_FIELDS`, `*_SLOTS`) are documented once,
            at the macro, and are not counted
  plain     a `/* @brief */`-style tag in a plain comment: Doxygen skips it

and every comment in include/ and src/ (Sony's include/psyq/ excepted) is
screened for process text, which track 12 moves to the match reports:

  process   registers, instructions, the toolchain, retail, rounds, runners,
            tools/ and docs/ paths, placeholder symbols, retail addresses
  banner    a comment block in a .c longer than BANNER_LINES: split it into
            the header's class doc and the functions' own docs

A comment whose text starts `MATCHING:` in a .c is the sanctioned one-line
note (FINISHING-PLAN §3) and is exempt if it spans at most two lines (a
wrapped line); in a header it is a `process` hit,
because a header holds no matching notes. The patterns are a floor, never the
definition of documented: a `@brief` that restates the name has met nothing.

--item takes a track-12 item name from plan.py (e.g. `area-ui`) and
restricts the census to its files; exit status is 1 when anything is left.
"""
import argparse
import re
import sys
import tempfile
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))

BANNER_LINES = 20
KINDS = ("file", "proto", "param", "return", "global", "type", "field", "plain", "process", "banner")

# Each item owns a set of .c files AND the headers they define, so a runner
# moving a banner into its class's header, or a header's matching note into
# its .c, stays inside its own edit set (plan revision 45). Sized to roughly
# equal debt when written; a header goes to the item defining most of what it
# declares, and one no .c defines anything of goes to the psyq/shared item.
AREAS = {
    "area-app": ("src/app",),
    "area-cd-sound": ("src/cd", "src/sound", "src/main.c"),
    "area-graphics-core": ("src/graphics/sprite.c", "src/graphics/scene_node.c", "src/graphics/draw_system.c"),
    "area-graphics-res": ("src/graphics",),
    "area-ui": ("src/ui",),
    "area-dream-sys": ("src/world/dream_sys.c", "src/world/stage_grid.c", "src/world/tod_actor.c"),
    "area-dream-day": ("src/world/dream_day.c", "src/world/entity.c"),
    "area-dream-scene": ("src/world",),
    "area-psyq-shared": ("src/psyq",),
}
SHARED = "area-psyq-shared"

PROCESS_RE = [
    ("register", re.compile(r"\$(?:zero|at|gp|sp|fp|ra|[vatsk][0-9]|[0-9]{1,2})\b")),
    ("instruction", re.compile(r"\b(?:lwl|lwr|swl|swr|jalr?|addiu|lui|sltiu?|sll|sra|srl|mflo|mfhi|"
                               r"lbu|lhu|sb|sh|sw|lw|lb|lh|beqz?|bnez?|bgez|bltz|blez|bgtz|nop)\b")),
    ("delay slot", re.compile(r"\bdelay[- ]slot", re.I)),
    ("toolchain", re.compile(r"\b(?:cc1|gcc|GCC|maspsx|splat|m2c|permuter|asm-differ|objdump|"
                             r"INCLUDE_ASM|funcdiff|pycparser|codegen)\b|\bcompil(?:er|es|ed|ing)\b", re.I)),
    ("retail", re.compile(r"\bretail\b|\bbyte[- ](?:exact|identical|for[- ]byte|match)|\bnon[-_ ]?matching\b|"
                          r"\bmatch(?:es|ed)? (?:retail|the bytes)|\bstall(?:s|ed)?\b", re.I)),
    ("project", re.compile(r"\bround \d+|\brunners?\b|\bmatch[- ]reports?\b|\bFirecatFG\b|"
                           r"\bthe operator\b|\bFINISHING-PLAN\b|\bCLAUDE\.md\b|\bplan revision\b|\(no code\)", re.I)),
    ("path", re.compile(r"\b(?:tools|docs|asm|config|build)/[\w./-]+")),
    ("placeholder", re.compile(r"\b(?:func|D|jtbl)_(?:800)?[0-9A-Fa-f]{5,8}\b")),
    ("address", re.compile(r"\b0x800[0-9A-Fa-f]{5}\b")),
]
DOC_TAG = re.compile(r"@(?:brief|param|return|file)\b")
GROUPING = re.compile(r"@(?:file|\{|\}|defgroup|name|addtogroup|ingroup)\b")
PAD = re.compile(r"^_?pad\w*$|^unused\w*$", re.I)


def game_headers():
    return sorted(p for p in (REPO / "include").glob("*.h"))


def game_sources():
    return sorted((REPO / "src").rglob("*.c"))


def rel(p):
    try:
        return Path(p).resolve().relative_to(REPO).as_posix()
    except ValueError:
        return str(p)


# ---- the raw text: comments and where each one ends --------------------------

class Text:
    def __init__(self, path):
        self.raw = path.read_text(errors="replace")
        self.lines = self.raw.split("\n")
        # (start_line, end_line, text) of every comment, 1-based lines
        self.comments = []
        self.ends = {}
        for m in re.finditer(r"/\*.*?\*/", self.raw, re.S):
            a = self.raw.count("\n", 0, m.start()) + 1
            b = a + m.group(0).count("\n")
            self.comments.append((a, b, m.group(0)))
            self.ends.setdefault(b, []).append((a, m.group(0)))

    def comment_ending(self, line):
        """The last comment that ends on `line` and is all that line holds after it."""
        cs = self.ends.get(line)
        if not cs:
            return None
        a, txt = cs[-1]
        if not self.lines[line - 1].rstrip().endswith("*/"):
            return None
        head = self.lines[a - 1][: self.lines[a - 1].find("/*")].strip()
        return (a, txt) if not head else None

    def doc_above(self, line):
        """The doc block ending just above `line` (blank lines allowed between):
        ('doc', text), ('plain', text) for a plain comment carrying tags, or None.
        A run of stacked plain `/* @param */` comments is read as one."""
        i = line - 1
        while i >= 1 and not self.lines[i - 1].strip():
            i -= 1
        found, kind = [], None
        while i >= 1:
            c = self.comment_ending(i)
            if not c:
                break
            a, txt = c
            if txt.startswith("/**") and not txt.startswith("/**<"):
                if GROUPING.search(txt) and "@brief" not in txt:
                    break
                return ("doc", txt) if not found else ("plain", txt + "".join(found))
            if not DOC_TAG.search(txt):
                break
            found.insert(0, txt)
            kind = "plain"
            i = a - 1
        return (kind, "".join(found)) if kind else None

    def span(self, line):
        """First and last line of the declaration whose name is on `line`."""
        a = line
        while a > 1:
            p = self.lines[a - 2].strip()
            if not p or p.endswith((";", "{", "}", "*/", ",", "\\")) or p.startswith("#"):
                break
            a -= 1
        b = line
        while b < len(self.lines) and ";" not in re.sub(r"/\*.*?\*/", "", self.lines[b - 1]):
            b += 1
        return a, b

    def trailing_doc(self, a, b):
        return any("/**<" in self.lines[i - 1] for i in range(a, b + 1))


# ---- the parsed side: what cc1 sees declared in each header -------------------

class Entity:
    def __init__(self, kind, file, line, name, params=(), void=True):
        self.kind, self.file, self.line, self.name = kind, file, line, name
        self.params, self.void = list(params), void


def parse_headers():
    import declcheck
    from pycparser import c_ast, c_parser

    heads = {rel(h) for h in game_headers()}
    with tempfile.NamedTemporaryFile("w", suffix=".c", delete=False) as f:
        f.write("".join(f'#include "{Path(h).name}"\n' for h in sorted(heads)))
        stub = Path(f.name)
    try:
        text = declcheck.preprocess(stub)
    finally:
        stub.unlink()
    ast = c_parser.CParser().parse(text, filename=str(stub))
    out, seen = [], set()

    def add(e):
        if e.file in heads and (e.kind, e.file, e.line, e.name) not in seen:
            seen.add((e.kind, e.file, e.line, e.name))
            out.append(e)

    def is_void(t):
        return isinstance(t, c_ast.TypeDecl) and isinstance(t.type, c_ast.IdentifierType) \
            and t.type.names == ["void"]

    def params(fd):
        ps = []
        for p in (fd.args.params if fd.args else []):
            if isinstance(p, c_ast.EllipsisParam):
                continue
            if isinstance(p, c_ast.Typename) and is_void(p.type):
                continue
            ps.append(getattr(p, "name", None))
        return ps

    def composite(node, owner_file):
        """Structs/unions/enums defined inside a type, and a struct's members."""
        if isinstance(node, (c_ast.Struct, c_ast.Union)) and node.decls is not None:
            add(Entity("type", rel(node.coord.file), node.coord.line, node.name or "(anonymous)"))
            for m in node.decls:
                if m.name and not PAD.match(m.name):
                    add(Entity("field", rel(m.coord.file), m.coord.line, m.name))
                composite(m.type, owner_file)
        elif isinstance(node, c_ast.Enum) and node.values is not None:
            add(Entity("type", rel(node.coord.file), node.coord.line, node.name or "(anonymous)"))
        else:
            for _, c in node.children():
                if not isinstance(c, (c_ast.Constant, c_ast.ID, c_ast.BinaryOp)):
                    composite(c, owner_file)

    for ext in ast.ext:
        if not isinstance(ext, (c_ast.Decl, c_ast.Typedef)):
            continue
        f = rel(ext.coord.file)
        if isinstance(ext, c_ast.Typedef):
            inner = ext.type.type if isinstance(ext.type, c_ast.TypeDecl) else None
            ref_only = isinstance(inner, (c_ast.Struct, c_ast.Union, c_ast.Enum)) and \
                (inner.decls if not isinstance(inner, c_ast.Enum) else inner.values) is None
            has_body = isinstance(inner, (c_ast.Struct, c_ast.Union, c_ast.Enum)) and not ref_only
            if not ref_only and not has_body:
                add(Entity("type", f, ext.coord.line, ext.name))
            composite(ext.type, f)
            continue
        if isinstance(ext.type, c_ast.FuncDecl):
            add(Entity("proto", f, ext.coord.line, ext.name, params(ext.type), is_void(ext.type.type)))
        elif ext.name and "extern" in (ext.storage or []):
            add(Entity("global", f, ext.coord.line, ext.name))
        composite(ext.type, f)
    return out


# ---- the census ---------------------------------------------------------------

def census(files=None):
    """{file: [(line, kind, detail)]} for every game header and source, or
    only for `files` (repo-relative paths)."""
    hits = defaultdict(list)
    texts = {}

    def T(f):
        if f not in texts:
            texts[f] = Text(REPO / f)
        return texts[f]

    heads = [rel(h) for h in game_headers()]
    srcs = [rel(s) for s in game_sources()]
    want = set(files) if files is not None else set(heads + srcs)

    for h in heads:
        if h in want and not any(c[2].startswith("/**") and "@file" in c[2] for c in T(h).comments):
            hits[h].append((1, "file", "no /** @file */ block"))

    for e in parse_headers():
        if e.file not in want:
            continue
        t = T(e.file)
        code = re.sub(r"/\*.*?(\*/|$)", "", t.lines[e.line - 1]) if e.line <= len(t.lines) else ""
        if e.line > len(t.lines) or not re.search(rf"\b{re.escape(e.name)}\b", code) \
                and e.name != "(anonymous)":
            continue                        # a member a macro expands: documented at the macro
        a, b = t.span(e.line)
        if e.kind == "type":
            a = e.line
        above = t.doc_above(a)
        doc = above[1] if above and above[0] == "doc" else None
        if above and above[0] == "plain":
            hits[e.file].append((a, "plain", f"{e.name}: tags in a plain /* */ comment"))
        if e.kind == "proto":
            if not doc or "@brief" not in doc:
                hits[e.file].append((e.line, "proto", e.name))
                continue
            named = set(re.findall(r"@param(?:\[\w+\])?\s+(\w+)", doc))
            for i, p in enumerate(e.params):
                if p is None:
                    hits[e.file].append((e.line, "param", f"{e.name}: parameter {i + 1} is unnamed"))
                elif p not in named:
                    hits[e.file].append((e.line, "param", f"{e.name}: no @param {p}"))
            for p in sorted(named - set(e.params)):
                hits[e.file].append((e.line, "param", f"{e.name}: @param {p} names no parameter"))
            if not e.void and not re.search(r"@returns?\b", doc):
                hits[e.file].append((e.line, "return", f"{e.name}: no @return"))
        elif not doc and not (e.kind != "type" and t.trailing_doc(a, b)):
            hits[e.file].append((e.line, e.kind, e.name))

    for f in sorted(want):
        is_c = f.endswith(".c")
        for a, b, txt in T(f).comments:
            body = re.sub(r"^/\*+<?\s*", "", txt)
            if body.startswith("MATCHING:"):
                if not is_c:
                    hits[f].append((a, "process", "MATCHING: note in a header (it belongs in the .c)"))
                elif b - a > 1:
                    hits[f].append((a, "process", f"MATCHING: note of {b - a + 1} lines (one line; the rest to the report)"))
                continue
            if is_c and b - a + 1 > BANNER_LINES:
                hits[f].append((a, "banner", f"{b - a + 1}-line comment"))
            for n, ln in enumerate(txt.split("\n")):
                why = [f"{w} `{m.group(0)}`" for w, rx in PROCESS_RE for m in [rx.search(ln)] if m]
                if why:
                    hits[f].append((a + n, "process", ", ".join(why)))
    for f in hits:
        hits[f].sort()
    return hits


def definers():
    """{header: {.c file: definitions}}: how many of the functions and
    globals a header declares each .c defines, which decides its owner."""
    import progress
    names = defaultdict(set)
    for e in parse_headers():
        if e.kind in ("proto", "global"):
            names[e.name].add(e.file)
    own = defaultdict(lambda: defaultdict(int))
    obj = re.compile(r"^(?!extern\b|static\b|typedef\b|return\b)[A-Za-z_][\w \t*]*?\b(\w+)\s*"
                     r"(?:\[[^\]]*\])?\s*(?:=[^;]*)?;", re.M)
    for s in game_sources():
        raw = s.read_text(errors="replace")
        found = {m.group(1) for m in progress.DEF_RE.finditer(raw)} | {m.group(1) for m in obj.finditer(raw)}
        for n in found:
            for h in names.get(n, ()):
                own[h][rel(s)] += 1
    return own


def area_of(path):
    for item, dirs in AREAS.items():
        if any(path == d or path.startswith(d + "/") for d in dirs):
            return item
    return None


def areas():
    """{item: sorted files}: every game .c to the first item listing it or
    its directory, every header to the item whose .c files define most of
    what it declares (by name on a tie), the rest to SHARED."""
    out = defaultdict(set)
    for s in game_sources():
        out[area_of(rel(s))].add(rel(s))
    own = definers()
    for h in game_headers():
        h = rel(h)
        votes = defaultdict(int)
        for s, n in own.get(h, {}).items():
            votes[area_of(s)] += n
        best = max(votes, key=lambda k: (votes[k], k), default=None)
        out[best or SHARED].add(h)
    return {k: sorted(v) for k, v in out.items() if k}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("-v", "--verbose", action="store_true", help="list every hit")
    ap.add_argument("--item", help="restrict to one track-12 item's files")
    ap.add_argument("--areas", action="store_true", help="print each item's files and exit")
    ap.add_argument("paths", nargs="*")
    a = ap.parse_args()
    if a.areas:
        for k, v in sorted(areas().items()):
            print(f"{k}: {' '.join(v)}")
        return 0
    files = None
    if a.item:
        ar = areas()
        if a.item not in ar:
            sys.exit(f"unknown item {a.item}; items: {', '.join(sorted(ar))}")
        files = ar[a.item]
    elif a.paths:
        files = []
        for p in a.paths:
            q = REPO / p
            files += [rel(x) for x in sorted(q.rglob("*")) if x.suffix in (".c", ".h")] if q.is_dir() else [rel(q)]
    hits = census(files)
    tot = defaultdict(int)
    rows = []
    for f in sorted(hits):
        c = defaultdict(int)
        for _, k, _ in hits[f]:
            c[k] += 1
            tot[k] += 1
        rows.append((f, c))
    if a.verbose:
        for f in sorted(hits):
            for line, k, d in hits[f]:
                print(f"{f}:{line}: {k}: {d}")
    else:
        w = max([len(f) for f, _ in rows] + [4])
        print(f"{'file':<{w}} " + " ".join(f"{k:>7}" for k in KINDS))
        for f, c in rows:
            print(f"{f:<{w}} " + " ".join(f"{c[k] or '.':>7}" for k in KINDS))
    print(f"total {sum(tot.values())}: " + ", ".join(f"{k} {tot[k]}" for k in KINDS))
    return 1 if sum(tot.values()) else 0


if __name__ == "__main__":
    sys.exit(main())
