# Viewport__SetFogNear — MATCHED

> Renamed from `Unk18Obj__SetFogNear` on 2026-09-25 (tools/rename.py). Address 0x8003eac4.

> Renamed from `Unk18Obj__SetUnk60` on 2026-09-23 (tools/rename.py). Address 0x8003eac4.

> Renamed from `func_8003EAC4` on 2026-09-23 (tools/rename.py). Address 0x8003eac4.

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetFogNear(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x06C` slot occupant.

## What it does

One-instruction field setter, last of the `unk3C`/`unk40`/`unk54`/`unk60`
family.

```c
void Viewport__SetFogNear(Unk18Obj *self, s32 a1) {
    self->unk60 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk60` (`+0x060`).

## Naming

`Unk18Obj__SetFogNear` -- tier A. One-instruction plain field setter for `unk60`, no guard.

**Head review, round 73:** renamed to `Unk18Obj__SetFogNear` at merge, tier A: a plain setter of the `fogNear` field, whose name the runner established from its Sony consumer (`SetFogNear` in `Viewport__Update`/`Viewport__Flip`). Any line above saying the function name pre-dates the field rename is superseded.
