# Viewport__SetOtLength — MATCHED

> Renamed from `Unk18Obj__SetUnk3C` on 2026-09-25 (tools/rename.py). Address 0x8003ea24.

> Renamed from `func_8003EA24` on 2026-09-23 (tools/rename.py). Address 0x8003ea24.

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetOtLength(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x048` slot occupant (`tools/classtable.py gViewportMethods`).

## What it does

A one-instruction field setter — the whole body is `sw $a1, 0x3C($a0)`.

```c
void Viewport__SetOtLength(Unk18Obj *self, s32 a1) {
    self->unk3C = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `s32 unk3C` at `+0x03C` (splits the
`pad034[0x0AC-0x034]` span). See `Viewport__SetUnk44`'s report for the sibling
setters carved from the same span in one pass.

## Naming

`Unk18Obj__SetUnk3C` -- tier A. One-instruction (`sw $a1, 0x3C($a0)`) plain field setter, no guard.
