# Entity__MoodCue30

> Renamed from `func_8005F800` on 2026-09-24 (tools/rename.py). Address 0x8005f800.

**Unit:** Entity · **Size:** 92 words · **Status:** MATCHED (92/92 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. Not a mood handler. Lazily assigns a state code
into `unk44` (`0xB`/`0xC`, gated by `Unk94Obj::slot200`'s return value), then
dispatches on that state:

```c
void Entity__MoodCue30(Entity *this) {
    if (this->unk44 == 0) {
        if (this->unk94->methods->slot200(this->unk94) == 1) {
            this->unk44 = 0xB;
        } else {
            this->unk44 = 0xC;
        }
    }

    if (this->unk44 == 0xC) {
        this->methods->slot48(this, 1, SCALE_DOUBLE);
        this->methods->slotCC(this, -0x1E, 0);
    } else {
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        if (this->unk44 == 0xB) {
            this->methods->slotC4(this, -0x64, 0);
            if ((u32)(this->unkFC - 0x55) < 0x1E) {
                this->methods->slotCC(this, 0x50, 0);
            } else if (this->unkFC == 0x78) {
                this->unk44 = 0xD;
            }
        } else if (this->unk44 == 0xD) {
            this->methods->slotB8(this, &this->unk94->unk14->x);
            this->methods->slotBC(this, TRANSLATE_Y_MINUS1500_Z_PLUS1024);
        }
    }
}
```

New vtable slot: `EntityMethods::slotB8`, `void (*slotB8)(Entity *self,
void *arg1)`, added to `include/Entity.h` right before the existing
`slotBC`. Its one known call site here passes `&this->unk94->unk14->x` --
the address of the 3-word position vector's `x` field, same "vector
pointer" convention as `Entity__IsNearTarget`'s still-`INCLUDE_ASM` first argument.
Third confirmed caller of `Unk94Methods::slot200` after `Entity__MoodCue00` and
`Entity__MoodCue29` (this one compares its result against `1`; no new signature
information). `TRANSLATE_Y_MINUS1500_Z_PLUS1024` is a new rodata pointer, extern-declared
alongside this unit's other `D_80089*` constants.

## Attempt log (2 attempts)

1. **First attempt (89/92, register identity only, no address drift) —
   wrote the lazy-init as a conditional expression:**
   `this->unk44 = (this->unk94->methods->slot200(this->unk94) == 1) ? 0xB :
   0xC;`. Every other word matched; the only residue was the two `li`
   immediates (`0xC`/`0xB`) and the following `sw` landing in `$a0` instead
   of retail's `$v0` -- a pure register-identity difference, branch targets
   identical.
2. **Second attempt (92/92) — same logic as two full `if`/`else` statements
   assigning `this->unk44` in each arm**, rather than one conditional
   expression. Matched immediately; no other change needed.

## Proposed learning

**A lazy-init `x = cond ? A : B;` immediately after a call whose result
feeds `cond` can pick a different register than the equivalent `if (cond) {
x = A; } else { x = B; }`.** Here the ternary put the literal in `$a0`
(retail wanted `$v0`) with the branch structure otherwise identical -- a
pure register-identity residue, not a control-flow one. This is a new
instance of the existing "register identity" residue class in
docs/DECOMPILATION_LEARNINGS.md; the fix was the general one already
documented there (reshape the source), specifically: prefer `if`/`else`
over `?:` when the assigned value is a small integer constant that feeds a
struct-field store on both arms.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 30 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity/d/e/g.

**This handler also occupies row 122** of `gEntityMoodHandlerTable` (same `handler` word at both `0x80089EB0+0x10*30` and `0x80089EB0+0x10*122`; the row's other three words differ between the two rows, so it is one function shared by two distinct mood-row configurations, not a naming collision). Named for its lower/first row per the existing convention (same precedent as `Entity__MoodCue81`, Entity_e); not a second name.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
