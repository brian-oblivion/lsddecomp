# StageMap__ApplyToSenderFootprint — MATCH

> Renamed from `Class866E8__ApplyToSenderFootprint` on 2026-09-26 (tools/rename.py). Address 0x8004aea4.

> Renamed from `func_8004AEA4` on 2026-09-22 (tools/rename.py). Address 0x8004aea4.

**Unit:** class_3ac78 · **Size:** 79 instructions · **Result:** 79/79 words

## What it does

`StageMapMethods` slot `+0x12C`, dispatched from `StageMap__ForwardAcceptedCommand`'s
`slotD0` chain. Computes a "gate" argument (`0` normally, or
`(u8*)arg1->unk14 + 0x38` when `arg1->unk0C != 0`) and passes it to
`self->methods->slot110(self, &buf, gate)`; if that call returns nonzero,
returns immediately. Otherwise: saves `self->unk88` and the whole 0x30-byte
`self->unk8C` block; calls either `StageMap__SetFootprintFromCell(self, &buf, 3)` or
`StageMap__SetFootprintRect(self, &buf, 3)` depending on whether
`self->unk68->unk4 == 0`; calls `StageMap__DispatchToRectCells(self, arg1, arg2)`; then
restores `self->unk88` and `self->unk8C` from the saved copies.

## Final source

```c
extern void StageMap__SetFootprintFromCell(StageMap *self, UnkArgObj_3ac78 *arg1, s32 arg2);
extern void StageMap__SetFootprintRect(StageMap *self, UnkArgObj_3ac78 *arg1, s32 arg2);
extern void StageMap__DispatchToRectCells(StageMap *self, UnkListObj_3ac78 *arg1, s32 arg2);

void StageMap__ApplyToSenderFootprint(StageMap *self, UnkListObj_3ac78 *arg1, s32 arg2)
{
    s32 gateArg;
    HistoryBlock_3ac78 saved;
    UnkArgObj_3ac78 buf;
    s32 savedUnk88;

    if (arg1->unk0C != 0) {
        gateArg = (s32)((u8 *)arg1->unk14 + 0x38);
    } else {
        gateArg = 0;
    }

    if (self->methods->slot110(self, &buf, gateArg) != 0) {
        return;
    }

    savedUnk88 = self->unk88;
    saved = self->unk8C;

    if (self->unk68->unk4 == 0) {
        StageMap__SetFootprintFromCell(self, &buf, 3);
    } else {
        StageMap__SetFootprintRect(self, &buf, 3);
    }

    StageMap__DispatchToRectCells(self, arg1, arg2);

    self->unk88 = savedUnk88;
    self->unk8C = saved;
}
```

`StageMap__SetFootprintFromCell` and `StageMap__SetFootprintRect` are both defined later in this same
unit/file (matched and STALLED respectively) — the `extern` prototypes
above are ordinary forward declarations, needed only because ROM address
order puts `StageMap__ApplyToSenderFootprint` before both of them in `src/class_3ac78.c`.
`StageMap__DispatchToRectCells` (117 instructions, out of this round's scope) gets the
same treatment per DECOMPILATION_LEARNINGS' "calling into a function
still `INCLUDE_ASM` elsewhere is fine" precedent.

## Corrections this function forced to EARLIER struct knowledge

This function's evidence is stronger than `StageMap__SetFootprintRect`'s (which never
closed — see its own STALL report), and it directly contradicts two
fields that stalled attempt tentatively introduced. Both are now fixed in
`include/class_3ac78.h`, and `StageMap__SetFootprintRect.md` carries a matching
"Update" note per the round's instructions:

1. **`self->unk8C` is a 3-element, 0x30-byte block (`HistoryBlock_3ac78`
   wrapping `HistoryEntry_3ac78 e[3]`), not a standalone 4-byte pointer
   field.** `StageMap__SetFootprintRect`'s stalled attempt modeled `self->unk8C` as a
   single `void *` because that's the only field IT writes there. This
   function copies the WHOLE 0x30-byte region (`self+0x8C` to `self+0xBC`)
   with a batched 4-word-per-iteration loop on both the save and the
   restore side — the classic "whole-struct assignment, not an indexed
   loop" block-move signature already established by `Pad__LoadButtonTable`
   (see that report for the precedent). `StageMap__SetFootprintRect`'s single-pointer
   write is consistent with this as `unk8C.e[0].unk0` — the STALLED
   report's inline body is left as-is (it's preserved code, not doctrine)
   but the header field it depends on has moved.
2. **`self->unk68` is a pointer (`UnkPtr68Obj_3ac78 *`), not a plain
   `s32`.** This function dereferences it and reads a field at `+0x4`.
   `StageMap__SetConfig` (already matched) only ever STORES a raw word there and
   never dereferences it, so retyping its own parameter to the pointer
   type changes nothing about its compiled bytes — confirmed by rebuilding
   after the retype: still byte-exact, whole-image SHA1 still green.

Also extended `UnkListObj_3ac78` with `unk0C` (an `s32` gate flag) — safe,
it was previously undifferentiated padding that `StageMap__OnSlotEvent` (the
type's only other user) never touches.

## Residue and the fixes that closed it (70/79 -> 79/79 in one more attempt)

Two independent issues on the first attempt:

1. **The two stack locals landed in the wrong SLOTS relative to each
   other.** `buf` (`UnkArgObj_3ac78`, passed to `slot110`/`StageMap__SetFootprintFromCell`/
   `StageMap__SetFootprintRect`) needs to sit at `sp+0x40`; the save/restore
   `HistoryBlock_3ac78` needs `sp+0x10`. Declaring `buf` textually FIRST
   put it at `sp+0x10` instead. Swapping the declaration order (history
   block first, `buf` second) flipped the stack assignment to match.
2. **Branch polarity on the `unk68->unk4` dispatch.** Retail's actual
   instruction is `bnez $v0,ALT` where the FALLTHROUGH (untaken) path
   calls `StageMap__SetFootprintFromCell` and the branch target calls `StageMap__SetFootprintRect` — the
   fallthrough is the `!= 0`-is-false case. Writing the natural-reading
   `if (self->unk68->unk4 != 0) { StageMap__SetFootprintRect(...); } else {
   StageMap__SetFootprintFromCell(...); }` compiles to the OPPOSITE polarity (`beqz`,
   `StageMap__SetFootprintRect` as fallthrough) because GCC's default `if`/`else`
   lowering treats the `if` clause as the fallthrough branch. Inverting
   the source comparison (`== 0` calling `StageMap__SetFootprintFromCell` first) reproduced
   retail's actual branch sense.

### Proposed learning

**Local-variable stack SLOT ASSIGNMENT is sensitive to declaration order
in a way that isn't obviously the same as "GCC allocates biggest-first"
or any other simple rule** — here, reversing the order of two same-scope
locals (a small struct vs. a bigger one) flipped which one landed at the
lower stack offset. When two stack buffers both get passed to different
calls and their absolute SP offsets don't match retail, try reordering
their declarations before reaching for anything more invasive.

**`if (cond) A; else B;` and `if (!cond) B; else A;` are not
interchangeable at the byte level even though they're logically
identical** — GCC's default `if`/`else` codegen makes the FIRST clause the
fallthrough and the `else` the branch target, so which physical branch
instruction (`beqz` vs `bnez`) and which block is JUMPED TO vs. FALLS
THROUGH is entirely a function of how the condition is spelled, not just
what it evaluates to. When retail's branch-taken target and your
source's `if`-body don't match up, try inverting the comparison before
suspecting anything deeper.

## Provenance

round 2026-09-02 (head-requested extension), runner ALPHA, unit
class_3ac78. First attempt 70/79 (two independent, both diagnosed and
fixed); second attempt 79/79.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AEA4` | `StageMap__ApplyToSenderFootprint` | B | Occupant of vtable slot `+0x12C`, reached only from `StageMap__ForwardAcceptedCommand`. Body: derive a query argument from the sender, call `slot110` to resolve it into a descriptor, SAVE `rectCount` and the whole `rects` block, overwrite them with a single rectangle built from that descriptor (`StageMap__SetFootprintFromCell` or `StageMap__SetFootprintRect`, selected by `config->unk4`), run `StageMap__DispatchToRectCells` over it, then RESTORE both. The save/restore bracket is what makes "footprint" the right word: the rectangle list is borrowed for the duration of one sender's notification. Tier B -- the mechanism is complete, the game meaning is not. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x08C` | `rects` | A | Saved and restored as one 0x30-byte block here; written a rectangle at a time by `StageMap__SetFootprintRect`; read as a bounded array of grid rectangles by `StageMap__DispatchToRectCells` here and by `class_3bb8c_b`'s BYTE-MATCHED `StageMap__SetFootprintVisible`. |

Local types renamed this round: `HistoryEntry_3ac78` -> `GridRect_3ac78`,
`HistoryBlock_3ac78` -> `GridRectList_3ac78`. The "History" name was a
hypothesis from before the block's reader was understood; `StageMap__SetFootprintVisible`
(matched, `class_3bb8c_b`) reads the same 0xC bytes as
`{elemIdx, startCol, startRow, width, height}` and walks a grid rectangle with
them. The members' own names (`elemIdx`/`col`/`row`/`width`/`height`) were
already correct and are unchanged.

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).
