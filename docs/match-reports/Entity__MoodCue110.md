# Entity__MoodCue110 — MATCH (59/59 words)

> Renamed from `func_80064D48` on 2026-09-24 (tools/rename.py). Address 0x80064d48.

**Unit:** Entity_g · **Size:** 59 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `slot48`/`slot130` unconditionally,
then a distance check (`slot144(this, this->unk94) < 0x800`) that sets
`unk44=0xA`/`unkFC=0` when `unk44` was still 0, followed by two `unkFC`
range gates on `unk44==0xA`.

## The C

```c
void Entity__MoodCue110(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, SCALE_TWO_FIFTHS);
    this->methods->slot130(this);
    if (this->unk44 == 0) {
        if (this->methods->slot144(this, this->unk94) < 0x800) {
            this->unk44 = 0xA;
            this->unkFC = 0;
        }
    }
    if (this->unk44 == 0xA) {
        if (this->unkFC < 0x2D) {
            this->methods->slot44(this, 0, ROTATION_YAW_PLUS4);
        }
        if (this->unkFC >= 0x1F5) {
            this->unk44 = 0;
        }
    }
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue110`: the function's address sits in `gEntityMoodHandlerTable`
row 110 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity through Entity_f), so the
names sort in table order.

## Data constant decoded this round

`ROTATION_YAW_PLUS4` (0x80089D00), this function's `updateRotation` argument
inside its `moodState == 0xA` branch, decoded from `disk/SLPS_015.56` as
four s16 `{num,den}` pairs: `(0,1, 4,1, 0,1, 90,1)` -- only Z is a whole
degree (4/1), X=Y=0; W=90/1 is ignored per the established precedent that
the 4th pair is never reflected in the name (`ROTATION_YAW_MINUS120` at
0x80089CE8 has an equally nonzero, equally unnamed W=50/1). Matches the
existing `ROTATION_ZPLUS9` naming shape exactly, just a different Z
amount.

## Data constant left unnamed this round

`SCALE_TWO_FIFTHS` (`updateScale` arg, unconditional at function entry; also
used by `Entity__MoodCue125`): s16-pair decoded `(2,5, 2,5, 2,5, 1,1)` --
uniform X=Y=Z=2/5. A non-unit fraction, unlike every currently-named
`SCALE_*` (`HALF`=1/2, `EIGHTH`=1/8, `QUARTER`=1/4, all unit fractions, or
`SIX`/`X3`/`Y2`/`Y4`/`DOUBLE`, all whole multiples) -- no precedent covers
a fraction like 2/5, so left as `D_` rather than inventing a new word
(`SCALE_TWO_FIFTHS`) with no anchor in the existing convention.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-26, round 94, alpha)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80089E44` | `SCALE_TWO_FIFTHS` | A (by value) | `.word 0x00050002` x3 = uniform 2/5, like `SCALE_EIGHT_SEVENTHS` |
| `ROTATION_ZPLUS4` | `ROTATION_YAW_PLUS4` | A (by value) | `.word 0x00010000, 0x00010004, 0x00010000` = {0/1, 4/1, 0/1}: the 4 is the SECOND pair, Y (yaw), as in `ROTATION_YAW_PLUS9` = {0, 9, 0}; `ROTATION_ZPLUS9` = {0, 0, 9} has it third |

**Correction** to "Data constant decoded this round" above: it read
`(0,1, 4,1, 0,1, ...)` correctly and then called the second pair Z. The
name followed the misreading; this function is its only user.
