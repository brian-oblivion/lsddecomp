#!/usr/bin/env python3
"""objdiff.json and an objdiff progress report, for decomp.dev.

    python3 tools/objdiff_report.py            # objdiff.json + build/objdiff/target/**.o
    python3 tools/objdiff_report.py --report   # ... then objdiff-cli report generate -o build/report.json
    python3 tools/objdiff_report.py --with-sdk # also list Sony's code, to browse it in objdiff

Run after ./build-and-verify.sh: the BASE objects are the matching build's
own (build/src/**.c.o), and the TARGET objects are retail's bytes, from a
second splat split with `make_full_disasm_for_code` (one full .s per `c`
segment) written under build/objdiff/ so the matching build's asm/ and
lsdde.ld are never touched. That split is disc-derived, so this needs the
same disk/ and lib/ as the build.

Units, in yaml order (decomp.dev integration guide, decomp.wiki/tools/decomp-dev):
  c    target: its full disassembly;  base: build/src/<path>.c.o
  asm  target: its disassembly;       base: none (not decompiled)
  o    target = base: Sony's own object from lib/ (linked, not decompiled)
Sony's code is every `o`, every `psyq_*`/crt0 `asm` segment, and src/psyq/;
the rest is game code. The report counts game code only, as progress.py
does: Sony's objects compared with themselves would score as matched work
nobody did, and the SDK code no shipped object matches would score as work
left that isn't a goal. `--with-sdk` adds Sony's units under category `sdk`
for browsing, and is never what CI publishes.

Target sources have their `nonmatching` lines stripped: that macro emits a
`.NON_MATCHING` label, which objdiff reads as "not decompiled", and retail's
own bytes are by definition the target. The base objects keep theirs, so an
INCLUDE_ASM function still counts as not done.
"""
import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import unitfile  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/objdiff"
YAML = ROOT / "config/splat.slps01556.lsdde.yaml"
DERIVED = ROOT / "config/objdiff.splat.yaml"   # temporary, deleted after the split
VERSION = "SLPS_015.56"


def makevar(name):
    """A Makefile variable's value: its `:=`/`=` line plus every `+=` after it."""
    out = []
    for op, val in re.findall(rf"^{name}\s*(\+=|:=|=)\s*(.*)$", (ROOT / "Makefile").read_text(), re.M):
        out = [val.strip()] if op != "+=" else out + [val.strip()]
    return " ".join(out)


def derived_yaml():
    t = YAML.read_text()
    # beside the real config, so base_path `..` and every path splat writes into
    # the tree (include/include_asm.h) come out exactly as `make extract` has them
    t = t.replace("  base_path: ..\n", "  base_path: ..\n  make_full_disasm_for_code: True\n", 1)
    for k, v in [("asm_path", "build/objdiff/asm/"), ("asset_path", "build/objdiff/assets/"),
                 ("ld_script_path", "build/objdiff/lsdde.ld"),
                 ("undefined_funcs_auto_path", "build/objdiff/undefined_funcs_auto.txt"),
                 ("undefined_syms_auto_path", "build/objdiff/undefined_syms_auto.txt")]:
        t, n = re.subn(rf"^(  {k}:\s*)\S+", rf"\g<1>{v}", t, count=1, flags=re.M)
        if not n:
            sys.exit(f"FATAL: no `{k}` in {YAML.name}; this script's derived config is out of date")
    OUT.mkdir(parents=True, exist_ok=True)
    DERIVED.write_text(t)


def assemble(src, obj):
    """Assemble retail's disassembly with the Makefile's own as and flags."""
    as_ = makevar("CROSS") + "as"
    flags = makevar("AS_FLAGS").split()
    stripped = OUT / "target-src" / src.relative_to(OUT / "asm")
    stripped.parent.mkdir(parents=True, exist_ok=True)
    stripped.write_text("".join(l for l in src.read_text().splitlines(True)
                                if not l.lstrip().startswith("nonmatching ")))
    obj.parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run([as_, *flags, "-o", str(obj), str(stripped)], cwd=ROOT,
                       capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"FATAL: assembling {stripped.relative_to(ROOT)}:\n{r.stderr[-2000:]}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--report", action="store_true", help="also run objdiff-cli report generate")
    ap.add_argument("--with-sdk", action="store_true", help="also list Sony's code (category sdk); not for the report")
    a = ap.parse_args()
    if a.report and a.with_sdk:
        sys.exit("FATAL: --with-sdk is for browsing; the published report counts game code only")
    if not (ROOT / "build/lsdde.elf").exists() and not list((ROOT / "build/src").glob("**/*.c.o")):
        sys.exit("FATAL: no build/; run ./build-and-verify.sh first (the base objects are its output)")

    derived_yaml()
    shutil.rmtree(OUT / "asm", ignore_errors=True)
    try:
        r = subprocess.run([str(ROOT / ".venv/bin/python3"), "-m", "splat", "split", DERIVED.relative_to(ROOT).as_posix()],
                           cwd=ROOT, capture_output=True, text=True)
    finally:
        DERIVED.unlink()
    if r.returncode:
        sys.exit(f"FATAL: splat split of the derived config:\n{(r.stdout + r.stderr)[-2000:]}")

    units = []
    for _, _, ty, name in unitfile.seg_rows(unitfile.yaml_lines()):
        if ty not in ("c", "asm", "o"):
            continue
        sdk = ty == "o" or name.startswith(("psyq/", "psyq_")) or name == "crt0"
        if sdk and not a.with_sdk:
            continue
        unit = {"name": f"{'sdk' if sdk else 'game'}/{name}",
                "metadata": {"progress_categories": ["sdk" if sdk else "game"]}}
        if ty == "o":
            obj = ROOT / "lib" / f"{name}.o"
            unit["target_path"] = unit["base_path"] = obj.relative_to(ROOT).as_posix()
        else:
            tobj = OUT / "target" / f"{name}.s.o"
            assemble(OUT / "asm" / f"{name}.s", tobj)
            unit["target_path"] = tobj.relative_to(ROOT).as_posix()
            base = ROOT / "build/src" / f"{name}.c.o"
            if ty == "c":
                if not base.exists():
                    sys.exit(f"FATAL: {base.relative_to(ROOT)} missing; run ./build-and-verify.sh first")
                unit["base_path"] = base.relative_to(ROOT).as_posix()
        units.append(unit)

    cfg = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "true",      # the matching build is ./build-and-verify.sh, never objdiff's
        "build_target": False,
        "build_base": False,
        "units": units,
        "progress_categories": [{"id": "game", "name": "Game"}]
                               + ([{"id": "sdk", "name": "Psy-Q SDK"}] if a.with_sdk else []),
    }
    (ROOT / "objdiff.json").write_text(json.dumps(cfg, indent=2) + "\n")
    print(f"objdiff.json: {len(units)} units "
          f"({sum(u['name'].startswith('game/') for u in units)} game, "
          f"{sum(u['name'].startswith('sdk/') for u in units)} sdk)")

    if a.report:
        cli = shutil.which("objdiff-cli") or str(ROOT / "tools/objdiff-cli")
        if not Path(cli).exists():
            sys.exit("FATAL: objdiff-cli not found (PATH or tools/objdiff-cli); "
                     "https://github.com/encounter/objdiff/releases")
        r = subprocess.run([cli, "report", "generate", "-o", "build/report.json"], cwd=ROOT)
        if r.returncode:
            sys.exit(r.returncode)
        print(f"build/report.json written; upload it as the `{VERSION}_report` artifact")


if __name__ == "__main__":
    main()
