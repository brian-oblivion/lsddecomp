# FlushStyleCue -- MATCHED, round 46 (2026-09-15)

> Renamed from `func_800557DC` on 2026-09-23 (tools/rename.py). Address 0x800557dc.

Unit `ObjMStyleActor`. **20/20 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted. Already forward-declared
by `StyleTeardown` earlier in this unit (`extern s32 FlushStyleCue(ObjN14
*arg0);`), which fixed the parameter type before this function's own
derivation started.

## Derivation

```
/* 45FDC 800557DC 7404828F */  lw   $v0, %gp_rel(gStyleSceneRefs)($gp)
/* 45FE4 800557E4 1000B0AF */  sw   $s0, 0x10($sp)
/* 45FE8 800557E8 21808000 */  addu $s0, $a0, $zero
/* 45FF0 800557F0 0000448C */  lw   $a0, 0x0($v0)
/* 45FF4 800557F4 21B3000C */  jal  FlushSoundCueSet
/* 45FF8 800557F8 14000526 */   addiu $a1, $s0, 0x14
/* 45FFC 800557FC 0000038E */  lw   $v1, 0x0($s0)
/* 46004 80055804 06006290 */  lbu  $v0, 0x6($v1)
/* 4600C 8005580C 23100200 */  negu $v0, $v0
/* 46010 80055810 060062A0 */  sb   $v0, 0x6($v1)
/* 46014 80055814 21100000 */  addu $v0, $zero, $zero
...
jr $ra
```

`gStyleSceneRefs` is a plain `s32` (established in `ObjMStyleActor.c`) holding the
address of a small descriptor object; this function reads *that object's*
own offset 0 (a value, not the `FieldAC7CHolder.unkC` field
`ObjMStyleActor.c` names at +0xC -- a different offset of the same base
pointer, kept as its own independent local reading rather than importing
that unit's type). The offset-0 value is passed as `FlushSoundCueSet`'s `self`
argument, matching that function's existing loose declaration in
`include/Entity.h`/`include/DreamSys.h` (`extern void FlushSoundCueSet(s32
arg0, void *arg1);`) -- each caller already carries its own local reading of
`self`'s real type, so this unit does the same rather than pulling in
`ObjDA34`.

`arg0` (this unit's own `ObjN14`, introduced by `StyleTeardown`) supplies
both the embedded sub-object handed to `FlushSoundCueSet` (`&arg0->unk14`) and
the byte toggled after the call (`arg0->unk0->unk6`, negated in place).

```c
extern s32 gStyleSceneRefs;
extern void FlushSoundCueSet(s32 arg0, void *arg1);

s32 FlushStyleCue(ObjN14 *arg0) {
    FlushSoundCueSet(*(s32 *) gStyleSceneRefs, &arg0->unk14);
    arg0->unk0->unk6 = -arg0->unk0->unk6;
    return 0;
}
```

`arg0` needs to survive the `FlushSoundCueSet` call (a caller-saved register),
which is exactly why retail spills it to `$s0` -- the C above doesn't need
to say anything about that, GCC does it on its own from the two uses
straddling the call.

### Proposed learning

None beyond confirming the multiple-independent-local-views convention:
`gStyleSceneRefs`'s pointed-to object is read at offset 0 here and offset 0xC in
a sibling unit, with two unrelated local structs describing it -- both
correct locally, neither claiming to be the whole object.

## Naming

**`FlushStyleCue`, tier B.**

`FlushSoundCueSet` on the slot's embedded `cueSet`, then toggles the
claimed entry's sign tag back (releasing it for reuse), always returns 0.
Name mirrors the already-established `FlushSoundCueSet` it calls, for the
same "flush this slot's pending sound state" mechanic. Called from
`StyleTeardown` (unconditionally, both slots) and from `TickStyle` (per
slot, when `ServiceStyleCueIfNear` reports the cue is no longer near). MATCHED,
20/20, first build.

## Round 93 polish (delta, track 7)

### Naming

Round 93: returns `StyleCueSlot *` (always NULL, stored back into sStyleCueSlots) rather than `s32`; FlushSoundCueSet given its real prototype. Parameter `slot`.
