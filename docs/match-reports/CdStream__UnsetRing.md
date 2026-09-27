# CdStream__UnsetRing -- MATCHED (exact length, 8/8 words), round 81

> Renamed from `CdStreamObj__UnsetRing` on 2026-09-26 (tools/rename.py). Address 0x80047890.

> Renamed from `func_80047890` on 2026-09-25 (tools/rename.py). Address 0x80047890.

Round 81, runner echo. Unit `src/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x074 of gCdStreamMethods.
- **What:** tail wrapper: `StUnSetRing()` (libcd). Return type unconstrained by the bytes.
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/CdStream.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `CdStream__UnsetRing` -- slot +0x074. Evidence: tail-wraps `StUnSetRing()`.

## Source

```c
void CdStream__UnsetRing(CdStreamObj *self) {
    StUnSetRing();
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__UnsetRing (tools/rename.py), the class rename only.
