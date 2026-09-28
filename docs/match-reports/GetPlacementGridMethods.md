# GetPlacementGridMethods

> Renamed from `GetClass6D940Methods` on 2026-09-26 (tools/rename.py). Address 0x8002c3a8.

> Renamed from `func_8002C3A8` on 2026-09-24 (tools/rename.py). Address 0x8002c3a8.

**Unit:** PlacementGridVabSound · **Size:** 4 instructions (0x10 bytes) ·
**Status: MATCHED 4/4**, whole-image SHA1 green.

## Role

Plain accessor, no parameters: returns `&gPlacementGridMethods`. Same shape as the
class-framework "get vtable" accessors documented elsewhere in this
project (`docs/research/class-framework.md`), but this unit is NOT
class-framework code (per the unit's own header comment / charlie's
sibling-slice finding) -- so this is written as a plain local
function-pointer-table getter, not claimed to be a real vtable accessor.
`New_PlacementGrid` and `PlacementGrid__PlacementGrid` (both this unit, this round) dispatch
through the returned table.

```c
Table6D940 *GetPlacementGridMethods(void)
{
    return &gPlacementGridMethods;
}
```

## New local types

`Table6D940` (this file only) -- a function-pointer table with two known
slots: `+0x008` (`slot08`, 2-arg `(self, s32)` -- this IS `PlacementGrid__PlacementGrid`
itself, confirmed by `New_PlacementGrid`'s own dispatch through this exact
slot) and `+0x06C` (`slot6C`, same 2-arg shape, dispatched conditionally
from inside `PlacementGrid__PlacementGrid`'s own body). `gPlacementGridMethods` declared `extern
Table6D940 gPlacementGridMethods;`.

Also added `BaseTable6D940` (this file only) -- a SEPARATE table reached
only via the uncarved accessor `GetActiveDataSourceMethods()`, with three known slots
(`+0x008`, `+0x00C`, `+0x064`) used by `PlacementGrid__PlacementGrid`/`PlacementGrid__Finalize`/
`PlacementGrid__SetFlag` respectively (all this unit, this round). Kept entirely
local to `PlacementGridVabSound.c`, no shared header, per this round's rule for the
`code_179d8` slices.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C3A8 -> GetPlacementGridMethods`, tier A. This unit's earlier
"NOT class-framework code" finding (round 16) was WRONG for `gPlacementGridMethods`
specifically -- see the unit header comment's round-77 correction.
`gPlacementGridMethods` is a real 30-slot FileResource-derived vtable
(`tools/classtable.py 0x8006D940`), and this function is its getter,
confirmed as the FIRST entry of `gDataSourceClientGetters` (code_1677c.c's
NULL-terminated array of "class-method-table getters of every
FileResource-derived client", `asm/data/5DB70.data.s`). Matches the
established `GetXXXMethods` convention for every other entry in that same
array (`GetVabStreamObjMethods`) and elsewhere (`GetFileResourceMethods`,
`GetGameApplicationMethods`). The stale "NOT class-framework" language in this
report's `## Role` section predates the correction and is left as written
history rather than edited (the unit header comment and this `## Naming`
section are authoritative).

## Track 4 (2026-09-26, round 87, echo)

The paragraph above ("NOT class-framework code") is superseded: gPlacementGridMethods is a FileResource method table and this is its getter, the first entry of gDataSourceClientGetters. Declared in `include/PlacementGrid.h`.


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
into that element's GridCell lattice (20 x 20, 0x800 apart: GridCell.h),
chained records into the overflow cells. So the class is the placements of
one grid element's cells. **`CellPlacement`** is ResolveEntry's output, one
model's placement in one cell. The name says what the records are, not
what the game draws with them (terrain tiles is plausible, not shown).
