# Entity__MoodCue109 — MATCH (23/23 words)

> Renamed from `func_80064CEC` on 2026-09-24 (tools/rename.py). Address 0x80064cec.

**Unit:** Entity_g · **Size:** 23 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `slot48(this, 1, SCALE_HALF)` then
`slotC4(this, -0xA, 0)`.

## The C

```c
void Entity__MoodCue109(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, SCALE_HALF);
    this->methods->slotC4(this, -0xA, 0);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.


## Naming

Why `MoodCue109`: the function's address sits in `gEntityMoodHandlerTable`
row 109 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.
