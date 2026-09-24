# new_class_6d940

**Unit:** code_179d8_d · **Size:** 24 instructions (0x60 bytes) ·
**Status: MATCHED 24/24**, whole-image SHA1 green. Matched on the first
attempt.

## Role

Allocator: `BMemPMgrAlloc(0x34)`, and on success dispatches
`func_8002C3A8()->slot08(self, arg1)` (that slot IS `func_8002C18C`, this
unit, matched this round -- see its own report), returning the new
instance; returns `NULL` on allocation failure.

```c
void *new_class_6d940(s32 arg1)
{
    void *self;
    Table6D940 *table;

    self = BMemPMgrAlloc(0x34);
    if (self != NULL) {
        table = func_8002C3A8();
        table->slot08(self, arg1);
        return self;
    }
    return NULL;
}
```

This is the EXACT same shape as `class_3bb8c_k`'s `func_80052B70`
(matched earlier this round, same runner) -- success path's `return self;`
inside the `if`-body, failure path's `return NULL;` trailing and
unconditional. Reused directly rather than re-derived, and it matched on
the first attempt: further confirmation of the proposed learning in
`func_80052B70`'s own report (GCC 2.6.3 -O2 folds only THAT shape into a
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
