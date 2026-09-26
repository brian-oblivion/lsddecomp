# Entity__MoodCue87

> Renamed from `func_80063CC8` on 2026-09-25 (tools/rename.py). Address 0x80063cc8.

**Unit:** Entity_f · **Size:** 30 words · **Status:** MATCHED (30/30 words)

## What it does

`out->unk10 = this->methods->slot148(this); if (out->unk4 == 0) {
out->unk1C = 0x12; } if (out->unk4 >= this->unk80 - 1) { out->unk4 = -1;
}`.

## Derivation

Direct transcription. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue87` | B | `gEntityMoodHandlerTable` row 87 |

Why `MoodCue87`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 87 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: Attenuation from proximity; voice-0 tone 18 on set tick 0; resets the set's tick to -1 once it reaches `moodDuration - 1` (loops the cue).

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
