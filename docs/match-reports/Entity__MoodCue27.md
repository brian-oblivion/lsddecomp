# Entity__MoodCue27 -- MATCHED (51/51 words)

> Renamed from `func_8005F608` on 2026-09-24 (tools/rename.py). Address 0x8005f608.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue27(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 70 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x1B;
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->unkFC < 0x64) {
        this->methods->slotCC(this, 0x20, 0);
    } else if (this->unkFC >= 0x12D) {
        this->methods->slotCC(this, -0x20, 0);
    }
}
```

## Notes

- The `% 70` gate is the confirmed "x % N for compile-time constant N: just
  write %" idiom -- divisor 70 recovered arithmetically from the
  mult/mfhi/sll/addu/subu reconstruction chain (`v1*8 -> v1*9 -> v1*36 ->
  v1*35 -> v1*70`), not guessed.
- Three-way dispatch on `this->unkFC`: below 100 does one thing, 100..300
  does nothing, 301+ does the opposite thing -- a plain `if`/`else if` with
  no `else` body reproduces retail's middle-range no-op exactly.
- Clean of both open toolchain blockers.

Matched first attempt (1/30).

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 27 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
