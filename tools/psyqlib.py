#!/usr/bin/env python3
"""Unpack a Psy-Q `.LIB` archive into its member `.OBJ` (LNK) files.

Format, derived from the bytes (there is no public spec): a 4-byte magic
`LIB\\x01`, then module records back to back. Each record is

    8 bytes   module name, space padded (e.g. `PAD     `)
    4 bytes   date (ignored)
    4 bytes   offset from the START OF THIS RECORD to the LNK object data
    4 bytes   total record size, including the header and the object
    ...       exported symbol names, each a length byte plus the name,
              terminated by a zero length byte
    ...       the LNK object (`LNK\\x02...`), which psyq-obj-parser reads

Usage: psyqlib.py LIBGPU.LIB outdir/          -> outdir/<module>.OBJ per member
       psyqlib.py --list LIBGPU.LIB           -> names, sizes and exported symbols
"""
import struct
import sys
from pathlib import Path


def iter_modules(data: bytes):
    if data[:4] != b"LIB\x01":
        raise SystemExit(f"not a Psy-Q LIB archive (magic {data[:4]!r})")
    pos = 4
    while pos + 20 <= len(data):
        name = data[pos:pos + 8].rstrip(b" ").decode("ascii")
        _date, obj_off, size = struct.unpack_from("<III", data, pos + 8)
        syms = []
        p = pos + 20
        while True:
            n = data[p]
            p += 1
            if n == 0:
                break
            syms.append(data[p:p + n].decode("ascii", "replace"))
            p += n
        obj = data[pos + obj_off:pos + size]
        if obj[:3] != b"LNK":
            raise SystemExit(f"module {name}: expected LNK data at +{obj_off:#x}, got {obj[:4]!r}")
        yield name, syms, obj
        pos += size


def main(argv):
    if not argv or argv[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    listing = argv[0] == "--list"
    if listing:
        argv = argv[1:]
    lib = Path(argv[0])
    data = lib.read_bytes()
    if listing:
        for name, syms, obj in iter_modules(data):
            print(f"{name:<8} {len(obj):6d}  {' '.join(syms)}")
        return 0
    out = Path(argv[1])
    out.mkdir(parents=True, exist_ok=True)
    n = 0
    for name, _syms, obj in iter_modules(data):
        (out / f"{name}.OBJ").write_bytes(obj)
        n += 1
    print(f"{lib.name}: {n} modules -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
