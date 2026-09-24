# Entity__MoodCue118 — MATCH (13/13 words)

> Renamed from `func_800651D0` on 2026-09-24 (tools/rename.py). Address 0x800651d0.

**Unit:** Entity_g · **Size:** 13 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. Byte-identical body to
`Entity__MoodCue117`: `slot48(this, 1, SCALE_SIX)`.

## The C

```c
void Entity__MoodCue118(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, SCALE_SIX);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue118`: the function's address sits in `gEntityMoodHandlerTable`
row 118 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.
