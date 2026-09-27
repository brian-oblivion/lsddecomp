#!/usr/bin/env python3
"""Which game headers and units still re-declare a Sony name their own way.

    python3 tools/sonyheaders.py            # every collision, per file
    python3 tools/sonyheaders.py --check    # exit 1 if any
    python3 tools/sonyheaders.py --json

Game code takes Sony's types and prototypes from Sony's headers
(FINISHING-PLAN track 6, setup `sdk-headers`): `#include "common.h"`, then
<libgte.h>, <libgpu.h>, <libgs.h> and whichever of the others it calls.
A file that declares a Sony name with its own type (`GsIMAGE` as a local
struct, `PadInit` with another signature) cannot sit beside those headers:
cc1 stops with `conflicting types` or `redefinition`. This tool compiles each
game header, and each unit, after the whole Sony set and lists what collides.
Every hit is a Sony-type substitution for track 6 (or, for a unit-local
prototype, deleting it in favour of Sony's). So is spelling one of Sony's
untagged typedefs as a struct tag (`struct GsIMAGE *`), which compiles and
leaves the type incomplete. Warnings are not collisions:
passing a `u32 *` where Sony takes `u_long *` is fine (include/types.h).
"""
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
from typeviews import CPP, CC1, makefile_flags  # noqa: E402

SONY = ["libgte", "libgpu", "libgs", "libetc", "libcd", "libsnd", "libspu", "libpress"]
SKIP = {"include_asm.h", "types.h", "common.h", "gte.h"}
HIT = re.compile(r"^([^:]+):(\d+): (conflicting types for|redefinition of|redeclaration of) `([^']+)'")


def probe(text, name):
    pre = '#include "common.h"\n' + "".join(f"#include <{h}.h>\n" for h in SONY) + f'# 1 "{name}"\n'
    cpp = subprocess.run([str(CPP)] + makefile_flags("CPP_FLAGS"), input=pre + text,
                         capture_output=True, text=True, cwd=ROOT)
    cc1 = subprocess.run([str(CC1)] + makefile_flags("CC_FLAGS"), input=cpp.stdout,
                         capture_output=True, text=True, cwd=ROOT)
    return sorted({m.group(4) for line in cc1.stderr.splitlines()
                   if (m := HIT.match(line)) and m.group(1) == name})


def anonymous_typedefs():
    """Sony's typedefs of an UNTAGGED struct or union (`typedef struct {...}
    GsIMAGE;`). A game file that spells one as `struct GsIMAGE` names a tag
    Sony never defines: it compiles beside Sony's headers, but the type stays
    incomplete, so every accessor through it fails (round 95: Sprite.h's
    `struct GsIMAGE *image`, once TimImage.h took Sony's GsIMAGE)."""
    names = set()
    for h in (ROOT / "include" / "psyq").glob("*.h"):
        t = re.sub(r"/\*.*?\*/", " ", h.read_text(errors="replace"), flags=re.S)
        for m in re.finditer(r"\btypedef\s+(?:struct|union)\s*\{", t):
            depth, i = 1, m.end()
            while depth and i < len(t):
                depth += {"{": 1, "}": -1}.get(t[i], 0)
                i += 1
            n = re.match(r"\s*(\w+)\s*;", t[i:])
            if n:
                names.add(n.group(1))
    return names


def tag_uses(text, anon):
    code = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return sorted({f"struct {n}" for n in re.findall(r"\b(?:struct|union)\s+(\w+)", code) if n in anon})


def collect():
    out = {}
    anon = anonymous_typedefs()
    for h in sorted((ROOT / "include").glob("*.h")):
        if h.name not in SKIP:
            names = probe(f'#include "{h.name}"\n', f"include/{h.name}") + tag_uses(h.read_text(errors="replace"), anon)
            if names:
                out[f"include/{h.name}"] = names
    for c in sorted((ROOT / "src").rglob("*.c")):
        text = c.read_text(errors="replace")
        names = probe(text, c.relative_to(ROOT).as_posix()) + tag_uses(text, anon)
        if names:
            out[c.relative_to(ROOT).as_posix()] = names
    return out


def main():
    res = collect()
    if "--json" in sys.argv:
        print(json.dumps(res, indent=1))
    else:
        for f, names in res.items():
            print(f"{f}: {', '.join(names)}")
        print(f"{len(res)} file(s) re-declare a Sony name; a unit including one of those headers "
              "inherits its collision")
    return 1 if ("--check" in sys.argv and res) else 0


if __name__ == "__main__":
    sys.exit(main())
