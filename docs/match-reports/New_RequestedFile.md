# New_RequestedFile -- MATCHED (24/24 words), round 82

> Renamed from `New_Class6EED8` on 2026-09-26 (tools/rename.py). Address 0x800422cc.

> Renamed from `New_D8006EED8` on 2026-09-26 (tools/rename.py). Address 0x800422cc.

> Renamed from `func_800422CC` on 2026-09-25 (tools/rename.py). Address 0x800422cc.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator). Called by code_2a0e0.c (`self->seqData = New_RequestedFile(arg)`).
- **What:** `BMemPMgrAlloc(0x30)`; if non-NULL, calls slot +0x008 (ctor, RequestedFile__RequestedFile) of `GetRequestedFileMethods()` (the gRequestedFileMethods table) with `(obj, arg)` and returns obj, else NULL.
- **Result:** byte-exact, 24/24 words, 0 ins / 0 del, whole-image SHA1 green. First build (round-82 allocator shape, one pass-through argument kept in `$s1`).
- **Types:** unit-local `CtorArg1Methods_322b4` (ctor at +0x008 taking one s32) and a prototype for `GetRequestedFileMethods`; no shared header touched.

## Source

```c
typedef struct CtorArg1Methods_322b4 {
    u8 pad00[0x8];
    void (*ctor)(void *self, s32 arg); /* +0x008 */
} CtorArg1Methods_322b4;
void *GetRequestedFileMethods(void);

void *New_RequestedFile(s32 arg) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        ((CtorArg1Methods_322b4 *)GetRequestedFileMethods())->ctor(obj, arg);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_RequestedFile` -- tier A. Allocator: BMemPMgrAlloc(0x30) then the ctor slot.

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `RequestedFile` in `include/RequestedFile.h`
(FILERESOURCE_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`FileResourceMethods *`. The Source block above is the round-82 text; the live
body in `src/code_322b4.c` is byte-identical.

Renamed from `New_D8006EED8` with rename.py (the class name). It now
takes `char *name` and returns `RequestedFile *`: the ctor it forwards to
copies the argument with `strcpy`, so it is a name; the only caller,
`WBgm__SetSeq`, casts its `s32` argument. No cast view is needed any more,
`GetRequestedFileMethods()->ctor(obj, name)` is the inherited ctor slot.
