# StageMap__InitFootprintRect — MATCHED (32/32 words)

> Renamed from `StageMap__InitFootprintSlot` on 2026-09-27 (tools/rename.py). Address 0x8004cda4.

> Renamed from `Class866E8__InitFootprintSlot` on 2026-09-26 (tools/rename.py). Address 0x8004cda4.

> Renamed from `func_8004CDA4` on 2026-09-24 (tools/rename.py). Address 0x8004cda4.

Writes a fresh copy of a constant 3-word struct (`sFullSlotRect`) into
`self+0x8C+key*0xC`, then overwrites just the first word of that copy
with the return value of a vtable call (`self->methods->slot124(self,
arg3)`), and returns `key + 1`.

`self`'s `+0x8C` region is a currently-uncharted array of 0xC-byte
(`Unk54Struct`-shaped) slots indexed by `key`; only ONE index is exercised
here and its true element count is not established (only `StageMap__SetFootprintFromQuery`
calls this, with `key` values that would need that function fully
derived to bound), so this stays explicit pointer arithmetic rather than
a sized array field in `struct Obj866E8` — same policy already used for
`Elem::unk10`'s walk in this header.

The 4-arg register layout is notable: the second parameter (`$a1`) is
loaded fresh from `sFullSlotRect`'s own third word (`lw $a1, 0x8($a2)`)
partway through the function and is NEVER READ as an incoming argument —
it is a dead/unused parameter from this function's own perspective
(its callers, in `StageMap__SetFootprintFromQuery`, do pass a real value there, but this
function itself discards it).

## New struct knowledge (`include/class_3bb8c.h`)

- New vtable slot `Obj866E8Methods::slot124` (`s32 (*)(Obj866E8*, s32)`,
  +0x124) — the struct previously ended right after `slot118` (+0x118)
  with no trailing padding; added `pad11C[0x124-0x11C]` before this new
  slot.
- New extern `sFullSlotRect` (`Unk54Struct`, whole-struct copy source) —
  reuses the existing `Unk54Struct` type (already established from
  `self->unk54` and `ComputeCellWorldOffsets`'s `arg3`).

## Final C

```c
s32 StageMap__InitFootprintRect(Obj866E8 *self, s32 unused, s32 key, s32 arg3) {
    Unk54Struct *slot;

    slot = (Unk54Struct *) ((u8 *) self + 0x8C + key * sizeof(Unk54Struct));
    *slot = sFullSlotRect;
    slot->unk0 = self->methods->slot124(self, arg3);
    return key + 1;
}
```

## Attempts

1 (matched on first attempt). The `key * sizeof(Unk54Struct)` (`key * 12`)
multiply-by-constant naturally lowers to retail's `sll 1; addu; sll 2`
(`*2, +key, *4` = `*12`) shift/add chain, and the whole-struct assignment
(`*slot = sFullSlotRect;`) naturally lowers to the three-word load/store
sequence — both already-confirmed idioms from
`docs/DECOMPILATION_LEARNINGS.md`, so no iteration was needed once the
register trace was right.

### Proposed learning

None new — confirms two existing idioms (constant-multiply-to-shift/add,
whole-struct assignment for a block copy) compose cleanly when used
together in the same statement.

## Naming

**Tier B.** Not a vtable slot. Writes the constant `Unk54Struct` template
`sFullSlotRect` into a `self->gridSlots[]`-shaped entry, then overwrites its
`elemIdx` word via `slot124`. Called by both
`StageMap__BuildFootprintRects`'s sibling paths and
`StageMap__SetFootprintFromQuery`, always to seed a fresh slot -- hence
"init", not "set" (it does not preserve any prior content of the slot).

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

## Round 96 (track 7, delta)

Renamed from `StageMap__InitFootprintSlot` (tools/rename.py), tier A: it
fills rects[index] (a CellRect) with sFullSlotRect and the slot index of the
chunk. Parameters `key` -> `index`, `arg3` -> `chunkIndex` (passed to
findSlotIndexByChunk), `slot` -> `rect`; the prototype follows.
