# Entity__MoodCue117 — MATCH (13/13 words)

> Renamed from `func_8006519C` on 2026-09-24 (tools/rename.py). Address 0x8006519c.

**Unit:** Entity_g · **Size:** 13 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `slot48(this, 1, SCALE_SIX)` --
the same row pointer `Entity__MoodCue98` also reaches.

## The C

```c
void Entity__MoodCue117(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, SCALE_SIX);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue117`: the function's address sits in `gEntityMoodHandlerTable`
row 117 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.
