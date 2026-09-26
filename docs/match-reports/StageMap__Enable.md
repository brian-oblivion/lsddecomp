# StageMap__Enable — MATCHED (3/3 words)

> Renamed from `Class866E8__Enable` on 2026-09-26 (tools/rename.py). Address 0x8004b570.

> Renamed from `StageMap__func_8004B570` on 2026-09-24 (tools/rename.py). Address 0x8004b570.

> Renamed from `func_8004B570` on 2026-09-24 (tools/rename.py). Address 0x8004b570.

`Obj866E8`'s vtable slot +0x0EC (`gStageMapMethods`, resolved with
`tools/classtable.py 0x800866E8` — a different, independently-typed local
view of the same table already exists in `include/class_3ac78.h` as
`StageMapMethods`; see `include/class_3bb8c.h`'s header comment for why
this unit keeps its own, per the project's multiple-local-views
convention). This is the first function of `class_3bb8c`'s newly-carved
first slice, and the first match report for it.

## Disassembly

```
ori $v0, $zero, 0x1
jr  $ra
 sw $v0, 0x70($a0)
```

## Final C

```c
void StageMap__Enable(Obj866E8 *self) {
    self->unk70 = 1;
}
```

**Return type corrected by the head from `s32` to `void`** (byte-identical;
re-verified 3/3 with the whole-image SHA1 green). The body is three words —
`ori $v0, $zero, 1` / `jr $ra` / `sw $v0, 0x70($a0)` in the delay slot — and
the literal has to be materialized in *some* register to be stored, with
`$v0` the natural first choice. So `void` and `s32` are byte-identical here
and the bytes carry NO information about the return type. This is the same
trap as the documented "a `void` wrapper around an `s32` tail call is
byte-identical", in its store-instead-of-tail-call form: the value in `$v0`
is a side effect of needing a register, not a returned result.

What broke the tie is the slot, not this function. `StageMap__Enable` occupies
`+0x0EC` of `gStageMapMethods`, and a cross-table survey of that offset
(`tools/classtable.py` over all 60 tables) finds five distinct occupants —
`TaskCore__FindPrevFreeSlot` (the base implementation, shared by four separate class
tables), `FadeBox__PopPosition`, `StyleEffect__Update`, `Actor__SetPendingExtra` and this one.
`TaskCore__FindPrevFreeSlot`'s body ends in a bare `jr $ra` after a `jalr`, materializing
no return value on either of its two paths. A base implementation that
returns nothing is evidence the SLOT is `void`, so an override asserting
`s32` is the less supported reading. None of the other four occupants is
matched yet, so this can be revisited when one is.

The original `return self->unk70 = 1;` form was not wrong about the bytes —
only about what the bytes prove.

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8::unk70` (s32) — a flag, set to 1 here.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new — a direct instance of an already-documented idiom.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B570` | `StageMap__Enable` | A | Occupant of `gStageMapMethods` +0x0EC. Body is exactly `self->enabled = 1;`. Paired with `StageMap__Disable` (+0x0F0, same struct, clears the same field) and cross-confirmed by `class_3ac78`'s own INDEPENDENT local view of the same field, already named `enabled` there (`docs/match-reports/StageMap__UpdateIfEnabled.md`, round 67) from the identical set/clear evidence. A pure setter of a named boolean field is tier A by the naming rule's own "getter/clamp/list-push" clause. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Obj866E8::unk70` | `enabled` | A | Set 1 here, cleared 0 (after a `slotC0` teardown dispatch) by `StageMap__Disable`. Same field, same evidence, and the same conclusion `class_3ac78`'s independent view already reached for its own copy of this struct -- see `StageMap__UpdateIfEnabled.md`. Renamed in `include/class_3bb8c.h`'s own `Obj866E8` definition; rebuild after the rename touched only `src/class_3bb8c.c` (`StageMap__Enable`/`StageMap__Disable`), confirming no other unit accesses this struct's `unk70`/`enabled` field. |

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
