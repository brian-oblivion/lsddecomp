# SceneNode__SetSubdivision -- MATCHED (11/11 words)

> Renamed from `SceneNode__GetSetUnk10Field9` on 2026-09-26 (tools/rename.py). Address 0x8001d480.

> Renamed from `Class6B5CC__GetSetUnk10Field9` on 2026-09-26 (tools/rename.py). Address 0x8001d480.

> Renamed from `func_8001D480` on 2026-09-18 (tools/rename.py). Address 0x8001d480.

Round 12, runner delta. `code_d294`.

## Summary

Same family as `SceneNode__SetLightDim` (see that report) -- raw pass-through wrapper
around `GetSetBitField(&self->unk10, shift, width, value)`, shift 9, width 3.

```c
u32 SceneNode__SetSubdivision(SceneNodeObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 9, 3, a1);
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294/SceneNode__SetSubdivision.s`):
```
addu $a3, $a1, $zero      # a3 (value) = a1
addiu $a0, $a0, 0x10      # a0 = &self->unk10
ori  $a1, $zero, 0x9      # a1 (shift) = 9
jal  GetSetBitField
 ori $a2, $zero, 0x3      # a2 (width) = 3
```
No post-processing of `$v0`.

### Proposed learning

None beyond the family census already noted in `SceneNode__SetLightDim.md`.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D480` via `tools/rename.py`. **Tier A** -- same
reasoning as `SceneNode__SetUseZ`: a pure bitfield accessor
(shift 9, width 3, raw pass-through), mechanics fully known, field's
real purpose not established. Purely local to this unit + its header.

## Track 6 (round 91, echo): named `SceneNode__SetSubdivision`, tier A

`GetSetBitField(&self->attribute, 9, 3, value)`: bits 9-11 are libgs.h's GsDIV1..GsDIV5 (polygon subdivision). Was `GetSetUnk10Field9`. Slot +0x07C renamed `setSubdivision` (no accessor). The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).

## Round 100 (delta): track 7

Parameter `a1` -> `value`. Shift 9 -> `ATTR_DIV_SHIFT` (GsDIV1..5 are 1..5 << 9).

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Same family as SceneNode__SetLightDim, shift 9 width 3. Raw pass-through. */
```
