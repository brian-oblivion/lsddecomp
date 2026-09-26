# GetPlacementGridMethods

> Renamed from `GetClass6D940Methods` on 2026-09-26 (tools/rename.py). Address 0x8002c3a8.

> Renamed from `func_8002C3A8` on 2026-09-24 (tools/rename.py). Address 0x8002c3a8.

**Unit:** code_179d8_d · **Size:** 4 instructions (0x10 bytes) ·
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
local to `code_179d8_d.c`, no shared header, per this round's rule for the
`code_179d8` slices.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C3A8 -> GetPlacementGridMethods`, tier A. This unit's earlier
"NOT class-framework code" finding (round 16) was WRONG for `gPlacementGridMethods`
specifically -- see the unit header comment's round-77 correction.
`gPlacementGridMethods` is a real 30-slot FileResource-derived vtable
(`tools/classtable.py 0x8006D940`), and this function is its getter,
confirmed as the FIRST entry of `gDataSourceClientGetters` (code_171e0.c's
NULL-terminated array of "class-method-table getters of every
FileResource-derived client", `asm/data/5DB70.data.s`). Matches the
established `GetXXXMethods` convention for every other entry in that same
array (`GetVabStreamObjMethods`) and elsewhere (`GetFileResourceMethods`,
`GetClass6D3C8Methods`). The stale "NOT class-framework" language in this
report's `## Role` section predates the correction and is left as written
history rather than edited (the unit header comment and this `## Naming`
section are authoritative).

## Track 4 (2026-09-26, round 87, echo)

The paragraph above ("NOT class-framework code") is superseded: gPlacementGridMethods is a FileResource method table and this is its getter, the first entry of gDataSourceClientGetters. Declared in `include/PlacementGrid.h`.
