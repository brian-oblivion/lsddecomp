# PlacementGrid__OnRequestDone

> Renamed from `PlacementGrid__SetFlag` on 2026-09-28 (tools/rename.py). Address 0x8002c238.

> Renamed from `Class6D940__SetFlag` on 2026-09-26 (tools/rename.py). Address 0x8002c238.

> Renamed from `func_8002C238` on 2026-09-24 (tools/rename.py). Address 0x8002c238.

**Unit:** vab_sound · **Size:** 16 instructions (0x40 bytes) ·
**Status: MATCHED 16/16**, whole-image SHA1 green.

## Role

Sets a field then forwards to the same base accessor `PlacementGrid__Finalize`
uses, a different slot:

```c
s32 PlacementGrid__OnRequestDone(s32 *self)
{
    self[0xC] = 1;
    return GetActiveDataSourceMethods()->slot64(self);
}
```

`self[0xC]` writes byte offset `0x30` (`s32` index * 4) -- matches
retail's `sw $v0, 0x30($s0)` exactly, `$v0` having just been loaded with
the literal `1`.

Same tail-call return-type ambiguity as `PlacementGrid__Finalize` (this unit, same
round): the call is the function's last action with nothing touching
`$v0` afterward, so `void` and `s32` compile identically. Typed `s32` and
returned per the project's default, absent positive void evidence.
`slot64` (`BaseTable6D940::slot64`, `+0x064`) added to the same unit-local
table `PlacementGrid__Finalize` uses.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C238 -> PlacementGrid__OnRequestDone`, tier B (mechanics, not
purpose). Occupies `+0x064` of `gPlacementGridMethods` -- the exact slot
`FileResource__OnRequestDone` fills in the base class (`tools/classtable.py
0x8006D430`) and its own verbatim-shared copy in `gCdDriverMethods`
(`tools/classtable.py 0x8006D4E8`). Named by SLOT POSITION, not by
asserted behavior: this override does NOT just OR in a flag bit like the
base -- it sets `self[0xC]` (offset 0x30, a field beyond `FileResource`'s
own layout) then forwards through `GetActiveDataSourceMethods()->slot64(self)`.
That mechanics difference is why this is tier B and not A.

## Track 4 (2026-09-26, round 87, echo)

Now `void PlacementGrid__OnRequestDone(PlacementGrid *self)`, the type of slot +0x064 (`setFlag`, include/file_resource.h), byte-identical. `self[0xC]` is `loaded` (+0x030), zeroed by the ctor. `BaseTable6D940` was the active driver's table, `FileResourceMethods`; the call is `GetActiveDataSourceMethods()->setFlag`.


## Track 6 (2026-09-26, round 93, charlie)

Renamed with `python3 tools/renametype.py Class6D940 PlacementGrid` (the
whole family: object, `Class6D940Methods`, the getter, constructors,
methods, `Class6D940Record` -> `PlacementGridRecord`,
`Class6D940ResolveEntryFn` -> `PlacementGridResolveEntryFn`,
`Class6D940GetModelFn` -> `PlacementGridGetModelFn`, the header
`include/Class6D940.h` -> `include/placement_grid.h`), then
`python3 tools/rename.py D_8006D940 gPlacementGridMethods` (the table,
g<Class>Methods) and
`python3 tools/renametype.py PlacementGridPlacement CellPlacement --any-stem`
(ex-`Class6D940Placement`). The tools rewrote every old token in these
reports too, history lines included, so an earlier section above that says
`PlacementGrid`/`gPlacementGridMethods`/`CellPlacement` named
`Class6D940`/`D_8006D940`/`Class6D940Placement` at the time (pending an
operator decision on renametype.py and history prose; not hand-edited).

**Class name `PlacementGrid`, tier A.** From the body of
PlacementGrid__ResolveEntry and its only caller, which agree: the buffer
is a row-major 20 x 20 grid of 12-byte records (cell < 400, row = cell / 20,
column = cell % 20, record at buffer + 8 + cell * 12), each holding a model
index, a height, a y rotation and a byte of flags, with `next` chaining
further records in the same cell; ResolveEntry places the record at the
cell's centre (column/row * 0x800 + 0x400) and returns the model
linkResource's getModel gives for its index. StageMap__PopulateSlotCells
points `buffer` at the grid element's LbdFile header block +
`placementsOffset` (LbdFileHeader's own field name) and puts each result
into that element's GridCell lattice (20 x 20, 0x800 apart: grid_cell.h),
chained records into the overflow cells. So the class is the placements of
one grid element's cells. **`CellPlacement`** is ResolveEntry's output, one
model's placement in one cell. The name says what the records are, not
what the game draws with them (terrain tiles is plausible, not shown).
