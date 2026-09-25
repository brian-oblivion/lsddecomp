# New_D8006EED8 -- MATCHED (24/24 words), round 82

> Renamed from `func_800422CC` on 2026-09-25 (tools/rename.py). Address 0x800422cc.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator). Called by code_2a0e0.c (`self->seqData = New_D8006EED8(arg)`).
- **What:** `BMemPMgrAlloc(0x30)`; if non-NULL, calls slot +0x008 (ctor, func_8004232C) of `func_800423F0()` (the D_8006EED8 table) with `(obj, arg)` and returns obj, else NULL.
- **Result:** byte-exact, 24/24 words, 0 ins / 0 del, whole-image SHA1 green. First build (round-82 allocator shape, one pass-through argument kept in `$s1`).
- **Types:** unit-local `CtorArg1Methods_322b4` (ctor at +0x008 taking one s32) and a prototype for `func_800423F0`; no shared header touched.

## Source

```c
typedef struct CtorArg1Methods_322b4 {
    u8 pad00[0x8];
    void (*ctor)(void *self, s32 arg); /* +0x008 */
} CtorArg1Methods_322b4;
void *func_800423F0(void);

void *New_D8006EED8(s32 arg) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        ((CtorArg1Methods_322b4 *)func_800423F0())->ctor(obj, arg);
        return obj;
    }
    return NULL;
}
```
