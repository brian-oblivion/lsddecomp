# GetGridCellMethods

> Renamed from `GetClass86AA0Methods` on 2026-09-26 (tools/rename.py). Address 0x8004d508.

> Renamed from `func_8004D508` on 2026-09-22 (tools/rename.py). Address 0x8004d508.

**Unit:** title_menu · **Size:** 4 words · **Status:** MATCHED (4/4)

## What it does

Get-vtable helper for a second small sibling class: returns `&gGridCellMethods`,
this unit's `GridCellMethods`. Same shape and role as GetNodeGuardedViewportMethods.

## The C

```c
GridCellMethods *GetGridCellMethods(void)
{
    return &gGridCellMethods;
}
```

## New type: GridCell / GridCellMethods

`gGridCellMethods` (asm/data/76DC8.data.s, header word 0x24) is gNodeGuardedViewportMethods's
sibling: same opening shape (header, then `BasicClass__Release` at
+0x004, ctor at +0x008 -- here GridCell__GridCell). Only +0x008 (`ctor`) and
+0x0B8 (dispatched by GridCell__DispatchLinkCommand, this unit's own slot +0x09C in the
same table) are typed; see `include/class_3bb8c.h`.

## Proposed learning

See GetNodeGuardedViewportMethods.md -- same finding, second instance.

## Naming

**GetGridCellMethods** -- tier A. Same pure-getter shape and evidence as
`GetNodeGuardedViewportMethods` (see that report); `D_80086AA0` renamed alongside
it to `gGridCellMethods` for the same reason.
