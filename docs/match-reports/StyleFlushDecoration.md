# StyleFlushDecoration -- MATCHED, round 46 (2026-09-15)

> Renamed from `func_80054714` on 2026-09-23 (tools/rename.py). Address 0x80054714.

Unit `ObjMStyleActor`. **17/17 words, byte-exact.** Second build (one-word
struct-offset miss on the first).

## What it was

Fresh ground, carved round 45, never attempted. No blockers (no `gp_rel`,
no `mflo`/`mfhi` hazard, not a trampoline).

## Derivation

```
/* 44F14 80054714 4C03828F */  lw    $v0, %gp_rel(sStyleDecorColor)($gp)
/* 44F18 80054718 E8FFBD27 */  addiu $sp, $sp, -0x18
/* 44F1C 8005471C 0A004010 */  beqz  $v0, .L80054748
/* 44F20 80054720 1000BFAF */   sw   $ra, 0x10($sp)
/* 44F24 80054724 8C04848F */  lw    $a0, %gp_rel(sStyleDecorObj)($gp)
/* 44F2C 8005472C 0000828C */  lw    $v0, 0x0($a0)
/* 44F34 80054734 0400428C */  lw    $v0, 0x4($v0)
/* 44F3C 8005473C 09F84000 */  jalr  $v0
/* 44F44 80054744 4C0380AF */  sw    $zero, %gp_rel(sStyleDecorColor)($gp)
.L80054748:
...
jr $ra
```

`sStyleDecorColor` is a `.sdata` pointer, already established in
`src/world/ObjMStyleActor.c` as `extern const u8 *sStyleDecorColor;`, and used there as an
actual colour-table pointer (`sStyleDecorColor = gStylePalette[cfg[2]];`). Here it is
only ever tested against zero, so it reads as a one-shot "pending" flag on
top of the same storage. `sStyleDecorObj` is that unit's `LocalM4D0Obj *`
(round 15's own local type, unrelated to this unit) with named slots at
`+0x04C`/`+0x064`/`+0x068`. This function dispatches `+0x004`, a slot that
unit never names, so it gets its own minimal local view here rather than
importing `ObjMStyleActor.c`'s type (multiple-independent-local-views
convention; that unit is not this one's to edit).

```c
typedef struct ObjAB54 ObjAB54;
typedef struct ObjAB54Methods ObjAB54Methods;
struct ObjAB54Methods {
    u8 pad0[0x4];
    void (*slot4)(ObjAB54 *self); /* +0x004 */
};
struct ObjAB54 {
    ObjAB54Methods *methods; /* +0x000 */
};

extern const u8 *sStyleDecorColor;
extern s32 sStyleDecorObj;

void StyleFlushDecoration(void) {
    if (sStyleDecorColor != 0) {
        ((ObjAB54 *) sStyleDecorObj)->methods->slot4((ObjAB54 *) sStyleDecorObj);
        sStyleDecorColor = 0;
    }
}
```

**First attempt missed by one word** by putting `slot4` at methods-struct
offset 0 instead of 0x4 (i.e. forgetting the leading `u8 pad0[0x4]`) --
`funcdiff.py` pinpointed it immediately (`retail=0400428c built=0000428c`,
a `lw $v0, 0x4($v0)` vs `lw $v0, 0x0($v0)` diff with no other residue).
Adding the pad matched on the next build.

### Proposed learning

None beyond the routine "an ordinary field/slot offset miss reads as a
single-instruction diff with the rest of the function byte-identical" --
already well covered by existing entries on struct-offset mistakes.

## Naming

**`StyleFlushDecoration`, tier B.**

Guards `sStyleDecorColor` (formerly `D_8008AB54`), dispatches
`sStyleDecorObj->methods->slot4()`, then clears the guard -- the same
test/dispatch/clear shape as `StyleReleaseDecorSet`/`StyleReleaseEffectSlots`
below (all three called together, in this order, from `StyleTeardown`).
`sStyleDecorColor` and `sStyleDecorObj` are both established members of
ObjMStyleActor.c's already-named "Style" subsystem
(`RegisterStyleConfig`/`ApplyStyleConfig`/`ApplyStyleDecorationIfSet`, round
69) -- that cross-unit naming is the evidence for the `Style` prefix, not a
guess. "Flush" mirrors this file's own `FlushSoundCueSet`/`FlushStyleCue`
naming for the identical mechanical shape (consume pending state, dispatch,
clear). The target's own purpose (what `slot4` does) is not established
beyond "dispatch", so the verb describes the CALLER's mechanics, not the
callee's.

## Track 4 (2026-09-25, round 85, charlie)

sStyleDecorObj is a BoxFill (include/BoxFill.h); the deleted `ObjAB54` view's +0x004 is release. Zero bytes.

## Round 93 polish (delta, track 7)

### Comments moved here from src/world/ObjMStyleActor.c

Verbatim as they stood before the round-93 comment pass (identifiers already carry this round's renames).

```c
/* sStyleDecorObj and gStyleDecorSlots[] hold BoxFill objects
 * (include/BoxFill.h, New_BoxFill), in globals typed `s32`/`void *[]`
 * (track 4b's to retype). */
```
