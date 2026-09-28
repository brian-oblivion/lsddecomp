# CdStream__Sync -- MATCHED (exact length, 10/10 words), round 81

> Renamed from `CdStreamObj__Sync` on 2026-09-26 (tools/rename.py). Address 0x800478d0.

> Renamed from `func_800478D0` on 2026-09-25 (tools/rename.py). Address 0x800478d0.

Round 81, runner echo. Unit `src/cd/cd_stream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot of gCdStreamMethods.
- **What:** `CdSync(mode, &self->cdResult)`: the object carries a CdSync result buffer at +0x24. Return type unconstrained by the bytes.
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/cd/cd_stream.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `CdStream__Sync` -- free function (not a slot), a thin wrapper: `CdSync(mode, &self->cdResult)`. Evidence: the libcd call and the result buffer it writes into name the operation directly.

## Source

```c
int CdStream__Sync(CdStreamObj *self, int mode) {
    return CdSync(mode, self->cdResult);
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/cd_stream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__Sync (tools/rename.py), the class rename only.
