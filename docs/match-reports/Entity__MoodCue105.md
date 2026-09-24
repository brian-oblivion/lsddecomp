# Entity__MoodCue105 — MATCH (27/27 words)

> Renamed from `func_80064B14` on 2026-09-24 (tools/rename.py). Address 0x80064b14.

**Unit:** Entity_g · **Size:** 27 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `this->methods->slot60(this,
rand() % 20 == 0)` -- the plain magic-number `% 20` idiom (mult by
`0x66666667`, `sra 3`, sign-fix, rebuild `*20`, subtract), not hand-expanded.

## The C

```c
void Entity__MoodCue105(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot60(this, rand() % 20 == 0);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.
