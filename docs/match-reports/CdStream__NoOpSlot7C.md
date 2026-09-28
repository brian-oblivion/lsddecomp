# CdStream__NoOpSlot7C -- MATCHED (exact length, 2/2 words), round 81

> Renamed from `CdStreamObj__func_800478F8` on 2026-09-26 (tools/rename.py). Address 0x800478f8.

> Renamed from `func_800478F8` on 2026-09-25 (tools/rename.py). Address 0x800478f8.

Round 81, runner echo. Unit `src/cd/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x07C of gCdStreamMethods (last slot).
- **What:** an empty method (`jr $ra; nop`).
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/cd/CdStream.c`, declared from the Psy-Q prototypes.

## Naming

Tier C. Kept the tier-C `Class__func_xxxxx` form: slot +0x07C, the table's last slot, an empty override with no evidence of what it would do if implemented.

## Source

```c
void CdStream__NoOpSlot7C(CdStreamObj *self) {
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__func_800478F8 (tools/rename.py): the empty occupant of slot +0x07C, named for its slot. The slot has callers: MoviePlayer__Stop passes (MoviePlayer__MarkStopped, player) and MoviePlayer__Abort (0, 0), so the slot is typed `(self, fn, arg)` from their bytes while this body takes self only; with an empty body the slot's intent (a callback setter, by its arguments) is not evidenced here, so it is not named for it.
