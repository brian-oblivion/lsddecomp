# Entity__MoodCue108 — MATCH (18/18 words)

> Renamed from `func_80064CA4` on 2026-09-24 (tools/rename.py). Address 0x80064ca4.

**Unit:** Entity_g · **Size:** 18 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row that forwards straight to another handler,
`Entity__MoodCue71` (already matched, `Entity_e.c`), passing its own `(this,
out)` through unchanged, then dispatches `slot48(this, 1, SCALE_SIX)`.

## The C

```c
void Entity__MoodCue108(Entity *this, EntityMoodHandlerArg *out) {
    Entity__MoodCue71(this, out);
    this->methods->slot48(this, 1, SCALE_SIX);
}
```

Matched on the first build.

## Header note

Added an extern for `Entity__MoodCue71` (already matched, `Entity_e.c`) to
`include/Entity.h`, at the bottom after `EntityMoodHandlerArg`'s own
definition -- needed there rather than earlier since the type isn't
`typedef`'d until that struct. First cross-unit caller of that function.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.
