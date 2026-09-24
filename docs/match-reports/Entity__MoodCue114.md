# Entity__MoodCue114 — MATCH (42/42 words)

> Renamed from `func_800650F4` on 2026-09-24 (tools/rename.py). Address 0x800650f4.

**Unit:** Entity_g · **Size:** 42 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. Returns immediately on the first
`rand() % 3 == 0`. Otherwise dispatches `slot44(this, 0, a2)` where `a2` is
`ROTATION_YAW_MINUS9` when a SECOND `rand() % 3 == 0`, else `ROTATION_YAW_PLUS9`.

## The C

```c
void Entity__MoodCue114(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (rand() % 3 == 0) {
        return;
    }
    if (rand() % 3 != 0) {
        a2 = ROTATION_YAW_PLUS9;
    } else {
        a2 = ROTATION_YAW_MINUS9;
    }
    this->methods->slot44(this, 0, a2);
}
```

## Residue chased: swapped row pointers

First attempt swapped which `rand() % 3` outcome selects `ROTATION_YAW_MINUS9` vs
`ROTATION_YAW_PLUS9` (also flipping a `beq` to `bne`, a genuine control-flow
difference, not just a value swap) -- 39/42. Reading the `beq
v0,v1,L8006516C` (branch on remainder==0, i.e. `rand()%3==0`) against
which `a2` value each fallthrough/branch path sets closed it to 42/42 on
the second build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. 2 attempts.
