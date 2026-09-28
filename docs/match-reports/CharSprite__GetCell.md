# CharSprite__GetCell -- MATCHED (5/5 words), round 82

> Renamed from `D8006EC74__GetCell` on 2026-09-26 (tools/rename.py). Address 0x80041c28.

> Renamed from `func_80041C28` on 2026-09-25 (tools/rename.py). Address 0x80041c28.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gCharSpriteMethods slot +0x0C8 (`tools/classtable.py`).
- **What:** Returns the byte at +0x0A8. Retail opens and closes a 0x10-byte frame around a single `lbu`.
- **Result:** byte-exact; 5/5 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK).
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Lever

The frame comes from an UNUSED local `u8 pad[16];`. Measured: deleting that line gives 0/5 (2 insertions / 2 deletions, frame gone, image shifts); with it, byte-exact.

## Source

```c
/* CharSprite slot +0x0C8: read the byte at +0x0A8. */
u8 CharSprite__GetCell(CharSprite *self) {
    u8 pad[16]; /* unused: it is what gives retail its 0x10-byte frame */

    return self->cellIndex;
}
```

### Proposed learning

`addiu $sp,-0x10` ... `addiu $sp,+0x10` around a leaf body with no stack access at all is an unused local array (`u8 pad[16];`): GCC 2.6.3 -O2 still reserves its frame. Measured both ways here.

## Naming

- `D8006EC74__GetCell` -- tier A. Slot +0x0C8: returns the stored cell index (self->cellIndex). Pure getter.

## Track 4

2026-09-26, round 86 (bravo): class 0x1144 unified as CharSprite in `include/char_sprite.h`. Renamed from `D8006EC74__GetCell`, tier A: slot +0x0C8, named `getCell` in the header (returns `u8`: this occupant's type; no C call through the slot exists). `self` is `CharSprite *` (was `D_8006EC74Obj`). gTextRowMethods overrides the slot with an empty `TextRow__NoOpGetCell` that its own view types as a void setter; that is the subclass's to settle. The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

The source comment on the unused `u8 pad[16]` read "unused: it is what gives retail its 0x10-byte frame"; it is now a `MATCHING` line. The 16 stays a literal: it is the frame size, and a name would only restate it. Byte-exact.
