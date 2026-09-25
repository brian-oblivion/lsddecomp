# Class6B5CC__RotateLocalVector -- MATCHED (29/29 words)

> Renamed from `func_8001E58C` on 2026-09-17 (tools/rename.py). Address 0x8001e58c.

Unit: `code_d294_c` (round 14). Calls a new `Class6B5CCMethods` slot
(`+0x084`) to fill a 0x20-byte stack buffer, copies a 3-element `s16`
source into `dst`'s leading three `Class6B5CCSub44` fields, then forwards
the buffer's leading 0xC bytes into `ApplyMatrixToLVArray` (this unit, also
matched this round).
`void Class6B5CC__RotateLocalVector(Class6B5CCObj *self, Class6B5CCSub44 *dst, s16 *src)`.

## Final source

```c
void Class6B5CC__RotateLocalVector(Class6B5CCObj *self, Class6B5CCSub44 *dst, s16 *src) {
    u8 buf[0x20];

    self->methods->slot84(self, buf, 0);
    dst->unk0 = src[0];
    dst->unk4 = src[1];
    dst->unk8 = src[2];
    ApplyMatrixToLVArray(dst, dst, 1, buf);
}
```

## New struct/extern knowledge (`include/code_d294.h`, additive)

- **`Class6B5CCMethods` gains `slot84`** (`void(Class6B5CCObj*, void*,
  s32)`), split out of the `pad060[0x094-0x060]` range this round
  established (now `pad060[0x084-0x060]` + `slot84` + `pad088[0x094-
  0x088]`). Confirmed against `tools/classtable.py gClass6B5CCMethods`: the
  occupant is `Class6B5CC__GetRotMatrix`, in `code_d294_b` (out of this carve's
  scope, not decompiled here).
- **New forward declaration for `ApplyMatrixToLVArray`** (this unit, matched
  separately this round) and a new opaque extern for `func_80015618`
  (a different, still-uncarved segment) that `ApplyMatrixToLVArray` calls.

No existing field was retyped or renamed.

## Derivation notes

- **The stack buffer's true size (0x20 bytes) is MEASURED from the
  caller's own frame, not from any known output shape of `slot84`.**
  Sizing it to `0xC` (just enough for the 3 words `ApplyMatrixToLVArray` reads
  back) compiled to a 0x30-byte frame; retail's is 0x40. The 0x10-byte gap
  only closes with a 0x20-byte buffer -- `slot84`'s real output shape
  past the leading 12 bytes is unknown (its occupant is out of this
  carve's scope), so the extra 20 bytes are opaque, unread-here output
  space, not evidence of a specific larger type.
- The `dst->unk0/unk4/unk8 = src[i]` triple is the same "three `s32`
  fields populated from a 3-element `s16` source" shape as
  `Class6B5CC__UpdateScale`/`Class6B5CC__UpdateRotation` (both of which populate the SAME
  `Class6B5CCSub44` fields via a fixed-point conversion instead) --
  here the source values are used directly, no `RatioToFixed12` call.
- First-try match once the buffer size was corrected; no register-order
  or CSE residue.

### Proposed learning

**When a callee's output buffer size is unknown, measure it from the
CALLER's own stack frame size rather than sizing it to only what the
caller's OWN subsequent code reads back.** A buffer sized to just the
bytes actually consumed compiles to a smaller, wrong-sized frame; the
frame's total size is direct evidence of the buffer's real (possibly
larger, partially-opaque) extent.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001E58C` -> `Class6B5CC__RotateLocalVector`. Tier B.** It calls
  `slot84(self, buf, 0)`, whose occupant is `Class6B5CC__GetRotMatrix` (code_d294_b,
  matched: `RotMatrix(&param->rotate, buf)` with the angles NOT negated when
  the 3rd argument is 0), then applies that matrix to a widened copy of its
  own 3-element `s16` argument. So "Rotate" is the operation and "Local" is
  the frame the input is in -- inferred from the flag-0 (un-negated) branch
  of `Class6B5CC__GetRotMatrix`, which is why this is B and not A.
- **Method prefix `Class6B5CC__`:** the first parameter is a
  `Class6B5CCObj *` and the body dispatches through its method table. The
  function is NOT itself a vtable slot (`tools/classtable.py gClass6B5CCMethods`
  ends at `Class6B5CC__NotifyTaggedParents`); the prefix records the receiver, matching
  `BasicClass__*` and `DreamSys__*` already in the symbols file.
- **Parameter `dst` retyped `Class6B5CCSub44 *` -> `Vec3_d294 *`.** Evidence:
  only three words at +0/+4/+8 are ever written, and `class_3bb8c_o`'s
  `Actor__AddLocalTranslation` -- the one external call site -- passes the address of a
  bare 3-word local (`Vec3O buf`). The old typing matched by offset
  coincidence with `GsCOORD2PARAM.scale`. Byte-identical after the retype.
