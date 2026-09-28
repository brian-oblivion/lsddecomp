# Entity__UpdateSoundCueStart

> Renamed from `func_8005DEE0` on 2026-09-23 (tools/rename.py). Address 0x8005dee0.

**Unit:** Entity · **Size:** 47 words · **Status:** MATCHED (47/47 words,
whole-image build verified byte-exact)

## What it does

Occupies `EntityMethods` slot `+0x17C` (dispatched by `Entity__Update` in
`Entity.c` as `this->methods->slot17C(this)`, already typed `s32` in the
header before this round). Early-returns `this->unkF8` unless
`this->unkF0 != 0 && this->unkF8 == 0 && this->unk44 != 1`; otherwise looks up
`sEntityMoodTable[this->moodIndex]` and, if `row->unkB != 0`, computes a distance
from `row->unkB` (absolute value), calls `Entity__IsNearTarget(this,
&this->unk14->x, dist, row->unk9)`, and dispatches
`this->methods->slot168(this)` if that returned non-zero. Always returns
`this->unkF8`.

Same overall shape as its sibling `Entity__UpdateTargetProximity`, but note `row->unkB` is a
genuinely SEPARATE `EntityMoodRow` field from `Entity__UpdateTargetProximity`'s `row->unk6` —
different offset (`+0xB` vs `+0x6`), not the same byte reinterpreted.

## Final C

```c
s32 Entity__UpdateSoundCueStart(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    if (this->unkF0 != 0 && this->unkF8 == 0 && this->unk44 != 1) {
        row = &sEntityMoodTable[this->moodIndex];
        if (row->unkB != 0) {
            xptr = &this->unk14->x;
            dist = row->unkB;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) != 0) {
                this->methods->slot168(this);
            }
        }
    }
    return this->unkF8;
}
```

## Attempt log

Written immediately after `Entity__UpdateTargetProximity` established the shape; needed the
exact same two fixes carried over directly:

1. The `s32`-not-`s8` retype of `Entity__IsNearTarget`'s parameters
   (`include/Entity.h`), and
2. The dedicated `xptr` local for `&this->unk14->x`, assigned in the same
   statement position retail computes it (right after entering the
   `row->unkB != 0` block, before the sign-check on `dist`).

No new residues beyond those two — matched on the attempt right after
applying both.

## Proposed learning

See `Entity__UpdateTargetProximity`'s report for the two levers (`Entity__IsNearTarget`'s real
parameter width, and the "eager pointer into its own local, in statement
order" scheduling lever) — both generalized cleanly to this sibling with zero
adaptation needed.

## Naming

`Entity__UpdateSoundCueStart` -- tier A (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005DEE0`.

gEntityMethods +0x17C; `Entity__Update` calls it every tick and runs slot +0x180 (`Entity__UpdateSoundCueStop`) when it returns non-zero. The body does exactly what the name says: while `active`, no cue running (`soundCueActive == 0`) and `unk44 != 1`, if the row's `cueRange` is non-zero and the target is within `|cueRange|` (`Entity__IsNearTarget`), it calls `startSoundCue` (+0x168, `Entity__StartSoundCue`). It returns `soundCueActive`. It is the start half of a pair, like `Entity__UpdateActivationState`/`DeactivationState` at +0x170/+0x174.

Also renamed here: `EntityMoodRow::unkB` -> `cueRange` (compiler-checked: only this function and `Entity__UpdateSoundCueStop` access it).

## Proposed field names

| member | proposed | tier | evidence |
| --- | --- | --- | --- |
| `EntityMethods::slot17C` (+0x17C) | `updateSoundCueStart` | A | occupant is this function; only accessor is Entity__Update (Entity.c), so it is cross-unit |

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

Local `xptr` renamed `pos` (tier A, as in Entity__UpdateTargetProximity). `this->state != 1` is `ENTITY_STATE_DONE`: the cue does not auto-start for an entity whose handler has finished, the same test UpdateActivationState makes. `~dist + 1` is MATCHING (measured; see Entity__UpdateTargetProximity.md).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
