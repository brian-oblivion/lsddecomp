# Entity__MoodCue33 -- MATCHED (54/54 words)

> Renamed from `func_8005FA94` on 2026-09-24 (tools/rename.py). Address 0x8005fa94.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue33(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkF4 != 0) {
        this->methods->slot48(this, 1, SCALE_QUARTER);
    } else if (out->unk4 % 30 == 0) {
        out->unk10 = 0;
        out->unk1C = 3;
    }
    this->methods->slotD0(this, -0x1E, 0);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
```

## Notes

- `if (unkF4 != 0) { call A } else if (mod-30-gate) { fields }` are two
  disjoint alternatives that both fall through to the unconditional
  `slotD0` call -- retail's `unkF4 != 0` branch literally `j`umps past the
  mod-gate straight to the shared `slotD0` call site, which an `if`/`else
  if` with no shared statements in between reproduces exactly.
- `out->unk4 % 30` divisor recovered arithmetically from the
  mult(0x88888889)/mfhi/sra/subu/sll chain (`v0*16 -> v0*15 -> v0*30`) --
  same magic constant as `Entity__MoodCue24`'s `% 15` gate in this same unit,
  different shift, different divisor. A reminder that the magic multiplier
  alone does not identify the divisor; the shift and the reconstruction
  chain do.
- Extern added: `SCALE_QUARTER`.
- Clean of both open toolchain blockers.

Matched first attempt (1/30).

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 33 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
