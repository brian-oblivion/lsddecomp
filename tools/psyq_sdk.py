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

R_MIPS_32, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 2, 5, 6, 7

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
# When several discs place an object at the same offset the bytes are identical
# and it does not matter which one the manifest names; prefer the disc whose
# library builds are closest to the game's (measured: 3.3 places the most).
PREFER = ["3.3", "3.5", "3.6", "3.0"]


def placed_objects():
    """{'lib/module': (version, text_fileoff, text_size)} over every disc's match.txt.

    A placement whose span lies strictly INSIDE another placement's span is a
    finer-grained module of the same bytes -- the 3.5/3.6 discs split several
    3.3 modules (libgte/mtx_00 -> mtx_003 + mtx_00b + ...; libc2/bcopy ->
    bcopy + bzero + memcmp; libsnd/ssplay -> playmode + ssplay + ssplayb) --
    or a false positive whose tiny body equals a piece of a real object
    (libgs/gs_125 inside libetc/vmode). Either way the coarse object is the
    one to link: its bytes tile the range exactly, the fine ones leave gaps or
    overlap. Such placements are dropped here, so `runs`, `coverage`,
    `symbols` and the bss plan all see one object per byte. Two objects with
    IDENTICAL spans (libc/a56 == libc2/exit) are both kept; `runs` shows them
    as `a|b` alternates. Every PARTIAL OVERLAP `runs` reported on 2026-09-11
    was this pattern and every one resolved to the 3.3 object."""
    out = {}
    versions = sorted((m.parent.name for m in WORK.glob("*/match.txt")),
                      key=lambda v: PREFER.index(v) if v in PREFER else 99)
    for ver in versions:
        for line in (WORK / ver / "match.txt").read_text().splitlines():
            m = re.match(r"(\S+)\.o\s+text=0x([0-9a-f]+)\s+fileoff=0x([0-9a-f]+)", line)
            if m:
                out.setdefault(m.group(1), (ver, int(m.group(3), 16), int(m.group(2), 16)))
    spans = [(off, off + size, name) for name, (_v, off, size) in out.items()]
    superseded = {n for a, b, n in spans
                  if any((a2 <= a and b <= b2) and (a2, b2) != (a, b) for a2, b2, _n2 in spans)}
    return {n: v for n, v in out.items() if n not in superseded}


def superseded_objects():
    """The placements placed_objects() dropped, as {'lib/module': 'container'}."""
    out = {}
    versions = sorted((m.parent.name for m in WORK.glob("*/match.txt")),
                      key=lambda v: PREFER.index(v) if v in PREFER else 99)
    for ver in versions:
        for line in (WORK / ver / "match.txt").read_text().splitlines():
            m = re.match(r"(\S+)\.o\s+text=0x([0-9a-f]+)\s+fileoff=0x([0-9a-f]+)", line)
            if m:
                out.setdefault(m.group(1), (ver, int(m.group(3), 16), int(m.group(2), 16)))
    kept = placed_objects()
    res = {}
    for name, (_v, off, size) in out.items():
        if name in kept:
            continue
        for k, (_kv, koff, ksize) in kept.items():
            if koff <= off and off + size <= koff + ksize:
                res[name] = k
                break
    return res

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
    from match_obj import masked_text
    spans = []
    for ver, name, off in rows:
        src = WORK / ver / "elf" / f"{name}.o"
        if src.exists():
            data, _ = masked_text(src)
            spans.append((off, off + len(data or b""), name))
    spans.sort()
    for (a0, a1, n0), (b0, b1, n1) in zip(spans, spans[1:]):
        if b0 < a1:
            die(f"manifest objects overlap in retail: {n0} 0x{a0:X}..0x{a1:X} and {n1} 0x{b0:X}..0x{b1:X}.\n"
                f"  Two objects of identical shape can both match one spot (a 4-instruction getter is a\n"
                f"  4-instruction getter); only one of them is really there. Check the segment's glabels.")
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
        spans = sorted((off, off + int(m.group(1), 16), l.split()[0]) for off, l in rows
                       for m in [re.search(r"text=0x([0-9a-f]+)", l)] if "fileoff" in l)
        for (a0, a1, n0), (b0, b1, n1) in zip(spans, spans[1:]):
            if b0 < a1:
                print(f"  OVERLAP: {n0} 0x{a0:x}..0x{a1:x} and {n1} 0x{b0:x}..0x{b1:x} -- at most one is real")
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


def cmd_place(args):
    """For each object, derive where retail put its DATA sections from the
    relocations in its (already placed) .text: a HI16/LO16 pair, a R_MIPS_32
    word or a GPREL16 against a symbol in section S, read out of retail at the
    placed offset, gives S's retail address once the in-place addend is
    removed. Data sections are also confirmed by a masked byte search. Prints
    the yaml lines to add."""
    import struct as st
    from elftools.elf.elffile import ELFFile
    from match_obj import masked_text, find_all
    exe = EXE.read_bytes()
    gp = 0x8008A808
    placed = {k: (v[0], v[1]) for k, v in placed_objects().items()}
    for name in args.objects:
        if name not in placed:
            print(f"# {name}: not placed by any disc (run `match`)")
            continue
        ver, toff = placed[name]
        if args.version:
            ver = args.version
        opath = WORK / ver / "elf" / f"{name}.o"
        print(f"# {name}  (Psy-Q {ver})  .text at 0x{toff:X} / vram 0x{toff-HDR+VRAM:08X}")
        with open(opath, "rb") as f:
            elf = ELFFile(f)
            secs = {i: s for i, s in enumerate(elf.iter_sections())}
            symtab = elf.get_section_by_name(".symtab")
            syms = list(symtab.iter_symbols())
            text = elf.get_section_by_name(".text").data()
            rel = elf.get_section_by_name(".rel.text")
            # section index -> list of derived retail base addresses
            derived = defaultdict(list)
            pending_hi = None
            for r in (rel.iter_relocations() if rel else []):
                off, typ, si = r["r_offset"], r["r_info_type"], r["r_info_sym"]
                sym = syms[si]
                shndx = sym["st_shndx"]
                if shndx in ("SHN_UNDEF", "SHN_ABS", "SHN_COMMON"):
                    pending_hi = None
                    continue
                obj_word = st.unpack_from("<I", text, off)[0]
                ret_word = st.unpack_from("<I", exe, toff + off)[0]
                if typ == R_MIPS_HI16:
                    pending_hi = (shndx, sym["st_value"], obj_word & 0xFFFF, ret_word & 0xFFFF)
                    continue
                if typ == R_MIPS_LO16 and pending_hi and pending_hi[0] == shndx:
                    _, sval, ohi, rhi = pending_hi
                    olo = st.unpack_from("<h", text, off)[0]
                    rlo = st.unpack_from("<h", exe, toff + off)[0]
                    inplace = (ohi << 16) + olo
                    retail = (rhi << 16) + rlo
                    derived[shndx].append(retail - inplace - sval)
                    pending_hi = None
                elif typ == R_MIPS_32:
                    derived[shndx].append(ret_word - obj_word - sym["st_value"])
                elif typ == R_MIPS_GPREL16:
                    olo = st.unpack_from("<h", text, off)[0]
                    rlo = st.unpack_from("<h", exe, toff + off)[0]
                    derived[shndx].append((gp + rlo) - olo - sym["st_value"])
                else:
                    pending_hi = None
            for idx, sec in secs.items():
                nm = sec.name
                if nm in (".text", "") or nm.startswith((".rel", ".sym", ".str", ".shstr", ".note")):
                    continue
                size = sec["sh_size"]
                bases = sorted(set(derived.get(idx, [])))
                bytes_hit = ""
                if sec["sh_type"] != "SHT_NOBITS" and size >= 4:
                    data = sec.data()
                    mask = bytes(b"\xff" * len(data))
                    hits = list(find_all(exe, data, mask, min(8, len(data))))
                    bytes_hit = " bytes@" + ",".join(f"0x{h:X}" for h in hits[:4]) if hits else " bytes:not-found"
                if len(bases) == 1:
                    b = bases[0]
                    fo = b - VRAM + HDR
                    print(f"      - [0x{fo:X}, o, {name}, {nm}]   # vram 0x{b:08X} size 0x{size:X}{bytes_hit}")
                elif bases:
                    print(f"# {nm}: relocations DISAGREE {[hex(b) for b in bases]} size 0x{size:X}{bytes_hit}")
                else:
                    print(f"# {nm}: no relocation names it -- size 0x{size:X}{bytes_hit}")


def reloc_addresses(exe, opath, toff, gp=0x8008A808):
    """Yield (symbol_name, retail_address) for every reference the placed object
    makes to a NAMED symbol (defined here or undefined), read out of retail."""
    import struct as st
    from elftools.elf.elffile import ELFFile
    with open(opath, "rb") as f:
        elf = ELFFile(f)
        syms = list(elf.get_section_by_name(".symtab").iter_symbols())
        text = elf.get_section_by_name(".text").data()
        rel = elf.get_section_by_name(".rel.text")
        if rel is None:
            return
        pending = None
        for r in rel.iter_relocations():
            off, typ, si = r["r_offset"], r["r_info_type"], r["r_info_sym"]
            sym = syms[si]
            name = sym.name
            if not name or sym["st_info"]["type"] == "STT_SECTION":
                pending = None
                continue
            oword = st.unpack_from("<I", text, off)[0]
            rword = st.unpack_from("<I", exe, toff + off)[0]
            if typ == R_MIPS_HI16:
                pending = (name, oword & 0xFFFF, rword & 0xFFFF)
            elif typ == R_MIPS_LO16 and pending and pending[0] == name:
                _, ohi, rhi = pending
                olo = st.unpack_from("<h", text, off)[0]
                rlo = st.unpack_from("<h", exe, toff + off)[0]
                yield name, ((rhi << 16) + rlo) - ((ohi << 16) + olo)
                pending = None
            elif typ == R_MIPS_32:
                yield name, rword - oword
            elif typ == R_MIPS_GPREL16:
                olo = st.unpack_from("<h", text, off)[0]
                rlo = st.unpack_from("<h", exe, toff + off)[0]
                yield name, (gp + rlo) - olo
            elif typ == 4:  # R_MIPS_26: target = (pc & 0xF0000000) | (imm << 2)
                pc = toff - HDR + VRAM + off
                yield name, ((pc & 0xF0000000) | ((rword & 0x03FFFFFF) << 2)) - ((oword & 0x03FFFFFF) << 2)
            else:
                pending = None


def cmd_symbols(_args):
    """Retail address of every Psy-Q symbol the placed objects reference, in
    symbols-file syntax. Two sources: an object's own exports (text symbols
    at .text base + st_value) and relocations from placed text to named
    symbols (which is how bss/sbss variables and other objects' globals get
    their addresses). Conflicts are reported, never silently resolved."""
    from elftools.elf.elffile import ELFFile
    exe = EXE.read_bytes()
    addrs = defaultdict(set)   # name -> {addr}
    origin = defaultdict(set)
    for name, (ver, toff, _size) in placed_objects().items():
        opath = WORK / ver / "elf" / f"{name}.o"
        with open(opath, "rb") as f:
            elf = ELFFile(f)
            text_idx = [i for i, s in enumerate(elf.iter_sections()) if s.name == ".text"][0]
            for sym in elf.get_section_by_name(".symtab").iter_symbols():
                if sym["st_shndx"] == text_idx and sym.name and sym["st_info"]["type"] != "STT_SECTION":
                    addrs[sym.name].add(toff - HDR + VRAM + sym["st_value"])
                    origin[sym.name].add(f"{name}:def")
        for sname, addr in reloc_addresses(exe, opath, toff):
            addrs[sname].add(addr)
            origin[sname].add(f"{name}:ref")
    conflicts = 0
    for sname in sorted(addrs, key=lambda n: min(addrs[n])):
        a = sorted(addrs[sname])
        if len(a) == 1:
            print(f"{sname} = 0x{a[0]:08X}; // {' '.join(sorted(origin[sname]))[:80]}")
        else:
            conflicts += 1
            print(f"// CONFLICT {sname}: {[hex(x) for x in a]}  ({' '.join(sorted(origin[sname]))[:100]})")
    print(f"// {len(addrs)} symbols, {conflicts} conflicts", file=sys.stderr)


LDFRAG = ROOT / "config/psyq-objects.ld"


def bss_plan(objects):
    """For each (name, path, text_fileoff): where its NOBITS sections must sit and
    which of its symbols get pinned. Returns (sections, pins, notes).

    Sony's linker allocated uninitialised variables individually, across
    objects, so an object's .bss is NOT contiguous in retail (libetc/pad:
    pad_buf at 0x8008B3C8, PadIdentifier at 0x8008E984). Two rules therefore:
    a NOBITS section is placed where its SECTION-RELATIVE references demand
    (or at its first symbol's retail address, or in a spare NOLOAD area if
    nothing references it); and EVERY named symbol it defines is pinned to its
    retail address by a linker-script assignment, which ld lets override the
    object's own definition (verified for WEAK and GLOBAL alike).
    """
    import struct as st
    from elftools.elf.elffile import ELFFile
    exe = EXE.read_bytes()
    gp = 0x8008A808
    # Retail addresses of named symbols, from every placed object's references.
    # The discovery corpus (sdk/work, every object every disc placed) knows far
    # more than the handful of objects in lib/, so use it when it is present.
    known = defaultdict(set)
    corpus = [(n, WORK / v / "elf" / f"{n}.o", off) for n, (v, off, _sz) in placed_objects().items()]
    for name, path, toff in (corpus or objects):
        for sname, addr in reloc_addresses(exe, path, toff, gp):
            known[sname].add(addr)
    sections, pins, notes = [], [], []
    # Externals: a linked object calls SDK (or game) symbols nothing in the link
    # defines yet -- InterruptCallback, printf, putchar ... Pin each to the
    # address RETAIL'S OWN CODE uses, read from the referencing object's
    # relocations against the executable. A wrong pin cannot pass silently:
    # it changes a jal/lui/addiu and the whole-image SHA1 fails.
    defined = set()
    undefined = defaultdict(set)          # name -> {retail addr} from OUR objects' refs
    for name, path, toff in objects:
        with open(path, "rb") as f:
            elf = ELFFile(f)
            for sym in elf.get_section_by_name(".symtab").iter_symbols():
                if sym.name and sym["st_shndx"] != "SHN_UNDEF" and sym["st_info"]["type"] != "STT_SECTION":
                    defined.add(sym.name)
        for sname, addr in reloc_addresses(exe, path, toff, gp):
            undefined[sname].add(addr)
    externs = []
    for sname in sorted(undefined):
        if sname in defined:
            continue
        addrs = undefined[sname]
        if len(addrs) == 1:
            externs.append((sname, next(iter(addrs))))
        else:
            notes.append(f"extern {sname} referenced at several addresses {[hex(a) for a in addrs]}; not pinned, link will fail")
    spare = 0x80100000   # well past the game's bss; only for sections nothing addresses
    for name, path, toff in objects:
        with open(path, "rb") as f:
            elf = ELFFile(f)
            secs = list(elf.iter_sections())
            syms = list(elf.get_section_by_name(".symtab").iter_symbols())
            text = elf.get_section_by_name(".text").data()
            rel = elf.get_section_by_name(".rel.text")
            derived = defaultdict(set)
            pending = None
            for r in (rel.iter_relocations() if rel else []):
                off, typ, si = r["r_offset"], r["r_info_type"], r["r_info_sym"]
                sym = syms[si]
                if sym["st_info"]["type"] != "STT_SECTION":
                    pending = None
                    continue
                shndx = sym["st_shndx"]
                oword = st.unpack_from("<I", text, off)[0]
                rword = st.unpack_from("<I", exe, toff + off)[0]
                if typ == R_MIPS_HI16:
                    pending = (shndx, oword & 0xFFFF, rword & 0xFFFF)
                elif typ == R_MIPS_LO16 and pending and pending[0] == shndx:
                    olo = st.unpack_from("<h", text, off)[0]
                    rlo = st.unpack_from("<h", exe, toff + off)[0]
                    derived[shndx].add(((pending[2] << 16) + rlo) - ((pending[1] << 16) + olo))
                    pending = None
                elif typ == R_MIPS_32:
                    derived[shndx].add(rword - oword)
                elif typ == R_MIPS_GPREL16:
                    olo = st.unpack_from("<h", text, off)[0]
                    rlo = st.unpack_from("<h", exe, toff + off)[0]
                    derived[shndx].add((gp + rlo) - olo)
                else:
                    pending = None
            for idx, sec in enumerate(secs):
                if sec["sh_type"] != "SHT_NOBITS" or sec["sh_size"] == 0:
                    continue
                members = [(sym["st_value"], sym.name) for sym in syms
                           if sym["st_shndx"] == idx and sym.name and sym["st_info"]["type"] != "STT_SECTION"]
                bases = sorted(derived.get(idx, set()))
                if len(bases) > 1:
                    notes.append(f"{name} {sec.name}: section-relative references disagree {[hex(b) for b in bases]} -- object needs editing (parasite-eve-2 hit this too)")
                if bases:
                    base, how = bases[0], "section-relative refs"
                else:
                    anchored = [(v, n) for v, n in members if len(known.get(n, ())) == 1]
                    if anchored:
                        v, n = sorted(anchored)[0]
                        base, how = next(iter(known[n])) - v, f"first symbol {n}"
                    else:
                        base, how = spare, "unreferenced; spare NOLOAD area"
                        spare += (sec["sh_size"] + 15) & ~15
                sections.append((name, sec.name, base, sec["sh_size"], how))
                for v, n in sorted(members):
                    addrs = known.get(n, set())
                    if len(addrs) == 1:
                        pins.append((n, next(iter(addrs)), name))
                    elif len(addrs) > 1:
                        notes.append(f"{name}: {n} referenced at several addresses {[hex(a) for a in addrs]}; not pinned")
                    elif base != spare:
                        pass  # falls where the section puts it; nothing references it by name
    return sections, pins, externs, notes


def cmd_ldfrag(args):
    """Write config/psyq-objects.ld: NOLOAD placement of every lib/ object's
    bss/sbss section and a pin for every bss symbol. Passed to ld BEFORE the
    splat script (Makefile), so the sections are claimed before /DISCARD/."""
    rows = read_manifest()
    objects = []
    for ver, name, off in rows:
        path = LIB_DIR / f"{name}.o"
        if not path.exists():
            die(f"lib/{name}.o missing -- run `psyq_sdk.py install`")
        objects.append((name, path, off))
    sections, pins, externs, notes = bss_plan(objects)
    out = ["/* GENERATED by `tools/psyq_sdk.py ldfrag` from config/psyq-objects.txt and",
           " * the objects in lib/ -- do not edit; `psyq_sdk.py check` verifies it is current.",
           " *",
           " * Sony's linker allocated uninitialised library variables one by one across",
           " * objects, so a Psy-Q object's .bss is not contiguous in retail. Each NOBITS",
           " * section is placed where its section-relative references demand, and every",
           " * variable is pinned to its retail address (a script assignment overrides the",
           " * object's own definition). NOLOAD: none of this is in the executable image. */",
           "SECTIONS {"]
    for name, sec, base, size, how in sections:
        tag = f".psyq_bss.{name.replace('/', '_')}{sec.replace('.', '_')}"
        out.append(f"    {tag} 0x{base:08X} (NOLOAD) : {{ build/lib/{name}.o({sec}) }}   /* 0x{size:X} bytes; {how} */")
    out.append("}")
    for n, a, name in sorted(pins, key=lambda p: p[1]):
        out.append(f"{n} = 0x{a:08X};   /* {name} */")
    out.append("/* externals the linked objects reference and nothing in the link defines yet;")
    out.append(" * each is the address retail's own code calls. Once the defining object is")
    out.append(" * linked the pin still holds (a script assignment wins) and must agree. */")
    for n, a in sorted(externs, key=lambda e: e[1]):
        out.append(f"{n} = 0x{a:08X};")
    for note in notes:
        out.append(f"/* NOTE: {note} */")
    text = "\n".join(out) + "\n"
    if args.check:
        if not LDFRAG.exists() or LDFRAG.read_text() != text:
            print(f"{LDFRAG.relative_to(ROOT)} is out of date -- run `tools/psyq_sdk.py ldfrag`")
            sys.exit(1)
        print(f"OK: {LDFRAG.relative_to(ROOT)} is current")
        return
    LDFRAG.write_text(text)
    print(f"wrote {LDFRAG.relative_to(ROOT)}: {len(sections)} NOLOAD sections, {len(pins)} pinned bss symbols, "
          f"{len(externs)} pinned externals, {len(notes)} notes")
    for note in notes:
        print("  NOTE:", note)


def cmd_runs(_args):
    """The conversion work queue: for every segment that still holds placed
    objects, the contiguous RUNS of objects (one run = one conversion step),
    with data/bss flags and overlapping (false-positive) placements marked."""
    from elftools.elf.elffile import ELFFile
    segs = yaml_segments()
    placed = placed_objects()
    # Objects with IDENTICAL spans have identical bytes (libc/a56 == libc2/exit):
    # either links; show them as one entry "a|b" and let the converter pick.
    by_span = defaultdict(list)
    for name, (ver, off, size) in placed.items():
        by_span[(off, off + size)].append((name, ver))
    objs = sorted((a, b, "|".join(n for n, _ in sorted(v)), v[0][1]) for (a, b), v in by_span.items())

    def secs(name, ver):
        name = name.split("|")[0]
        with open(WORK / ver / "elf" / f"{name}.o", "rb") as f:
            return {sec.name for sec in ELFFile(f).iter_sections()
                    if sec.name in (".data", ".rdata", ".sdata", ".bss", ".sbss")}

    total_runs = total_objs = 0
    for i, (off, kind, sname) in enumerate(segs):
        if kind == "o":
            continue
        end = segs[i + 1][0] if i + 1 < len(segs) else 0x7B800
        inside = [o for o in objs if o[0] >= off and o[0] < end]
        if not inside:
            continue
        where = "SDK asm" if sname.startswith("psyq_") else f"GAME unit ({kind})"
        print(f"\n{sname}  0x{off:X}..0x{end:X}  [{where}]  {len(inside)} placed objects")
        run = []
        runs = []
        for o in inside:
            if run and o[0] > run[-1][1]:      # a gap; an overlap (o[0] < end) stays in the run and is flagged
                runs.append(run)
                run = []
            run.append(o)
        runs.append(run)
        for r in runs:
            flags = []
            for a, b in zip(r, r[1:]):
                if b[0] < a[1]:
                    flags.append(f"PARTIAL OVERLAP {a[2]} vs {b[2]} -- a false positive; the segment's glabels decide")
            secflags = set()
            for o in r:
                secflags |= secs(o[2], o[3])
            tag = " ".join(sorted(secflags)) or "text-only"
            names = " ".join(o[2] for o in r)
            print(f"  run 0x{r[0][0]:X}..0x{r[-1][1]:X}  {len(r):2d} obj  [{tag}]  {names[:110]}{'...' if len(names) > 110 else ''}")
            for fl in flags:
                print(f"      {fl}")
            total_runs += 1
            total_objs += len(r)
    print(f"\nTOTAL: {total_objs} objects in {total_runs} runs")
    sup = superseded_objects()
    if sup:
        print(f"\n{len(sup)} placements dropped as finer-grained modules (or false positives) of a coarser object -- link the coarse one:")
        by = defaultdict(list)
        for fine, coarse in sorted(sup.items()):
            by[coarse].append(fine)
        for coarse in sorted(by):
            print(f"  {coarse:18s} supersedes {' '.join(by[coarse])}")


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
    if ok and any(WORK.glob("*/match.txt")):
        class A: check = True
        cmd_ldfrag(A())
    print("OK: manifest, yaml and lib/ agree" if ok else "FAILED")
    sys.exit(0 if ok else 1)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd")
    sub.add_parser("install")
    m = sub.add_parser("match")
    m.add_argument("--version")
    sub.add_parser("coverage")
    sub.add_parser("runs", help="the conversion work queue: contiguous runs of placed objects per segment")
    sub.add_parser("check")
    lf = sub.add_parser("ldfrag", help="write config/psyq-objects.ld (bss placement + symbol pins)")
    lf.add_argument("--check", action="store_true")
    sub.add_parser("symbols", help="retail address of every Psy-Q symbol the placed objects define or reference")
    pl = sub.add_parser("place", help="derive yaml lines for an object's data sections")
    pl.add_argument("objects", nargs="+", help="lib/module, e.g. libetc/intr_dma")
    pl.add_argument("--version")
    args = ap.parse_args()
    {"install": cmd_install, "match": cmd_match, "coverage": cmd_coverage,
     "check": cmd_check, "place": cmd_place, "symbols": cmd_symbols, "ldfrag": cmd_ldfrag, "runs": cmd_runs, None: cmd_install}[args.cmd](args)


if __name__ == "__main__":
    main()
