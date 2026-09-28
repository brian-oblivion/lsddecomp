# GetCdStreamMethods -- MATCHED (exact length, 4/4 words), round 81

> Renamed from `Get_vtable_CdStream` on 2026-09-28 (tools/rename.py). Address 0x80047900.

> Renamed from `Get_vtable_CdStreamObj` on 2026-09-26 (tools/rename.py). Address 0x80047900.

> Renamed from `func_80047900` on 2026-09-25 (tools/rename.py). Address 0x80047900.

Round 81, runner echo. Unit `src/cd/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** returns the address of the class method table gCdStreamMethods (game code per revision 18, not the SDK adjacency lead).
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/cd/CdStream.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `GetCdStreamMethods` -- returns `&gCdStreamMethods`. Evidence: matches the established `Get_vtable_<Class>` convention used by `GetBasicClassMethods`, `Get_vtable_Pad`, `Get_vtable_Entity`, etc.

## Source

```c
CdStreamObjMethods *GetCdStreamMethods(void) {
    return &gCdStreamMethods;
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from Get_vtable_CdStreamObj (tools/rename.py), the class rename.
