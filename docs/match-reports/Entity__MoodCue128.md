# Entity__MoodCue128 — MATCH (29/29 words)

> Renamed from `func_800654A0` on 2026-09-24 (tools/rename.py). Address 0x800654a0.

**Unit:** Entity_g · **Size:** 29 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `Class6B5CC__FaceTarget(this, this->unk94,
1, 0, 0)`, `slot48(this, 1, SCALE_THIRTY_SECOND)`, `slotC4(this, -0x1E, 1)`.

## The C

```c
void Entity__MoodCue128(Entity *this, EntityMoodHandlerArg *out) {
    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    this->methods->slot48(this, 1, SCALE_THIRTY_SECOND);
    this->methods->slotC4(this, -0x1E, 1);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.
