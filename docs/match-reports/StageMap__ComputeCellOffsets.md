# StageMap__ComputeCellOffsets — MATCHED (13/13 words)

> Renamed from `Class866E8__ComputeCellOffsets` on 2026-09-26 (tools/rename.py). Address 0x8004b418.

> Renamed from `func_8004B418` on 2026-09-24 (tools/rename.py). Address 0x8004b418.

`Obj866E8`'s vtable slot +0x0E8. A thin wrapper: builds an argument list
(one of them a fresh, discarded-by-the-caller local scratch buffer) and
tail-calls `ComputeCellWorldOffsets` (an uncarved helper elsewhere in this same
unit, not this round's own target), returning its result directly.

## Disassembly

```
addiu $sp, $sp, -0x30
addu  $a3, $a0, $zero         ; a3 = self (original a0)
addu  $a0, $a1, $zero         ; a0 = arg1
addiu $a1, $sp, 0x18          ; a1 = &outBuf (local scratch, 3 words)
sw    $ra, 0x28($sp)
sw    $a2, 0x10($sp)          ; arg2 homed to the o32 5th-argument slot
lw    $a2, 0x68($a3)          ; a2 = self->unk68
jal   ComputeCellWorldOffsets
 addiu $a3, $a3, 0x54         ; a3 = &self->unk54 (delay slot)
lw    $ra, 0x28($sp)
addiu $sp, $sp, 0x30
jr    $ra
 nop                           ; return value passed straight through in $v0
```

Same "incoming argument stored at `sp+0x10` is the o32 outgoing 5th-argument
slot for a call made later in the same function" shape already documented
this project (`DayTask__StartObjM`, `dream_day`) — `arg2` here is silently
forwarded as `ComputeCellWorldOffsets`'s 5th parameter, confirmed by reading
`ComputeCellWorldOffsets`'s OWN prologue (`lw $t2, 0x10($sp)`).

## Final C

```c
s32 StageMap__ComputeCellOffsets(Obj866E8 *self, void *arg1, void *arg2) {
    s32 outBuf[3];

    return ComputeCellWorldOffsets(arg1, outBuf, self->unk68, &self->unk54, arg2);
}
```

Per CLAUDE.md's one-line-wrapper rule, `return ComputeCellWorldOffsets(...);` is
written rather than a `void` wrapper — `ComputeCellWorldOffsets`'s own body computes
its return value with ordinary integer arithmetic right before its `jr
$ra`, positive evidence it is a real, non-`void` result, not just an
unread tail call.

## New struct/extern knowledge (`include/class_3bb8c.h`)

- `Obj866E8::unk54` — an INLINE (not pointer) 3-word sub-struct
  (`Unk54Struct`, new opaque type), address taken and forwarded to
  `ComputeCellWorldOffsets` without this function itself touching its contents.
- `Obj866E8::unk68` — `void *`, forwarded opaquely (never dereferenced
  here; `ComputeCellWorldOffsets`'s own body does dereference it, but that function
  is out of this round's scope).
- `ComputeCellWorldOffsets` declared locally with the minimal signature this one
  call site demonstrates (5 params, 5th via the stack); not this round's
  function to match, so typed loosely (`void *` for anything not
  dereferenced HERE).

## Attempts

1 (matched on first attempt).

### Proposed learning

None new — direct reuse of the "stack-homed incoming arg is really an
outgoing 5th argument for a later call" lesson from `DayTask__StartObjM`
(`dream_day`), now confirmed a second time in a different unit.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B418` | `StageMap__ComputeCellOffsets` | B | Occupant of `gStageMapMethods` +0x0E8. A thin wrapper: forwards `self->unk68`/`&self->unk54` and its own two arguments straight into `ComputeCellWorldOffsets`, discarding that call's own `outBuf` (a fresh unread local). Named for the mechanic it performs (call the world-offset computation with this object's own divisor/count/gate and cell-base state), not a guessed purpose. |

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

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals, tier A: `arg1` -> `outPos`, `arg2` -> `cell`, `outBuf` -> `chunkCentre` (discarded), as in SetTargetAndLoadChunks.
