# Entity__MoodCue105 — MATCH (27/27 words)

> Renamed from `func_80064B14` on 2026-09-24 (tools/rename.py). Address 0x80064b14.

**Unit:** Entity_g · **Size:** 27 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `this->methods->slot60(this,
rand() % 20 == 0)` -- the plain magic-number `% 20` idiom (mult by
`0x66666667`, `sra 3`, sign-fix, rebuild `*20`, subtract), not hand-expanded.

## The C

```c
void Entity__MoodCue105(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot60(this, rand() % 20 == 0);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue105`: the function's address sits in `gEntityMoodHandlerTable`
row 105 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity through Entity_f), so the
names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
