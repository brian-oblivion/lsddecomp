# func_8003EAA4 — MATCHED

Unit: `code_2cc8c_d`. Round 14, runner delta. 8/8 words, full match.

## Signature

```c
void func_8003EAA4(Unk18Obj *self, SByte3_d294 *src);
```

`Unk18ObjMethods`'s own `+0x068` slot occupant.

## What it does

Sibling of `func_8003EA84` (same report has the full derivation — signed
bytes, whole-struct assignment), writing `self->unk5B` instead of
`self->unk58`.

```c
void func_8003EAA4(Unk18Obj *self, SByte3_d294 *src) {
    self->unk5B = *src;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk5B` (`+0x05B`, `SByte3_d294`).
