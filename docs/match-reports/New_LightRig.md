# New_LightRig -- MATCHED (20/20 words), round 82

> Renamed from `New_D8006EFAC` on 2026-09-26 (tools/rename.py). Address 0x80042694.

> Renamed from `func_80042694` on 2026-09-25 (tools/rename.py). Address 0x80042694.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator).
- **What:** `BMemPMgrAlloc(0x54)`; if non-NULL, calls slot +0x008 (ctor) of `GetLightRigMethods()` (the gLightRigMethods table) on it and returns it, else NULL.
- **Result:** byte-exact, 20/20 words, 0 ins / 0 del, whole-image SHA1 green. First build, with the round-82 allocator shape (same as New_D8006EF50).
- **Types:** prototype `void *GetLightRigMethods(void);` added to the unit (defined later in the same unit); no shared header touched.

## Source

```c
/* Allocate and construct a LightRig (0x54 bytes). */
void *New_LightRig(void) {
    void *obj = BMemPMgrAlloc(0x54);

    if (obj != NULL) {
        ((Slot08Methods_322b4 *)GetLightRigMethods())->init(obj);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_D8006EFAC` -- tier A. Allocator: BMemPMgrAlloc(0x54) then the ctor slot.
