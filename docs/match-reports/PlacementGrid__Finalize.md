# PlacementGrid__Finalize

> Renamed from `Class6D940__Finalize` on 2026-09-26 (tools/rename.py). Address 0x8002c200.

> Renamed from `PlacementGrid__Destroy` on 2026-09-26 (tools/rename.py). Address 0x8002c200.

> Renamed from `func_8002C200` on 2026-09-24 (tools/rename.py). Address 0x8002c200.

**Unit:** vab_sound · **Size:** 14 instructions (0x38 bytes) ·
**Status: MATCHED 14/14**, whole-image SHA1 green.

## Role

Plain forwarding wrapper: dispatches `GetActiveDataSourceMethods()->slot0C(self)`. One
of the "plain forwarding wrapper" shapes flagged as recurring in this
region by charlie's sibling-slice note.

```c
s32 PlacementGrid__Finalize(void *self)
{
    return GetActiveDataSourceMethods()->slot0C(self);
}
```

This function's WHOLE body is one call with nothing after it -- exactly
the ambiguous case CLAUDE.md warns about ("a void wrapper around a
non-void tail call is byte-identical"). No caller of `PlacementGrid__Finalize`
exists in this window, so there is no positive evidence either way; per
the project's stated default, `slot0C` is typed `s32` and the value is
returned rather than discarded. Byte-verified identical either way -- this
is a documented choice, not a measured one.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C200 -> PlacementGrid__Finalize`, tier A. `+0x00C` (dtor)
slot of `gPlacementGridMethods` (confirmed by `tools/classtable.py 0x8006D940`),
matching the `FileResource__Finalize` naming precedent at the same slot
position in the base class.

## Track 4 (2026-09-26, round 87, echo)

Renamed `PlacementGrid__Destroy -> PlacementGrid__Finalize`: it occupies slot
+0x00C of gPlacementGridMethods, which is `finalize` in every class
(`include/basic_class.h`; `tools/classtable.py gPlacementGridMethods --vs gFileResourceMethods`
shows it overriding `FileResource__Finalize`), and its body is only the
parent's finalize, reached through the active data source's table
(`GetActiveDataSourceMethods()->finalize`). The slot is `void`, so the
function is now `void` too: byte-identical, which settles the tail-call
ambiguity above in favour of the slot's type.


## Track 6 (2026-09-26, round 93, charlie)

Renamed with `python3 tools/renametype.py Class6D940 PlacementGrid` (the
whole family: object, `Class6D940Methods`, the getter, constructors,
methods, `Class6D940Record` -> `PlacementGridRecord`,
`Class6D940ResolveEntryFn` -> `PlacementGridResolveEntryFn`,
`Class6D940GetModelFn` -> `PlacementGridGetModelFn`, the header
`include/Class6D940.h` -> `include/PlacementGrid.h`), then
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
