# Entity__MoodCue114 — MATCH (42/42 words)

> Renamed from `func_800650F4` on 2026-09-24 (tools/rename.py). Address 0x800650f4.

**Unit:** Entity · **Size:** 42 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. Returns immediately on the first
`rand() % 3 == 0`. Otherwise dispatches `slot44(this, 0, a2)` where `a2` is
`sRotationYawMinus9` when a SECOND `rand() % 3 == 0`, else `sRotationYawPlus9`.

## The C

```c
void Entity__MoodCue114(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (rand() % 3 == 0) {
        return;
    }
    if (rand() % 3 != 0) {
        a2 = sRotationYawPlus9;
    } else {
        a2 = sRotationYawMinus9;
    }
    this->methods->slot44(this, 0, a2);
}
```

## Residue chased: swapped row pointers

First attempt swapped which `rand() % 3` outcome selects `sRotationYawMinus9` vs
`sRotationYawPlus9` (also flipping a `beq` to `bne`, a genuine control-flow
difference, not just a value swap) -- 39/42. Reading the `beq
v0,v1,L8006516C` (branch on remainder==0, i.e. `rand()%3==0`) against
which `a2` value each fallthrough/branch path sets closed it to 42/42 on
the second build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity. 2 attempts.


## Naming

Why `MoodCue114`: the function's address sits in `gEntityMoodHandlerTable`
row 114 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity through Entity_f), so the
names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4b (round 93, charlie) — 2026-09-26

The motion templates are declared once, in `include/entity.h` (`ROTATION_*`/`SCALE_*` as `Ratio16[]`, `TRANSLATE_*` as `LongVec3[]`); the unit-local `u8[]` externs are gone. The local `a2`, which holds `sRotationYawPlus9` or `sRotationYawMinus9` and is passed to `updateRotation`, is now `Ratio16 *` (was `void *`). A pointer local's pointee type changes no instruction and the slot takes `void *`, so the bytes held: whole image green, 0 new `-Wall` warnings, nonmatching green. (`Entity__MoodCue102`'s `a2` and `Entity__MoodCue76`/`78`'s `table` stay `void *`: they also hold unnamed `u8[]` tables, `sScaleUnit`/`sScaleXFourFifthsYSixFifths`/`sScaleMinusSixtyFourth`.)

## Track 7 (2026-09-26, round 94, alpha)

Local `a2` renamed `rotation` (the `updateRotation` argument).
