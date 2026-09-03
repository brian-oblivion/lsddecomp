# func_80064CEC — MATCH (23/23 words)

**Unit:** Entity_g · **Size:** 23 instructions

## Blocker screen

No hits.

## What it does

`D_80089EB0` handler row; `out` unused. `slot48(this, 1, D_80089DD8)` then
`slotC4(this, -0xA, 0)`.

## The C

```c
void func_80064CEC(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089DD8);
    this->methods->slotC4(this, -0xA, 0);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g.
