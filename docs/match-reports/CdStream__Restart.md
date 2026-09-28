# CdStream__Restart -- MATCHED (exact length, 21/21 words), round 82

> Renamed from `CdStreamObj__Restart` on 2026-09-26 (tools/rename.py). Address 0x80047574.

> Renamed from `func_80047574` on 2026-09-25 (tools/rename.py). Address 0x80047574.

Round 82, runner delta. Unit `src/cd/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** slot +0x058 of gCdStreamMethods.
- **What:** if state (+0x2C) is 4 and this object is the active one (`sActiveCdStream`), reset state to 0 and re-issue `seek(self, self->loc)` (slot +0x4C) on the saved location at +0x0C.
- **Levers:** go through the loaded global (`cur = sActiveCdStream; if (cur == self) cur->...`), not `self`: retail uses `$a0` = global for every access and keeps `self` in `$a1`.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `sActiveCdStream` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `CdStream__Restart` -- slot +0x058. Evidence: only fires from the stopped state; resets to idle and re-issues `seek` on the same saved location -- restarting the stream from its start point.

## Source

```c
void CdStream__Restart(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->unk2C == 4) {
        cur = sActiveCdStream;
        if (cur == self) {
            cur->unk2C = 0;
            cur->methods->seek(cur, cur->loc);
        }
    }
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__Restart (tools/rename.py), the class rename only.
