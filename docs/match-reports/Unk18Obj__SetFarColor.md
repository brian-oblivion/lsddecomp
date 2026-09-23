# Unk18Obj__SetFarColor — MATCHED

> Renamed from `Unk18Obj__SetUnk5B` on 2026-09-23 (tools/rename.py). Address 0x8003eaa4.

> Renamed from `func_8003EAA4` on 2026-09-23 (tools/rename.py). Address 0x8003eaa4.

Unit: `code_2cc8c_d`. Round 14, runner delta. 8/8 words, full match.

## Signature

```c
void Unk18Obj__SetFarColor(Unk18Obj *self, SByte3_d294 *src);
```

`Unk18ObjMethods`'s own `+0x068` slot occupant.

## What it does

Sibling of `Unk18Obj__SetClearColor` (same report has the full derivation — signed
bytes, whole-struct assignment), writing `self->unk5B` instead of
`self->unk58`.

```c
void Unk18Obj__SetFarColor(Unk18Obj *self, SByte3_d294 *src) {
    self->unk5B = *src;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk5B` (`+0x05B`, `SByte3_d294`).

## Naming

`Unk18Obj__SetFarColor` -- tier A. Sibling of `Unk18Obj__SetClearColor`: same whole-struct-assignment shape, writes `unk5B`.

**Head review, round 73:** renamed to `Unk18Obj__SetFarColor` at merge, tier A: a plain setter of the `farColor` field, whose name the runner established from its Sony consumer (`SetFarColor` in `Unk18Obj__Update`/`Unk18Obj__Flip`). Any line above saying the function name pre-dates the field rename is superseded.
