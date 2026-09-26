# RequestedFile__RequestedFile -- MATCHED (31/31 words), round 82

> Renamed from `Class6EED8__Class6EED8` on 2026-09-26 (tools/rename.py). Address 0x8004232c.

> Renamed from `D8006EED8__D8006EED8` on 2026-09-26 (tools/rename.py). Address 0x8004232c.

> Renamed from `func_8004232C` on 2026-09-25 (tools/rename.py). Address 0x8004232c.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** gRequestedFileMethods slot +0x008 (ctor; the object New_RequestedFile allocates).
- **What:** calls the base ctor (slot +0x008 of `GetActiveDataSourceMethods()`) on self, installs the gRequestedFileMethods table (`GetRequestedFileMethods()`), clears +0x02C, and if `name` is non-NULL copies it into a 32-byte stack buffer with `strcpy` and passes the buffer to slot +0x06C.
- **Result:** byte-exact, 31/31 words, 0 ins / 0 del, whole-image SHA1 green. First build. The 0x40 frame is buf[32] at sp+0x10 plus s0/s1/ra.
- **Types:** unit-local `D_8006EED8Obj` gained `methods` at +0x000; local `D_8006EED8Methods` (slot6C at +0x06C) and `Slot08Arg0Methods_322b4` (a no-arg ctor view for GetActiveDataSourceMethods' table, which the unit already views as `Slot0CMethods_322b4` for finalize); `extern char *strcpy(char *, char *);` as other units spell it. No shared header touched.

## Source

```c
typedef struct D_8006EED8Methods D_8006EED8Methods;
typedef struct D_8006EED8Obj {
    D_8006EED8Methods *methods; /* +0x000 */
    u8 pad04[0x2C - 0x4];
    s32 flag2C;
} D_8006EED8Obj;
struct D_8006EED8Methods {
    u8 pad00[0x6C];
    void (*slot6C)(D_8006EED8Obj *self, char *name); /* +0x06C */
};
typedef struct Slot08Arg0Methods_322b4 {
    u8 pad00[0x8];
    void (*ctor)(void *self); /* +0x008 */
} Slot08Arg0Methods_322b4;
extern char *strcpy(char *dst, char *src);

void RequestedFile__RequestedFile(D_8006EED8Obj *self, char *name) {
    char buf[32];

    ((Slot08Arg0Methods_322b4 *)GetActiveDataSourceMethods())->ctor(self);
    self->methods = GetRequestedFileMethods();
    self->flag2C = 0;
    if (name != NULL) {
        strcpy(buf, name);
        self->methods->slot6C(self, buf);
    }
}
```

## Naming

- `RequestedFile__RequestedFile` -- tier A. Ctor (slot +0x008): the base (GetActiveDataSourceMethods) ctor, installs the table, clears flag2C, and if given a name, copies it to a stack buffer and hands it to slot +0x06C.

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `RequestedFile` in `include/RequestedFile.h`
(FILERESOURCE_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`FileResourceMethods *`. The Source block above is the round-82 text; the live
body in `src/code_322b4.c` is byte-identical.

Renamed from `D8006EED8__D8006EED8` with rename.py (the class name).
`flag2C` is `loaded`: the only setter of 1 is `RequestedFile__MarkLoaded`, the
+0x064 setFlag override, and the CD driver calls setFlag when a queued
operation completes (`CdDriver__LoadFile`, the request dispatch in
code_179d8_s); the one reader, `WBgm__HandleMonitorEvent`, waits for it
before handing `buffer` to `SsSeqOpen`. `slot6C` is FileResource's
inherited `requestLoadFile` (CD occupant `CdDriver__RequestLoadFile`).
The base ctor call is `GetActiveDataSourceMethods()->ctor((FileResource *)self)`.
