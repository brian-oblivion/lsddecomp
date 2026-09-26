# New_GridCell

> Renamed from `New_Class86AA0` on 2026-09-26 (tools/rename.py). Address 0x8004d38c.

> Renamed from `func_8004D38C` on 2026-09-22 (tools/rename.py). Address 0x8004d38c.

**Unit:** class_3bb8c_c · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

`New_GridCell`: the allocator for `GridCell`, mirroring New_NodeGuardedViewport
exactly but for the sibling class (`0x3C`-byte allocation, ctor
`GridCell__GridCell` fetched through `GetGridCellMethods()->ctor`).

## The C

```c
GridCell *New_GridCell(void)
{
    GridCell *self;

    self = BMemPMgrAlloc(0x3C);
    if (self != NULL) {
        GetGridCellMethods()->ctor(self);
        return self;
    }
    return NULL;
}
```

## Notes

Matched on the first attempt; same idiom as New_NodeGuardedViewport. No residue.

## Naming

**New_GridCell** -- tier A. Same `New_X` allocator idiom as
`New_NodeGuardedViewport` (see that report), mirrored exactly for this sibling
class.
