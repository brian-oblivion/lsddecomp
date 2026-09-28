# SceneNode__TransformAndNotifyParents -- MATCHED (32/32 words)

> Renamed from `Class6B5CC__TransformAndNotifyParents` on 2026-09-26 (tools/rename.py). Address 0x8001d624.

> Renamed from `func_8001D624` on 2026-09-18 (tools/rename.py). Address 0x8001d624.

Round 12, runner delta. `SceneNode`.

## Summary

```c
void SceneNode__TransformAndNotifyParents(SceneNodeObj *self, GenericCountList_d294 *a1, s32 a2) {
    ApplyMatrixToSVArray(&a1->unk4, &a1->unk4, a1->unk0 * 8, &self->unk14->unk24);
    self->unk28 = 0;
    self->unk2C = 0;
    self->unk30 = a1;
    self->methods->slot30(self, a2);
    self->unk30 = NULL;
}
```

Seeded with m2c (`--sig 'void SceneNode__TransformAndNotifyParents(SceneNodeObj *self, void *a1, s32 a2)'`),
which independently produced the same shape (a CSE'd `temp = a1 + 4;` used
for both `ApplyMatrixToSVArray` args, the two zero-stores, the stash-and-clear of
`unk30`, and the vtable dispatch) -- matched first try after typing m2c's
loose `void *`/`*a1` reads into real fields.

## New types/fields

- **`GenericCountList_d294`** (new local type, `include/SceneNode.h`): a
  third "just enough to dispatch" view, seen only through this call site.
  `unk0` (its own first field) is read once and multiplied by 8 to form
  `ApplyMatrixToSVArray`'s iteration count; `&unk4` (address only, never
  dereferenced here) is `ApplyMatrixToSVArray`'s src/dest base. Both `ApplyMatrixToSVArray`
  args are the SAME address -- confirmed from the disassembly
  (`addiu $a0,$s1,0x4` then `addu $a1,$a0,$zero`, not reasoned from any
  naming), which is exactly what let m2c CSE it into one `temp_a0`.
- **`SceneNodeMethods::slot30`** (+0x030, new field): `BasicClass__NotifyParents`,
  inherited verbatim into this class's own table (already established by the
  file banner's `--vs gBasicClassMethods` census) -- not decompiled here, since
  BasicClass belongs to a different, still-uncarved unit. Dispatched here as
  `(self, s32 arg1)`.
- **`SceneNodeSub14::unk24`** (renamed from `pad24`): only its address is
  taken (`&self->unk14->unk24`, forwarded to `ApplyMatrixToSVArray` as a
  write-destination base) -- stays an opaque byte span, just renamed now
  that something references it.
- **`SceneNodeObj::unk28`/`unk2C`** (new `s32` fields, both just zeroed
  here) and **`unk30`** (new `GenericCountList_d294 *` field): set to this
  call's own `a1` only for the duration of the `slot30` dispatch, then
  cleared again right after -- reads like a "currently processing" scratch
  slot, not a durable one. (`unk28` was already added by `SceneNode__DispatchLinkCommand`,
  matched earlier this round; this function only adds `unk2C`/`unk30`.)
- **`ApplyMatrixToSVArray`** (extern, `asm/SceneNode.s`, the next slice, still
  uncarved): declared `(void *src, void *dest, s32 count, void *out)`,
  typed only to this call site's own shape.

## Evidence

Disassembly (`asm/nonmatchings/SceneNode/SceneNode__TransformAndNotifyParents.s`).

### Proposed learning

None new -- this just extends the "opaque view typed only to the one call
site that needs it" convention already established repeatedly in this file
(`GenericObj_d294`, `UnkOwner_d294`) to a third shape.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D624` via `tools/rename.py`. **Tier B** -- slot
`+0x090` occupant. Transforms `a1`'s array through
`self->unk14->unk24` then stashes `a1` into `self->unk30` for exactly
one `self->methods->slot30` (`BasicClass__NotifyParents`, inherited)
dispatch. Mechanics measured from the disassembly; the in-game reason
for notifying parents with a freshly-transformed array is not
established. Purely local to this unit + its header.

## Track 6 (round 91, echo)

The argument type GenericCountList_d294 (`unk0`, `unk4`) is TmdModel.h's TmdHull (`count`, `v`): the buffer NotifyWithHull passes is TmdModel__GetHull's output. Byte-identical.

## Round 100 (delta): track 7

Parameters `a1`/`a2` -> `verts`/`event` (SceneNode.h's names). `* 8` ->
`* HULL_BOX_CORNERS` (new in TmdModel.h: a TmdHull box's eight corners).
`linkTarget = 0` -> `NULL`.

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Copies a1's own count*8 elements into self->unk14->unk24 (via
 * ApplyMatrixToSVArray, both its src and dest args are &a1->unk4 -- computed once,
 * copied, per the disassembly), zeroes unk28/unk2C, stashes a1 into unk30
 * for the duration of a single self->methods->slot30(self, a2) dispatch
 * (an inherited BasicClass slot, not this unit's own code), then clears
 * unk30 again. */
```
