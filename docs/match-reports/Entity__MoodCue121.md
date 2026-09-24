# Entity__MoodCue121 — MATCH (13/13 words)

> Renamed from `func_80065204` on 2026-09-24 (tools/rename.py). Address 0x80065204.

**Unit:** Entity_g · **Size:** 13 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `slot48(this, 1, D_80089DCC)`.

## The C

```c
void Entity__MoodCue121(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089DCC);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.
