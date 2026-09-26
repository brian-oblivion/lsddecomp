# New_Class6EED8 -- MATCHED (24/24 words), round 82

> Renamed from `New_D8006EED8` on 2026-09-26 (tools/rename.py). Address 0x800422cc.

> Renamed from `func_800422CC` on 2026-09-25 (tools/rename.py). Address 0x800422cc.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator). Called by code_2a0e0.c (`self->seqData = New_Class6EED8(arg)`).
- **What:** `BMemPMgrAlloc(0x30)`; if non-NULL, calls slot +0x008 (ctor, Class6EED8__Class6EED8) of `GetClass6EED8Methods()` (the gClass6EED8Methods table) with `(obj, arg)` and returns obj, else NULL.
- **Result:** byte-exact, 24/24 words, 0 ins / 0 del, whole-image SHA1 green. First build (round-82 allocator shape, one pass-through argument kept in `$s1`).
- **Types:** unit-local `CtorArg1Methods_322b4` (ctor at +0x008 taking one s32) and a prototype for `GetClass6EED8Methods`; no shared header touched.

## Source

```c
typedef struct CtorArg1Methods_322b4 {
    u8 pad00[0x8];
    void (*ctor)(void *self, s32 arg); /* +0x008 */
} CtorArg1Methods_322b4;
void *GetClass6EED8Methods(void);

void *New_Class6EED8(s32 arg) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        ((CtorArg1Methods_322b4 *)GetClass6EED8Methods())->ctor(obj, arg);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_Class6EED8` -- tier A. Allocator: BMemPMgrAlloc(0x30) then the ctor slot.

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `Class6EED8` in `include/Class6EED8.h`
(CLASS6D430_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`Class6D430Methods *`. The Source block above is the round-82 text; the live
body in `src/code_322b4.c` is byte-identical.

Renamed from `New_D8006EED8` with rename.py (the class name). It now
takes `char *name` and returns `Class6EED8 *`: the ctor it forwards to
copies the argument with `strcpy`, so it is a name; the only caller,
`WBgm__SetSeq`, casts its `s32` argument. No cast view is needed any more,
`GetClass6EED8Methods()->ctor(obj, name)` is the inherited ctor slot.
