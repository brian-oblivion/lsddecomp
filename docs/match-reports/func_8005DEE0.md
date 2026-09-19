# func_8005DEE0

**Unit:** Entity_b · **Size:** 47 words · **Status:** MATCHED (47/47 words,
whole-image build verified byte-exact)

## What it does

Occupies `EntityMethods` slot `+0x17C` (dispatched by `Entity__Update` in
`Entity.c` as `this->methods->slot17C(this)`, already typed `s32` in the
header before this round). Early-returns `this->unkF8` unless
`this->unkF0 != 0 && this->unkF8 == 0 && this->unk44 != 1`; otherwise looks up
`gEntityMoodTable[this->moodIndex]` and, if `row->unkB != 0`, computes a distance
from `row->unkB` (absolute value), calls `Entity__IsNearTarget(this,
&this->unk14->x, dist, row->unk9)`, and dispatches
`this->methods->slot168(this)` if that returned non-zero. Always returns
`this->unkF8`.

Same overall shape as its sibling `func_8005DE18`, but note `row->unkB` is a
genuinely SEPARATE `EntityMoodRow` field from `func_8005DE18`'s `row->unk6` —
different offset (`+0xB` vs `+0x6`), not the same byte reinterpreted.

## Final C

```c
s32 func_8005DEE0(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    if (this->unkF0 != 0 && this->unkF8 == 0 && this->unk44 != 1) {
        row = &gEntityMoodTable[this->moodIndex];
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

Written immediately after `func_8005DE18` established the shape; needed the
exact same two fixes carried over directly:

1. The `s32`-not-`s8` retype of `Entity__IsNearTarget`'s parameters
   (`include/Entity.h`), and
2. The dedicated `xptr` local for `&this->unk14->x`, assigned in the same
   statement position retail computes it (right after entering the
   `row->unkB != 0` block, before the sign-check on `dist`).

No new residues beyond those two — matched on the attempt right after
applying both.

## Proposed learning

See `func_8005DE18`'s report for the two levers (`Entity__IsNearTarget`'s real
parameter width, and the "eager pointer into its own local, in statement
order" scheduling lever) — both generalized cleanly to this sibling with zero
adaptation needed.
