# Entity__MoodCue89

> Renamed from `func_80063DC8` on 2026-09-25 (tools/rename.py). Address 0x80063dc8.

**Unit:** Entity_f · **Size:** 40 words · **Status:** MATCHED (40/40 words)

## What it does

`if (this->unkFC == 0x14) { out->unk1C=0x12; out->unk10=0; out->unk30=3;
return; } if (this->unkFC == this->unk80) { this->methods->slot16C(this);
this->unk44=1; if (rand() & 1) { this->methods->slot30(this, 0xB); } }`.

## Derivation

Direct transcription. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue89` | B | `gEntityMoodHandlerTable` row 89 |

Why `MoodCue89`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 89 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: Tones 18/3 at full volume at `moodTimer` 20; at `moodDuration` stops the cue, `moodState = 1`, and `notifyParents(0xB)` on a coin flip.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 95, bravo)

### Constants

- `state = 1` after `stopSoundCue` is `ENTITY_STATE_DONE`: nothing in this handler reads `state == 1`, and it is the value Entity__UpdateActivationState and Entity__UpdateSoundCueStart read as "do not reactivate / restart the cue"
- `notifyParents(this, 0xB)` is `ENTITY_EFFECT_EVENT_VIDEO` (enum EntityEffect: case 11 ends the dream into the video `getEventVideo` names).
- Every other literal went to decimal (tick counts, TOD frames, distances, VAB programs; no masks): they are this handler's tuning, named by nothing else.
