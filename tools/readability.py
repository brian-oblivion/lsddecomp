#!/usr/bin/env python3
"""Readability debt, measured: what still reads like a decompiler wrote it.

    python3 tools/readability.py                 # totals, then the worst units
    python3 tools/readability.py --units         # every unit's row
    python3 tools/readability.py --types         # every placeholder type name, where defined
    python3 tools/readability.py --unit <unit> -v   # one unit: every hit, with its line
    python3 tools/readability.py --json

The companion of FINISHING-PLAN tracks 6 to 9 the way plan.py is of the plan:
the doc says what a name or a constant must be, this tool counts what is not
there yet. It measures GAME code only: a body progress.py counts as library
is Sony's and is never renamed as game code (track 3's rule), so its literals
and locals are not debt. Dead code (`#if 0`, NON_MATCHING bodies, comments)
is stripped first, exactly as progress.py strips it.

Every metric is a PATTERN, so each is a guide for staffing and a floor for
review, never a definition of readable. A runner who meets a pattern by
renaming `var_s0` to `v0` has met nothing; the head's review samples the names.

Placeholder TYPE names (track 6), any of:
  - an address-derived run: four or more UPPERCASE hex characters holding a
    digit (Class6B5CC, Obj865C8, Pair32E99C, BE54LoadReq);
  - a unit-stem suffix: `_d294`, `_fa50`, `_2864`, `_3bb8c_l`, `_d294b`;
  - `Unk` as a word in the name (Unk50Struct, UnkCObj);
  - an offset-derived member type: `Sub14`, `Sub44` at the end.
Placeholder FILES (track 7): a unit or header named `code_<hex>` or
`class_<hex>`, with or without slice suffixes.
Per-unit debt (track 8), over game bodies:
  - func_: definitions still `func_800xxxxx` or `Class__func_xxxxx`;
  - D_: distinct raw `D_800xxxxx` globals referenced;
  - unk: `->unkNN` / `.unkNN` accesses (a field no code reads is a pad, not a name);
  - slot: `->slotNN(` calls;
  - magic: integer literals of 10 or more (0x10 and up in hex) outside
    `#define` and `enum` lines -- the ones a reader has to decode;
  - rawoff: byte-pointer casts `(u8 *)`, `(s8 *)`, `(char *)`, the signature
    of offset arithmetic that should be a field access;
  - m2c: m2c-style local names (`var_s0`, `temp_v0`, `sp10`, `arg0`; m2c writes stack slots in uppercase hex, so `speed` is not one),
    counted once per function they appear in;
  - history: project history inside comments, `round NN` and retail
    addresses `0x800xxxxx` (the whole file, headers too: operator decision
    2026-09-26, history belongs in the match reports);
  - ph_prefix: definitions whose class prefix is a placeholder type
    (`Class6B5CC__AddChild`, `New_Obj865C8`): track 6's, counted here so
    track 8 knows a unit's names are settled.
"""
import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import progress  # noqa: E402
import srcpath   # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
NOT_DEF = {"if", "while", "for", "switch", "do", "return", "sizeof"}
FUNC_PH = re.compile(r"^(?:\w+__)?func_(?:800)?[0-9A-Fa-f]{5}$")

# --- placeholder type names -------------------------------------------------
TYPE_HEX = re.compile(r"(?<![0-9A-F])[0-9A-F]*[0-9][0-9A-F]*(?![0-9A-F])")
TYPE_UNIT_SUFFIX = re.compile(r"_[0-9a-f]*[0-9][0-9a-f]*[a-z]?(?:_[a-z][0-9a-z]?)*$")
# An `Unk`/`unk` name COMPONENT, case-sensitive: with re.I the "unk" inside
# "Chunk" matched (round 93: ChunkSlot, StageChunk counted as placeholders).
TYPE_UNK = re.compile(r"(?:^|[a-z0-9_])Unk(?=[A-Z0-9_]|$)|(?:^|_)unk(?=[A-Z0-9_]|$)")
TYPE_SUB = re.compile(r"Sub[0-9A-F]{1,3}$")
# Headers that are not game types: Sony's SDK, and the project's plumbing.
NOT_GAME_HEADERS = {"gte.h", "include_asm.h", "types.h"}
PLACEHOLDER_FILE = re.compile(r"^(?:code|class)_[0-9a-f]{3,6}(?:_[a-z0-9]{1,2})*$")


def placeholder_type_reason(name):
    """Why a type name reads as a placeholder, or None."""
    for m in TYPE_HEX.finditer(name):
        if len(m.group(0)) >= 4:
            return "address"
    if TYPE_UNIT_SUFFIX.search(name) and not re.search(r"_t$", name):
        return "unit-suffix"
    if TYPE_UNK.search(name):
        return "unk"
    if TYPE_SUB.search(name):
        return "offset"
    return None


def strip_comments_and_strings(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
    text = re.sub(r"'(?:\\.|[^'\\])+'", "0", text)
    return text


def type_names(text):
    """Every struct/union/enum tag and typedef name DEFINED in C text."""
    t = strip_comments_and_strings(progress.strip_dead_code(text))
    names = set(re.findall(r"\b(?:struct|union|enum)\s+(\w+)\s*\{", t))
    # typedef statements: scan to the ';' at brace depth 0
    for m in re.finditer(r"\btypedef\b", t):
        i, depth = m.end(), 0
        while i < len(t):
            ch = t[i]
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
            elif ch == ";" and depth == 0:
                break
            i += 1
        stmt = t[m.end():i]
        tail = re.sub(r"\{.*\}", " ", stmt, flags=re.S)
        fp = re.search(r"\(\s*\*\s*(\w+)\s*\)", tail)
        if fp:
            names.add(fp.group(1))
            continue
        for part in tail.split(","):
            ids = re.findall(r"[A-Za-z_]\w*", re.sub(r"\[[^\]]*\]", "", part))
            if ids:
                names.add(ids[-1])
    return names


def game_headers():
    return sorted(p for p in (ROOT / "include").glob("*.h") if p.name not in NOT_GAME_HEADERS)


def placeholder_types():
    """{type name: (reason, [files defining it])}, game headers and src only."""
    out = {}
    files = game_headers() + list(srcpath.src_files())
    for p in files:
        rel = p.relative_to(ROOT).as_posix()
        for n in type_names(p.read_text(errors="replace")):
            why = placeholder_type_reason(n)
            if why:
                out.setdefault(n, (why, []))[1].append(rel)
    return out


# --- per-unit debt ----------------------------------------------------------
M2C_RE = re.compile(r"\b(var_[a-z][0-9a-z_]*|temp_[a-z][0-9a-z_]*|sp[0-9A-F]{2,3}|arg[0-9]+)\b")
RAWOFF_RE = re.compile(r"\(\s*(?:u8|s8|char|u_char|unsigned char|signed char)\s*\*\s*\)")
NUM_RE = re.compile(r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)[uUlL]*(?![\w.])")
UNK_RE = re.compile(r"(?:->|\.)unk_?0?x?[0-9A-Fa-f]+\b")
SLOT_RE = re.compile(r"->slot_?0?x?[0-9A-Fa-f]+\s*\(")
D_RE = re.compile(r"\bD_800[0-9A-F]{5}\b")


def game_bodies(live, info):
    """[(name, body text)] for each GAME definition in comment-free live code."""
    ms = [m for m in progress.DEF_RE.finditer(live) if m.group(1) not in NOT_DEF]
    out = []
    for k, m in enumerate(ms):
        name = m.group(1)
        if info and (name not in info or progress.is_library(info[name][0])):
            continue
        end = ms[k + 1].start() if k + 1 < len(ms) else len(live)
        out.append((name, live[m.start():end]))
    return out


def magic_literals(body):
    hits, cont = [], False
    for line in body.split("\n"):
        s = line.strip()
        pp, cont = cont or s.startswith("#"), (cont or s.startswith("#")) and s.endswith("\\")
        if pp or re.match(r"^(?:typedef\s+)?enum\b", s):
            continue
        for m in NUM_RE.finditer(line):
            v = m.group(1)
            n = int(v, 16) if v[:2] in ("0x", "0X") else int(v, 10)
            if n >= 10:
                hits.append(v)
    return hits


def unit_debt(unit_path, info, detail=False):
    raw = unit_path.read_text(errors="replace")
    live = strip_comments_and_strings(progress.strip_dead_code(raw))
    bodies = game_bodies(live, info)
    text = "\n".join(b for _, b in bodies)
    row = {
        "defs": len(bodies),
        "func_": sum(1 for n, _ in bodies if FUNC_PH.match(n)),
        "D_": len(set(D_RE.findall(text))),
        "unk": len(UNK_RE.findall(text)),
        "slot": len(SLOT_RE.findall(text)),
        "magic": sum(len(magic_literals(b)) for _, b in bodies),
        "rawoff": len(RAWOFF_RE.findall(text)),
        "m2c": sum(len(set(M2C_RE.findall(b))) for _, b in bodies),
        "history": history(raw),
        "ph_prefix": sum(1 for n, _ in bodies if ph_prefix(n)),
        "placeholder_file": bool(PLACEHOLDER_FILE.match(unit_path.stem)),
    }
    if detail:
        row["_hits"] = {
            "func_": [n for n, _ in bodies if FUNC_PH.match(n)],
            "D_": sorted(set(D_RE.findall(text))),
            "m2c": sorted({f"{n}: {v}" for n, b in bodies for v in M2C_RE.findall(b)}),
            "rawoff": [n for n, b in bodies if RAWOFF_RE.search(b)],
            "magic": [f"{n}: {', '.join(magic_literals(b))}" for n, b in bodies if magic_literals(b)],
        }
    return row


DEBT_KEYS = ("func_", "D_", "unk", "slot", "magic", "rawoff", "m2c", "history")
HISTORY_RE = re.compile(r"\bround \d+|\b0x800[0-9A-Fa-f]{5}\b", re.I)


def history(raw):
    """History mentions inside the comments of a file's raw text."""
    return sum(len(HISTORY_RE.findall(c)) for c in re.findall(r"/\*.*?\*/", raw, re.S))


SYMLINE = re.compile(r"^([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)$")
UPPER_GLOBAL = re.compile(r"^[A-Z][A-Z0-9]*(?:_[A-Z0-9]+)+$|^[A-Z]{2,}[0-9]*$")


def upper_globals():
    """Game DATA symbols named like a constant (STAGE_TIME_LIMITS): the
    conventions say globals `gName`, unit-static data `sName`, UPPER_SNAKE
    only for macros and enum members (FINISHING-PLAN §3). Sony data keeps
    Sony's names (VBLANK_MINUS) and is excluded. Round 101: StageGrid
    measured zero debt while holding 16 of them."""
    import rename
    out = []
    for line in rename.SYMBOLS.read_text().splitlines():
        m = SYMLINE.match(line.strip())
        if not m or "type:func" in m.group(3) or rename.PLACEHOLDER.match(m.group(1)):
            continue
        if UPPER_GLOBAL.match(m.group(1)) and not rename.sony_data_owner(int(m.group(2), 16)):
            out.append(m.group(1))
    return out


def ph_prefix(name):
    stem = name[4:] if name.startswith("New_") else name.split("__")[0] if "__" in name else None
    return bool(stem and placeholder_type_reason(stem))


def collect(info=None, detail_unit=None):
    if info is None:
        info = progress.text_symbols()
    units = {}
    for p in srcpath.src_files():
        units[p.stem] = unit_debt(p, info, detail=(p.stem == detail_unit))
    types = placeholder_types()
    headers = [p.name for p in game_headers() if PLACEHOLDER_FILE.match(p.stem)]
    header_history = {p.relative_to(ROOT).as_posix(): history(p.read_text(errors="replace"))
                      for p in game_headers()}
    totals = {k: sum(u[k] for u in units.values()) for k in DEBT_KEYS}
    totals["placeholder_units"] = sum(u["placeholder_file"] for u in units.values())
    totals["placeholder_headers"] = len(headers)
    totals["placeholder_types"] = len(types)
    totals["header_history"] = sum(header_history.values())
    totals["ph_prefix"] = sum(u["ph_prefix"] for u in units.values())
    uppers = upper_globals()
    totals["upper_globals"] = len(uppers)
    return {"units": units, "types": types, "placeholder_headers": headers, "upper_globals": uppers,
            "header_history": header_history,
            "totals": totals, "elf_present": bool(info)}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--units", action="store_true")
    ap.add_argument("--types", action="store_true")
    ap.add_argument("--globals", action="store_true", help="list game globals named UPPER_SNAKE")
    ap.add_argument("--unit")
    ap.add_argument("-v", action="store_true")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    d = collect(detail_unit=a.unit)
    if a.json:
        print(json.dumps({k: v for k, v in d.items()}, indent=1, default=list))
        return
    if not d["elf_present"]:
        print("WARNING: build/lsdde.elf missing; run ./build-and-verify.sh first "
              "(library bodies cannot be told from game ones).")
    t = d["totals"]
    print("readability debt (game code; tools/readability.py --help for each pattern)")
    print(f"  files: {t['placeholder_units']} unit(s) and {t['placeholder_headers']} header(s) "
          f"still named code_/class_<hex>")
    print(f"  types: {t['placeholder_types']} placeholder type name(s); {t['ph_prefix']} definition(s) "
          f"under a placeholder class prefix; {t['header_history']} history mention(s) in headers")
    print(f"  names: {t['upper_globals']} game global(s) named UPPER_SNAKE (--globals lists them)")
    print("  code : " + ", ".join(f"{t[k]} {k}" for k in DEBT_KEYS))
    if a.globals:
        print()
        for n in d["upper_globals"]:
            print(f"  {n}")
    if a.types:
        print()
        for n, (why, files) in sorted(d["types"].items(), key=lambda kv: (kv[1][1][0], kv[0])):
            print(f"  {n:<32} {why:<12} {', '.join(files)}")
    if a.unit:
        u = d["units"].get(a.unit)
        if u is None:
            sys.exit(f"no unit {a.unit}")
        print()
        print(f"  {a.unit}: " + ", ".join(f"{u[k]} {k}" for k in DEBT_KEYS))
        if a.v:
            for k, hits in u["_hits"].items():
                if hits:
                    print(f"    {k}:")
                    for h in hits:
                        print(f"      {h}")
        return
    rows = sorted(d["units"].items(), key=lambda kv: -sum(kv[1][k] for k in DEBT_KEYS))
    if not a.units:
        rows = rows[:15]
    print()
    print(f"  {'unit':<18} {'defs':>5} " + " ".join(f"{k:>6}" for k in DEBT_KEYS) + "  file")
    for name, u in rows:
        print(f"  {name:<18} {u['defs']:>5} " + " ".join(f"{u[k]:>6}" for k in DEBT_KEYS)
              + ("  placeholder" if u["placeholder_file"] else ""))


if __name__ == "__main__":
    main()
