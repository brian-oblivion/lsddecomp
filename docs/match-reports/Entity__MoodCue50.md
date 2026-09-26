# Entity__MoodCue50 -- MATCHED (36/36 words)

> Renamed from `func_80060CF0` on 2026-09-24 (tools/rename.py). Address 0x80060cf0.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue50(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue50(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC < this->unk80 * 5) {
        if (this->unk84 == 0xF || this->unk84 == 0x46) {
            out->unk10 = 0;
            out->unk1C = 7;
            out->unk30 = 7;
            out->unk44 = 7;
        }
    } else {
        this->methods->slot160(this);
        this->unk44 = 1;
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed. Uses the already-documented
`EntityMethods::slot160` (`void (*)(Entity *self)`, previously known from
`Entity__DetachFromParent`/`Entity__OnGridCellLinkCommand`/`Entity__UpdateDeactivationState` in the earlier Entity units)
-- no header change required here.

### Proposed learning

None -- straightforward two-way dispatch, no residue.

## Naming

`Entity__MoodCue50` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 50, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Named: `ENTITY_STATE_DONE` (evidence on each definition: EntityEffect and ENTITY_STATE_DONE in include/Entity.h, SOUND_CUE_STOP in include/SoundCueSet.h). Byte-identical (whole image green).
