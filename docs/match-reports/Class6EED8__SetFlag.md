# Class6EED8__SetFlag -- MATCHED (3/3 words), round 82

> Renamed from `D8006EED8__SetFlag2C` on 2026-09-26 (tools/rename.py). Address 0x800423e4.

> Renamed from `func_800423E4` on 2026-09-25 (tools/rename.py). Address 0x800423e4.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gClass6EED8Methods slot +0x064 (a Class6D430-derived table, id 0xB03) (`tools/classtable.py`).
- **What:** sets the object's +0x02C word to 1.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/SceneNode.h` (untouched). The FrameClock and Class6EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gClass6EED8Methods slot +0x064. */
void Class6EED8__SetFlag(D_8006EED8Obj *self) {
    self->flag2C = 1;
}
```

## Naming

- `Class6EED8__SetFlag` -- tier A. Slot +0x064: sets flag2C = 1. A pure setter; the deeper game meaning of flag2C is not established (kept as flagNN rather than invented), but the setter's own mechanics ARE its purpose.

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `Class6EED8` in `include/Class6EED8.h`
(CLASS6D430_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`Class6D430Methods *`. The Source block above is the round-82 text; the live
body in `src/code_322b4.c` is byte-identical.

Renamed from `D8006EED8__SetFlag2C` with rename.py: the occupant of
Class6D430's +0x064 `setFlag`, named for its slot as `Class6D940__SetFlag`
is; the body does nothing beyond what the slot says. The field it sets is
`loaded` (see Class6EED8__Class6EED8's Track 4 paragraph for the evidence:
setFlag is the driver's completion callback).
