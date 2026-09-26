# ObjM__Dtor -- MATCH

> Renamed from `func_80052CD8` on 2026-09-24 (tools/rename.py). Address 0x80052cd8.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py ObjM__Dtor`: 14/14 words match.

This is vtable slot `+0x00C` (`dtor`) of `D_80087034` -- a SECOND class
table this unit's tail overlaps (`tools/classtable.py D_80087034`, 53
slots, header word `0x0002F230`, distinct from `gClass86F88Methods` which covers
this unit's earlier functions). `ObjM__ObjM` (this unit, still
`INCLUDE_ASM`) is that same table's `ctor` (`+0x008`).

## Source

```c
void ObjM__Dtor(Obj865C8 *self)
{
    GetClass86668Methods()->dtor(self);
}
```

## Notes

This class's `dtor` override forwards straight to the shared base class
dtor (`GetClass86668Methods()->dtor(self)`, `Class86668Methods::dtor`,
`include/class_39e08.h`), the same pattern already documented there for
`Class86668__Finalize` (`class_39e08`'s own sibling override). `Obj865C8` and
`GetClass86668Methods` are both already declared in `include/class_39e08.h`, so
this function needed no new struct at all -- `self` is typed directly as
the base class's own object type rather than inventing a `Class87034`
wrapper, since nothing here reads any field specific to this unit's own
class. `#include "class_39e08.h"` added to this unit's includes for this
declaration (and `Obj865C8`/`GetClass86668Methods` used by nothing else in this
unit). Matched first attempt.

## Naming

Round 75 (bravo, track 3). `func_80052CD8` -> `ObjM__Dtor`, **tier A**.

Slot +0x00C of D_80087034 (`tools/classtable.py 0x80087034`). Forwards to the base's dtor (GetClass86668Methods()->dtor). Named after the base family's own +0x00C names (Class86668__Dtor, Class865C8__Finalize; the first renamed `Class86668__Finalize` in round 84).
