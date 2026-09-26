# StageMap__ClearSlotCells

> Renamed from `StageMap__ResetElementCells` on 2026-09-26 (tools/rename.py). Address 0x8004c0ac.

> Renamed from `Class866E8__ResetElementCells` on 2026-09-26 (tools/rename.py). Address 0x8004c0ac.

> Renamed from `func_8004C0AC` on 2026-09-24 (tools/rename.py). Address 0x8004c0ac.

**Unit:** class_3bb8c · **Size:** 43 words · **Status:** MATCHED (5 attempts).

## Result

```c
void StageMap__ClearSlotCells(Obj866E8 *self, Elem *entry) {
    EntryChildObj **p;
    EntryChildObj **end;

    if (entry->unk4->unk30 >= 0) {
        entry->unk4->methods->slot7C(entry->unk4, entry);
        end = entry->unk10 + 0x19A;
        for (p = entry->unk10; p < end; p++) {
            (*p)->unk10 |= 0x80000000;
            (*p)->unk20 = 0;
            (*p)->unk18 = 0;
        }
    }
}
```

## Derivation

`entry` is one of `Obj866E8::arr[7]`'s own elements (class_3ac78's
independent view of the same class calls it `UnkSlotEntry_3ac78`, confirming
the `+0x010` array-of-pointers field this function reads).
`entry->unk4->methods->slot7C(entry->unk4, entry)` dispatches through the
TARGET object's own method table (`entry->unk4->methods`), not `self`'s --
the call's `$a0` is `entry->unk4` itself, not `self`, established by tracing
which register held which value at the `jalr`.

The loop scans `entry->unk10` (an array of pointers) across a fixed 0x668
BYTE span (`0x19A` = 410 pointer-sized elements, confirmed
`0x19A*4 == 0x668` exactly) and unconditionally sets a flag bit and zeroes two
fields on every element in range.

## Residue and the fix

First four attempts stalled at various partial scores (16/43, with a
"differs outside range" warning on two of them) chasing what looked like a
`p`/`end` REGISTER-ROLE swap: retail computes `p = entry->unk10` into `$v0`
first, then an explicit `move $a0,$v0` establishes `p`'s home register while
`end` (`$a1`) is computed directly from the SAME `$v0` -- one extra `move`
retail has that a straightforward `p = entry->unk10; end = p + 0x19A;`
optimizes away entirely (GCC just puts the loaded value directly into
whichever register `p` ends up in, no separate copy).

**The fix: mention `entry->unk10` TWICE in the source, once for each of `end`
and `p`'s initializers, in that order** (`end = entry->unk10 + 0x19A; for (p =
entry->unk10; ...)`) -- not through an intermediate named local. GCC's CSE
still collapses this to a SINGLE `lw` (recognizing the two mentions are the
same value), but because `end`'s initializer is evaluated (and its temporary
established) before `p`'s, the compiler is left needing an explicit `move` to
give `p` its own register out of the already-computed temporary -- exactly
retail's shape. A cached-through-one-local version, or reversing which of
`p`/`end` is initialized first, either failed to produce the extra `move` at
all or shifted it to the wrong register pairing.

### Proposed learning

**When retail has a "redundant" `move` establishing a loop pointer from an
already-loaded value used to also compute a second, related quantity (an
`end`/bound pointer, typically) — try mentioning the SAME source expression
TWICE, in the order [dependent-quantity-first, loop-variable-second], rather
than through a single cached local.** GCC's own CSE still emits one load, but
this specific write order is what makes it need the extra `move` rather than
folding the loop variable directly into the temporary's register. This is a
constructive counterpart to the broadcast-#1/#2 "redundant move" lever, for
the case where the extra `move` genuinely IS present in retail rather than
being a smell to remove.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C0AC` | `StageMap__ClearSlotCells` | B | Occupant of `gStageMapMethods` +0x108 (`slot108`), called by `class_3ac78`'s own `StageMap__UnloadAllSlots` (already matched) once per element. Walks `entry->unk10` over exactly `0x668` bytes -- the SAME 0x668-byte figure `class_3ac78`'s own unit header names as "a 0x668-byte heap block holding that element's grid of CELL objects" -- clearing a flag bit and two fields on every cell. "Reset...Cells" names this directly against that established vocabulary. |

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

This function: `StageMap__ResetElementCells` -> `StageMap__ClearSlotCells` (`python3 tools/rename.py StageMap__ResetElementCells StageMap__ClearSlotCells`, tier B): when the slot holds a chunk: releases the LbdFile header and clears every cell's model/tmd with GsDOFF set.
