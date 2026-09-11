#!/usr/bin/env python3
"""Turn the user's Psy-Q SDK disc(s) into the prebuilt library objects in lib/.

    python3 tools/psyq_sdk.py install     # sdk/*.zip -> lib/<lib>/<module>.o per the manifest
    python3 tools/psyq_sdk.py match       # locate EVERY converted object in retail (discovery)
    python3 tools/psyq_sdk.py coverage    # which SDK functions the matched objects own
    python3 tools/psyq_sdk.py check       # manifest and splat yaml agree, lib/ is complete

WHY. The executable links Sony's Psy-Q libraries. Those libraries shipped as
`.LIB` archives of `.OBJ` files on the SDK discs, with full symbol names, so
instead of re-deriving that code as C we link the objects themselves, exactly
as the game did, through splat `o` segments. Bytes are Sony's, so nothing here
is committed: `sdk/` holds the user's discs, `lib/` is generated from them,
both are gitignored, and `config/psyq-objects.txt` is the committed manifest
that says which object from which disc lands where.

Pipeline, per disc, all under sdk/work/<version>/ and each step idempotent:
    track1.bin   the data track pulled out of the redump zip
    psx/         PSX/LIB/*.LIB, PSX/LIB/*.OBJ and PSX/INCLUDE/** read straight
                 out of the raw 2352-byte sectors (tools/extract_exe.py's reader)
    obj/<lib>/   .LIB members unpacked (tools/psyqlib.py)
    elf/<lib>/   ELF objects, via psyq-obj-parser (pcsx-redux; fetched by setup.sh)
    match.txt    every object placed in retail by tools/match_obj.py

The approach follows parasite-eve-2-decomp (CC0) — see CREDITS.md.
"""
import argparse
import re
import shutil
import struct
import subprocess
import sys
import zipfile
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import extract_exe  # noqa: E402  (Disc / open_disc: raw-sector ISO 9660 reader)
import psyqlib      # noqa: E402  (iter_modules: .LIB -> .OBJ)

SDK_DIR = ROOT / "sdk"
WORK = SDK_DIR / "work"
LIB_DIR = ROOT / "lib"
MANIFEST = ROOT / "config/psyq-objects.txt"
YAML = ROOT / "config/splat.slps01556.lsdde.yaml"
EXE = ROOT / "disk/SLPS_015.56"
PARSER = ROOT / "tools/psyq-obj-parser/psyq-obj-parser"
VRAM, HDR = 0x80010000, 0x800

ARCHIVE = "https://archive.org/download/ps1_sdks"
DISC_NAMES = {  # version -> the redump zip on archive.org, for the error message
    "3.0": "Programmer Tool - Runtime Library Version 3.0 (Japan) (En,Ja)_DTL-S2180_redump.zip",
    "3.3": "Programmer Tool - Runtime Library Version 3.3 (Japan)_DTL-S2190_redump.zip",
    "3.5": "Programmer Tool - Runtime Library Version 3.5 (Japan)_DTL-S2300_redump.zip",
    "3.6": "Programmer Tool - Runtime Library Version 3.6 (Japan)_DTL-S2310_redump.zip",
    "4.0": "Programmer Tool - Runtime Library Version 4.0 (Japan)_DTL-S2320_redump.zip",
    "4.1": "Programmer Tool - Runtime Library Version 4.1 (Japan)_DTL-S2330_redump.zip",
    "4.3": "Programmer Tool - Runtime Library Version 4.3 (Japan)_DTL-S2340_redump.zip",
    "4.4": "Programmer Tool - Runtime Library Version 4.4 (Japan)_DTL-S2350_redump.zip",
    "4.6": "Programmer Tool - Runtime Library Version 4.6 (Japan)_DTL-S2360_redump.zip",
}


def die(msg):
    print(f"psyq_sdk: {msg}", file=sys.stderr)
    sys.exit(1)


# --- discs -----------------------------------------------------------------

def discs():
    """{version: path} for every SDK disc dropped into sdk/ (zip, bin or iso)."""
    found = {}
    if not SDK_DIR.is_dir():
        return found
    for p in sorted(SDK_DIR.iterdir()):
        m = re.search(r"Version (\d\.\d)", p.name)
        if m and p.suffix.lower() in (".zip", ".bin", ".iso"):
            found.setdefault(m.group(1), p)
    return found


def data_track(ver, src):
    """The disc's data track as a file on disk (extracted from the zip once)."""
    if src.suffix.lower() != ".zip":
        return src
    out = WORK / ver / "track1.bin"
    if out.exists():
        return out
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src) as z:
        bins = [i for i in z.infolist() if i.filename.lower().endswith(".bin")]
        if not bins:
            die(f"{src.name}: no .bin track inside")
        track = [i for i in bins if "track 1" in i.filename.lower()] or [max(bins, key=lambda i: i.file_size)]
        print(f"  extracting {track[0].filename} ({track[0].file_size // 2**20} MB)")
        with z.open(track[0]) as f, open(out, "wb") as o:
            shutil.copyfileobj(f, o, 1 << 20)
    return out


def dir_entries(disc, lba, size):
    """(name, lba, size, is_dir) for one ISO 9660 directory."""
    data = disc.read(lba, size)
    out, i = [], 0
    while i < len(data):
        rec_len = data[i]
        if rec_len == 0:
            i = (i // 2048 + 1) * 2048
            continue
        rec = data[i:i + rec_len]
        name_len = rec[32]
        name = rec[33:33 + name_len]
        if name not in (b"\x00", b"\x01"):
            out.append((name.decode("ascii", "replace").split(";")[0],
                        struct.unpack("<I", rec[2:6])[0],
                        struct.unpack("<I", rec[10:14])[0],
                        bool(rec[25] & 2)))
        i += rec_len
    return out


def extract_tree(disc, lba, size, dest):
    dest.mkdir(parents=True, exist_ok=True)
    for name, elba, esize, is_dir in dir_entries(disc, lba, size):
        if is_dir:
            extract_tree(disc, elba, esize, dest / name)
        else:
            (dest / name).write_bytes(disc.read(elba, esize))


def extract_psx(ver, track):
    """PSX/LIB and PSX/INCLUDE out of the disc into sdk/work/<ver>/psx/."""
    dest = WORK / ver / "psx"
    if (dest / "LIB").is_dir() and any((dest / "LIB").glob("*.LIB")):
        return dest
    disc, _ = extract_exe.open_disc(str(track))
    pvd = disc.sector(16)
    root_lba = struct.unpack("<I", pvd[158:162])[0]
    root_size = struct.unpack("<I", pvd[166:170])[0]
    psx = [e for e in dir_entries(disc, root_lba, root_size) if e[0].upper() == "PSX" and e[3]]
    if not psx:
        die(f"{track}: no PSX/ directory on this disc -- is it a Runtime Library disc?")
    wanted = {"LIB", "INCLUDE"}
    for name, lba, size, is_dir in dir_entries(disc, psx[0][1], psx[0][2]):
        if is_dir and name.upper() in wanted:
            print(f"  reading PSX/{name}")
            extract_tree(disc, lba, size, dest / name.upper())
    return dest


def convert(ver, psx):
    """Every .LIB member and loose .OBJ -> sdk/work/<ver>/elf/<lib>/<module>.o."""
    elf = WORK / ver / "elf"
    if elf.is_dir() and any(elf.rglob("*.o")):
        return elf
    if not PARSER.exists():
        die(f"{PARSER.relative_to(ROOT)} missing -- run tools/setup.sh")
    ok = bad = 0
    fails = []
    for lib in sorted((psx / "LIB").glob("*.LIB")):
        libname = lib.stem.lower()
        objdir = WORK / ver / "obj" / libname
        objdir.mkdir(parents=True, exist_ok=True)
        (elf / libname).mkdir(parents=True, exist_ok=True)
        for name, _syms, obj in psyqlib.iter_modules(lib.read_bytes()):
            src = objdir / f"{name}.OBJ"
            src.write_bytes(obj)
            dst = elf / libname / f"{name.lower()}.o"
            r = subprocess.run([str(PARSER), str(src), "-o", str(dst)], capture_output=True, text=True)
            if r.returncode == 0 and dst.exists():
                ok += 1
            else:
                bad += 1
                fails.append(f"{libname}/{name.lower()}: {(r.stderr or r.stdout).strip().splitlines()[-1:]}")
                dst.unlink(missing_ok=True)
    for obj in sorted((psx / "LIB").glob("*.OBJ")):   # loose objects: 2MBYTE.OBJ, MALLOC.OBJ ...
        (elf / "_obj").mkdir(parents=True, exist_ok=True)
        dst = elf / "_obj" / f"{obj.stem.lower()}.o"
        r = subprocess.run([str(PARSER), str(obj), "-o", str(dst)], capture_output=True, text=True)
        ok, bad = (ok + 1, bad) if r.returncode == 0 else (ok, bad + 1)
    (WORK / ver / "convert-failures.txt").write_text("\n".join(fails) + "\n")
    print(f"  {ver}: {ok} objects converted, {bad} failed (sdk/work/{ver}/convert-failures.txt)")
    return elf


def prepare(ver, src):
    print(f"Psy-Q {ver}: {src.name}")
    return convert(ver, extract_psx(ver, data_track(ver, src)))


# --- manifest ----------------------------------------------------------------

def read_manifest():
    """[(version, 'lib/module', file_offset)] from config/psyq-objects.txt."""
    rows = []
    for line in MANIFEST.read_text().splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) != 3:
            die(f"{MANIFEST.name}: bad line {line!r} (want: <version> <lib>/<module> <fileoff>)")
        rows.append((parts[0], parts[1], int(parts[2], 0)))
    return rows


def verify_at(obj: Path, exe: bytes, off: int):
    """The object's masked .text is exactly what retail holds at file offset off."""
    from match_obj import masked_text
    data, mask = masked_text(obj)
    if data is None:
        return False
    win = exe[off:off + len(data)]
    return len(win) == len(data) and all((win[i] & mask[i]) == (data[i] & mask[i]) for i in range(len(data)))


# --- commands ----------------------------------------------------------------

def cmd_install(_args):
    rows = read_manifest()
    have = discs()
    need = sorted({v for v, _, _ in rows})
    missing = [v for v in need if v not in have]
    if missing:
        lines = [f"the manifest needs Psy-Q {', '.join(missing)} and sdk/ has "
                 f"{', '.join(sorted(have)) or 'no SDK disc'}.",
                 "Drop the redump zip(s) into sdk/ (do not unpack):"]
        for v in missing:
            lines.append(f"    {DISC_NAMES.get(v, 'Runtime Library Version ' + v)}")
        lines.append(f"  from {ARCHIVE}  -- see sdk/README.md")
        die("\n  ".join(lines))
    exe = EXE.read_bytes() if EXE.exists() else None
    for ver in need:
        prepare(ver, have[ver])
    installed = 0
    for ver, name, off in rows:
        src = WORK / ver / "elf" / f"{name}.o"
        if not src.exists():
            die(f"{name}.o did not come out of the Psy-Q {ver} disc (see sdk/work/{ver}/convert-failures.txt)")
        if exe is not None and not verify_at(src, exe, off):
            die(f"{name}.o from Psy-Q {ver} does NOT match retail at 0x{off:X}.\n"
                f"  Either the manifest offset is wrong or this disc is a different library build.")
        dst = LIB_DIR / f"{name}.o"
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
        installed += 1
    print(f"lib/: {installed} objects installed and verified against retail")


def cmd_match(args):
    from match_obj import masked_text, find_all
    exe = EXE.read_bytes()
    have = discs()
    vers = [args.version] if args.version else sorted(have)
    for ver in vers:
        if ver not in have:
            die(f"no Psy-Q {ver} disc in sdk/")
        elf = prepare(ver, have[ver])
        tally = defaultdict(lambda: [0, 0, 0])
        rows = []
        for o in sorted(elf.rglob("*.o")):
            data, mask = masked_text(o)
            if data is None:
                continue
            lib = o.parent.name
            tally[lib][2] += 1
            hits = list(find_all(exe, data, mask, 8))
            rel = o.relative_to(elf)
            if len(hits) == 1:
                tally[lib][0] += 1
                rows.append((hits[0], f"{rel}  text=0x{len(data):x}  fileoff=0x{hits[0]:x}  vram=0x{hits[0]-HDR+VRAM:08x}"))
            elif hits:
                tally[lib][1] += 1
                rows.append((hits[0], f"{rel}  text=0x{len(data):x}  AMBIGUOUS x{len(hits)}: " + " ".join(f"0x{h:x}" for h in hits[:6])))
        out = WORK / ver / "match.txt"
        out.write_text("\n".join(l for _, l in sorted(rows)) + "\n")
        print(f"\nPsy-Q {ver}: {sum(t[0] for t in tally.values())} objects placed exactly -> {out.relative_to(ROOT)}")
        print(f"  {'library':<10}{'exact':>6}{'ambig':>6}{'total':>6}")
        for lib in sorted(tally):
            m, a, t = tally[lib]
            print(f"  {lib:<10}{m:>6}{a:>6}{t:>6}")


def yaml_segments():
    segs = []
    for line in YAML.read_text().splitlines():
        m = re.match(r"\s+- \[\s*(0x[0-9A-Fa-f]+)\s*,\s*(asm|c|hasm|o)\s*,\s*([\w/]+)\s*\]", line)
        if m:
            segs.append((int(m.group(1), 16), m.group(2), m.group(3)))
    segs.sort()
    return segs


def cmd_coverage(_args):
    """Per psyq_* segment: how many of its functions a placed object owns."""
    import glob
    placed = {}
    for mfile in sorted(WORK.glob("*/match.txt")):
        ver = mfile.parent.name
        for line in mfile.read_text().splitlines():
            m = re.match(r"(\S+)\s+text=0x([0-9a-f]+)\s+fileoff=0x([0-9a-f]+)", line)
            if m:
                placed.setdefault(int(m.group(3), 16), (ver, m.group(1), int(m.group(2), 16)))
    if not placed:
        die("no sdk/work/*/match.txt -- run `psyq_sdk.py match` first")

    def owner(fileoff):
        for off, (ver, name, size) in placed.items():
            if off <= fileoff < off + size:
                return ver, name
        return None

    segs = yaml_segments()

    def seg_of(fileoff):
        cur = None
        for off, _kind, name in segs:
            if off <= fileoff:
                cur = name
            else:
                break
        return cur

    per = defaultdict(lambda: [0, 0])
    for f in glob.glob(str(ROOT / "asm/psyq_*.s")):
        seg = Path(f).stem
        cur = None
        for line in open(f):
            if line.startswith("glabel "):
                cur = line.split()[1]
            elif cur:
                m = re.match(r"\s*/\* ([0-9A-F]+) [0-9A-F]{8} ", line)
                if m:
                    per[seg][1] += 1
                    if owner(int(m.group(1), 16)):
                        per[seg][0] += 1
                    cur = None
    print("SDK functions still in asm psyq_* segments that a placed object already owns:")
    tot = [0, 0]
    for seg in sorted(per):
        c, t = per[seg]
        tot[0] += c
        tot[1] += t
        print(f"  {seg:<22}{c:>5}/{t}")
    print(f"  {'TOTAL':<22}{tot[0]:>5}/{tot[1]}")
    print("\nPlaced objects that fall inside GAME-code segments (SDK code miscounted as game):")
    for off, (ver, name, size) in sorted(placed.items()):
        s = seg_of(off)
        if s and not s.startswith("psyq_") and not s.startswith("lib"):
            print(f"  {name:<26} size=0x{size:<5x} vram=0x{off-HDR+VRAM:08x}  {s}  (Psy-Q {ver})")


def cmd_check(_args):
    rows = read_manifest()
    by_off = {off: (ver, name) for ver, name, off in rows}
    ok = True
    for off, kind, name in yaml_segments():
        if kind == "o":
            if off not in by_off:
                print(f"yaml `o` segment {name} at 0x{off:X} is not in {MANIFEST.name}")
                ok = False
            elif by_off[off][1] != name:
                print(f"0x{off:X}: yaml says {name}, manifest says {by_off[off][1]}")
                ok = False
    yaml_o = {off for off, kind, _ in yaml_segments() if kind == "o"}
    for ver, name, off in rows:
        if off not in yaml_o:
            print(f"manifest entry {name} at 0x{off:X} has no `o` segment in the yaml")
            ok = False
        if not (LIB_DIR / f"{name}.o").exists():
            print(f"lib/{name}.o missing -- run `psyq_sdk.py install`")
            ok = False
    print("OK: manifest, yaml and lib/ agree" if ok else "FAILED")
    sys.exit(0 if ok else 1)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd")
    sub.add_parser("install")
    m = sub.add_parser("match")
    m.add_argument("--version")
    sub.add_parser("coverage")
    sub.add_parser("check")
    args = ap.parse_args()
    {"install": cmd_install, "match": cmd_match, "coverage": cmd_coverage,
     "check": cmd_check, None: cmd_install}[args.cmd](args)


if __name__ == "__main__":
    main()
