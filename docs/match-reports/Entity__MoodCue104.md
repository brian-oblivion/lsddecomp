# Entity__MoodCue104 — MATCH (28/28 words)

> Renamed from `func_80064AA4` on 2026-09-24 (tools/rename.py). Address 0x80064aa4.

**Unit:** Entity_g · **Size:** 28 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `this->methods->slot48(this, 1,
D_80089DCC)` unconditionally, then `this->methods->slotCC(this, -0x20, 0)`
when `this->unkFC` falls in `[0xC9, 0x12C)` (an unsigned-subtract range
check, `(u32)(this->unkFC - 0xC9) < 0x63`).

## The C

```c
void Entity__MoodCue104(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089DCC);
    if ((u32)(this->unkFC - 0xC9) < 0x63) {
        this->methods->slotCC(this, -0x20, 0);
    }
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.
