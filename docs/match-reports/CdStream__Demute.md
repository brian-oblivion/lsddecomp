# CdStream__Demute -- MATCHED (exact length, 23/23 words), round 82

> Renamed from `CdStreamObj__Demute` on 2026-09-26 (tools/rename.py). Address 0x80047638.

> Renamed from `func_80047638` on 2026-09-25 (tools/rename.py). Address 0x80047638.

Round 82, runner delta. Unit `src/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** slot +0x068 of gCdStreamMethods.
- **What:** if muted (+0x30) and active, busy-wait `CdControl(0xC /*CdlDemute*/, 0, 0)` until it succeeds, then clear +0x30.
- **Levers:** `while (CdControl(...) == 0) {}` gives the retail loop with the constant re-materialised in the delay slot.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStream` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `CdStream__Demute` -- slot +0x068. Evidence: busy-waits `CdControl(CdlDemute)` and clears the `muted` flag, only when active and currently muted.

## Source

```c
void CdStream__Demute(CdStreamObj *self) {
    if (self->muted != 0 && gActiveCdStream == self) {
        while (CdControl(0xC, 0, 0) == 0) {
        }
        self->muted = 0;
    }
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__Demute (tools/rename.py), the class rename only.
