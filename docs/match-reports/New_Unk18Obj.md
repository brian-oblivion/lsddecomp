# New_Unk18Obj — MATCH (20/20 words)

> Renamed from `func_8003E5D8` on 2026-09-19 (tools/rename.py). Address 0x8003e5d8.

**Unit:** code_2cc8c_c · **Size:** 20 instructions

## What it does

The `New_X` allocator for the class whose table is returned by
`GetUnk18ObjMethods` (external, still-uncarved remainder of this segment): the
same allocator `IntermediateBase__Init` calls to fill `self->unk18` when no override
was supplied. Allocates a 0xBC-byte instance and, on success, runs the
class's own constructor through the table's slot +0x008
(`GetUnk18ObjMethods()->ctor(self)`), returning the new instance; returns `NULL`
explicitly on allocation failure.

This unit's SECOND shared-table region: `D_8006E8E4` (`tools/classtable.py
D_8006E8E4`, 45 slots) is the table `GetUnk18ObjMethods` returns and this
function constructs an instance of, and it accounts for the rest of this
round's queue (`Unk18Obj__Unk18Obj`/`Unk18Obj__Finalize`/`Unk18Obj__AddChild`/
`Unk18Obj__RemoveChild` = its own slots `+0x008`/`+0x00C`/`+0x010`/`+0x014`) --
`gIntermediateBaseMethods` (see `IntermediateBase__OnNotify.md`) explained everything else.

## The C

```c
Unk18Obj *New_Unk18Obj(void)
{
    Unk18Obj *self;

    self = BMemPMgrAlloc(0xBC);
    if (self != NULL) {
        GetUnk18ObjMethods()->ctor(self);
        return self;
    }
    return NULL;
}
```

Matched on the first build. The explicit `return NULL;` (rather than
falling off the end of the function relying on the allocator's own `$v0`)
was chosen over the `New_Class6D3C8`/no-explicit-return idiom because
retail's own `beqz`-delay-slot zeroes `$v0` explicitly right at the branch
(redundant with the allocator's own already-zero return on failure) --
the same explicit-return shape `class_39e08.c`'s `New_Obj865C8` uses, and
the byte match confirms the reading.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**New_Unk18Obj** (renamed from `func_8003E5D8`, round 55, runner alpha).
Tier A: standard `New_Class` allocator idiom already established in this
project (allocate `0xBC` bytes via `BMemPMgrAlloc`, then invoke the class's
own ctor through its vtable getter) -- matches this unit's own class
placeholder name `Unk18Obj`.
