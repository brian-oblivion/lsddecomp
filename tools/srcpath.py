#!/usr/bin/env python3
"""Where a unit's source, object and disassembly live.

WHY THIS IS A MODULE RATHER THAN A GLOB EVERY TOOL SPELLS ITSELF. `src/` is
flat today, and it will not stay flat -- the moment a `psyq/` or a subsystem
grouping appears (splat writes those via a segment's `dir:` key), every tool
that spelled the two operations below by hand goes wrong, and **they go wrong
in opposite directions**:

    ROOT / f"src/{unit}.c"        # resolver -- fails LOUDLY, path not found
    sorted(ROOT.glob("src/*.c"))  # enumerator -- fails SILENTLY, `*` does not
                                  # descend, so it just returns fewer units and
                                  # every count derived from it drops without
                                  # a word

The silent one is the dangerous one. A progress figure that quietly stops
counting a directory looks exactly like a progress figure. One definition here
is what keeps the twenty-first tool from re-inventing the broken version.

A UNIT NAME STAYS FLAT AND UNIQUE: `DreamSys`, never `psyq/DreamSys`. The
directory is a filesystem detail, not part of the identity, so tool CLIs, match
reports and `config/symbols.*.txt` comments keep working wherever splat puts
the file. `assert_unique()` is what keeps that promise honest.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
BUILD_SRC = ROOT / "build/src"
NONMATCHINGS = ROOT / "asm/nonmatchings"


def src_files():
    """Every C unit, in a stable order. Replaces sorted(glob("src/*.c"))."""
    return sorted(SRC.rglob("*.c"), key=lambda p: p.stem)


def units():
    """Every unit NAME, in a stable order."""
    return [p.stem for p in src_files()]


def unit_src(unit):
    """The .c file for a unit name, or None. Replaces f"src/{unit}.c"."""
    for p in sorted(SRC.rglob(f"{unit}.c")):
        return p
    return None


def unit_obj(unit):
    """The object the canonical build leaves for a unit, or None.

    The Makefile's pattern is `build/%.c.o: %.c`, so the object keeps the `.c`
    in its name -- build/src/DreamSys.c.o, not build/src/DreamSys.o.
    """
    p = unit_src(unit)
    if p is None:
        return None
    rel = p.relative_to(SRC)
    return BUILD_SRC / rel.parent / (rel.name + ".o")


def nm_dir(unit):
    """asm/nonmatchings/<...>/<unit>/. Returned whether or not it exists;
    callers glob it and tolerate empty."""
    if "/" in unit:
        raise ValueError(
            f"srcpath.nm_dir: {unit!r} is a path, not a unit name. An "
            "INCLUDE_ASM string carries the subdirectory, and passing it whole "
            "doubles the directory. The unit is its last component.")
    p = unit_src(unit)
    sub = "" if p is None else str(p.parent.relative_to(SRC)).strip(".")
    return NONMATCHINGS / sub / unit if sub else NONMATCHINGS / unit


def nm_find(fn):
    """Every asm/nonmatchings/**/<fn>.s for a function name.

    `*/{fn}.s` matches only the flat tree, and a tool that finds no `.s`
    concludes the function is MATCHED -- the silent-failure shape again.
    """
    return sorted(NONMATCHINGS.rglob(f"{fn}.s"))


def nm_all():
    """Every generated `.s`, at any depth."""
    return sorted(NONMATCHINGS.rglob("*.s"))


def assert_unique():
    """Two units with the same name in different directories would make
    unit_src() return an arbitrary one. Nothing prevents that but this."""
    seen = {}
    for p in SRC.rglob("*.c"):
        if p.stem in seen:
            raise SystemExit(
                f"srcpath: duplicate unit name {p.stem}: "
                f"{seen[p.stem]} and {p}")
        seen[p.stem] = p
    return len(seen)


if __name__ == "__main__":
    from collections import Counter
    n = assert_unique()
    counts = Counter(str(p.parent.relative_to(SRC)) for p in SRC.rglob("*.c"))
    print(f"{n} units, all names unique")
    for d, k in sorted(counts.items()):
        print(f"  src/{'' if d == '.' else d + '/'}  {k}")
