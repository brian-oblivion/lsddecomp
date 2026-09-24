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
