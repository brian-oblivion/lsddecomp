# Unk18Obj__SetUnk48 — MATCHED

> Renamed from `func_8003EA48` on 2026-09-23 (tools/rename.py). Address 0x8003ea48.

Unit: `code_2cc8c_d`. Round 14, runner delta. 7/7 words, full match.

## Signature

```c
void Unk18Obj__SetUnk48(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x050` slot occupant.

## What it does

Same guarded-setter shape as `Unk18Obj__SetUnk44`, writing `unk48` instead of
`unk44`, under the same `self->unk70` latch.

```c
void Unk18Obj__SetUnk48(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk48 = a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk48` (`+0x048`), part of the
same carve pass as `Unk18Obj__SetUnk44`.

## Naming

`Unk18Obj__SetUnk48` -- tier A. Sibling of `Unk18Obj__SetUnk44`: same `unk70`-guarded setter shape, writes `unk48`.
