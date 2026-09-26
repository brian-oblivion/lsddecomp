# Entity__MoodCue61 -- MATCHED (10/10 words)

> Renamed from `func_80061C04` on 2026-09-24 (tools/rename.py). Address 0x80061c04.

Unit: `Entity_e` (round 12, first carve of this unit). Smallest function in
the unit's queue, a one-shot mood handler with no loop or nested branch.
`void Entity__MoodCue61(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue61(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 == 0x1E) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk20 = -1;
    }
}
```

## Derivation notes

Matched first attempt. Straight read of the disassembly: `this->unk84`
(already a known field, compared against literals elsewhere in Entity_d)
tested against `0x1E`; on match, three stores into the `EntityMoodHandlerArg
*out` parameter, all to already-known offsets (`unk1C`, `unk10`, `unk20`).
No new struct knowledge -- everything used was already established in
`include/Entity.h` from Entity_d's work.

### Proposed learning

None beyond what's already documented; this one confirms the existing
`EntityMoodHandlerArg` field set is enough to read this unit's handlers
without further struct excavation.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 61 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity_b/Entity_d/Entity_g): `attenuation`, `voice0Tone`, `voice0Pitch`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
