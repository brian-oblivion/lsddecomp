# SceneNode__SetBackClip -- MATCHED (12/12 words)

> Renamed from `SceneNode__GetSetUnk10Flag8` on 2026-09-26 (tools/rename.py). Address 0x8001d4ac.

> Renamed from `Class6B5CC__GetSetUnk10Flag8` on 2026-09-26 (tools/rename.py). Address 0x8001d4ac.

> Renamed from `func_8001D4AC` on 2026-09-18 (tools/rename.py). Address 0x8001d4ac.

Round 12, runner delta. `code_d294_b`.

## Summary

Same family as `SceneNode__SetUseZ` (see that report) -- double-inversion
(`a1 == 0` in, `== 0` on the result) wrapper around `GetSetBitField`, shift 8
width 1, `s32` return type.

```c
s32 SceneNode__SetBackClip(SceneNodeObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 8, 1, a1 == 0) == 0;
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/SceneNode__SetBackClip.s`):
```
sltiu $a3, $a1, 0x1       # a3 (value) = (a1 == 0)
addiu $a0, $a0, 0x10      # a0 = &self->unk10
ori   $a1, $zero, 0x8     # a1 (shift) = 8
jal   GetSetBitField
 ori  $a2, $zero, 0x1     # a2 (width) = 1
sltiu $v0, $v0, 0x1       # result = (raw == 0)
```

Note: `include/class_3ac78.h` documents an UNRELATED cross-unit call that
also names this symbol `SceneNode__SetBackClip` but through a different table
(`TimedTask::unk34`) with a 4-argument `(self, arg1, arg2, arg3)` shape at
its own local slot `+0x080`. That's the same "same code address, different
argument count per call site" precedent already established for
`GetSceneNodeMethods` elsewhere in this unit -- it does not affect this unit's own
implementation, which is typed only to this unit's own call sites
(`GetSetBitField` and the vtable slot `SceneNodeMethods::+0x080`, confirmed
via `tools/classtable.py gSceneNodeMethods`).

### Proposed learning

None new -- extends the family census.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `SceneNode__SetBackClip`
(tier A: pure bitfield accessor, shift 8 width 1, double-inverted
boolean -- same shape as the renamed `SceneNode__SetUseZ`).
Held back because this symbol is name-checked (in comments, not calls)
from `src/class_3bb8c_k.c:186` and `include/class_3ac78.h:173` -- two
different units' own vtable-slot census comments, both discussing a
coincidental address match in an unrelated table (`gStyleEffectMethods`'s own
slot80, a different class entirely). Renaming would edit those files
too, out of this round's scope. Posted to the broadcast.

## Track 6 (round 91, echo): named `SceneNode__SetBackClip`, tier A

`GetSetBitField(&self->attribute, 8, 1, on == 0) == 0`: `on` clears libgs.h's GsNBACKC ("no back clip"), the same inverted shape as SetDisplay/GsDOFF and SetLighting/GsLOFF. Was `GetSetUnk10Flag8`. Slot +0x080 kept as `getSetUnk10Flag8`: its callers are in class_3bb8c_o.c and class_3bb8c_k.c, outside this job; `setBackClip` proposed. The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).

## Round 100 (delta): track 7

Parameter `a1` -> `on`. Shift 8 -> `ATTR_NBACKC_SHIFT` (GsNBACKC is 1 << 8).

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Same family as SceneNode__SetUseZ: double-inversion shape, shift 8 width 1. */
```
