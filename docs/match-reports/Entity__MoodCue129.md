# Entity__MoodCue129 — MATCH (48/48 words)

> Renamed from `func_80065514` on 2026-09-24 (tools/rename.py). Address 0x80065514.

**Unit:** Entity_g · **Size:** 48 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. Sets `this->unk44 = rand() % 2 +
0xA` (either 0xA or 0xB) when `this->unkFC == 0`. Dispatches `slot130`
unconditionally. When `this->unkFC >= 0xC9`: calls `Class6B5CC__FaceTarget(this,
this->unk94, 1, 0, 0)`, then `slotD0(this, -0x200, 0)` if `this->unk44 ==
0xA`.

## The C

```c
void Entity__MoodCue129(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        this->unk44 = rand() % 2 + 0xA;
    }
    this->methods->slot130(this);
    if (this->unkFC >= 0xC9) {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        if (this->unk44 == 0xA) {
            this->methods->slotD0(this, -0x200, 0);
        }
    }
}
```

Matched on the first build. Last function in this round's Entity_g queue.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue129`: the function's address sits in `gEntityMoodHandlerTable`
row 129 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
