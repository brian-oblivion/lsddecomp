# NotifyGridCell

> Renamed from `func_8004B2D4` on 2026-09-22 (tools/rename.py). Address 0x8004b2d4.

**Unit:** class_3ac78 · **Size:** 18 words · **Status:** MATCHED (18/18 words)

## What it does

`StageMap`'s slot +0x0E0-adjacent helper: if `self` is non-NULL and a flag
bit is set on it, calls its own slot +0x038.

## Derivation

```
beqz $a0, .L8004B30C          ; if (self == NULL) skip
lhu  $v0, 0x36($a0)           ; self->flags36
andi $v0, $v0, 0x80
beqz $v0, .L8004B30C          ; if (!(flags36 & 0x80)) skip
lw   $v0, 0x0($a0)
lw   $v0, 0x38($v0)           ; ->slot38
jalr $v0                      ; self->methods->slot38(self), no extra args
```

Slot +0x038 is `StageMap__OnNotify` (also this unit, still `INCLUDE_ASM`, not
implemented this round). No literal/forwarded args are set up before the
`jalr` beyond `self` itself (unlike `StageMap__DispatchLinkCommand`/`TimedTask__PlaySound`, this
function has no second parameter to forward — nothing else reads `$a1` in
its body), so the call is `slot38(self)` only.

`flags36` (`u16` at `StageMap`+0x36) is a new field established this
round; bit `0x80` gates the dispatch.

## Proposed learning

None beyond what's already documented for `StageMap` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B2D4` | `NotifyGridCell` | B | A free function, not a vtable slot and not a method of `StageMap` -- its `self` is a GRID CELL, which is why it is named `VerbNoun` rather than `Class__Method`. Its only caller is `StageMap__DispatchToRectCells`, which passes a cell out of an element's grid and then every cell chained behind it. The body dispatches the cell's own `+0x038` slot when the cell is non-NULL and its `flags36 & 0x80` is set; `include/code_8220.h` establishes `+0x038` as `BasicClassMethods::onNotify`. The two extra parameters are forwarded implicitly -- the call sets up no registers, so `$a1`/`$a2` still hold this function's own incoming arguments, which is exactly why the signature was widened in an earlier round. |

Parameters renamed: `self` -> `cell`, `arg1` -> `sender`, `arg2` -> `command`.
A parameter rename does not move a byte; whole-image SHA1 re-verified.

Tier B rather than A because "grid cell" comes from the caller, not from this
body, and because `flags36`'s bit `0x80` has no established meaning.

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

## Track 7 (2026-09-27, round 98, charlie)

Moved here from the `.c` comment above the function: "Widened this round (StageMap__DispatchToRectCells) from a single-param signature to accept two more, unused, forwarded params: StageMap__DispatchToRectCells's own call sites explicitly set up $a1/$a2 before every call here (unlike GetSceneNodeMethods's "leftover, already-there" args -- these are real, explicit `move` instructions), so the call itself needs a matching 3-param prototype to compile. Confirmed harmless to THIS function's own already-matched body: neither extra param is read, and GCC does not reserve stack space for unused trailing integer/pointer args on this target, so the definition's own bytes are unaffected (reverified 18/18 after the widening)." (Note: the parameters ARE read -- the body forwards them to `onNotify` -- so "neither extra param is read" was already stale.)

Constant: `flags36 & 0x80` is `GRIDCELL_FLAG_TAKES_COMMANDS`, new in include/GridCell.h, tier B: this function forwards a command only to a cell with the bit; the bit comes from the placement record's `cellFlags` (StageMap__PopulateSlotCells). What the game uses such a cell for is not established; DreamSys__NotifyLinkAttempt reads the same word's low seven bits as a voice index.
