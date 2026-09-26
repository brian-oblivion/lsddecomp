# StageMap__RefreshFootprint — MATCHED (34/34 words)

> Renamed from `Class866E8__RefreshFootprint` on 2026-09-26 (tools/rename.py). Address 0x8004c620.

> Renamed from `func_8004C620` on 2026-09-24 (tools/rename.py). Address 0x8004c620.

Dispatcher: gated by `self->unk1B8`, doubles `self->unk78` into an index,
calls `StageMap__SetFootprintCellFlag(self, 0)` unconditionally, then branches on
`self->unk68->unk4` between `StageMap__ComputeFootprintFromRotation` and `StageMap__SetFootprintFromQuery`, finishing
with `StageMap__SetFootprintCellFlag(self, 1)`.

## New struct knowledge (`include/class_3bb8c.h`)

Two new fields carved out of the previously-opaque `pad74[0xBC-0x74]`
padding in `struct Obj866E8`:

- `unk78` (`s16`, +0x078) — read here, doubled and passed on as an index.
- `unk7A` (`s16`, +0x07A) — read here, passed straight through as an arg.

`pad74` was split into `pad74[0x78-0x74]` + `unk78` + `unk7A` +
`pad7C[0xBC-0x7C]`; total padding covered is unchanged (still 0x48 bytes),
so `Obj866E8`'s size and every later field's offset are untouched.

`Obj866E8::unk68->unk4` (already-typed field of the existing `Unk68Struct`)
is read here too, no header change needed for that one.

## Final C

```c
/* Forward declarations: all three are defined later in this file (in ROM
 * order, after StageMap__RefreshFootprint), but StageMap__RefreshFootprint calls them before their
 * own definitions appear. Signatures are typed from the registers loaded
 * at each call site, per this unit's established convention for calling a
 * same-unit function whose body is still INCLUDE_ASM. */
extern void StageMap__SetFootprintCellFlag(Obj866E8 *self, s32 arg1);
extern void StageMap__SetFootprintFromQuery(Obj866E8 *self);
extern void StageMap__ComputeFootprintFromRotation(Obj866E8 *self, s32 arg1, s32 arg2);

void StageMap__RefreshFootprint(Obj866E8 *self) {
    s32 idx;

    if (self->unk1B8 == 0) {
        return;
    }
    idx = self->unk78 * 2;
    StageMap__SetFootprintCellFlag(self, 0);
    if (self->unk68->unk4 == 0) {
        StageMap__ComputeFootprintFromRotation(self, idx, self->unk7A);
    } else {
        StageMap__SetFootprintFromQuery(self);
    }
    StageMap__SetFootprintCellFlag(self, 1);
}
```

## Attempts

1. First attempt wrote the natural reading of the branch
   (`if (self->unk68->unk4 != 0) { StageMap__SetFootprintFromQuery(self); } else {
   StageMap__ComputeFootprintFromRotation(...); }`) — compiled, size matched, but 8/34 words
   differed: retail's branch is a `bnez` jumping FORWARD to the
   `StageMap__SetFootprintFromQuery` call (which sits as an out-of-line target after a `j`
   over it), with `StageMap__ComputeFootprintFromRotation` as the in-line fallthrough. My version
   produced the mirror image: a `beqz` with `StageMap__ComputeFootprintFromRotation` as the jump
   target and `StageMap__SetFootprintFromQuery` in-line.
2. Inverting the condition and swapping the two arms
   (`if (self->unk68->unk4 == 0) { StageMap__ComputeFootprintFromRotation(...); } else {
   StageMap__SetFootprintFromQuery(self); }`, semantically identical) reproduced retail's
   exact branch polarity and instruction layout — 34/34.

### Proposed learning

**For a two-way `if`/`else` with no other differences, GCC 2.6.3 can lay
out either arm as the fallthrough** — which arm becomes fallthrough vs.
out-of-line jump target is NOT determined purely by source order (writing
the "true" arm first does not guarantee it becomes the fallthrough). When
a residue is "correct instructions, but the branch polarity and arm
layout are mirrored" (a `bnez`/`beqz` swap plus the two call blocks
swapped), try inverting the condition and swapping the arm bodies before
looking for anything more exotic. Confirmed here: `if (!c) A else B`
picked the opposite layout from `if (c) B else A` for a byte-identical
body pair.

## Naming

**Tier B.** Vtable slot +0x128. Mechanics are solid (clear the grid-cell
flag over the current footprint, recompute it via one of two strategies
gated on `self->unk68->unk4`, set the flag again), but WHY the object's
grid footprint needs refreshing (what game event triggers it) is not
established from this unit alone. "Footprint" is not a guess -- it is
class_3ac78's own already-established vocabulary for the identical
mechanism on this same class (`StageMap__ApplyToSenderFootprint`,
`StageMap__SetFootprintRect`), confirmed by that unit's independent view
reaching the same `self->unk68->unk4` dispatch.
