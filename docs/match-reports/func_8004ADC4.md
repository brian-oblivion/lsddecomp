# func_8004ADC4

**Unit:** class_3ac78 · **Size:** 3 words · **Status:** MATCHED (3/3 words)

## What it does

`Class866E8`'s slot +0x0C8 setter: stores its two arguments into
`self->unk60` and `self->unk64`.

## Derivation

```
sw $a1, 0x60($a0)
jr $ra
 sw $a2, 0x64($a0)
```

A two-field setter, both `s32` (plain `sw`, no shift/sign-extend). Field
offsets and the `Class866E8` type come from `include/class_3ac78.h`
(established this round; see `func_8004A478.md` for how the class was
identified via `tools/classtable.py D_800866E8`).

## Proposed learning

None beyond what's already documented for `Class866E8` in `func_8004A478.md`.
