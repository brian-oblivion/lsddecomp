# Entity__MoodCue103 — MATCH (95/95 words)

> Renamed from `func_80064928` on 2026-09-24 (tools/rename.py). Address 0x80064928.

**Unit:** Entity_g · **Size:** 95 instructions

## Blocker screen

No `gp_rel`/`addiu_at`/`nop_mflo_mfhi` hits.

## What it does

`gEntityMoodHandlerTable` row 103. Takes the standard handler signature but, like
`Entity__MoodCue98`, never reads `out`. Two `rand() % N == 0` gates (`% 3` and
`% 5`, both the plain compiler-generated magic-number division idiom, not
hand-expanded) branch on `this->unkFC`/`this->unk44` state codes, dispatch
a few vtable calls, and end with the same
`this->methods->slotC4(this, -0x1E, 0)` tail shape as `Entity__MoodCue98`.

## The C

```c
void Entity__MoodCue103(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0x2BC) {
        if (rand() % 3 == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC < 0x3FC) {
            this->methods->slot44(this, 0, ROTATION_YAW_MINUS_HALF);
            this->methods->slotCC(this, 0x1E, 0);
        }
        if (this->unkFC == 0x3A2) {
            this->methods->slot30(this, 0xA);
        }
    } else if (this->unkFC == 0x64 || this->unkFC == 0x320) {
        if (rand() % 5 == 0) {
            this->unk4C->methods->slot138(this->unk4C, 4, 0);
        }
    }
    this->methods->slotC4(this, -0x1E, 0);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. Matched on the first
build. Adds `ROTATION_YAW_MINUS_HALF` to this unit's local externs.


## Naming

Why `MoodCue103`: the function's address sits in `gEntityMoodHandlerTable`
row 103 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Data constant left unnamed this round

`ROTATION_YAW_MINUS_HALF` (`updateRotation` arg, `moodState == 0xB` branch): s16-pair
decoded `(0,1, -1,2, 0,1, 0,1)` -- only Y nonzero, -1/2 degree. Same
reasoning as `ROTATION_YAW_MINUS_THIRD` (Entity__MoodCue102's report): no fractional-degree
rotation constant is named anywhere in the project, so a half-degree
per-tick wobble rate does not fit the established whole-degree
`ROTATION_YAW_*` convention.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-26, round 94, alpha)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80089CB8` | `ROTATION_YAW_MINUS_HALF` | A (by value) | `.word 0x00010000, 0x0002FFFF, 0x00010000` = {0/1, -1/2, 0/1}: yaw -1/2 degree per call |

The fractional-degree precedent the section above wanted is
`ROTATION_XPLUS_EIGHTH` (0x80089C58, x = 1/8).
