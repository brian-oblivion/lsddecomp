# New_PlacementGrid

> Renamed from `New_Class6D940` on 2026-09-26 (tools/rename.py). Address 0x8002c12c.

> Renamed from `new_class_6d940` on 2026-09-24 (tools/rename.py). Address 0x8002c12c.

**Unit:** code_179d8_d · **Size:** 24 instructions (0x60 bytes) ·
**Status: MATCHED 24/24**, whole-image SHA1 green. Matched on the first
attempt.

## Role

Allocator: `BMemPMgrAlloc(0x34)`, and on success dispatches
`GetPlacementGridMethods()->slot08(self, arg1)` (that slot IS `PlacementGrid__PlacementGrid`, this
unit, matched this round -- see its own report), returning the new
instance; returns `NULL` on allocation failure.

```c
void *New_PlacementGrid(s32 arg1)
{
    void *self;
    Table6D940 *table;

    self = BMemPMgrAlloc(0x34);
    if (self != NULL) {
        table = GetPlacementGridMethods();
        table->slot08(self, arg1);
        return self;
    }
    return NULL;
}
```

This is the EXACT same shape as `class_3bb8c_k`'s `New_ObjM`
(matched earlier this round, same runner) -- success path's `return self;`
inside the `if`-body, failure path's `return NULL;` trailing and
unconditional. Reused directly rather than re-derived, and it matched on
the first attempt: further confirmation of the proposed learning in
`New_ObjM`'s own report (GCC 2.6.3 -O2 folds only THAT shape into a
single branch with the failure value in the delay slot, no extra jump).

Despite the "New_X + ctor-dispatch-through-a-table" shape being identical
to this project's class-framework allocator pattern, this unit is
confirmed NOT class-framework code (see the unit's own header comment /
charlie's sibling-slice finding) -- `Table6D940` is written as a plain
local function-pointer table, not claimed to be a real vtable. The shape
recurring here says only that "allocate, call an init function through a
function-pointer slot, return the pointer or NULL" is a common C idiom in
this codebase generally, independent of whether the object is a
class-framework instance.

`BMemPMgrAlloc` (the pool allocator, already established in
`include/class_16334.h`/`include/code_8220.h`) declared LOCAL to this file
since neither shared header is included here.

## Naming (round 77, charlie -- track 3)

Renamed `new_class_6d940 -> New_PlacementGrid`, tier A. Matches the project's
`New_Class` allocator convention exactly. This function's own class,
`gPlacementGridMethods`/`PlacementGridMethods`, IS real class-framework data
(`tools/classtable.py 0x8006D940`, 30 slots) -- the "NOT class-framework
code" language in the `## Role` section above predates the round-77
correction recorded in the unit header comment and is left as written
history.

## Track 4 (2026-09-26, round 87, echo)

Now `PlacementGrid *New_PlacementGrid(char *name)`, the ctor's parameter; the one caller (Class866E8__Class866E8) passes 0, so nothing is loaded. Byte-identical.
