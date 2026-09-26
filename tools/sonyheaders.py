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
prototype, deleting it in favour of Sony's). Warnings are not collisions:
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


def collect():
    out = {}
    for h in sorted((ROOT / "include").glob("*.h")):
        if h.name not in SKIP:
            names = probe(f'#include "{h.name}"\n', f"include/{h.name}")
            if names:
                out[f"include/{h.name}"] = names
    for c in sorted((ROOT / "src").rglob("*.c")):
        names = probe(c.read_text(errors="replace"), c.relative_to(ROOT).as_posix())
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
