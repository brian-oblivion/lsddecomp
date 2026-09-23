# Unk18Obj__SetLightMode — MATCHED

> Renamed from `Unk18Obj__SetUnk54` on 2026-09-23 (tools/rename.py). Address 0x8003ea7c.

> Renamed from `func_8003EA7C` on 2026-09-23 (tools/rename.py). Address 0x8003ea7c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Unk18Obj__SetLightMode(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x060` slot occupant.

## What it does

One-instruction field setter.

```c
void Unk18Obj__SetLightMode(Unk18Obj *self, s32 a1) {
    self->unk54 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk54` (`+0x054`).

## Naming

`Unk18Obj__SetLightMode` -- tier A. One-instruction plain field setter for `unk54`, no guard.

**Head review, round 73:** renamed to `Unk18Obj__SetLightMode` at merge, tier A: a plain setter of the `lightMode` field, whose name the runner established from its Sony consumer (`GsSetLightMode` in `Unk18Obj__Update`/`Unk18Obj__Flip`). Any line above saying the function name pre-dates the field rename is superseded.
