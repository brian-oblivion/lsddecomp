# Entity__MoodCue113 — MATCH (8/8 words)

> Renamed from `func_800650D4` on 2026-09-24 (tools/rename.py). Address 0x800650d4.

**Unit:** Entity_g · **Size:** 8 instructions

## Blocker screen

No hits.

## What it does

A `gEntityMoodHandlerTable` handler row that forwards straight to another handler,
`Entity__MoodCue51` (already matched, `Entity_d.c`), passing `(this, out)`
through unchanged.

## The C

```c
void Entity__MoodCue113(Entity *this, EntityMoodHandlerArg *out) {
    Entity__MoodCue51(this, out);
}
```

Matched on the first build. Adds an `include/Entity.h` extern for
`Entity__MoodCue51` (first cross-unit caller).

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue113`: the function's address sits in `gEntityMoodHandlerTable`
row 113 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.
