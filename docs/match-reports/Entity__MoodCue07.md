# Entity__MoodCue07

> Renamed from `func_8005E4D0` on 2026-09-23 (tools/rename.py). Address 0x8005e4d0.

**Unit:** Entity_b · **Size:** 113 words · **Status:** MATCHED (113/113
words, whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. A larger member of this
unit's mood-dispatch handler family:

1. `out->unk10 = this->methods->slot148(this);` (the usual opener).
2. If `this->unk84 == this->unk80 / 2`, sets four `out` fields
   (`unk1C=0x7`, `unk20=-0x2`, `unk30=0x3`, `unk34=-0x2`).
3. If `out->unk4 % 90 < 3`, sets `out->unk44 = 0x6` and `out->unk48 = -0x1`.
4. Dispatches on `this->unkFC`:
   - `>= 0x79` (121): `this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);` then
     `this->methods->slotC4(this, -0x140, 0);`
   - `< 0x79` and (`>= 0x38` (56) or `Entity__IsNearTarget(this, &this->unk14->x, 1,
     1) != 0`): `this->methods->slotBC(this, TRANSLATE_Y_MINUS64);`
   - `< 0x38` and `Entity__IsNearTarget(...) == 0`, sub-dispatch on `this->unkFC`
     again: `>= 0xA` (10) calls `Class6B5CC__FaceTarget(...)` then
     `this->methods->slotC4(this, -0x100, 0)`; `< 0xA` calls only
     `Class6B5CC__FaceTarget(...)`.

The magic-multiply constant `0xB60B60B7` at shift 6, reconstructed by
retail's own multiply-back sequence (`*3`, `*15` via `<<4` minus itself,
`*2` = `*90`), is division by 90 — writing `out->unk4 % 90` reproduces it
exactly with no manual constant derivation needed.

## New fields

- `EntityMethods::slot44`, `void (*)(Entity *self, s32 arg1, void *arg2)` —
  filled the last gap in the `0x40`..`0x48` slot run (`slot40`/`slot48`
  already existed either side of it).
- `EntityMoodHandlerArg::unk20`/`unk34`/`unk48` — each paired one word after
  an already-known field (`unk1C`/`unk30`/`unk44` respectively), splitting
  the existing padding runs.
- `ROTATION_YAW_PLUS2` — a fourth `D_8008xxxx` opaque data row, same convention as
  `SCALE_HALF`/`SCALE_DOUBLE`/`TRANSLATE_Y_MINUS64` already declared at the top of this
  file.

## Final C

```c
void Entity__MoodCue07(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == this->unk80 / 2) {
        out->unk1C = 0x7;
        out->unk20 = -0x2;
        out->unk30 = 0x3;
        out->unk34 = -0x2;
    }
    if (out->unk4 % 90 < 3) {
        out->unk44 = 0x6;
        out->unk48 = -0x1;
    }
    if (this->unkFC >= 0x79) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
        this->methods->slotC4(this, -0x140, 0);
    } else if (this->unkFC >= 0x38 ||
               Entity__IsNearTarget(this, &this->unk14->x, 1, 1) != 0) {
        this->methods->slotBC(this, TRANSLATE_Y_MINUS64);
    } else if (this->unkFC >= 0xA) {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        this->methods->slotC4(this, -0x100, 0);
    } else {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    }
}
```

Note the duplicated `Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);` call in the
last two arms — retail's own bytes call it identically in both, one arm just
has an extra `slotC4` call afterward. Not a shared/hoisted call: written as
two literal statements, matching retail's own (redundant-looking but
byte-real) duplication.

## Attempt log

Two attempts. First attempt wrote the top-level dispatch as `if (unkFC <
0x79) { <0x79 body } else { slot44/slotC4 }` (the more "natural" reading
order, `<0x79` case first) — 59/113, diverging right at that branch. Reading
the raw disassembly showed retail places the `>= 0x79` body as the
PHYSICAL FALL-THROUGH (no jump) and reaches the `< 0x79` body via an
explicit forward branch — the same "GCC lays the WRITTEN condition's true
branch as fall-through" shape from `Entity__MoodCue13.md` in this same round.
Swapping to test `>= 0x79` FIRST (as the primary `if`, with the `< 0x79`
logic as the `else`) matched immediately, with every inner branch unchanged.

## Proposed learning

A second confirming instance of `Entity__MoodCue13.md`'s finding, now definitely
a pattern rather than a one-off: **when a residue is "right content, wrong
physical position, otherwise byte-identical," try testing the OPPOSITE
condition as the primary `if`** — GCC 2.6.3 consistently lays the written
condition's true-branch body as the fall-through and the `else` as a jump
target, so which comparison is written first controls which body sits where
in the instruction stream, even though both spellings are semantically
identical C.
