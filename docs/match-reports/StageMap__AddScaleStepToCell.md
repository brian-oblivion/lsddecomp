# StageMap__AddScaleStepToCell — MATCHED (14/14 words)

> Renamed from `StageMap__ApplyRateToChild` on 2026-09-26 (tools/rename.py). Address 0x8004d0d0.

> Renamed from `Class866E8__ApplyRateToChild` on 2026-09-26 (tools/rename.py). Address 0x8004d0d0.

> Renamed from `func_8004D0D0` on 2026-09-24 (tools/rename.py). Address 0x8004d0d0.

## How the arguments were resolved (worth recording — this one was not
obvious from the function's own body alone)

`StageMap__AddScaleStepToCell`'s only two references anywhere in the executable are as a
function-pointer VALUE (`lui`/`addiu` of its address, never a direct
`jal`) inside `StageMap__StepScaleRamp` (this unit), passed as the second argument
to `StageMap__ForEachSlot`. Reading `StageMap__ForEachSlot` in isolation makes it look
like `StageMap__AddScaleStepToCell` is invoked there directly as `callback(self, &arr[i])`
— but `StageMap__ForEachSlot` (own body: `s5 = a1`, forwarded unchanged to
`StageMap__ForEachSlotCell`'s own second argument) actually treats its OWN second
argument as a pass-through value, not a callback it calls itself. The
real call site is one level further down, inside `StageMap__ForEachSlotCell`, which
walks `item->unk10[]` (an array of `Unk10ChildObj_3bb8c_b*`, up to
`+0x668` bytes from the base) and calls `callback(self, element)` for
each entry. So `StageMap__AddScaleStepToCell`'s true "item" parameter is one of THOSE
array elements, not `&self->arr[i]` directly — reading only the
one-hop-removed caller would have produced the wrong type for `arg1`.

## Disassembly

```
addiu $sp, $sp, -0x18
move  $v1, $a0            ; v1 = self
move  $a0, $a1            ; a0 = item
sw    $ra, 0x10($sp)
lw    $v0, 0x0($a0)       ; v0 = item->methods
lw    $a2, 0x1E4($v1)     ; a2 = self->unk1E4  (set up before the call)
lw    $v0, 0x48($v0)      ; v0 = item->methods->slot48
jalr  $v0
 move $a1, $zero            ; a1 = 0
...epilogue
```

`m2c` (seeded with `void StageMap__AddScaleStepToCell(void *arg0, void *arg1)`) already
called this shape correctly: `(*arg1)->unk48(arg1, 0, arg0->unk1E4);` —
confirming the vtable-slot read before any header work was done.

## Final C

```c
void StageMap__AddScaleStepToCell(Obj866E8 *self, Unk10ChildObj_3bb8c_b *item) {
    item->methods->slot48(item, 0, self->unk1E4);
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- New type `Unk10ChildObj_3bb8c_b` / `Unk10ChildMethods_3bb8c_b` — the
  object type held in `Elem::unk10[]`. Only `slot48` is typed
  (`void (*slot48)(Unk10ChildObj_3bb8c_b *self, s32 arg1, void *arg2)`),
  resolved from this function and its sibling `StageMap__ResetCellScale`.
- `Elem::unk10` (`Unk10ChildObj_3bb8c_b **`, +0x010) — same real field
  `dream_day.h`'s independent view already names `unk10`
  (`GenericObject **`) on its own `UnkSlotEntry_3ac78` type; both
  descriptions agree on offset and "array of pointers, walked to +0x668".
- `Obj866E8::unk1E4` (`void *`, +0x1E4) — forwarded opaquely as `slot48`'s
  third argument; never dereferenced in this unit.
- `extern void StageMap__ForEachSlot(...)` / `extern void StageMap__ForEachSlotCell(...)` —
  both still `INCLUDE_ASM` in this same unit; forward-declared per the
  established "calling into a still-`INCLUDE_ASM` function is fine"
  convention, typed from their own call sites (see those functions' future
  reports for the full derivation).

## Attempts

1 (matched on first attempt, once the true call chain — two hops through
`StageMap__ForEachSlot`/`StageMap__ForEachSlotCell`, not one — was traced).

### Proposed learning

**A function passed by address is not necessarily called by its immediate
receiver.** `StageMap__ForEachSlot` receives `StageMap__AddScaleStepToCell`'s address only to
forward it, unclobbered, to a second function (`StageMap__ForEachSlotCell`) that does
the actual `jalr`. Reading the receiver's own body (which never does
`jalr` on that register) is itself the signal to keep tracing one hop
further before typing the passed function's parameters from the wrong
call site.

## Naming

**Tier B.** Not a vtable slot -- a callback, passed as a function pointer
to `StageMap__ForEachSlot`/`StageMap__ForEachSlotCell` by
`StageMap__StepScaleRamp`. Body: `item->methods->slot48(item, 0,
self->rateEntry)`. Named for what it does to each child entry (forwards
the parent's current rate entry to it), mirrored by
`StageMap__ResetCellScale`'s sibling shape.

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

This function: `StageMap__ApplyRateToChild` -> `StageMap__AddScaleStepToCell` (`python3 tools/rename.py StageMap__ApplyRateToChild StageMap__AddScaleStepToCell`, tier A): one-line callback: `updateScale(cell, 0 = add, scaleStep)`.

## Round 96 (track 7, delta)

Parameter `item` -> `cell`. The `0` to updateScale is its `set` flag (add).
