# Entity__MoodCue24 -- MATCHED (59/59 words)

> Renamed from `func_8005F368` on 2026-09-24 (tools/rename.py). Address 0x8005f368.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue24(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 15 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 7;
        out->unk20 = -2;
    }
    this->methods->slot48(this, 1, SCALE_DOUBLE);
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
    this->methods->slotC4(this, -0x200, 0);
}
```

## Notes

- `% 15` gate: same magic multiplier (0x88888889) as `Entity__MoodCue33`'s
  `% 30` gate, different shift amount in the mfhi/sra chain -- confirms the
  DECOMPILATION_LEARNINGS caution that the constant alone never identifies
  the divisor; here the reconstruction is `v0*16 -> v0*15`, shift 3 instead
  of `Entity__MoodCue33`'s shift 4.
- Three unconditional vtable calls in a row after the gate -- straight
  sequential statements, no reshaping needed.
- Extern added: `ROTATION_YAW_PLUS2` (own file-scope declaration; already declared
  in `Entity_b.c`).
- Clean of both open toolchain blockers.

Matched first attempt (1/30).

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 24 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
