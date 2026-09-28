# StageMap__CountPendingLoads — MATCHED (13/13 words)

> Renamed from `StageMap__CountFlaggedElements` on 2026-09-26 (tools/rename.py). Address 0x8004bce0.

> Renamed from `Class866E8__CountFlaggedElements` on 2026-09-26 (tools/rename.py). Address 0x8004bce0.

> Renamed from `func_8004BCE0` on 2026-09-24 (tools/rename.py). Address 0x8004bce0.

Not a vtable slot — `StageMap__CountPendingLoads` does not appear in `gStageMapMethods`
(confirmed via `tools/classtable.py 0x800866E8`), so it is a plain,
non-virtual helper. Takes `self` directly as its only argument.

## Disassembly

```
addu $a1, $zero, $zero       ; a1 = count = 0
addu $v1, $zero, $zero       ; v1 = i = 0
.loop:
lhu  $v0, 0xEC($a0)          ; self->arr[i].flag  (a0 walks by 0x1C/iteration)
nop
beqz $v0, .skip
 addiu $a0, $a0, 0x1C        ; a0 += 0x1C (delay slot -- ALWAYS executes)
addiu $a1, $a1, 0x1          ; count++ (only reached when flag != 0)
.skip:
addiu $v1, $v1, 0x1          ; i++
slti $v0, $v1, 0x7
bnez $v0, .loop
 nop
jr $ra
 addu $v0, $a1, $zero        ; return count
```

The moving-pointer codegen (`a0` incremented by `0x1C` each iteration, and
`+0xEC` always read relative to the CURRENT `a0`) is GCC's natural -O2
expansion of a plain indexed loop over a fixed-size array — no manual
pointer-walking needed in the source; see the "final C" below.

## Final C

```c
s32 StageMap__CountPendingLoads(Obj866E8 *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 7; i++) {
        if (self->arr[i].flag != 0) {
            count++;
        }
    }
    return count;
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8::arr` — a 7-element array at `+0x0EC`, each element `0x1C`
  bytes (new `Elem` type). Only `Elem::flag` (`u16`, +0x000) is established
  so far, from this function's nonzero check.

## Attempts

1 (matched on first attempt).

### Proposed learning

Confirms (doesn't newly establish) the project's existing "let GCC hoist
its own loop invariants" guidance: writing the natural indexed-array-access
loop, rather than hand-rolling pointer arithmetic to mimic the observed
`lhu 0xEC($a0)` / `addiu $a0,$a0,0x1C` register moves, reproduces retail
exactly. The instinct to transcribe the moving-pointer shape literally
would have been wrong and unnecessary.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004BCE0` | `StageMap__CountPendingLoads` | A | Pure leaf: loops `self->arr[7]`, counts entries with `flag != 0`, returns the count. A pure count is tier A by the naming rule's own "getter/clamp/list-push" clause -- the mechanics ARE the purpose. Called by `StageMap__ApplyChunkLoads` to refresh `self->unk1B4` after flagging/unflagging elements. |

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
the per-stage config. Header now `include/stage_map.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/stage_grid.h), `Unk54Struct` is `LongVec3` (include/scene_node.h),
`EntryDesc866E8` is `Ratio16[3]` (include/scene_node.h), all by layout and
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

This function: `StageMap__CountFlaggedElements` -> `StageMap__CountPendingLoads` (`python3 tools/rename.py StageMap__CountFlaggedElements StageMap__CountPendingLoads`, tier A): a counter leaf: slots with `loadPending` set.

## Track 7 (2026-09-27, round 95, charlie)

The loop bound 7 -> `ARRAY_COUNT(self->slots)`. Nothing else renamed (`count`, `i` are roles already).
