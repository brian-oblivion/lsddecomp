# New_CdStreamObj -- MATCHED (exact length, 31/31 words), round 82

> Renamed from `func_80046F0C` on 2026-09-25 (tools/rename.py). Address 0x80046f0c.

Round 82, runner delta. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** the class's allocator (`New_` shape).
- **What:** `BMemPMgrAlloc(0x5C)`, and if non-NULL run the ctor (slot +0x08, `CdStreamObj__CdStreamObj`) with three arguments, return the object, else NULL.
- **Levers:** the documented `if (obj != NULL) { ctor; return obj; } return NULL;` shape. The early-return form (`if (obj == NULL) return NULL; ctor; return obj;`) measured 2 words LONG: it adds `j` + `move v0,s0` in the delay slot instead of retail's `move v0,zero` in the `beqz` delay slot.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStreamObj` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `New_CdStreamObj` -- the class allocator (`New_Class` convention): `BMemPMgrAlloc(0x5C)` then runs the ctor slot, returning the object or NULL. Evidence: the body itself (allocate, ctor, return-or-NULL), the `New_Pad`/`New_Class6B5CC` precedent in include/Pad.h and include/Class6B5CC.h.

## Source

```c
CdStreamObj *New_CdStreamObj(s32 arg1, s32 arg2, s32 arg3) {
    CdStreamObj *obj = BMemPMgrAlloc(0x5C);

    if (obj != NULL) {
        Get_vtable_CdStreamObj()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}
```
