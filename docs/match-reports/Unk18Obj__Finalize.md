# Unk18Obj__Finalize — MATCH (41/41 words)

> Renamed from `func_8003E6CC` on 2026-09-19 (tools/rename.py). Address 0x8003e6cc.

**Unit:** code_2cc8c_c · **Size:** 41 instructions

## What it does

`D_8006E8E4+0x00C` -- `Unk18Obj`'s dtor, the teardown counterpart to
`Unk18Obj__Unk18Obj`'s ctor: dispatches `slot90`, `slot74`, releases `self->unkAC`
(inherited BasicClass "release", corroborating and extending
`Unk18Obj__Unk18Obj`'s earlier discovery of that field), dispatches `slotA8`
with a literal `0`, then runs `Get_vtable_BasicClass()->slot0C` (BasicClass's own
`finalize`, `BasicClass__func_17f2c`).

## The C

```c
void Unk18Obj__Finalize(Unk18Obj *self)
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
same convention as `Obj86B60__Deinit`.

## Struct/table knowledge established

- `Unk18ObjMethods`: added `slot74`, `slot90`, `slotA8` (all `(Unk18Obj
  *self)` or `(Unk18Obj *self, s32 a1)`).
- `Unk18Obj->unkAC`: retyped from `void *` (as `Unk18Obj__Unk18Obj`'s report
  left it, "never dereferenced by this unit") to `Unk18AcObj *` -- this
  function is the dereferencing counter-evidence. New type
  `Unk18AcObj`/`Unk18AcObjMethods` models the one slot (`slot4`, inherited
  BasicClass release) this function reaches. `New_Class6B5CC`'s own extern
  declaration retyped to match (`include/code_d294.h`'s own view,
  `Class6B5CCObj *`, is a separate header and unaffected).
- `BasicClassMethodsCC8C`: added `slot0C` (`BasicClass__func_17f2c`,
  "finalize").

### Proposed learning

Confirms `Unk18Obj__Unk18Obj.md`'s own note as a general pattern in this unit:
a ctor/dtor PAIR occupying adjacent table slots (`+0x008`/`+0x00C` here,
same as `Obj86B60__Init`/`Obj86B60__Deinit`'s `+0x044`/`+0x048`) is worth
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

## Naming

**Unk18Obj__Finalize** (renamed from `func_8003E6CC`, round 55, runner
alpha). Tier A: calls its own teardown steps (`slot90`, `slot74`, releases
`self->unkAC` via `Unk18AcObjMethods::release` (renamed from
`slot4`, exclusive to this unit, tier A), `slotA8(self, 0)`) and THEN forwards to
`Get_vtable_BasicClass()->finalize` (that base slot's own confirmed name,
matching `include/code_8220.h`'s canonical `BasicClassMethods::finalize`
at the identical offset `+0x00C`) -- the standard "derived finalize does
its own cleanup, then calls the base finalize" idiom, which is what
licenses the name despite `slot90`/`slot74`/`slotA8`'s own occupants being
unidentified (left as `slotNN`, insufficient evidence for a name, tier
C/unnamed).
