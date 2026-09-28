# Entity__MoodCue65 -- MATCHED (71/71 words)

> Renamed from `func_80061F30` on 2026-09-24 (tools/rename.py). Address 0x80061f30.

Unit: `Entity` (round 13). Ignores its `out` argument entirely (same shape
as `Entity__MoodCue67`/`Entity__MoodCue70` in this unit): a one-shot 1-in-3 dice
roll on the first tick fires three vtable calls and sets `unk44 = 0xB`,
then a second block guarded by that flag fires an `unkFC`-threshold call
and an unconditional `slotC4`.
`void Entity__MoodCue65(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue65(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->methods->slot48(this, 1, SCALE_HALF);
            this->methods->slotCC(this, -0x12C, 0);
            this->methods->slot44(this, 1, sRotationYawPlus90);
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC == 0x7D0) {
            this->methods->slot44(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotC4(this, -0x14, 0);
    }
}
```

## Derivation notes

Same "1-in-3" `rand() % 3 == 0` idiom as `Entity__MoodCue67` in this unit
(magic `0x55555556`, no post-`mfhi` shift, `sll 1`/`addu` reconstructing
`q*3`, compared directly against the dividend). This one matched clean on
the first pass, unlike `Entity__MoodCue71`'s residue -- the difference is that
here the modulo result is never multiplied by anything afterward, it is
only compared to the dividend inline; `Entity__MoodCue71`'s stall was specific
to scaling the remainder by a further constant.

`slot48` (already `s32`-returning), `slotCC` (`s32`-returning), `slot44`
(`void`, `(self, s32, void*)`), and `slotC4` (`void`) are all pre-existing
vtable slot types from earlier work in this unit and `Entity`; every
call here discards its return value, consistent with those slots' existing
types. `sRotationYawPlus90` already has an extern/callsite later in this same file
(`Entity__MoodCue73`); `SCALE_HALF` and `ROTATION_YAW_MINUS90` are new per-unit `extern
u8 [];` data-table externs, same convention as the rest of this file.

No new struct or vtable-slot knowledge.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 65 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). Byte-identical (whole image green).
