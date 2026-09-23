# Class6B5CC__RemoveAllChildren

> Renamed from `func_8001CD20` on 2026-09-23 (tools/rename.py). Address 0x8001cd20.

**Unit:** code_d294 · **Size:** 16 words · **Status:** MATCHED (16/16 words)

## What it does

`Class6B5CC` vtable slot `+0x018`, the last of the five BasicClass
overrides. Calls `Class6B5CC__UnlinkModel(self)` (zeroes `self->unk18`/`unk20`),
then unconditionally forwards to the base class's own `+0x018` slot.

## The C

```c
void Class6B5CC__RemoveAllChildren(Class6B5CCObj *self) {
    Class6B5CC__UnlinkModel(self);
    Get_vtable_BasicClass()->slot18(self);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. Matched on the first build.
`Class6B5CC__UnlinkModel`'s own (measured) body is what confirmed
`Class6B5CCObj::unk18`/`unk20` independently of the ctor's own zeroing —
see `include/code_d294.h`.

## Naming

Round 71 (alpha). `func_8001CD20` -> `Class6B5CC__RemoveAllChildren`, **tier A**. Overrides BasicClass slot +0x018 `removeAllChildren`: Class6B5CC__UnlinkModel, then the base implementation.
