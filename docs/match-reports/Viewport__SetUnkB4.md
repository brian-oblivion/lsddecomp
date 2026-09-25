# Viewport__SetUnkB4 — MATCHED

> Renamed from `Unk18Obj__SetUnkB4` on 2026-09-25 (tools/rename.py). Address 0x8003f23c.

> Renamed from `func_8003F23C` on 2026-09-23 (tools/rename.py). Address 0x8003f23c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetUnkB4(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x0B0` slot occupant.

## What it does

One-instruction field setter.

```c
void Viewport__SetUnkB4(Unk18Obj *self, s32 a1) {
    self->unkB4 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unkB4` (`+0x0B4`), after the
existing `unkB0` field.

## Naming

`Unk18Obj__SetUnkB4` -- tier A. One-instruction plain field setter for `unkB4`, no guard.
