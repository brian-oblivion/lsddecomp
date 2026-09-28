# ObjM__TeardownPauseOverlay

> Renamed from `func_800543FC` on 2026-09-23 (tools/rename.py). Address 0x800543fc.

**Unit:** ObjMStyleActor · **Size:** 54 instructions · **Status:** MATCHED (54/54 words)

## Context

Establishes three new `ObjM` fields and their target types: `unk34`
(`FieldM34`, vtable slot `0x8C` here, `0x88` in `ObjM__AdvancePauseSetup`), `unk54`
(`FieldM50`, vtable slot `0x50` here, `0x4C` in `ObjM__AdvancePauseSetup`), and
`unk7C` (`FieldM7C`, vtable slot `0x4` here, `0x4C`/`0xB8` in
`ObjM__AdvancePauseSetup`).

**One RETYPE of an existing field, flagged explicitly (per the shared-
header rule):** `ObjM::unk10` was `s32` from round 15a's
`ObjM__StartFadeUp` (established there as a plain forwarded register value,
never dereferenced). This function dereferences the SAME field's vtable
directly (`self->unk10->methods->slot50(self->unk10)`), so it IS a
pointer -- retyped to `FieldM50 *`. This is the established ABI-neutral
retype pattern: a pointer value forwarded as a raw register argument
compiles identically whether declared `s32` or a pointer type. Verified:
full rebuild stays whole-image green, and `ObjM__StartFadeUp` itself still
scores 52/52 unchanged.

`FieldM50` is deliberately NOT unified with `include/task.h`'s
`Unk64Elem`, despite `unk7C` sharing every slot NUMBER and SIGNATURE with
it (0x004/0x04C/0x0B8) -- see the `FieldM7C` comment in
`include/class_3bb8c.h` for the reasoning (this unit keeps its own
independent view per the multiple-independent-local-views convention,
and `ObjM__AdvancePauseSetup`'s own call to `self->unk10->methods->slot4C` sets up
only ONE argument, an arity conflict with `Unk64ElemMethods::slot4C`'s
established 3-argument signature -- real counter-evidence against
unifying `unk10`/`unk54`'s type with `Unk64Elem`).

## What this function does

```c
void ObjM__TeardownPauseOverlay(ObjM *self) {
    if (self->unk80 != 0) {
        self->unk7C->methods->slot4(self->unk7C);
    }
    self->unk34->methods->slot8C(self->unk34);
    self->unk54->methods->slot50(self->unk54);
    self->unk10->methods->slot50(self->unk10);
    self->unk18->methods->slotB4(self->unk18, 1);
    self->unk80 = 0;
}
```

`self->unk18->methods->slotB4` confirms the slot round 15a's
`FieldM18Methods` comment anticipated ("ObjM__TeardownPauseOverlay (not this round's
target)").

## Residue

None -- matched on the first attempt, once the retype and the three new
field types were in place.

## Provenance

round 15b (2026-09-04), runner echo, second pass on `ObjMStyleActor`.

## Naming

**ObjM__TeardownPauseOverlay** -- tier B. Mirror of `ObjM__AdvancePauseSetup`: if the pause-setup counter is non-zero, destroys the "Pause"-named object (`self->unk7C`'s `slot4`) and notifies the same siblings (`unk34`/`unk54`/`unk10`/`unk18`), then resets the counter. Same evidence and tier as its counterpart.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and dream_day.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. Holders now typed: pauseText (TextRow release), sound (VabStreamObj unmute), bgm (WBgm resume), unk10 (FrameClock resume), viewport (NodeGuardedViewport setDrawEnabled).
