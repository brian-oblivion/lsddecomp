# CdStream__Mute -- MATCHED (exact length, 24/24 words), round 82

> Renamed from `CdStreamObj__Mute` on 2026-09-26 (tools/rename.py). Address 0x800475d8.

> Renamed from `func_800475D8` on 2026-09-25 (tools/rename.py). Address 0x800475d8.

Round 82, runner delta. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** slot +0x064 of gCdStreamMethods.
- **What:** if not muted and active, busy-wait `CdControl(0xB /*CdlMute*/, 0, 0)`, then set +0x30 = 1.
- **Levers:** none needed.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStream` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `CdStream__Mute` -- slot +0x064. Evidence: busy-waits `CdControl(CdlMute)` and sets the `muted` flag, only when active and not already muted.

## Source

```c
void CdStream__Mute(CdStreamObj *self) {
    if (self->muted == 0 && gActiveCdStream == self) {
        while (CdControl(0xB, 0, 0) == 0) {
        }
        self->muted = 1;
    }
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__Mute (tools/rename.py), the class rename only.
