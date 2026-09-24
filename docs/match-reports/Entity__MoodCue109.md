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
