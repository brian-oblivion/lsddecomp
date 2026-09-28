# CdStream__FreeRing -- MATCHED (exact length, 8/8 words), round 81

> Renamed from `CdStreamObj__FreeRing` on 2026-09-26 (tools/rename.py). Address 0x80047870.

> Renamed from `func_80047870` on 2026-09-25 (tools/rename.py). Address 0x80047870.

Round 81, runner echo. Unit `src/cd/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x070 of gCdStreamMethods.
- **What:** tail wrapper: `StFreeRing(base)` on the second argument (libcd, LIBCD.H `u_long StFreeRing(u_long *base)`). The byte match says nothing about the return type.
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/cd/CdStream.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `CdStream__FreeRing` -- slot +0x070. Evidence: tail-wraps `StFreeRing(base)`; the libcd call names the operation.

## Source

```c
u32 CdStream__FreeRing(CdStreamObj *self, u32 *base) {
    return StFreeRing(base);
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__FreeRing (tools/rename.py), the class rename only.
