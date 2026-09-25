# Viewport__SetClearColor — MATCHED

> Renamed from `Unk18Obj__SetClearColor` on 2026-09-25 (tools/rename.py). Address 0x8003ea84.

> Renamed from `Unk18Obj__SetUnk58` on 2026-09-23 (tools/rename.py). Address 0x8003ea84.

> Renamed from `func_8003EA84` on 2026-09-23 (tools/rename.py). Address 0x8003ea84.

Unit: `code_2cc8c_d`. Round 14, runner delta. 8/8 words, full match (2 real
attempts).

## Signature

```c
void Viewport__SetClearColor(Unk18Obj *self, SByte3_d294 *src);
```

`Unk18ObjMethods`'s own `+0x064` slot occupant.

## What it does

Copies a 3-byte record from `src` into `self->unk58`, as one whole-struct
assignment.

```c
void Viewport__SetClearColor(Unk18Obj *self, SByte3_d294 *src) {
    self->unk58 = *src;
}
```

## Two things the first attempt got wrong

1. **The bytes are signed.** A first attempt read `src[0..2]` through a
   `u8 *` (three `lbu` loads); retail uses `lb` (signed). The source really
   is a pointer to signed bytes.
2. **It's ONE struct assignment, not three sequential scalar ones.** Retail
   loads all three source bytes into three separate registers BEFORE
   storing any of them (`lb v0,0(a1); lb v1,1(a1); lb a2,2(a1); sb v0,...;
   sb v1,...; sb a2,...`) — a first attempt writing `self->unk58[0] =
   src[0]; self->unk58[1] = src[1]; ...` compiled to interleaved
   load/store pairs instead, one word short per field. New type
   `SByte3_d294` (`{ s8 b0, b1, b2; }`) makes the single `self->unk58 =
   *src;` reproduce the load-all-then-store-all shape.

## Header changes

`include/code_2cc8c.h`: new `SByte3_d294` type; `Unk18Obj` gains `unk58`
(`+0x058`, `SByte3_d294`) and its sibling `unk5B` (`+0x05B`, see
`Viewport__SetFarColor`'s report).

## Proposed learning

A 3-(or other odd-sized-)element copy where retail loads every source
element before storing any of them is a whole-struct-assignment tell, same
family as the already-confirmed "all-`s16` struct compiles to `lwl`/`lwr`"
idiom — but this one doesn't need `lwl`/`lwr` at all (3 plain byte
ops); the tell here is purely the LOAD-then-STORE ORDERING, not the
instruction form. Worth checking register order (not just opcode choice)
when a small fixed-size field-by-field copy doesn't match.

## Naming

`Unk18Obj__SetClearColor` -- tier A. Whole-struct-assignment setter for `unk58` (a 3-signed-byte `SByte3_d294`).

**Head review, round 73:** renamed to `Unk18Obj__SetClearColor` at merge, tier A: a plain setter of the `clearColor` field, whose name the runner established from its Sony consumer (`GsSortClear` in `Viewport__Update`/`Viewport__Flip`). Any line above saying the function name pre-dates the field rename is superseded.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetClearColor`. Slot +0x064 `setClearColor`; the colour is a `ViewportRgb *` (the former SByte3_d294). The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
