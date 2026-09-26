# Entity__MoodCue88

> Renamed from `func_80063D40` on 2026-09-25 (tools/rename.py). Address 0x80063d40.

**Unit:** Entity_f · **Size:** 34 words · **Status:** MATCHED (34/34 words)

## What it does

Near-sibling of `Entity__MoodCue87`: `out->unk10 = this->methods->slot148(this);
if (out->unk4 == this->unk80 / 2) { out->unk1C = 0x12; } if (out->unk4 >=
this->unk80 - 1) { out->unk4 = -1; }`.

## Derivation

The `unk80 / 2` comparison compiles to the signed-divide-by-2 idiom
already documented in `include/Entity.h`'s own comment on `Entity::unk80`
(`(x + (unsigned)x>>31) >> 1`, from `Entity__MoodCue09`/`Entity__MoodCue11` in
Entity.c) — confirmed the plain `/2` operator reproduces it here too
without needing to hand-transcribe the shift/add sequence, consistent
with this project's established "just write `%`/`/`, trust the compiler"
policy for constant divisors.

An early draft hand-transcribed the idiom directly as
`(this->unk80 + (u32)this->unk80 >> 31) >> 1` — this is WRONG C even
setting the toolchain question aside: `+` binds tighter than `>>` in C, so
that expression parses as `(unk80 + (u32)unk80) >> 31`, not the intended
`unk80 + ((u32)unk80 >> 31)`. Caught before it ever got a build (a
precedence bug, not a residue), and replaced with the plain `/ 2` per the
existing documented idiom.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue88` | B | `gEntityMoodHandlerTable` row 88 |

Why `MoodCue88`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 88 (base 0x80089EB0, stride 0x10; the row's
first word), read from `disk/SLPS_015.56` directly rather than inferred from
address order (rounds 76-77 measured that row order does not track code
address). Nothing else references it. `Entity__StartSoundCue` hands the row's
handler to `InitSoundCueSet`, and `ServiceSoundCueSet` calls it once per tick
as `callback(owner, set)`, so `out` is the `SoundCueSet` (`EntityMoodHandlerArg`
is Entity.h's local view; field readings in `Entity__MoodCue07.md`
`## Proposed field names`: `unk4` tick, `unk10` attenuation, `unk1C`/`unk30`/`unk44`
voice 0/1/2 tone request (-2 = stop), `unk20`/`unk34`/`unk48` pitch offset).
Tier B, same as every sibling `Entity__MoodCueNN` (Entity_b..Entity_g): the
row mapping is a fact of the binary, which dream object or state a row is
for is not established. Row kept decimal so names sort in table order.

What it does, in the unit's current field names: As `Entity__MoodCue87` but the tone fires at set tick `moodDuration / 2`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
