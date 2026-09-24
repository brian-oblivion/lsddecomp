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
