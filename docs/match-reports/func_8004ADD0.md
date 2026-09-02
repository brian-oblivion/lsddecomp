# func_8004ADD0

**Unit:** class_3ac78 · **Size:** 2 words · **Status:** MATCHED (2/2 words)

## What it does

`Class866E8`'s slot +0x0CC setter: stores its argument into `self->unkE8`.

## Derivation

```
jr $ra
 sw $a1, 0xE8($a0)
```

A one-field `s32` setter, leaf, tail instruction in the delay slot of `jr`.

## Proposed learning

None beyond what's already documented for `Class866E8` in `func_8004A478.md`.
