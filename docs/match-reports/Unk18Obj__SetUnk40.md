# Unk18Obj__SetUnk40 — MATCHED

> Renamed from `func_8003EA64` on 2026-09-23 (tools/rename.py). Address 0x8003ea64.

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Unk18Obj__SetUnk40(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x054` slot occupant.

## What it does

One-instruction field setter, the sibling of `Unk18Obj__SetUnk3C`.

```c
void Unk18Obj__SetUnk40(Unk18Obj *self, s32 a1) {
    self->unk40 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk40` (`+0x040`).
