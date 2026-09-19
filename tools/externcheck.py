#!/usr/bin/env python3
"""Find `extern` function declarations whose ARITY disagrees with the
function's definition or with another declaration of the same name.

    python3 tools/externcheck.py            # whole tree; exit 1 on any conflict
    python3 tools/externcheck.py <func>...  # just these names

WHY THIS EXISTS (round 57). `class_3bb8c_p.c` declared
`extern s32 func_8001E7BC(void)` and called it with no arguments, while the
function's real signature, established when it was matched that round, takes
three. The call site was byte-identical either way, because the values the
callee reads were already sitting in $a0..$a2. The byte oracle CANNOT see a
wrong prototype, and a wrong one survives for as long as the callee stays
INCLUDE_ASM, since no definition exists to conflict with. The compiler only
catches it once both a definition and the wrong extern land in one
translation unit, which the project's local-extern convention makes rare.

What this reads: every `extern <type> NAME(<params>);` in src/ and include/,
and every function DEFINITION in src/ (live code: `#if 0` and NON_MATCHING
blocks stripped, like progress.py). It compares PARAMETER COUNTS only, which
is what the register-forwarding trap changes; types are left to the compiler.
`(void)` and `()` both count as zero. A variadic `...` is skipped.

Run it after any round that MATCHES a function (PARALLEL-RUNS 3.9), because
a match is the moment a real signature first exists to check externs against.
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import progress  # noqa: E402
import srcpath   # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
EXTERN_RE = re.compile(r"^\s*extern\s+[^;=(]*?\b(\w+)\s*\(([^;]*?)\)\s*;", re.M)
DEF_RE = re.compile(r"^\w[^;=]*?\b(\w+)\s*\(([^;{]*)\)\s*\{", re.M)
NOT_DEF = {"if", "while", "for", "switch", "do", "return", "sizeof"}


def arity(params):
    p = re.sub(r"/\*.*?\*/", "", params, flags=re.S).strip()
    if p in ("", "void"):
        return 0
    if "..." in p:
        return None
    depth, n = 0, 1
    for ch in p:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == "," and depth == 0:
            n += 1
    return n


def main():
    only = set(sys.argv[1:])
    defs, externs = {}, {}
    for c in srcpath.src_files():
        live = progress.strip_dead_code(c.read_text(errors="replace"))
        for name, params in DEF_RE.findall(live):
            if name in NOT_DEF:
                continue
            defs.setdefault(name, []).append((arity(params), str(c.relative_to(ROOT))))
        for name, params in EXTERN_RE.findall(live):
            externs.setdefault(name, []).append((arity(params), str(c.relative_to(ROOT))))
    for h in ROOT.glob("include/*.h"):
        text = re.sub(r"/\*.*?\*/", "", h.read_text(errors="replace"), flags=re.S)
        for name, params in EXTERN_RE.findall(text):
            externs.setdefault(name, []).append((arity(params), str(h.relative_to(ROOT))))

    bad = 0
    for name in sorted(externs):
        if only and name not in only:
            continue
        if name in ("void", "int", "s32", "u32", "char"):
            continue            # `extern void (*fp)(...)`: a function POINTER, not a function
        if any(a is None for a, _ in externs[name]) or any(a is None for a, _ in defs.get(name, [])):
            continue            # variadic somewhere (printf): arity is not a fixed number
        decls = [d for d in externs[name] if d[0] is not None]
        truth = [d for d in defs.get(name, []) if d[0] is not None]
        counts = {a for a, _ in decls} | {a for a, _ in truth}
        if len(counts) <= 1:
            continue
        bad += 1
        print(f"{name}: parameter counts disagree")
        for a, where in truth:
            print(f"    DEFINITION {a}  {where}")
        for a, where in decls:
            print(f"    extern     {a}  {where}")
    if bad:
        print(f"\n{bad} function(s) with conflicting arity. The byte oracle cannot see these;"
              " fix the extern to the definition (or, with no definition, to the disassembly).")
        return 1
    print(f"OK: {sum(len(v) for v in externs.values())} extern declarations, no arity conflicts.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
