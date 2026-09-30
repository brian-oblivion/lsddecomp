#!/usr/bin/env python3
"""Rename a symbol everywhere it lives, then re-extract and re-verify.

    python3 tools/rename.py OLD NEW              # do it
    python3 tools/rename.py OLD NEW --dry-run    # show what would change
    python3 tools/rename.py OLD NEW --no-build   # edit only (batching renames)

WHY THIS EXISTS. A symbol name lives in five places that nothing keeps in
step: the splat symbols file (which decides what `make extract` writes into
asm/), the C in src/ and include/, the match report whose FILENAME is the
function's name (tools/progress.py keys STALL vs FRESH on that file existing),
and every other report or doc that mentions it. Renaming by hand gets one of
them wrong, and the wrong one is usually the report filename, which silently
turns a documented stall back into fresh ground. Readability work (FINISHING-
PLAN.md, track 3) is hundreds of renames, so this has to be one command.

WHAT IT DOES, in order:
  1. resolves OLD's address: from the name if it is a splat placeholder
     (`func_800XXXXX`, `D_800XXXXX`), else from the symbols file;
  2. refuses if NEW is not a C identifier, is already a symbol or visible where
     OLD is used (an unrelated local elsewhere is only noted), or is a
     splat placeholder spelling other than OLD's own (renaming to OLD's own
     placeholder UNNAMES it); refuses a game-style `gName`/`sName` for data
     every accessor of which is Sony library code (tools/sonydata.py);
  3. rewrites the symbols file (replaces OLD's line, or appends a line for a
     placeholder that had none) and every whole-word OLD in src/, include/,
     the report files and the live docs -- NOT docs/PROGRESS.md and NOT
     docs/archive/, which are narrative frozen at the time of writing, and NOT
     the rule docs (RULE_DOCS), whose mentions are listed instead;
  4. renames docs/match-reports/OLD.md to NEW.md and prepends a note so the
     old name stays greppable;
  5. `make extract` (asm/ is generated from the symbols file), then
     `./build-and-verify.sh`, then `python3 tools/gpsyms.py --check` for a
     data symbol. A rename changes ZERO bytes, so a red build means the rename
     is wrong; the tool prints the revert commands and exits 1.

WHAT IT DOES NOT DO. It does not judge the name. The naming rules (what the
code does, not what you guess it is for; tiers; conventions) are in
FINISHING-PLAN.md track 3, and the evidence for a name goes in the match
report. It does not touch config/psyq-objects.ld or the splat yaml: Sony's
object symbols are Sony's names and are not renamed; yaml comments are
provenance.
"""
import argparse
import datetime
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SYMBOLS = ROOT / "config/symbols.slps01556.lsdde.txt"
REPORTS = ROOT / "docs/match-reports"
IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
PLACEHOLDER = re.compile(r"^(func|D|jtbl|jpt)_(800[0-9A-Fa-f]{5})$")
SYMLINE = re.compile(r"^(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)$")
# The whole of PS1 main RAM. The old bound was the end of the FILE image, and
# bss lives past it: round 51 was refused `D_8008E248 -> gShadeTex`.
VRAM_LO, VRAM_HI = 0x80010000, 0x80200000


HEADING = re.compile(r"^(#+)\s")
HISTORY = re.compile(r"histor", re.I)
IF0 = re.compile(r"^#if 0\b.*?^#endif\b", re.M | re.S)


def frozen_spans(text):
    """[(start, end)] character spans of a match report's history sections,
    which every rename tool leaves as written: a heading naming history ("##
    File history", "## Earlier history") down to the next heading of its
    level or above. They record the names in use when written; a rename made
    them quote commands never run and name retired types (rounds 90, 94, 97).
    A preserved `#if 0` body inside one stays live (tools/research/stalesyms.py)."""
    spans, start, level, pos = [], None, 0, 0
    for line in text.split("\n"):
        m = HEADING.match(line)
        if m and start is not None and len(m.group(1)) <= level:
            spans.append((start, pos))
            start = None
        if m and start is None and HISTORY.search(line):
            start, level = pos, len(m.group(1))
        pos += len(line) + 1
    if start is not None:
        spans.append((start, len(text)))
    out = []
    for s, e in spans:
        for b in IF0.finditer(text, s, e):
            out.append((s, b.start()))
            s = b.end()
        out.append((s, e))
    return [(s, e) for s, e in out if s < e]


# the provenance note step 4 below writes; it names OLD by design
RENAMED_NOTE = "> Renamed from `"


def sub_prose(rx, new, text, path, keep_lines=()):
    """rx.sub(new) line by line in PROSE, leaving a report's frozen history,
    a `> Renamed from` note, and any line that already names NEW (it is
    ABOUT the rename: "X became NEW", rounds 71 and 97)."""
    nx = re.compile(rf"(?<![A-Za-z0-9_]){re.escape(new)}(?![A-Za-z0-9_])")
    frozen = frozen_spans(text) if Path(path).parent.name == "match-reports" else []
    out, pos = [], 0
    for line in text.split("\n"):
        at, pos = pos, pos + len(line) + 1
        keep = (nx.search(line) or line in keep_lines or line.startswith(RENAMED_NOTE)
                or any(s <= at < e for s, e in frozen))
        out.append(line if keep else rx.sub(new, line))
    return "\n".join(out)


def is_code(path):
    return Path(path).resolve().relative_to(ROOT).as_posix().startswith(("src/", "include/", "config/"))


def text_files():
    """Every file a symbol name may appear in, excluding generated and archive."""
    out = []
    for pat in ("src/**/*.c", "include/*.h", "include/*.inc",
                "docs/*.md", "docs/match-reports/*.md", "docs/research/*.md",
                "CLAUDE.md", "config/gp-symbols.txt", "config/typeviews-warnings.txt"):
        out.extend(ROOT.glob(pat))
    # docs/PROGRESS.md is the append-only NARRATIVE: a past round's entry
    # describes what was observed under the name in use at the time, and
    # rewriting it makes round 46 talk about a name invented in round 50
    # (found by runner charlie, round 50). docs/archive/ is frozen for the
    # same reason and is not in the globs above.
    out = [p for p in out if p.name != "PROGRESS.md"]
    # RULE docs are not rewritten either (rounds 74, 75): their examples show a
    # FORM, and a rename turned the tier-C placeholder example into a tier-A
    # name. main() lists their mentions of OLD for the head to judge.
    out = [p for p in out if p.relative_to(ROOT).as_posix() not in RULE_DOCS]
    return sorted(set(out))


RULE_DOCS = {"CLAUDE.md", "docs/FINISHING-PLAN.md", "docs/PARALLEL-RUNS.md",
             "docs/MATCHING-GUIDE.md", "docs/SDK-OBJECTS-GUIDE.md", "docs/SDK-OBJECTS-RUNS.md"}


def symbol_address(name):
    # The symbols file FIRST, placeholder or not: a func_ with its own line
    # (round 81: ~48 stale track-2 records on code revision 18 moved to game
    # units) otherwise got a second line appended and splat refused the file.
    for i, line in enumerate(SYMBOLS.read_text().splitlines()):
        sm = SYMLINE.match(line.strip())
        if sm and sm.group(1) == name:
            return int(sm.group(2), 16), i
    m = PLACEHOLDER.match(name)
    if m:
        return int(m.group(2), 16), None
    return None, None


def name_in_use(name):
    """(code_hits, prose_hits): where NEW already appears as a whole word.

    Only CODE decides: src/, include/, the symbols file, and asm labels. A
    report or doc that already uses the intended name in prose is the normal
    state of a well-derived rename (round 51 was refused `BASICCLASS_METHODS`
    on six such files) and is reported as a warning, not a refusal."""
    pat = re.compile(rf"\b{re.escape(name)}\b")
    code, prose = [], []
    for p in text_files():
        text = p.read_text(errors="replace")
        rel = str(p.relative_to(ROOT))
        is_code = rel.startswith(("src/", "include/", "config/"))
        if is_code:
            # A comment in a header saying "PROPOSED RENAME: NEW" is prose,
            # not a definition (round 51's BASICCLASS_METHODS refusal).
            stripped = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
            if pat.search(stripped):
                code.append(rel)
            elif pat.search(text):
                prose.append(rel)
        elif pat.search(text):
            prose.append(rel)
    # An ENTRY, not a mention: a comment naming NEW as the identification
    # being applied blocked round 71's track-2 renames again and again.
    if re.search(rf"^\s*{re.escape(name)}\s*=", SYMBOLS.read_text(), re.M):
        code.append(str(SYMBOLS.relative_to(ROOT)))
    for p in ROOT.glob("asm/**/*.s"):
        if re.search(rf"^glabel {re.escape(name)}$|^dlabel {re.escape(name)}$",
                     p.read_text(errors="replace"), re.M):
            code.append(str(p.relative_to(ROOT)))
    return code, prose


def is_text_symbol(name, addr):
    """Is this symbol a FUNCTION? Decided from splat's own labelling first:
    `glabel NAME` in a text `.s` is a function, `dlabel NAME` under asm/data/
    is data. The linked ELF is only a fallback, because splat's data labels
    come out of the link typed `T` (D_8006B58C, a method table, reads as a
    function there), and a wrong `type:func` in the symbols file would make
    splat disassemble a table as code."""
    esc = re.escape(name)
    for p in ROOT.glob("asm/data/*.s"):
        if re.search(rf"^dlabel {esc}$", p.read_text(errors="replace"), re.M):
            return False
    for p in list(ROOT.glob("asm/*.s")) + list(ROOT.glob("asm/nonmatchings/**/*.s")):
        if re.search(rf"^glabel {esc}$", p.read_text(errors="replace"), re.M):
            return True
    # Already-C functions have no .s: a src/ definition, or a symbols-file
    # line that says type:func (an untyped line proves nothing either way).
    for p in ROOT.glob("src/**/*.c"):
        if re.search(rf"^\w[^;=]*?\b{esc}\s*\([^;{{]*\)\s*\{{", p.read_text(errors="replace"), re.M):
            return True
    for line in SYMBOLS.read_text().splitlines():
        if re.match(rf"^\s*{esc}\s*=", line) and "type:func" in line:
            return True
    nm = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
    elf = ROOT / "build/lsdde.elf"
    if not (nm.exists() and elf.exists()):
        return None
    out = subprocess.run([str(nm), "-n", str(elf)], capture_output=True,
                         text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and int(parts[0], 16) == addr and parts[2] == name:
            return parts[1] in ("T", "t")
    return None


GAME_STYLE = re.compile(r"^[gs][A-Z]")


def IS_OWN_PLACEHOLDER(new, old):
    """NEW is the splat placeholder for OLD's own address: an UNNAME, which is
    how a game name comes off a Sony static no disc names (round 86)."""
    m = PLACEHOLDER.match(new)
    addr, _ = symbol_address(old)
    return bool(m) and addr is not None and int(m.group(2), 16) == addr


def sony_only_accessors(name):
    """Sorted accessor functions when EVERY one is library code (progress.py),
    read from the built objects' relocations; [] when any is game code or none
    is found. The guard for Sony objects that never placed, which therefore
    have no psyq-objects.ld pin for sony_data_owner to see (round 86:
    libsnd/vmanager.o's bss had taken game names)."""
    sys.path.insert(0, str(ROOT / "tools"))
    import progress
    import typeviews
    fs = typeviews.global_accessors([name])[name]
    syms = progress.text_symbols()
    if fs and all(f in syms and progress.is_library(syms[f][0]) for f in fs):
        return sorted(fs)
    return []


def sony_data_owner(addr):
    """(pin_name, pin_addr, size) when `addr` is a Sony variable that
    config/psyq-objects.ld pins, or lies inside one; else None. Sizes come from
    the defining objects on the SDK discs (sdk/work/*/elf); a pin whose size no
    object gives covers its own address only. Round 75 named 24 libsnd
    variables in game words, five of them offsets inside `_svm_cur`."""
    ld = ROOT / "config/psyq-objects.ld"
    if not ld.exists():
        return None
    pins, owner = {}, {}
    for m in re.finditer(r"^(\w+) = (0x[0-9A-Fa-f]{8});(?:\s*/\*\s*(\S+)\s*\*/)?",
                         ld.read_text(), re.M):
        pins.setdefault(int(m.group(2), 16), m.group(1))
        if m.group(3):
            owner[m.group(1)] = m.group(3)
    if addr in pins:
        return pins[addr], addr, 0
    near = [(a, n) for a, n in pins.items() if a < addr <= a + 0x2000]
    if not near:
        return None
    # A symbol's size differs between SDK builds (libpress/vlc2's dc_cr is 8
    # bytes on 3.0 and 4 on 3.3, the build retail links), so a pin whose owning
    # object is placed takes its size from that object on the disc
    # config/psyq-objects.txt places it from; any other takes the largest size
    # any disc gives (round 99: the maximum refused D_8008E228, the word after
    # dc_cr, as inside it).
    placed = {}
    manifest = ROOT / "config/psyq-objects.txt"
    if manifest.exists():
        for line in manifest.read_text().splitlines():
            parts = line.split("#", 1)[0].split()
            if len(parts) == 3:
                placed[parts[1]] = parts[0]
    nm = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
    names = {n for _, n in near}

    def nm_sizes(o, want):
        out = subprocess.run([str(nm), "-S", "--defined-only", str(o)],
                             capture_output=True, text=True).stdout
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 4 and parts[3] in want:
                yield parts[3], int(parts[1], 16)

    sizes, exact = {}, set()
    for n in names:
        disc = placed.get(owner.get(n, ""))
        o = ROOT / "sdk/work" / str(disc) / "elf" / f"{owner.get(n, '')}.o"
        if disc and o.exists():
            for sym, size in nm_sizes(o, {n}):
                sizes[sym] = size
                exact.add(sym)
    for o in (ROOT / "sdk/work").glob("*/elf/**/*.o"):
        for sym, size in nm_sizes(o, names - exact):
            sizes[sym] = max(sizes.get(sym, 0), size)
    for a, n in sorted(near, reverse=True):
        if addr < a + sizes.get(n, 0):
            return n, a, sizes[n]
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("old")
    ap.add_argument("new")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--no-build", action="store_true",
                    help="edit files only; you run extract + verify yourself")
    a = ap.parse_args()
    os.chdir(ROOT)

    old, new = a.old, a.new
    if not IDENT.match(new):
        sys.exit(f"FATAL: {new!r} is not a C identifier")
    pm = PLACEHOLDER.match(new)
    if pm and not (IS_OWN_PLACEHOLDER(new, old)):
        sys.exit(f"FATAL: {new!r} is a splat placeholder spelling, not a name "
                 f"(the one placeholder allowed is OLD's own, to UNNAME it)")
    if old == new:
        sys.exit("FATAL: old and new are the same")

    addr, symline = symbol_address(old)
    if addr is None:
        sys.exit(f"FATAL: {old!r} is neither a placeholder nor in {SYMBOLS.name}")
    if not (VRAM_LO <= addr < VRAM_HI):
        sys.exit(f"FATAL: {old!r} resolves to {addr:#x}, outside the image")
    owner = sony_data_owner(addr)
    if owner and not (owner[1] == addr and new == owner[0]):
        pin, pa, size = owner
        where = "is" if pa == addr else f"lies at +{addr - pa:#x} inside"
        sys.exit(f"FATAL: {addr:#x} {where} Sony's `{pin}` ({pa:#x}, {size:#x} bytes, pinned in "
                 f"config/psyq-objects.ld). Sony data keeps Sony's name (FINISHING-PLAN track 3); "
                 f"the only rename allowed here is to `{pin}` itself.")

    code_hits, prose_hits = name_in_use(new)
    # A code hit is a COLLISION only if NEW is a symbol (symbols entry, asm
    # label, a definition or file-scope declaration) or shares a file with OLD,
    # where a local named NEW would shadow the renamed function. Otherwise it
    # is an unrelated local (`callback`, round 71) and the link decides.
    oldpat = re.compile(rf"\b{re.escape(old)}\b")
    scope = re.compile(rf"^\w[^;=(]*\b{re.escape(new)}\b\s*(\(|;|\[|=)", re.M)
    hard, soft = [], []
    for rel in code_hits:
        p = ROOT / rel
        if not rel.startswith(("src/", "include/")):
            hard.append(rel)
            continue
        t = re.sub(r"/\*.*?\*/", "", p.read_text(errors="replace"), flags=re.S)
        (hard if scope.search(t) or oldpat.search(t) else soft).append(rel)
    if hard:
        sys.exit(f"FATAL: {new!r} already exists in code: " + ", ".join(hard[:6]))
    if soft:
        print(f"note: {new!r} is a local identifier in {len(soft)} file(s) that never reference "
              f"{old!r} ({', '.join(soft[:3])}); not a collision, the build decides.")
    if prose_hits:
        print(f"note: {new!r} already appears in prose ({len(prose_hits)} file(s): "
              + ", ".join(prose_hits[:3]) + "); those mentions are left as they are.")

    pat = re.compile(rf"\b{re.escape(old)}\b")
    touched = [p for p in text_files() if pat.search(p.read_text(errors="replace"))]
    report_old = REPORTS / f"{old}.md"
    report_new = REPORTS / f"{new}.md"
    if report_new.exists():
        sys.exit(f"FATAL: {report_new.relative_to(ROOT)} already exists")

    is_text = is_text_symbol(old, addr)
    kind = "func" if is_text else ("data" if is_text is False else "unknown")
    if kind != "func" and GAME_STYLE.match(new):
        lib = sony_only_accessors(old)
        if lib:
            sys.exit(f"FATAL: every function that reads {old} is Sony library code "
                     f"({', '.join(lib[:4])}); Sony data takes no game name (FINISHING-PLAN "
                     f"track 3). `python3 tools/sonydata.py` gives Sony's name, or says it is a "
                     f"static, whose honest spelling is the placeholder.")

    print(f"rename {old} -> {new}   ({addr:#x}, {kind})")
    print(f"  symbols file: {'replace line' if symline is not None else 'append line'}")
    print(f"  {len(touched)} file(s) with whole-word references:")
    for p in touched:
        n = len(pat.findall(p.read_text(errors="replace")))
        print(f"    {n:4d}  {p.relative_to(ROOT)}")
    if report_old.exists():
        print(f"  report: {report_old.relative_to(ROOT)} -> {report_new.name}")
    if not touched and symline is None and not report_old.exists():
        sys.exit("FATAL: nothing references the old name; is it spelled right?")
    for rel in sorted(RULE_DOCS):
        rp = ROOT / rel
        if rp.exists():
            for i, line in enumerate(rp.read_text(errors="replace").split("\n"), 1):
                if pat.search(line):
                    print(f"  rule doc, NOT rewritten: {rel}:{i} mentions {old}; update by hand only if it is a reference, never an example")
    if a.dry_run:
        return

    # 3. symbols file
    lines = SYMBOLS.read_text().splitlines(keepends=True)
    if symline is not None:
        lines[symline] = re.sub(rf"^(\s*){re.escape(old)}\b", rf"\g<1>{new}", lines[symline])
    else:
        tag = " // type:func" if kind == "func" else ""
        if lines and not lines[-1].endswith("\n"):
            lines[-1] += "\n"
        lines.append(f"{new} = 0x{addr:08X};{tag}\n")
    SYMBOLS.write_text("".join(lines))

    # 3. every other file
    vacuous = re.compile(rf"\b{re.escape(new)}\b\s*(->|→|=>)\s*\b{re.escape(new)}\b")
    newpat = re.compile(rf"\b{re.escape(new)}\b")
    for p in touched:
        text = p.read_text(errors="replace")
        rel = str(p.relative_to(ROOT))
        if rel.startswith(("src/", "include/", "config/")):
            text = pat.sub(new, text)
        else:
            # Prose: a line that already names NEW is ABOUT the rename ("X is
            # libcd's getintr"), and rewriting it reads "getintr, libcd's
            # getintr" (round 71, twice). Leave it and say where.
            out, pos = [], 0
            frozen = frozen_spans(text) if p.parent.name == "match-reports" else []
            for i, line in enumerate(text.split("\n"), 1):
                at, pos = pos, pos + len(line) + 1
                if line.startswith(RENAMED_NOTE) or any(s <= at < e for s, e in frozen):
                    out.append(line)
                elif pat.search(line) and newpat.search(line):
                    print(f"  note: {rel}:{i} names both {old} and {new}; left as written, edit by hand if needed")
                    out.append(line)
                else:
                    # A quoted archive citation, `§"...func_X..."`, must keep
                    # matching the frozen heading it points to (round 77).
                    parts = re.split(r'(§"[^"]*")', line)
                    out.append("".join(q if q.startswith('§"') else pat.sub(new, q) for q in parts))
            text = "\n".join(out)
        p.write_text(text)
        # A "PROPOSED RENAME: old -> new" note whose proposal this rename just
        # applied now reads "new -> new" (round 67, code_8220.h). The rewrite
        # is right -- references stay resolvable -- but the note is done.
        for i, line in enumerate(text.split("\n"), 1):
            if vacuous.search(line):
                print(f"  note: {p.relative_to(ROOT)}:{i} now says {new} -> {new}; delete that proposal note")

    # 4. the report
    if report_old.exists():
        subprocess.run(["git", "mv", str(report_old), str(report_new)], check=True)
        today = datetime.date.today().isoformat()
        body = report_new.read_text()
        note = f"> Renamed from `{old}` on {today} (tools/rename.py). Address {addr:#x}.\n\n"
        # AFTER the title line, never above it: nearmiss.py ranks from the first
        # eight lines and a note above the `#` heading pushed 18 conforming
        # titles out of that window (round 63), de-ranking the queue in
        # proportion to naming progress.
        lines = body.split("\n")
        for i, line in enumerate(lines):
            if line.startswith("# "):
                lines.insert(i + 1, "\n" + note.rstrip("\n"))
                body = "\n".join(lines)
                break
        else:
            body = note + body
        report_new.write_text(body)

    if a.no_build:
        print("edited. You must run: make extract && ./build-and-verify.sh")
        return

    # 5. extract, verify
    print("== make extract")
    r = subprocess.run(["make", "extract"], capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stderr[-2000:])
        fail(old, new, touched, report_old.exists())
    print("== ./build-and-verify.sh")
    r = subprocess.run(["./build-and-verify.sh"], capture_output=True, text=True)
    if r.returncode != 0:
        print((r.stdout + r.stderr)[-3000:])
        fail(old, new, touched, report_old.exists())
    if kind == "data":
        r = subprocess.run([sys.executable, "tools/gpsyms.py", "--check"])
        if r.returncode != 0:
            print("gp-symbols.txt is stale: run python3 tools/gpsyms.py and re-verify")
            sys.exit(1)
    print(f"OK: {old} -> {new}, image byte-identical.")


def fail(old, new, touched, had_report):
    print("\nRENAME BROKE THE BUILD. A rename changes zero bytes, so the edit is wrong.")
    print("Revert with:")
    print(f"  git checkout -- {SYMBOLS.relative_to(ROOT)} " +
          " ".join(str(p.relative_to(ROOT)) for p in touched))
    if had_report:
        print(f"  git mv docs/match-reports/{new}.md docs/match-reports/{old}.md && "
              f"git checkout -- docs/match-reports/{old}.md")
    print("  make extract && ./build-and-verify.sh")
    sys.exit(1)


if __name__ == "__main__":
    main()
