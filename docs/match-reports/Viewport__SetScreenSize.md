# Viewport__SetScreenSize — MATCHED

> Renamed from `Unk18Obj__SetUnk34` on 2026-09-25 (tools/rename.py). Address 0x8003ea0c.

> Renamed from `func_8003EA0C` on 2026-09-23 (tools/rename.py). Address 0x8003ea0c.

Unit: `Task`. Round 14, runner delta. 6/6 words, full match (2
real attempts).

## Signature

```c
void Viewport__SetScreenSize(Unk18Obj *self, Pair32_d294 *pair);
```

`Unk18ObjMethods`'s own `+0x044` slot occupant.

## What it does

Copies a caller-supplied 2-word pair wholesale into `self->unk34`.

```c
void Viewport__SetScreenSize(Unk18Obj *self, Pair32_d294 *pair) {
    self->unk34 = *pair;
}
```

## What the first attempt got wrong

Same tell as `Viewport__SetClearColor`/`Viewport__SetFarColor` (this unit, this round):
retail loads BOTH source words into registers before storing either
(`lw v0,0(a1); lw v1,4(a1); sw v0,0x34(a0); sw v1,0x38(a0)`), which only a
whole-struct assignment reproduces. A first attempt writing two sequential
field assignments (`self->unk34 = pair->a; self->unk38 = pair->b;` — with
`unk34`/`unk38` as separate `s32` fields) compiled to interleaved
load/store pairs instead. Merged `unk34`/`unk38` into one `Pair32_d294
unk34` field so `self->unk34 = *pair;` is a single assignment.

## Header changes

`include/Task.h`: new `Pair32_d294` type (`{ s32 a, b; }`); `Unk18Obj`
gains `unk34` (`+0x034`, `Pair32_d294`, spanning what would have been
`+0x034`/`+0x038` as two scalars).

## Proposed learning

Third instance this unit of the same tell (after `SceneNode__GetRotMatrix`'s
all-`s16` `lwl`/`lwr` case last round and `Viewport__SetClearColor` earlier this
round): **when retail's disassembly loads every source field into a
register before storing any of them, the fix is a whole-struct assignment,
which usually means the DESTINATION fields need to be combined into one
struct field too** — not just casting the source pointer to a struct type
while leaving two separate scalar destination fields, which still compiles
to sequential per-field code.

## Naming

`Unk18Obj__SetUnk34` -- tier A. Whole-struct-assignment setter for `unk34` (a `Pair32_d294`); one instruction group, no guard, no other effect. Field's own real meaning is not established (kept `unk34`, not renamed -- see `## Proposed field names`), so the function is named after its mechanics only.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnk34`. Renamed for the field it stores: +0x034 is `screenSize` (width, height), which Viewport__DrawNode reads as the screen size its box and screen-space sprite paths take percentages of (DrawView's `width`/`height`). Slot +0x044 `setScreenSize`; the parameter is a `ViewportSize *` (the former Pair32_d294, same two words, same lw,lw,sw,sw copy). The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
