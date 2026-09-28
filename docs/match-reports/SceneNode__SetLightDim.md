# SceneNode__SetLightDim -- MATCHED (11/11 words)

> Renamed from `SceneNode__GetSetUnk10Field0` on 2026-09-26 (tools/rename.py). Address 0x8001d424.

> Renamed from `Class6B5CC__GetSetUnk10Field0` on 2026-09-26 (tools/rename.py). Address 0x8001d424.

> Renamed from `func_8001D424` on 2026-09-18 (tools/rename.py). Address 0x8001d424.

Round 12, runner delta. `code_d294`.

## Summary

Sibling of the five `self->unk10` bitfield accessors already matched in
`code_d294.c` (`SceneNode__SetDisplay`/`D374`/`D3A0`/`D3CC`/`D3F8`). Thin wrapper
around `GetSetBitField(&self->unk10, shift, width, value)`, shift 0, width 3,
value and result both pass straight through (no `== 0` boolean conversion on
either side -- same shape as `SceneNode__SetSemiTrans`/`D3A0`/`D3F8`).

```c
u32 SceneNode__SetLightDim(SceneNodeObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 0, 3, a1);
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294/SceneNode__SetLightDim.s`):
```
addu $a3, $a1, $zero      # a3 (value) = a1
addiu $a0, $a0, 0x10      # a0 = &self->unk10
addu $a1, $zero, $zero    # a1 (shift) = 0
jal  GetSetBitField
 ori $a2, $zero, 0x3      # a2 (width) = 3
```
No post-processing of `$v0` after the call -- the raw `GetSetBitField` result
is returned as-is.

### Proposed learning

None beyond what's already documented for the sibling family in
`include/code_d294.h` -- this just extends the same census (now 9
non-overlapping bitfields at `self->unk10`: shifts 0,3,6,7,8,9,28,30,31).

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `SceneNode__SetLightDim`
(tier A: pure bitfield accessor, shift 0 width 3, raw pass-through --
same reasoning as the renamed siblings `SceneNode__SetUseZ`/
`Field9`). Held back from an actual `tools/rename.py` run because this
exact symbol is name-checked (in a comment, not a call) from
`include/class_3bb8c.h:2360` -- a DIFFERENT unit's own vtable-slot
census, discussing a coincidental address match in an unrelated table.
Renaming would edit that file too, which is out of this round's scope
(`code_d294` only). Posted to the broadcast for the head to apply, or
for whoever next runs track 3 on `class_3bb8c.h`'s own unit to confirm
independently.

## Track 6 (round 91, echo): named `SceneNode__SetLightDim`, tier A

`GetSetBitField(&self->attribute, 0, 3, value)`: GsDOBJ2.attribute bits 0-2 are libgs.h's GsLDIM0..GsLDIM7 (light dimming). Was `GetSetUnk10Field0`. Slot +0x074 renamed `setLightDim` (no accessor). The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).

## Round 100 (delta): track 7

Parameter `a1` -> `value`. Shift 0 -> `ATTR_LDIM_SHIFT` (unit-local, beside
code_d294.c's `ATTR_*_SHIFT`; GsLDIM0..7 are bits 0-2 of GsDOBJ2.attribute,
include/psyq/libgs.h). The width 3 stays a literal, as in code_d294.c.

The file's own banner, before this pass, is kept below with this function's
comment (this is the unit's first function).

### History: the unit banner before this pass, verbatim

```c
/*
 * code_d294_b -- SceneNode (include/SceneNode.h), part 2 of 3: slots +0x074
 * to +0x0B4. The last four attribute setters; GetRotMatrix; the hull
 * notification chain (NotifyWithHull fills the model's TmdHull through
 * GetModelHull and hands it to TransformAndNotifyParents); the empty
 * onPadEvent/update defaults; DispatchLinkCommand and the proximity test it
 * runs (TryAttachNearby, with ComposeAndApplyRotation, CheckBoundsOverlap
 * and ClassifyAgainstPlanes); NotifyTaggedParents; the table getter; and
 * the free segment-against-box clippers the bounds tests use
 * (ClipSegmentToBox, BisectSegmentToBox).
 */
```

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Sibling of SceneNode__SetDisplay/D374/D3A0/D3CC/D3F8 (code_d294.c): a thin
 * wrapper around GetSetBitField over &self->unk10, shift 0 width 3. Raw
 * pass-through value and raw pass-through result -- same shape as
 * SceneNode__SetSemiTrans/D3A0/D3F8 (no `== 0` on either side). */
```
