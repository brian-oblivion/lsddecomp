# func_8003E6CC — MATCH (41/41 words)

**Unit:** code_2cc8c_c · **Size:** 41 instructions

## What it does

`D_8006E8E4+0x00C` -- `Unk18Obj`'s dtor, the teardown counterpart to
`func_8003E628`'s ctor: dispatches `slot90`, `slot74`, releases `self->unkAC`
(inherited BasicClass "release", corroborating and extending
`func_8003E628`'s earlier discovery of that field), dispatches `slotA8`
with a literal `0`, then runs `Get_vtable_BasicClass()->slot0C` (BasicClass's own
`finalize`, `BasicClass__func_17f2c`).

## The C

```c
void func_8003E6CC(Unk18Obj *self)
{
    self->methods->slot90(self);
    self->methods->slot74(self);
    self->unkAC->methods->slot4(self->unkAC);
    self->methods->slotA8(self, 0);
    Get_vtable_BasicClass()->slot0C(self);
}
```

Matched on the first build. Retail reloads `self->methods` fresh at each of
the three dispatch sites (no cached local) -- matched by not introducing one,
same convention as `func_8003E280`.

## Struct/table knowledge established

- `Unk18ObjMethods`: added `slot74`, `slot90`, `slotA8` (all `(Unk18Obj
  *self)` or `(Unk18Obj *self, s32 a1)`).
- `Unk18Obj->unkAC`: retyped from `void *` (as `func_8003E628`'s report
  left it, "never dereferenced by this unit") to `Unk18AcObj *` -- this
  function is the dereferencing counter-evidence. New type
  `Unk18AcObj`/`Unk18AcObjMethods` models the one slot (`slot4`, inherited
  BasicClass release) this function reaches. `func_8001CA94`'s own extern
  declaration retyped to match (`include/code_d294.h`'s own view,
  `Class6B5CCObj *`, is a separate header and unaffected).
- `BasicClassMethodsCC8C`: added `slot0C` (`BasicClass__func_17f2c`,
  "finalize").

### Proposed learning

Confirms `func_8003E628.md`'s own note as a general pattern in this unit:
a ctor/dtor PAIR occupying adjacent table slots (`+0x008`/`+0x00C` here,
same as `func_8003E10C`/`func_8003E280`'s `+0x044`/`+0x048`) is worth
reading together even when queued far apart in ROM order -- the dtor is
frequently the first place a field the ctor left generically typed gets
dereferenced, correcting an earlier "never dereferenced" note rather than
contradicting it. When a match report says a field is "never dereferenced
by this unit", read that as "not yet, by the functions attempted so far",
not as a permanent property.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.
