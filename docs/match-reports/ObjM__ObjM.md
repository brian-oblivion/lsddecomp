# ObjM__ObjM

> Renamed from `func_80052C10` on 2026-09-24 (tools/rename.py). Address 0x80052c10.

**Unit:** class_3bb8c_k · **Size:** 50 instructions (0xC8 bytes) ·
**Status: MATCHED 50/50**, whole-image SHA1 green (matched together with
`New_ObjM` in the same round; see that report for the residue that
mattered on the sibling function -- this one matched cleanly on the first
attempt at its final source shape).

## Role

The ctor for the class whose method table is `D_80087034` (`GetObjMMethods`'s
return value) -- this IS the function `New_ObjM`'s `New_X` allocator
dispatches to through that vtable's `+0x008` slot. First chains to the base
class's ctor (`GetClass86668Methods()->ctor(self, 0, arg1)`, the shared
`Class86668Methods` accessor already declared in `include/class_39e08.h`),
then sets `self->methods` to `GetObjMMethods()`'s vtable, fills several
fields from its own arguments, and finally dispatches `self->methods->slot40(self)`
as a post-construct hook.

## Struct/type notes

Added a unit-LOCAL pair of types, `Class87034Methods_3bb8c_k` /
`Obj87034_3bb8c_k`, in `src/class_3bb8c_k.c` itself (not in the shared
`include/class_3bb8c.h`) -- deliberately, per this round's header-contention
rule. `class_3bb8c_l` (echo, live in the same round) already has its OWN
independent view of the SAME table (`Obj87034Methods_3bb8c_l` in the shared
header), reaching a disjoint set of slots (0x004/0x010/0x014/0x048/0x074/
0x07C/0x080/0x084/0x088/0x08C/0x0C0/0x0C4/0x0C8/0x0D0/0x0D4). This unit's
two functions reach only +0x008 (ctor) and +0x040 (the post-construct
hook) -- no offset overlap with echo's view, so this stays additive in
principle, but since neither function outside this unit needs it, it was
kept fully local rather than added to the shared header at all (the
narrower/safer form of "additive").

`Obj87034_3bb8c_k`'s fields (`unk38`, `unk54`, `unk60`, `unk64`, `unk68`,
`unk6C` (`SubObjB *`), `unk70`, `unk74`, `unk80`, `unk84`) total exactly
0x88 bytes with the trailing padding -- matching `New_ObjM`'s own
alloc size, a nice independent confirmation that this is the right object.

## Signature

```c
void ObjM__ObjM(Obj87034_3bb8c_k *self, SubObjB *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
```

`arg1`'s type (`SubObjB *`) is fixed by the base-ctor call
(`GetClass86668Methods()->ctor(self, 0, arg1)` dispatches through
`Class86668Methods::ctor`, already typed `(Obj865C8 *, s32, SubObjB *)` in
`include/class_39e08.h`) -- `arg1` is forwarded there verbatim as that
call's `SubObjB *` argument, and separately stored into `self->unk6C`.

## Final source

```c
void ObjM__ObjM(Obj87034_3bb8c_k *self, SubObjB *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    GetClass86668Methods()->ctor((Obj865C8 *)self, 0, arg1);
    self->methods = GetObjMMethods();
    self->unk64 = 0;
    self->unk68 = 0;
    self->unk60 = 1;
    self->unk54 = arg2;
    self->unk38 = arg5;
    self->unk6C = arg1;
    self->unk74 = arg3;
    self->unk70 = arg4;
    self->unk80 = 0;
    self->unk84 = 0;
    self->methods->slot40(self);
}
```

Field-write order matters and follows retail exactly: the base ctor call
first, then `self->methods` assignment, then the three zero/one writes
(`unk64`, `unk68`, `unk60`) in that order, then `unk54`/`unk38`/`unk6C`/
`unk74`/`unk70` in that specific (non-declaration, non-offset) order, then
`unk80`/`unk84`, then the final dispatch. `self->methods->slot40(self)`
reloads `self->methods` from memory (rather than reusing the value already
in a register from the earlier assignment) -- consistent with the rest of
this project's observation that a struct-field read right after a
struct-field write commonly recompiles as a genuine reload, not a cached
register reuse.

## Naming

Round 75 (bravo, track 3). `func_80052C10` -> `ObjM__ObjM`, **tier A**.

Slot +0x008 of D_80087034 (`tools/classtable.py 0x80087034`), the ctor New_ObjM calls. Runs the base Class86668 ctor, sets methods = GetObjMMethods(), stores its arguments and clears fields, then calls +0x040.

Local view fields named round 75 (class_3bb8c_k's `ObjM_3bb8c_k` only):
`pauseSetupStep` (+0x080) and `closeReady` (+0x084), tier B, from the
named class_3bb8c_m methods that read them (ObjM__AdvancePauseSetup counts
+0x080; ObjM__UpdateCloseReadyFlag/ObjM__ClearCloseReadyFlag/
ObjM__CloseAndNotifyC/D set, clear and test +0x084). The other eight
fields stay `unkNN`: this ctor only stores arguments or constants into them.

## Proposed field names

For the SHARED `ObjM` struct in include/class_3bb8c.h (accessors in
class_3bb8c_m, not this unit, so not applied here):

| field | proposed | tier | evidence |
| --- | --- | --- | --- |
| `ObjM::unk80` | `pauseSetupStep` | B | ObjM__AdvancePauseSetup's step counter; ObjM__UpdateCloseReadyFlag requires it non-zero; ObjM__ObjM zeroes it |
| `ObjM::unk84` | `closeReady` | B | set by ObjM__UpdateCloseReadyFlag, cleared by ObjM__ClearCloseReadyFlag, gates ObjM__CloseAndNotifyC/D |
