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
  2. refuses if NEW is not a C identifier, is already used anywhere, or is a
     splat placeholder spelling;
  3. rewrites the symbols file (replaces OLD's line, or appends a line for a
     placeholder that had none) and every whole-word OLD in src/, include/,
     the report files and the live docs -- NOT docs/PROGRESS.md and NOT
     docs/archive/, which are narrative frozen at the time of writing;
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


def text_files():
    """Every file a symbol name may appear in, excluding generated and archive."""
    out = []
    for pat in ("src/**/*.c", "include/*.h", "include/*.inc",
                "docs/*.md", "docs/match-reports/*.md", "docs/research/*.md",
                "CLAUDE.md", "config/gp-symbols.txt"):
        out.extend(ROOT.glob(pat))
    # docs/PROGRESS.md is the append-only NARRATIVE: a past round's entry
    # describes what was observed under the name in use at the time, and
    # rewriting it makes round 46 talk about a name invented in round 50
    # (found by runner charlie, round 50). docs/archive/ is frozen for the
    # same reason and is not in the globs above.
    out = [p for p in out if p.name != "PROGRESS.md"]
    return sorted(set(out))


def symbol_address(name):
    m = PLACEHOLDER.match(name)
    if m:
        return int(m.group(2), 16), None
    for i, line in enumerate(SYMBOLS.read_text().splitlines()):
        sm = SYMLINE.match(line.strip())
        if sm and sm.group(1) == name:
            return int(sm.group(2), 16), i
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
    if pat.search(SYMBOLS.read_text()):
        code.append(str(SYMBOLS.relative_to(ROOT)))
    for p in ROOT.glob("asm/**/*.s"):
        if re.search(rf"^glabel {re.escape(name)}$|^dlabel {re.escape(name)}$",
                     p.read_text(errors="replace"), re.M):
            code.append(str(p.relative_to(ROOT)))
    return code, prose


def is_text_symbol(addr):
    """Is this address inside the executable's text, per the linked ELF?"""
    nm = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
    elf = ROOT / "build/lsdde.elf"
    if not (nm.exists() and elf.exists()):
        return None
    out = subprocess.run([str(nm), "-n", str(elf)], capture_output=True,
                         text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and int(parts[0], 16) == addr:
            return parts[1] in ("T", "t")
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
    if PLACEHOLDER.match(new):
        sys.exit(f"FATAL: {new!r} is a splat placeholder spelling, not a name")
    if old == new:
        sys.exit("FATAL: old and new are the same")

    addr, symline = symbol_address(old)
    if addr is None:
        sys.exit(f"FATAL: {old!r} is neither a placeholder nor in {SYMBOLS.name}")
    if not (VRAM_LO <= addr < VRAM_HI):
        sys.exit(f"FATAL: {old!r} resolves to {addr:#x}, outside the image")

    code_hits, prose_hits = name_in_use(new)
    if code_hits:
        sys.exit(f"FATAL: {new!r} already exists in code: " + ", ".join(code_hits[:6]))
    if prose_hits:
        print(f"note: {new!r} already appears in prose ({len(prose_hits)} file(s): "
              + ", ".join(prose_hits[:3]) + "); those mentions are left as they are.")

    pat = re.compile(rf"\b{re.escape(old)}\b")
    touched = [p for p in text_files() if pat.search(p.read_text(errors="replace"))]
    report_old = REPORTS / f"{old}.md"
    report_new = REPORTS / f"{new}.md"
    if report_new.exists():
        sys.exit(f"FATAL: {report_new.relative_to(ROOT)} already exists")

    is_text = is_text_symbol(addr)
    kind = "func" if is_text else ("data" if is_text is False else "unknown")

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
    for p in touched:
        text = p.read_text(errors="replace")
        p.write_text(pat.sub(new, text))

    # 4. the report
    if report_old.exists():
        subprocess.run(["git", "mv", str(report_old), str(report_new)], check=True)
        today = datetime.date.today().isoformat()
        body = report_new.read_text()
        note = f"> Renamed from `{old}` on {today} (tools/rename.py). Address {addr:#x}.\n\n"
        report_new.write_text(note + body)

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
