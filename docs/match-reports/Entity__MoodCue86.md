# Entity__MoodCue86

> Renamed from `func_80063BC0` on 2026-09-25 (tools/rename.py). Address 0x80063bc0.

**Unit:** Entity_f · **Size:** 49 words · **Status:** MATCHED (49/49 words)

## What it does

`if (this->unkFC < 0xA) { slot130 } else if (this->unkFC == 0xA) {
slot12C }` (sharing a merged `jalr` tail across both call targets, as
retail already produces naturally). Independently, `if (this->unk84 ==
0xA) { SetCueTones18_3_3(out); }`. Independently, `if (this->unkFC ==
this->unk80 + 0xA) { slot16C; this->unk44 = 1; }`.

## Derivation

Straightforward transcription; calls the not-yet-defined-at-this-point
`SetCueTones18_3_3` (defined later in this unit, ROM order), matching this
unit's forward-declaration convention. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue86` | B | `gEntityMoodHandlerTable` row 86 |

Why `MoodCue86`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 86 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: `slot130` (StopTod) for the first 10 ticks, `slot12C` (PlayTod) at tick 10, `SetCueTones18_3_3` at frame 10, stops the cue and `moodState = 1` at `moodDuration + 10`.

## Track 4 (2026-09-26, round 88, echo)

Measured: through TodActor's `s32 playTod` slot this function grows 3 words (the playTod call no longer cross-jumps with the void stopTod call). Every Entity playTod call casts the slot to `EntityPlayTodFn` (void), which emits no code.

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
