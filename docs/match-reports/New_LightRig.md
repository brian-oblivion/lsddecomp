# New_LightRig -- MATCHED (20/20 words), round 82

> Renamed from `New_D8006EFAC` on 2026-09-26 (tools/rename.py). Address 0x80042694.

> Renamed from `func_80042694` on 2026-09-25 (tools/rename.py). Address 0x80042694.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator).
- **What:** `BMemPMgrAlloc(0x54)`; if non-NULL, calls slot +0x008 (ctor) of `GetLightRigMethods()` (the gLightRigMethods table) on it and returns it, else NULL.
- **Result:** byte-exact, 20/20 words, 0 ins / 0 del, whole-image SHA1 green. First build, with the round-82 allocator shape (same as New_FrameClock).
- **Types:** prototype `void *GetLightRigMethods(void);` added to the unit (defined later in the same unit); no shared header touched.

## Source

```c
/* Allocate and construct a LightRig (0x54 bytes). */
LightRig *New_LightRig(void) {
    LightRig *obj = BMemPMgrAlloc(0x54);

    if (obj != NULL) {
        GetLightRigMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_D8006EFAC` -- tier A. Allocator: BMemPMgrAlloc(0x54) then the ctor slot.

## Track 4

2026-09-26, round 86 (delta): class 0x14 unified as LightRig in `include/LightRig.h`. Renamed from `New_D8006EFAC`, tier A: the allocator, `BMemPMgrAlloc(0x54)` (the object size in include/LightRig.h). Returns `LightRig *` (was `void *`) and calls `GetLightRigMethods()->ctor(obj)` (was a `Slot08Methods_322b4` cast). Its one caller, IntermediateBase__Init, casts the result to IntermediateBase's `BasicClass *unk14`. The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

Allocation size spelled `sizeof(LightRig)` (0x54). Byte-exact.
