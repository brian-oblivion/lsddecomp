# Entity__MoodCue92

> Renamed from `func_80064078` on 2026-09-25 (tools/rename.py). Address 0x80064078.

**Unit:** Entity_f · **Size:** 82 words · **Status:** MATCHED (82/82 words)

## What it does

On `this->unk7C == 0`: if `this->unkF4 != 0`, calls `Class6B5CC__FaceTarget`,
`slot128(this,1)`, and `this->unk94->methods->slot130(this->unk94,1)`;
else if `this->unk84 == 0`, runs the same `slot134` do-while loop as
`Entity__MoodCue91` (this unit). On `this->unk7C != 0`: if `this->unk84 == 0`,
sets `out->unk10=0; out->unk1C=0x16;`; else if `this->unk84 ==
this->unk80 - 1`, sets `out->unk10=0; out->unk30=0x12;` and calls
`slot30(this, 0xA)`. Unconditionally, `slot48(this, 1, D_80089DE4)`.

## Derivation

Direct transcription, reusing `EntityMethods::slot134`/`Entity::unk88`
established by `Entity__MoodCue91` (earlier in ROM order, same unit). No new
header entries needed here. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue92` | B | `gEntityMoodHandlerTable` row 92 |

Why `MoodCue92`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 92 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: With `unk7C` (TOD index, proposed) 0: if `targetReached`, faces the target, `slot128(1)` (SetTod) and `target->slot130(1)`, else fast-forwards 24 TOD frames as `Entity__MoodCue91` does; with `unk7C` nonzero: voice-0 tone 22 at frame 0, voice-1 tone 18 and `notifyParents(0xA)` at the last frame (`moodDuration - 1`); `updateScale(1, D_80089DE4)` every tick.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
