# PlacementGrid__PlacementGrid

> Renamed from `Class6D940__Class6D940` on 2026-09-26 (tools/rename.py). Address 0x8002c18c.

> Renamed from `func_8002C18C` on 2026-09-24 (tools/rename.py). Address 0x8002c18c.

**Unit:** code_179d8_d · **Size:** 29 instructions (0x74 bytes) ·
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
**and IS one**: round-77 correction, `D_8006D940` is a real 30-slot
FileResource-derived vtable (see the unit header comment). `Obj6D940` (the
0x34-byte allocated object, matching `New_PlacementGrid`'s own alloc size)
added as a new unit-local type with only the two fields this function
touches (`unk2C`, `unk30`) plus the `methods` pointer at offset 0.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C18C -> PlacementGrid__PlacementGrid`, tier A. `+0x008` (ctor)
slot of `D_8006D940`, confirmed by `tools/classtable.py 0x8006D940`. Matches
the `Class__Class` ctor convention exactly, same slot position as
`FileResource__FileResource` and `VabStreamObj__VabStreamObj`. Also renamed the
unit-local types `Table6D940 -> PlacementGridMethods`, `Obj6D940 -> PlacementGrid`
by hand (not splat symbols, so outside `rename.py`'s scope) for consistency
with the confirmed class-framework reading; this report's code block above
still shows the pre-rename type spelling (`Obj6D940`), left as written
history.

## Track 4 (2026-09-26, round 87, echo)

Now `(PlacementGrid *self, char *name)`: slot6C is FileResource's +0x06C `requestLoadFile(self, name)`, and slot08 of the active table is `ctor`. The two zeroed fields are `linkResource` (+0x02C) and `loaded` (+0x030). Byte-identical.
