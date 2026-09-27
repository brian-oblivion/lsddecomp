# New_Viewport — MATCH (20/20 words)

> Renamed from `New_Unk18Obj` on 2026-09-25 (tools/rename.py). Address 0x8003e5d8.

> Renamed from `func_8003E5D8` on 2026-09-19 (tools/rename.py). Address 0x8003e5d8.

**Unit:** code_2cc8c · **Size:** 20 instructions

## What it does

The `New_X` allocator for the class whose table is returned by
`GetViewportMethods` (external, still-uncarved remainder of this segment): the
same allocator `IntermediateBase__Init` calls to fill `self->unk18` when no override
was supplied. Allocates a 0xBC-byte instance and, on success, runs the
class's own constructor through the table's slot +0x008
(`GetViewportMethods()->ctor(self)`), returning the new instance; returns `NULL`
explicitly on allocation failure.

This unit's SECOND shared-table region: `gViewportMethods` (`tools/classtable.py
gViewportMethods`, 45 slots) is the table `GetViewportMethods` returns and this
function constructs an instance of, and it accounts for the rest of this
round's queue (`Viewport__Viewport`/`Viewport__Finalize`/`Viewport__AddChild`/
`Viewport__RemoveChild` = its own slots `+0x008`/`+0x00C`/`+0x010`/`+0x014`) --
`gIntermediateBaseMethods` (see `IntermediateBase__OnNotify.md`) explained everything else.

## The C

```c
Unk18Obj *New_Viewport(void)
{
    Unk18Obj *self;

    self = BMemPMgrAlloc(0xBC);
    if (self != NULL) {
        GetViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}
```

Matched on the first build. The explicit `return NULL;` (rather than
falling off the end of the function relying on the allocator's own `$v0`)
was chosen over the `New_GameApplication`/no-explicit-return idiom because
retail's own `beqz`-delay-slot zeroes `$v0` explicitly right at the branch
(redundant with the allocator's own already-zero return on failure) --
the same explicit-return shape `class_39e08.c`'s `New_DayTask` uses, and
the byte match confirms the reading.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c. Matched on the
first build.

## Naming

**New_Unk18Obj** (renamed from `func_8003E5D8`, round 55, runner alpha).
Tier A: standard `New_Class` allocator idiom already established in this
project (allocate `0xBC` bytes via `BMemPMgrAlloc`, then invoke the class's
own ctor through its vtable getter) -- matches this unit's own class
placeholder name `Unk18Obj`.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `New_Unk18Obj`. The allocator, `New_<Class>`: BMemPMgrAlloc(0xBC), which is the object size in the header. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Track 7 (round 98, echo)

`BMemPMgrAlloc(0xBC)` -> `BMemPMgrAlloc(sizeof(Viewport))`: Viewport.h's
struct is 0xBC bytes, and the oracle agrees (byte-identical).
