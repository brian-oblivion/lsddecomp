# PlacementGrid__PlacementGrid

> Renamed from `Class6D940__Class6D940` on 2026-09-26 (tools/rename.py). Address 0x8002c18c.

> Renamed from `func_8002C18C` on 2026-09-24 (tools/rename.py). Address 0x8002c18c.

**Unit:** PlacementGridVabSound · **Size:** 29 instructions (0x74 bytes) ·
**Status: MATCHED 29/29**, whole-image SHA1 green. Matched on the first
attempt.

## Role

This IS `Table6D940::slot08` -- `New_PlacementGrid`'s own dispatch target
(confirmed: `New_PlacementGrid` calls `GetPlacementGridMethods()->slot08(self, arg1)`
right after allocating, and this function's own signature/behavior is
exactly that 2-arg init shape). Chains to a base init via
`GetActiveDataSourceMethods()->slot08(self)` first (self-only, no `arg1` forwarded),
sets `self->methods` to this unit's OWN table (`GetPlacementGridMethods()`), zeroes
two fields, then conditionally dispatches through its own freshly-set
table's `slot6C` if `arg1` is non-zero.

```c
void PlacementGrid__PlacementGrid(Obj6D940 *self, s32 arg1)
{
    GetActiveDataSourceMethods()->slot08(self);
    self->methods = GetPlacementGridMethods();
    self->unk2C = 0;
    self->unk30 = 0;
    if (arg1 != 0) {
        self->methods->slot6C(self, arg1);
    }
}
```

Structurally identical to a class-framework ctor-chain (base ctor call,
own vtable install, field reset, conditional post-init dispatch) --
**and IS one**: round-77 correction, `gPlacementGridMethods` is a real 30-slot
FileResource-derived vtable (see the unit header comment). `Obj6D940` (the
0x34-byte allocated object, matching `New_PlacementGrid`'s own alloc size)
added as a new unit-local type with only the two fields this function
touches (`unk2C`, `unk30`) plus the `methods` pointer at offset 0.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C18C -> PlacementGrid__PlacementGrid`, tier A. `+0x008` (ctor)
slot of `gPlacementGridMethods`, confirmed by `tools/classtable.py 0x8006D940`. Matches
the `Class__Class` ctor convention exactly, same slot position as
`FileResource__FileResource` and `VabStreamObj__VabStreamObj`. Also renamed the
unit-local types `Table6D940 -> PlacementGridMethods`, `Obj6D940 -> PlacementGrid`
by hand (not splat symbols, so outside `rename.py`'s scope) for consistency
with the confirmed class-framework reading; this report's code block above
still shows the pre-rename type spelling (`Obj6D940`), left as written
history.

## Track 4 (2026-09-26, round 87, echo)

Now `(PlacementGrid *self, char *name)`: slot6C is FileResource's +0x06C `requestLoadFile(self, name)`, and slot08 of the active table is `ctor`. The two zeroed fields are `linkResource` (+0x02C) and `loaded` (+0x030). Byte-identical.


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
