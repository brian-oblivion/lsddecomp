#!/usr/bin/env python3
"""Every struct layout every unit compiles, read from the COMPILER, not grep.

    python3 tools/typeviews.py --list                 # every struct view, per unit
    python3 tools/typeviews.py --class Entity         # views of one class, by `this` type
    python3 tools/typeviews.py --struct EntityPos     # every layout named EntityPos
    python3 tools/typeviews.py --merge A B ...        # union the named struct layouts, report conflicts
    python3 tools/typeviews.py --tree                 # class hierarchy from the method-table ids
    python3 tools/typeviews.py --json                 # everything, machine-readable
    python3 tools/typeviews.py --warnings [--baseline] # every unit's -Wall warnings vs config/typeviews-warnings.txt
    python3 tools/typeviews.py --census               # per class: its own methods, and every view of its object and table
    python3 tools/typeviews.py --globals              # globals declared extern with more than one type (track 4b)
    python3 tools/typeviews.py --upcast Get_vtable_BasicClass BasicClass src/x.c ...
                                                      # base-table calls: cast `self` (and object args) to the base

WHY THIS EXISTS (FINISHING-PLAN track 4). Unifying a class means proving that
every unit-local view of it agrees with every other at every offset. Reading
that off the source by eye is where layout mistakes come from: a forgotten
`padNN`, a field that sits at +0x9C in one view and +0xA8 in another, an `s16`
against a `u16`. This tool compiles each unit through the pinned cpp and cc1
with `-gstabs -O0` (debug output only; the object is thrown away and the
build is never touched) and parses the stabs, so every offset and every size
below is the one cc1 itself computed. Two views are compatible exactly when,
at every byte they both describe, they name a field of the same offset, size
and signedness (codegen reads nothing else); a name difference is a naming
question, never a layout one.

THE CLASS TREE. Word +0x000 of every method table is a hierarchical class id:
each nibble above the lowest non-zero one is one more level of derivation, so
the parent of id 0x1F234 is 0xF234, then 0x234, 0x34, 0x4. Checked against the
tables themselves (the child is the parent's table with slots replaced, and
usually appended: `classtable.py <child> --vs <parent>`;
docs/research/class-framework.md has the measurement), and it is what the
`(hdr & 0xFFF) == id` / `(hdr & 0xFFFFF) == 0x1F234` tests in game code read:
an is-kind-of test is a prefix match on this id. An id with no table in
`classtable.py --scan` is an intermediate class with no methods of its own
that the scan could see (fewer than eight slots, or none).

The `--class` mapping: a struct is a view of class C when it is the declared
type of the FIRST parameter of a function that one of C's method-table slots
holds, or of a `C__*`-named function. That catches the object views; method
table views are found as the pointee type of the view's field at +0x000.
"""
import argparse
import json
import re
import struct as pystruct
import subprocess
import sys
from pathlib import Path

import srcpath

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
CPP = ROOT / "tools/gcc263/cpp"
CC1 = ROOT / "tools/gcc263/cc1"


def makefile_flags(var):
    """CPP_FLAGS as the Makefile spells them (never retyped: CLAUDE.md)."""
    out = []
    for line in (ROOT / "Makefile").read_text().splitlines():
        m = re.match(rf"{var}\s*(:?=|\+=)\s*(.*)$", line)
        if m:
            if m.group(1) != "+=":
                out = []
            out += m.group(2).split()
    return out


# --------------------------------------------------------------------------
# stabs parsing

class Ty:
    """A resolved stabs type."""
    __slots__ = ("kind", "size", "name", "signed", "target", "fields", "count", "tid")

    def __init__(self, kind, size=None, name=None, signed=None, target=None,
                 fields=None, count=None, tid=None):
        self.kind, self.size, self.name, self.signed = kind, size, name, signed
        self.target, self.fields, self.count, self.tid = target, fields, count, tid


BASE = {  # stabs builtin name -> (project spelling, size, signed)
    "int": ("s32", 4, True), "long int": ("s32", 4, True),
    "unsigned int": ("u32", 4, False), "long unsigned int": ("u32", 4, False),
    "short int": ("s16", 2, True), "short unsigned int": ("u16", 2, False),
    "char": ("u8", 1, False), "unsigned char": ("u8", 1, False),
    "signed char": ("s8", 1, True),
    "long long int": ("s64", 8, True), "long long unsigned int": ("u64", 8, False),
    "float": ("f32", 4, True), "double": ("f64", 8, True), "long double": ("f64", 8, True),
    "void": ("void", 0, None),
}


class Stabs:
    def __init__(self):
        self.types = {}        # tid -> Ty
        self.structs = []      # (tag or typedef name, Ty) in definition order
        self.typedef_of = {}   # tid -> typedef name (first seen)
        self.funcs = {}        # function name -> first param type (Ty) or None
        self.fwd = {}          # tid -> tag for forward refs

    def parse_all(self, stab_strings):
        cur_fn = None
        for s, kind in stab_strings:
            if kind == 36:            # N_FUN
                m = re.match(r"([^:]+):[Ff](.*)", s)
                if m:
                    cur_fn = m.group(1)
                    self.funcs.setdefault(cur_fn, None)
                    self.type_at(m.group(2), 0)
                continue
            if kind == 160:           # N_PSYM: a parameter
                m = re.match(r"([^:]+):p(.*)", s)
                if m:
                    # parse EVERY parameter's type: a later parameter can be
                    # where cc1 first defines a type number (round 88:
                    # New_TileMap's `atlas:p614=*368` defined TileAtlas * for
                    # the four TileAtlas methods after it, which then read
                    # `<t614>`); only the first names the function's `this`
                    t, _ = self.type_at(m.group(2), 0)
                    if cur_fn and self.funcs.get(cur_fn) is None:
                        self.funcs[cur_fn] = t
                continue
            if kind == 128:           # N_LSYM: types and typedefs
                m = re.match(r"([^:]*):([Tt])(.*)", s)
                if not m:
                    # a local variable (`name:NNN...`, no letter): parse its
                    # type anyway, since cc1 can first define a type number
                    # there (round 88: TimBlockSrc__AdvanceLoadState's
                    # `p:606=*607=*163` defined TimArraySrc * for every
                    # TimArraySrc method after it, which then read `<t607>`)
                    m = re.match(r"[^:]*:([0-9(].*)", s)
                    if m:
                        self.type_at(m.group(1), 0)
                    continue
                name, tt, rest = m.groups()
                t, _ = self.type_at(rest, 0, defname=name)
                if tt == "t" and t is not None and name:
                    if t.tid is not None:
                        self.typedef_of.setdefault(t.tid, name)
                    if t.kind in ("struct", "union") and t.name is None:
                        t.name = name
                if tt == "T" and t is not None and t.kind in ("struct", "union"):
                    t.name = t.name or name
        # post: give struct Ty their best name (tag)
        return self

    # type grammar ---------------------------------------------------------
    def type_at(self, s, i, defname=None):
        """Parse a type at s[i:]; returns (Ty, new_i)."""
        m = re.match(r"\d+", s[i:])
        if m:
            tid = int(m.group(0))
            i += len(m.group(0))
            if i < len(s) and s[i] == "=":
                t, i = self.type_def(s, i + 1, tid, defname)
                self.types[tid] = t
                return t, i
            t = self.types.get(tid)
            if t is None:
                t = Ty("ref", tid=tid)
                self.types[tid] = t
            return t, i
        if s.startswith("(", i):   # (file,num) form, not used by 2.6.3 but be safe
            j = s.index(")", i)
            return self.type_at(s, j + 1)
        return None, i

    def type_def(self, s, i, tid, defname):
        c = s[i]
        if c == "r":               # range: base type
            m = re.match(r"r(\d+);(-?\d+);(-?\d+);", s[i:])
            i += len(m.group(0))
            base = BASE.get(defname)
            if base:
                t = Ty("base", size=base[1], name=base[0], signed=base[2], tid=tid)
            else:
                t = Ty("base", size=4, name=defname or "?", signed=True, tid=tid)
            return self._fill(tid, t), i
        if c == "*":
            tgt, i = self.type_at(s, i + 1)
            return self._fill(tid, Ty("ptr", size=4, target=tgt, tid=tid)), i
        if c == "f":
            tgt, i = self.type_at(s, i + 1)
            return self._fill(tid, Ty("func", size=None, target=tgt, tid=tid)), i
        if c == "a":               # ar<index type>;lo;hi;<elem>
            m = re.match(r"ar(\d+(?:=r\d+;-?\d+;-?\d+;)?);(-?\d+);(-?\d+);", s[i:])
            i += len(m.group(0))
            lo, hi = int(m.group(2)), int(m.group(3))
            el, i = self.type_at(s, i)
            n = hi - lo + 1
            sz = (el.size * n) if (el and el.size is not None) else None
            return self._fill(tid, Ty("array", size=sz, target=el, count=n, tid=tid)), i
        if c in "su":
            m = re.match(r"[su](\d+)", s[i:])
            size = int(m.group(1))
            i += len(m.group(0))
            fields = []
            t = self._fill(tid, Ty("struct" if c == "s" else "union", size=size,
                                   name=None, fields=fields, tid=tid))
            while i < len(s) and s[i] != ";":
                j = s.index(":", i)
                fname = s[i:j]
                ft, k = self.type_at(s, j + 1)
                m2 = re.match(r",(-?\d+),(\d+);", s[k:])
                off, bits = int(m2.group(1)), int(m2.group(2))
                i = k + len(m2.group(0))
                fields.append((fname, ft, off, bits))
            return t, i + 1
        if c == "x":               # forward ref: xsTag:
            j = s.index(":", i)
            t = self._fill(tid, Ty("fwd", name=s[i + 2:j], tid=tid))
            return t, j + 1
        if c == "e":               # enum: e name:val,...;
            j = s.index(";", i)
            while not s.startswith(";", j) or s[j - 1] != ",":
                j = s.index(";", j + 1)
            return self._fill(tid, Ty("base", size=4, name="enum", signed=True, tid=tid)), j + 1
        if c.isdigit():            # alias of another type id
            m = re.match(r"\d+", s[i:])
            if int(m.group(0)) == tid:  # "void:t19=19": void is its own alias
                return self._fill(tid, Ty("base", size=0, name="void", signed=None, tid=tid)), i + len(m.group(0))
            t, i = self.type_at(s, i)
            self.types[tid] = t
            return t, i
        raise ValueError(f"unhandled stab type {s[i:i+20]!r}")

    def _fill(self, tid, t):
        old = self.types.get(tid)
        if old is not None and old.kind in ("ref", "fwd"):
            # upgrade in place so earlier references see the definition
            fwdname = old.name if old.kind == "fwd" else None
            for k in Ty.__slots__:
                setattr(old, k, getattr(t, k))
            if fwdname and old.name is None:
                old.name = fwdname
            return old
        self.types[tid] = t
        return t


def spell(t, depth=0):
    """A compact C-ish spelling of a type, used for display and comparison."""
    if t is None:
        return "?"
    if t.kind == "base":
        return t.name
    if t.kind in ("struct", "union"):
        return t.name or f"<anon{t.kind} {t.size}>"
    if t.kind == "fwd":
        return t.name or "<fwd>"
    if t.kind == "ref":
        return f"<t{t.tid}>"
    if t.kind == "ptr":
        if t.target is not None and t.target.kind == "func":
            return f"fn->{spell(t.target.target, depth + 1)}"
        return spell(t.target, depth + 1) + "*"
    if t.kind == "array":
        return f"{spell(t.target, depth + 1)}[{t.count}]"
    if t.kind == "func":
        return f"fn->{spell(t.target, depth + 1)}"
    return t.kind


def codegen_key(t):
    """What cc1's code generation can see of a field: its size and signedness
    for a scalar, 'ptr' for any pointer (a pointer's pointee changes nothing at
    the access; it changes what the NEXT access through it means, which is that
    field's own view), and the element key for an array."""
    if t is None:
        return "?"
    if t.kind == "base":
        return f"{'s' if t.signed else 'u'}{t.size}"
    if t.kind == "ptr":
        # A slot's RETURN type is codegen-visible at every call through it (a
        # void slot cannot be cross-jumped against a value-returning one:
        # include/Entity.h's slotC4/slotCC); its parameter types are not in
        # stabs at all, see proto_text().
        if t.target is not None and t.target.kind == "func":
            r = t.target.target
            return "fn->" + ("void" if r is None or (r.kind == "base" and r.size == 0)
                             else ("ptr" if r.kind == "ptr" else codegen_key(r)))
        return "ptr"
    if t.kind == "array":
        return f"arr({codegen_key(t.target)})"
    if t.kind in ("struct", "union"):
        return f"{t.kind}{t.size}"
    return t.kind


# --------------------------------------------------------------------------
# compile and collect

STAB_RE = re.compile(r'^\s*\.stabs\s+"((?:[^"\\]|\\.)*)",(\d+),')


def unit_stabs(unit):
    src = srcpath.unit_src(unit)
    cpp = subprocess.run([str(CPP)] + makefile_flags("CPP_FLAGS") + [str(src)],
                         capture_output=True, text=True, cwd=ROOT)
    cc = subprocess.run([str(CC1), "-mips1", "-quiet", "-funsigned-char", "-G0",
                         "-O0", "-gstabs", "-w"], input=cpp.stdout,
                        capture_output=True, text=True, cwd=ROOT)
    strings, pend = [], ""
    for line in cc.stdout.splitlines():
        m = STAB_RE.match(line)
        if not m:
            continue
        s, kind = m.group(1), int(m.group(2))
        if s.endswith("\\\\"):
            pend += s[:-2]
            continue
        strings.append((pend + s, kind))
        pend = ""
    st = Stabs().parse_all(strings)
    return st, cpp.stdout


def defining_files(pre):
    """struct tag / typedef name -> the file (header or unit) that defines it,
    from cpp's line markers in the preprocessed text."""
    where, cur = {}, None
    for line in pre.splitlines():
        m = re.match(r'# \d+ "([^"]+)"', line)
        if m:
            cur = m.group(1)
            if cur.startswith(str(ROOT) + "/"):
                cur = cur[len(str(ROOT)) + 1:]
            continue
        for m in re.finditer(r"\bstruct\s+(\w+)\s*\{", line):
            where.setdefault(m.group(1), cur)
        m = re.match(r"\}\s*(\w+)\s*;", line)
        if m:
            where.setdefault(m.group(1), cur)
    return where


WARN_BASELINE = ROOT / "config/typeviews-warnings.txt"


def unit_warnings(unit):
    """cc1's warnings for one unit under the Makefile's own CC_FLAGS (so -Wall
    and -O2 exactly as the build). Parameter types of a method slot are not in
    stabs; a slot or prototype that disagrees with a call site's argument is
    an `incompatible pointer type` / `makes integer from pointer` warning, so
    'no new warnings' is the parameter half of the layout check."""
    src = srcpath.unit_src(unit)
    cpp = subprocess.run([str(CPP)] + makefile_flags("CPP_FLAGS") + [str(src)],
                         capture_output=True, text=True, cwd=ROOT)
    cc = subprocess.run([str(CC1)] + makefile_flags("CC_FLAGS") + ["-o", "/dev/null"],
                        input=cpp.stdout, capture_output=True, text=True, cwd=ROOT)
    out = [l for l in (cpp.stderr + cc.stderr).splitlines()
           if "warning" in l or ": " in l and not l.startswith("In function")]
    out = [l.replace(str(ROOT) + "/", "") for l in out]
    # line numbers move with every edit; key on the message only
    return [f"{unit}: " + re.sub(r"^[^:]*:\d+: ", "", l) for l in out
            if not re.match(r"^[^:]*: In function", l)]


def upcast(getter, base, files, objargs=("addChild", "removeChild", "addParentRef",
                                         "removeParentRef")):
    """Rewrite `getter()->slot(a, b, ...)` so `a` (and, for the listed slots,
    `b`) is cast to `base *`. A pointer-to-pointer cast emits no code; this is
    the call-site half of replacing a unit's local view of a base table with
    the base's own type. Already-cast arguments are left alone."""
    pat = re.compile(rf"\b{re.escape(getter)}\(\)->(\w+)\(([^,()]+)(,\s*([^,()]+))?")
    cast = f"({base} *)"

    def one(m):
        slot, a, rest, b = m.group(1), m.group(2).strip(), m.group(3), m.group(4)
        a2 = a if a.startswith(cast) else f"{cast}{a}"
        if rest is None:
            return f"{getter}()->{slot}({a2}"
        b = b.strip()
        b2 = b if (slot not in objargs or b.startswith(cast)) else f"{cast}{b}"
        return f"{getter}()->{slot}({a2}, {b2}"
    for f in files:
        p = ROOT / f
        t = p.read_text()
        n = pat.sub(one, t)
        if n != t:
            p.write_text(n)
            print(f"{f}: {len(pat.findall(t))} call(s)")


def units():
    return srcpath.units()


def collect(only=None):
    views = []   # dicts
    funcs = {}   # fn -> (unit, first-param type spelling, pointee struct name)
    for u in (only or units()):
        st, pre = unit_stabs(u)
        where = defining_files(pre)
        seen = set()
        for t in st.types.values():
            if t.kind not in ("struct", "union") or id(t) in seen or not t.fields:
                continue
            seen.add(id(t))
            name = t.name or f"<anon {t.size}>"
            if name.startswith("complex "):
                continue
            src = where.get(name, "?")
            if src.startswith("include/psyq"):
                continue
            views.append({
                "unit": u, "name": name, "size": t.size, "file": src,
                "kind": t.kind,
                "fields": [{"name": fn, "off": off // 8, "bitoff": off, "bits": bits,
                            "size": bits // 8, "type": spell(ft), "key": codegen_key(ft),
                            "pointee": (ft.target.name if ft is not None and ft.kind == "ptr"
                                        and ft.target is not None and ft.target.kind in
                                        ("struct", "union", "fwd") else None)}
                           for fn, ft, off, bits in t.fields],
            })
        for fn, pt in st.funcs.items():
            pointee = None
            if pt is not None and pt.kind == "ptr" and pt.target is not None \
                    and pt.target.kind in ("struct", "union", "fwd"):
                pointee = pt.target.name
            funcs.setdefault(fn, (u, spell(pt), pointee))
    return views, funcs


# --------------------------------------------------------------------------
# the class tree

def class_tree():
    sys.path.insert(0, str(ROOT / "tools"))
    import classtable as ct
    import contextlib
    import io
    data = ct.RETAIL.read_bytes()
    syms = ct.load_symbols()
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        ct.scan(data, syms)
    tabs = {}
    for line in buf.getvalue().splitlines():
        p = line.split()
        if p and p[0].startswith("0x8"):
            a, n, h = int(p[0], 16), int(p[1]), int(p[4], 16)
            if h >= 0x100000 and h >> 28:      # not an id (a code pointer): D_8006C0F8
                continue
            # a table ends where another DATA symbol begins: D_8006D4AC, the
            # NULL-terminated list of Class6D430's subclass getters, sits right
            # after D_8006D430's last slot and the scan reads straight on
            for i in range(1, n):
                nm = syms.get(a + 4 + 4 * i)
                if nm and not nm.startswith("func_") and ct.DATA[0] <= a + 4 + 4 * i < ct.DATA[1] \
                        and nm != ct.name_of(a, syms):
                    n = i
                    break
            tabs[a] = {"addr": a, "slots": n, "id": h, "name": ct.name_of(a, syms),
                       "words": ct.words(data, a + 4, n)}
    # A method table shares slots with its relatives (BasicClass's own are
    # inherited nearly everywhere); a table sharing none with any other is a
    # callback array that happens to start with a zero word, not a class
    # (gStyleCueCallbacks).
    allw = {}
    for t in tabs.values():
        for w in set(t["words"]):
            if w:
                allw[w] = allw.get(w, 0) + 1
    tabs = {a: t for a, t in tabs.items() if any(allw[w] > 1 for w in set(t["words"]) if w)}
    byid = {}
    for t in tabs.values():
        byid.setdefault(t["id"], []).append(t)

    def parent(h):
        if h == 0:
            return None
        top = max(sh for sh in range(0, 32, 4) if (h >> sh) & 0xF)
        return h & ((1 << top) - 1)
    return tabs, byid, parent, syms


def census(views, funcs):
    """Per method table: the functions the class OWNS (slots that differ from
    its nearest ancestor's table, or lie beyond it), the struct types those
    functions take as `this` (the object views), the pointee of each object
    view's +0x000 field and every `extern T <table>;` (the table views), and
    where each view is defined. A class is unified when it has exactly one
    object view and one table view, both defined in include/<Class>.h."""
    tabs, byid, parent, syms = class_tree()
    ext = {}
    for f in srcpath.src_files() + list((ROOT / "include").glob("*.h")):
        for m in re.finditer(r"^\s*extern\s+(?:const\s+)?(\w+)\s+(\w+)\s*;", f.read_text(errors="replace"), re.M):
            ext.setdefault(m.group(2), set()).add((m.group(1), str(f.relative_to(ROOT))))
    # table getters: a C function whose body is `return &TABLE;` (or `return TABLE;`)
    getters, gdecl = {}, {}
    for f in srcpath.src_files():
        t = f.read_text(errors="replace")
        for m in re.finditer(r"^[\w *]*?\b(\w+)\s*\(\s*(?:void)?\s*\)\s*\{\s*return\s+&?\s*(\w+)\s*;\s*\}", t, re.M):
            getters.setdefault(m.group(2), set()).add(m.group(1))
    allg = {g for gs in getters.values() for g in gs}
    for f in srcpath.src_files() + list((ROOT / "include").glob("*.h")):
        t = re.sub(r"/\*.*?\*/", "", f.read_text(errors="replace"), flags=re.S)
        for m in re.finditer(r"^\s*(?:extern\s+)?(?:const\s+)?(\w+)\s*\*\s*(\w+)\s*\([^)]*\)\s*[;{]", t, re.M):
            if m.group(2) in allg:
                gdecl.setdefault(m.group(2), set()).add((m.group(1), str(f.relative_to(ROOT))))
    vbyname = {}
    for v in views:
        vbyname.setdefault(v["name"], []).append(v)
    out = []
    data = (ROOT / "disk/SLPS_015.56").read_bytes()

    def rw(v):
        o = v - 0x80010000 + 0x800
        return int.from_bytes(data[o:o + 4], "little") if 0 <= o < len(data) - 4 else 0

    def first_jal(fn):
        """Target of a function's first `jal`, read from retail (C and asm alike)."""
        for i in range(200):
            w = rw(fn + 4 * i)
            if w >> 26 == 3:
                return (fn & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
            if w == 0x03E00008:                   # jr ra
                return None
        return None

    def getter_table(g):
        """The table a getter returns: `lui v0,hi; jr ra; addiu v0,v0,lo`."""
        hi = None
        for i in range(4):
            w = rw(g + 4 * i)
            op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
            if op == 0x0F:
                hi = imm << 16
            elif op == 0x09 and hi is not None:
                a = (hi + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
                return tabs.get(a)
        return None

    def id_parent(t):
        p = parent(t["id"])
        while p is not None and p not in byid:
            p = parent(p)
        return byid[p][0] if p is not None and p in byid else None

    def struct_parent(t):
        """The id tree is not always inheritance (round 83: Tod, TodSet,
        ModelData and TriggerWorld sit under TimBlockSrc's id, 0xF03, and
        Tod and ModelData chain their ctors to the ACTIVE DATA SOURCE, as
        TimBlockSrc's does: siblings, not subclasses). A subclass's ctor
        calls its parent's first, so the ctor's first call decides: a getter
        of another constant table names the parent; the same call the id
        parent's ctor makes means the two are siblings, so move up."""
        pt = id_parent(t)
        ctor = t["words"][1] if len(t["words"]) > 1 else 0
        if pt is None or not ctor or (len(pt["words"]) > 1 and pt["words"][1] == ctor):
            return pt, "id"
        g = first_jal(ctor)
        if g is None:
            return pt, "id"
        gt = getter_table(g)
        if gt is not None and gt is not t:
            return gt, ("id" if gt is pt else "ctor")
        by = "id"
        while pt is not None and len(pt["words"]) > 1 and first_jal(pt["words"][1]) == g:
            pt, by = id_parent(pt), "ctor"
        return pt, by

    for t in sorted(tabs.values(), key=lambda t: (len(f"{t['id']:x}"), t["id"])):
        pt, parent_by = struct_parent(t)
        pw = pt["words"] if pt else []
        owned = sorted({syms.get(w) for i, w in enumerate(t["words"])
                        if w and (i >= len(pw) or pw[i] != w) and syms.get(w)})
        objs = {}
        for fn in owned:
            if fn in funcs and funcs[fn][2]:
                objs.setdefault(funcs[fn][2], set()).add(funcs[fn][0])
        tviews = {}
        for o in objs:
            for v in vbyname.get(o, []):
                f0 = next((f for f in v["fields"] if f["off"] == 0), None)
                if f0 and f0["pointee"]:
                    tviews.setdefault(f0["pointee"], set()).add(v["unit"])
        for ty, where in ext.get(t["name"], ()):
            tviews.setdefault(ty, set()).add(where)
        # every declared return type of the table's getter(s) is a table view too
        for g in getters.get(t["name"], ()):
            for ty, where in gdecl.get(g, ()):
                if ty not in ("void", "s32", "u32", "u8", "char"):
                    tviews.setdefault(ty, set()).add(where)

        def home(n):
            return sorted({v["file"] for v in vbyname.get(n, [])}) or ["?"]
        out.append({"table": t["name"], "addr": t["addr"], "id": t["id"], "slots": t["slots"],
                    "parent": pt["name"] if pt else None, "parent_by": parent_by, "owned": owned,
                    "owned_c": [fn for fn in owned if fn in funcs],
                    "objects": {o: {"units": sorted(u), "defined": home(o)} for o, u in objs.items()},
                    "tables": {o: {"users": sorted(u), "defined": home(o)} for o, u in tviews.items()}})
    return out


def shared_globals(sony=False):
    """Globals `extern`-declared with more than one type across src/ and
    include/ (track 4b): the same data described by several local types, or a
    field of one table declared as a symbol of its own. Comments stripped.

    Globals only Sony code reads are left out (sony=True returns them
    instead): they are track 2's, never 4b's, for the reason progress.py
    leaves library functions out of the game counts (round 85: 35 of the 45
    this listed were libsnd's and libcd's, the plan's "largest case" among
    them, `_svm_voice` from libsnd/vmanager.o)."""
    ext = {}
    for f in srcpath.src_files() + list((ROOT / "include").glob("*.h")):
        t = re.sub(r"/\*.*?\*/", "", f.read_text(errors="replace"), flags=re.S)
        for m in re.finditer(r"^\s*extern\s+(?:const\s+)?((?:struct\s+)?\w+)\s*(\**)\s*(\w+)\s*(\[[^\]]*\])?\s*;", t, re.M):
            ty = m.group(1) + m.group(2) + ("[]" if m.group(4) else "")
            ext.setdefault(m.group(3), {}).setdefault(ty, set()).add(str(f.relative_to(ROOT)))
    multi = {k: {t: sorted(fs) for t, fs in v.items()} for k, v in ext.items() if len(v) > 1}
    lib = sony_only_globals(multi)
    return {k: v for k, v in multi.items() if (k in lib) == sony}


def global_accessors(names):
    """name -> set of functions whose code references it, from the relocations
    in the BUILT objects (build/src/*.c.o), so an `__asm__("D_...")` alias or
    a macro counts and a comment does not. Empty when the build is absent."""
    objdump = ROOT / "tools/binutils/bin/mipsel-linux-gnu-objdump"
    acc = {n: set() for n in names}
    if not objdump.exists():
        return acc
    for o in sorted((ROOT / "build/src").rglob("*.c.o")):
        out = subprocess.run([str(objdump), "-dr", str(o)], capture_output=True, text=True).stdout
        fn = None
        for line in out.splitlines():
            m = re.match(r"^[0-9a-f]+ <(\S+)>:", line)
            if m:
                fn = m.group(1)
                continue
            m = re.search(r"R_MIPS_\w+\s+([A-Za-z_]\w*)", line)
            if m and fn and m.group(1) in acc:
                acc[m.group(1)].add(fn)
    return acc


def sony_only_globals(names):
    """The names every one of whose accessors progress.py counts as library.
    A global with no accessor found (no build, or never referenced) is NOT
    Sony's: unknown stays listed."""
    sys.path.insert(0, str(ROOT / "tools"))
    import progress
    syms = progress.text_symbols()
    out = set()
    for n, fs in global_accessors(names).items():
        if fs and all(f in syms and progress.is_library(syms[f][0]) for f in fs):
            out.add(n)
    return out


# --------------------------------------------------------------------------
# merge

def merge(vs):
    """Union several layouts of one thing. Returns (rows, conflicts, size)."""
    cells = {}   # byte offset -> list of (view, field)
    size = max(v["size"] for v in vs)
    for v in vs:
        for f in v["fields"]:
            if re.fullmatch(r"(pad|unk_pad|_pad)[0-9A-Fa-f_]*", f["name"]) or f["name"].startswith("pad"):
                continue
            cells.setdefault(f["bitoff"], []).append((v, f))
    rows, conflicts = [], []
    offs = sorted(cells)
    for o in offs:
        ents = cells[o]
        keys = {(f["bits"], f["key"]) for _, f in ents}
        names = sorted({f["name"] for _, f in ents})
        types = sorted({f["type"] for _, f in ents})
        rows.append({"off": o // 8, "bitoff": o, "bits": sorted({f['bits'] for _, f in ents}),
                     "names": names, "types": types,
                     "keys": sorted(k for _, k in keys),
                     "who": sorted({f"{v['unit']}:{v['name']}" for v, _ in ents}),
                     "per": sorted({(f"{v['unit']}:{v['name']}", f["name"], f["type"]) for v, f in ents})})
        if len(keys) > 1:
            conflicts.append(rows[-1])
    # overlaps: a named field that spans into the next named offset
    overl = []
    for i, o in enumerate(offs):
        end = o + max(f["bits"] for _, f in cells[o])
        for o2 in offs[i + 1:]:
            if o2 >= end:
                break
            overl.append((o // 8, o2 // 8))
    return rows, conflicts, overl, size


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--census", action="store_true")
    ap.add_argument("--globals", action="store_true")
    ap.add_argument("--struct", nargs="+")
    ap.add_argument("--class", dest="klass")
    ap.add_argument("--merge", nargs="+", help="struct names (or unit:name) to union")
    ap.add_argument("--tree", action="store_true")
    ap.add_argument("--units", nargs="+", help="restrict compilation to these units")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--local", action="store_true", help="with --list: only views defined in src/")
    ap.add_argument("--upcast", nargs="+", metavar="ARG",
                    help="GETTER BASE FILE...: cast base-table call arguments")
    ap.add_argument("--warnings", action="store_true")
    ap.add_argument("--baseline", action="store_true", help="with --warnings: write the baseline")
    a = ap.parse_args()

    if a.upcast:
        upcast(a.upcast[0], a.upcast[1], a.upcast[2:])
        return

    if a.warnings:
        got = sorted(w for u in units() for w in unit_warnings(u))
        if a.baseline:
            WARN_BASELINE.write_text("".join(w + "\n" for w in got))
            print(f"baseline: {len(got)} warning(s) -> {WARN_BASELINE.relative_to(ROOT)}")
            return
        base = WARN_BASELINE.read_text().splitlines() if WARN_BASELINE.exists() else []
        import collections
        new = collections.Counter(got) - collections.Counter(base)
        gone = collections.Counter(base) - collections.Counter(got)
        for w in sorted(new.elements()):
            print(f"NEW   {w}")
        for w in sorted(gone.elements()):
            print(f"GONE  {w}")
        print(f"{len(got)} warning(s); {sum(new.values())} new, {sum(gone.values())} gone vs baseline")
        sys.exit(1 if new else 0)

    if a.globals:
        g = shared_globals()
        for k in sorted(g):
            print(f"{k:24s} " + "; ".join(f"{t} in {', '.join(Path(x).name for x in fs)}" for t, fs in sorted(g[k].items())))
        print(f"\n{len(g)} global(s) declared with more than one type")
        s = shared_globals(sony=True)
        print(f"{len(s)} more read only by Sony library code (track 2's, not 4b's): "
              + " ".join(sorted(s)))
        return

    if a.tree:
        tabs, byid, parent, _ = class_tree()
        kids = {}
        for h in byid:
            p = parent(h)
            while p is not None and p not in byid and p != 0:
                p = parent(p)
            kids.setdefault(p, []).append(h)

        def show(h, d):
            for t in byid.get(h, []):
                print(f"{'  ' * d}0x{h:X}  {t['name']}  ({t['slots']} slots)")
            for k in sorted(kids.get(h, [])):
                if k != h:
                    show(k, d + 1)
        for r in sorted(kids.get(None, [])):
            show(r, 0)
        return

    views, funcs = collect(a.units)
    if a.json:
        json.dump({"views": views, "funcs": funcs}, sys.stdout, indent=1)
        return

    if a.census:
        for c in census(views, funcs):
            print(f"0x{c['id']:X} {c['table']}  ({c['slots']} slots, parent {c['parent']}, "
                  f"{len(c['owned'])} own methods, {len(c['owned_c'])} of them C)")
            for o, d in sorted(c["objects"].items()):
                print(f"    object {o:28s} in {', '.join(d['defined'])}   used by {', '.join(d['units'])}")
            for o, d in sorted(c["tables"].items()):
                print(f"    table  {o:28s} in {', '.join(d['defined'])}")
        return

    if a.list:
        for v in views:
            if a.local and not v["file"].startswith("src/"):
                continue
            print(f"{v['unit']:18s} {v['name']:32s} size=0x{v['size']:X} "
                  f"fields={len(v['fields']):3d}  {v['file']}")
        return

    if a.struct:
        for v in views:
            if v["name"] in a.struct:
                print(f"== {v['unit']}  {v['name']}  size 0x{v['size']:X}  ({v['file']})")
                for f in v["fields"]:
                    print(f"   +0x{f['off']:03X} {f['size']:3d}  {f['type']:28s} {f['name']}")
        return

    if a.klass:
        k = a.klass
        tabs, byid, parent, syms = class_tree()
        methods = set()
        for t in tabs.values():
            if t["name"] == k or t["name"].upper() == k.upper() or t["name"] == f"g{k}Methods":
                methods |= {syms.get(w) for w in t["words"] if w}
        hits = {}
        for fn, (u, sp, pointee) in funcs.items():
            if pointee and (fn.startswith(k + "__") or fn in methods):
                hits.setdefault((u, pointee), []).append(fn)
        for (u, p), fs in sorted(hits.items()):
            print(f"{u:18s} this-type {p:28s} {len(fs):3d} fn  e.g. {', '.join(sorted(fs)[:3])}")
        return

    if a.merge:
        want = a.merge
        vs = [v for v in views if v["name"] in want or f"{v['unit']}:{v['name']}" in want]
        # one copy per (file, name): a header view compiled into ten units is one view
        uniq = {}
        for v in vs:
            key = (v["file"], v["name"]) if not v["file"].startswith("src/") else (v["unit"], v["name"])
            uniq.setdefault(key, v)
        vs = list(uniq.values())
        rows, conflicts, overl, size = merge(vs)
        sizes = sorted({v["size"] for v in vs})
        print(f"{len(vs)} view(s); sizes {', '.join(hex(s) for s in sizes)}")
        for v in vs:
            print(f"   {v['unit']}:{v['name']} ({v['file']}) size 0x{v['size']:X}")
        print()
        for r in rows:
            flag = "CONFLICT " if r in conflicts else "         "
            print(f"{flag}+0x{r['off']:03X} {'/'.join(map(str, r['keys'])):14s} "
                  f"{' | '.join(r['names']):40s} {' | '.join(r['types'])}")
            if r in conflicts:
                for who, n, ty in r["per"]:
                    print(f"{'':24s}{ty:16s} {n:20s} <- {who}")
            elif len(r["names"]) > 1:
                print(f"{'':24s}<- {', '.join(r['who'])}")
        print(f"\n{len(conflicts)} codegen conflict(s); {len(overl)} overlap(s)"
              + (": " + ", ".join(f"+0x{x:X}/+0x{y:X}" for x, y in overl[:10]) if overl else ""))
        return
    ap.print_help()


if __name__ == "__main__":
    main()
