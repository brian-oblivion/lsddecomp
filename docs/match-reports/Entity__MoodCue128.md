# Entity__MoodCue128 — MATCH (29/29 words)

> Renamed from `func_800654A0` on 2026-09-24 (tools/rename.py). Address 0x800654a0.

**Unit:** Entity_g · **Size:** 29 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `SceneNode__FaceTarget(this, this->unk94,
1, 0, 0)`, `slot48(this, 1, SCALE_THIRTY_SECOND)`, `slotC4(this, -0x1E, 1)`.

## The C

```c
void Entity__MoodCue128(Entity *this, EntityMoodHandlerArg *out) {
    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
    this->methods->slot48(this, 1, SCALE_THIRTY_SECOND);
    this->methods->slotC4(this, -0x1E, 1);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue128`: the function's address sits in `gEntityMoodHandlerTable`
row 128 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Data constant decoded this round

`SCALE_THIRTY_SECOND` (0x80089E80), this function's `updateScale`
argument, decoded from `disk/SLPS_015.56` as four s16 `{num,den}` pairs:
`(1,32, 1,32, 1,32, 3,1)` -- uniform X=Y=Z=1/32, W=3/1 ignored per the
established precedent. Same unit-fraction-word convention as `SCALE_HALF`
(1/2)/`SCALE_EIGHTH` (1/8)/`SCALE_QUARTER` (1/4).

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
