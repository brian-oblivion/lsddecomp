# ObjM__ClearCloseReadyFlag

> Renamed from `func_80054200` on 2026-09-23 (tools/rename.py). Address 0x80054200.

**Unit:** ObjMStyleActor · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What this function does

Trivial single-statement setter, whole body is `jr $ra` / `sw $zero,
0x84($a0)` (the store lives in the branch delay slot, executing before the
return completes).

```c
void ObjM__ClearCloseReadyFlag(ObjM *self) {
    self->unk84 = 0;
}
```

## Residue

None.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `ObjMStyleActor`.

## Naming

**ObjM__ClearCloseReadyFlag** -- tier A. Trivial single-statement setter, `self->unk84 = 0` -- the whole body is `jr $ra` / `sw $zero`. A plain setter is tier A by the FINISHING-PLAN definition (mechanics ARE the purpose).


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.
