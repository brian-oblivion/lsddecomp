# Unk18Obj__SetUnkB8 — MATCHED

> Renamed from `func_8003F244` on 2026-09-23 (tools/rename.py). Address 0x8003f244.

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Unk18Obj__SetUnkB8(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x0B4` slot occupant.

## What it does

One-instruction field setter, sibling of `Unk18Obj__SetUnkB4`.

```c
void Unk18Obj__SetUnkB8(Unk18Obj *self, s32 a1) {
    self->unkB8 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unkB8` (`+0x0B8`).

## Naming

`Unk18Obj__SetUnkB8` -- tier A. One-instruction plain field setter for `unkB8`, no guard.
