# Entity__MoodCue20 -- MATCHED (57/57 words)

> Renamed from `func_8005EFF4` on 2026-09-24 (tools/rename.py). Address 0x8005eff4.

Unit: `Entity`. Runner: bravo.

## Shape

```c
void Entity__MoodCue20(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0 && rand() % 7 == 0) {
        this->methods->slot48(this, 1, sScaleY2);
    }
    if ((out->unk4 & 3) == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0x1C;
    }
    this->methods->slotC4(this, -0x64, 0);
}
```

## Notes

- The first gate is `rand() % 7 == 0`, guarded by `this->unkFC == 0` -- the
  usual "one-shot random effect on entry" shape already seen in
  `Entity__MoodCue16`/`Entity__MoodCue12` in `src/world/Entity.c`.
- **`out->unk4 & 3`, not `% 4`.** Retail emits a bare `andi $v0,$v0,0x3` with
  no sign-correction shift, unlike every modulus-by-non-power-of-2 gate in
  this unit (which all carry the mult/mfhi/sra/subu chain). Writing `% 4`
  here would risk GCC inserting a sign-fix for the general signed-modulus
  case; the plain bitwise `&` reproduces the bare `andi` byte-for-byte on the
  first attempt, so no sign correction was needed in practice -- worth
  remembering as a candidate discriminator: a bare `andi` with no correction
  present in the asm is a tell that the source used `&`, not `%`, even where
  a modulus by a power of two would read equally naturally.
- Extern added: `sScaleY2` (opaque row pointer, same convention as the
  other `D_80089Dxx`/`D_80089Cxx` rows already declared in `Entity.c`).
- Clean of both open toolchain blockers.

Matched first attempt (1/30).

### Proposed learning

A bare `andi $v0, $v0, N` (power-of-2 mask) with **no** accompanying sign-fix
sequence is a source-level `& (N-1)`, not `% N` -- GCC 2.6.3 still inserts a
correction for a genuinely signed `%` by a power of two, so its absence is a
real discriminator, not just an equally-valid alternate spelling.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 20 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
