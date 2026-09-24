# GetClass6D940Methods

> Renamed from `func_8002C3A8` on 2026-09-24 (tools/rename.py). Address 0x8002c3a8.

**Unit:** code_179d8_d · **Size:** 4 instructions (0x10 bytes) ·
**Status: MATCHED 4/4**, whole-image SHA1 green.

## Role

Plain accessor, no parameters: returns `&D_8006D940`. Same shape as the
class-framework "get vtable" accessors documented elsewhere in this
project (`docs/research/class-framework.md`), but this unit is NOT
class-framework code (per the unit's own header comment / charlie's
sibling-slice finding) -- so this is written as a plain local
function-pointer-table getter, not claimed to be a real vtable accessor.
`New_Class6D940` and `Class6D940__Class6D940` (both this unit, this round) dispatch
through the returned table.

```c
Table6D940 *GetClass6D940Methods(void)
{
    return &D_8006D940;
}
```

## New local types

`Table6D940` (this file only) -- a function-pointer table with two known
slots: `+0x008` (`slot08`, 2-arg `(self, s32)` -- this IS `Class6D940__Class6D940`
itself, confirmed by `New_Class6D940`'s own dispatch through this exact
slot) and `+0x06C` (`slot6C`, same 2-arg shape, dispatched conditionally
from inside `Class6D940__Class6D940`'s own body). `D_8006D940` declared `extern
Table6D940 D_8006D940;`.

Also added `BaseTable6D940` (this file only) -- a SEPARATE table reached
only via the uncarved accessor `GetActiveDataSourceMethods()`, with three known slots
(`+0x008`, `+0x00C`, `+0x064`) used by `Class6D940__Class6D940`/`Class6D940__Destroy`/
`func_8002C238` respectively (all this unit, this round). Kept entirely
local to `code_179d8_d.c`, no shared header, per this round's rule for the
`code_179d8` slices.
