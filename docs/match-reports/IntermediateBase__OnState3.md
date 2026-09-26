# IntermediateBase__OnState3 — MATCH (20/20 words)

> Renamed from `Obj86B60__NotifyChildReset` on 2026-09-25 (tools/rename.py). Address 0x8003e578.

> Renamed from `func_8003E578` on 2026-09-19 (tools/rename.py). Address 0x8003e578.

**Unit:** code_2cc8c_c · **Size:** 20 instructions

## What it does

`gIntermediateBaseMethods+0x068` (and `gTitleMenuMethods`'s own verbatim-inherited `+0x068`,
`IntermediateBase__SetState`'s `slot68` occupant): dereferences `self->unkC->unk0` (the
`Obj86B60InitArgs` field `IntermediateBase__Init`/`IntermediateBase__Deinit` only ever forward
opaquely, and the same field `IntermediateBase__OnState2` reaches independently through
the completely unrelated `Obj86B60UnkC->target` reading) as a real class
instance and dispatches its `slot4C`, then zeroes `self->unk1C`.

## The C

```c
void IntermediateBase__OnState3(Obj86B60 *self)
{
    Unk0ArgObj *obj0;

    obj0 = ((Obj86B60InitArgs *)self->unkC)->unk0;
    obj0->methods->slot4C(obj0);
    self->unk1C = 0;
}
```

Matched on the first build.

## Struct/table knowledge established

- New type `Unk0ArgObj`/`Unk0ArgObjMethods` (slot `slot4C`) --
  `Obj86B60InitArgs->unk0`'s real pointee type, same shape as `IntermediateBase__OnTag1Notify`
  round's `Unk4ArgObj` for the adjacent `unk4` field. Retyped
  `Obj86B60InitArgs.unk0` from `void *` to `Unk0ArgObj *`; the already-matched
  `IntermediateBase__Init`'s `methods->slot10(self, arg1->unk0)` call site is
  unaffected (implicit conversion to `void *`, same register, same bytes).

### Proposed learning

`Obj86B60InitArgs` now has THREE of its five fields (`unk0`, `unk4`, and by
extension the pattern likely extends to `unk8`/`unkC` too) independently
confirmed as real class-instance pointers rather than opaque children, each
discovered by a DIFFERENT function that happened to dereference it directly.
When a struct's fields are all first typed `void *` from an `addChild`-style
opaque-forwarding call site, expect later small dispatcher functions in the
same table region to upgrade them one at a time -- do not treat a `void *`
field in this codebase as evidence the field lacks a real type, only as
evidence no function *yet attempted* has dereferenced it.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**IntermediateBase__OnState3** (renamed from `func_8003E578`, round 55,
runner alpha). Tier B: `Obj86B60Methods::slot68` occupant (dispatched by
`IntermediateBase__SetState` on mode 3), mirroring `IntermediateBase__OnState2`'s
shape exactly but forwarding to `self->initArgs->unk0` instead --
`initArgs->unk0` is the SAME field `IntermediateBase__Init` registers as a child
via `addChild` (see `Obj86B60InitArgs`'s own header comment), which is why
"Child" rather than "Target" here; the notification's game-level meaning
remains unestablished (tier B).

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__NotifyChildReset. It occupies +0x068, which IntermediateBase__SetState runs on state 3: slot `onState3`, occupant named for it. It calls initArgs->unk0 at +0x04C and clears frameCounter; IntermediateBase__OnState2 does the same with +0x048 on the same object. Tier B.
