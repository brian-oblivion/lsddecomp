# Entity__MoodCue103 — MATCH (95/95 words)

> Renamed from `func_80064928` on 2026-09-24 (tools/rename.py). Address 0x80064928.

**Unit:** Entity_g · **Size:** 95 instructions

## Blocker screen

No `gp_rel`/`addiu_at`/`nop_mflo_mfhi` hits.

## What it does

`gEntityMoodHandlerTable` row 103. Takes the standard handler signature but, like
`Entity__MoodCue98`, never reads `out`. Two `rand() % N == 0` gates (`% 3` and
`% 5`, both the plain compiler-generated magic-number division idiom, not
hand-expanded) branch on `this->unkFC`/`this->unk44` state codes, dispatch
a few vtable calls, and end with the same
`this->methods->slotC4(this, -0x1E, 0)` tail shape as `Entity__MoodCue98`.

## The C

```c
void Entity__MoodCue103(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0x2BC) {
        if (rand() % 3 == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC < 0x3FC) {
            this->methods->slot44(this, 0, D_80089CB8);
            this->methods->slotCC(this, 0x1E, 0);
        }
        if (this->unkFC == 0x3A2) {
            this->methods->slot30(this, 0xA);
        }
    } else if (this->unkFC == 0x64 || this->unkFC == 0x320) {
        if (rand() % 5 == 0) {
            this->unk4C->methods->slot138(this->unk4C, 4, 0);
        }
    }
    this->methods->slotC4(this, -0x1E, 0);
}
```

Matched on the first build.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. Matched on the first
build. Adds `D_80089CB8` to this unit's local externs.
