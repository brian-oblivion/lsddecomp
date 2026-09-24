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
