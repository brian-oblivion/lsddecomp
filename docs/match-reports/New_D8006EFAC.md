# New_D8006EFAC -- MATCHED (20/20 words), round 82

> Renamed from `func_80042694` on 2026-09-25 (tools/rename.py). Address 0x80042694.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator).
- **What:** `BMemPMgrAlloc(0x54)`; if non-NULL, calls slot +0x008 (ctor) of `func_800428E4()` (the D_8006EFAC table) on it and returns it, else NULL.
- **Result:** byte-exact, 20/20 words, 0 ins / 0 del, whole-image SHA1 green. First build, with the round-82 allocator shape (same as New_D8006EF50).
- **Types:** prototype `void *func_800428E4(void);` added to the unit (defined later in the same unit); no shared header touched.

## Source

```c
/* Allocate and construct a D_8006EFAC object (0x54 bytes). */
void *New_D8006EFAC(void) {
    void *obj = BMemPMgrAlloc(0x54);

    if (obj != NULL) {
        ((Slot08Methods_322b4 *)func_800428E4())->init(obj);
        return obj;
    }
    return NULL;
}
```
