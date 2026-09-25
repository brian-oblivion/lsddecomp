# New_ScreenSprite -- MATCHED (31/31 words), round 82

> Renamed from `New_D8006ED4C` on 2026-09-25 (tools/rename.py). Address 0x80041c9c.

> Renamed from `func_80041C9C` on 2026-09-25 (tools/rename.py). Address 0x80041c9c.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator for ScreenSprite (gScreenSpriteMethods), the screen-space sprite, 0xA8 bytes).
- **What:** `BMemPMgrAlloc(0xA8)`; if non-NULL, calls slot +0x008 (ctor, ScreenSprite__ScreenSprite) of `GetScreenSpriteMethods()` (the gScreenSpriteMethods table) with `(obj, a1, a2, a3)` and returns obj, else NULL.
- **Result:** byte-exact, 31/31 words, 0 ins / 0 del, whole-image SHA1 green. First build (round-82 allocator shape, three pass-through args in `$s1..$s3`).
- **Types:** the three arguments are left `void *` (their meaning belongs to ScreenSprite__ScreenSprite, still INCLUDE_ASM). Unit-local `CtorArg3Methods_322b4` and a prototype for `GetScreenSpriteMethods`; no shared header touched.

## Source

```c
void *GetScreenSpriteMethods(void);
typedef struct CtorArg3Methods_322b4 {
    u8 pad00[0x8];
    void *(*ctor)(void *self, void *a1, void *a2, void *a3); /* +0x008 = ScreenSprite__ScreenSprite */
} CtorArg3Methods_322b4;

void *New_ScreenSprite(void *a1, void *a2, void *a3) {
    void *obj = BMemPMgrAlloc(0xA8);

    if (obj != NULL) {
        ((CtorArg3Methods_322b4 *)GetScreenSpriteMethods())->ctor(obj, a1, a2, a3);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_D8006ED4C` -- tier A. Allocator: BMemPMgrAlloc(0xA8) then the ctor slot. Same New_<Class> convention.
