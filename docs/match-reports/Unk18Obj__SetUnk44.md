# Unk18Obj__SetUnk44 — MATCHED

> Renamed from `func_8003EA2C` on 2026-09-23 (tools/rename.py). Address 0x8003ea2c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 7/7 words, full match.

## Signature

```c
void Unk18Obj__SetUnk44(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x04C` slot occupant.

## What it does

Writes `unk44` only the first time — guarded by `self->unk70`, a latch this
unit's queue never itself sets (its own setter, if any, lies outside this
carve).

```c
void Unk18Obj__SetUnk44(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk44 = a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk44` (`+0x044`) and `unk70`
(`+0x070`, the guard flag; no evidence of its own set site within this
unit). Same carve pass as `Unk18Obj__SetUnk3C`/`EA48`/`EA64`/`EA7C`/`EAC4`
(`+0x03C..+0x060` span), all typed `s32` for lack of further evidence
beyond "a stored word".

## Naming

`Unk18Obj__SetUnk44` -- tier A. Plain setter for `unk44`, guarded by the `unk70` one-time-init latch (only writes the first time). The guard is part of the mechanics; the field's real meaning is unestablished.
