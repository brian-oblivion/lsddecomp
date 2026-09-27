# CdStream__OnStreamEnd -- MATCHED (exact length, 24/24 words), round 82

> Renamed from `CdStreamObj__OnStreamEnd` on 2026-09-26 (tools/rename.py). Address 0x80047810.

> Renamed from `func_80047810` on 2026-09-25 (tools/rename.py). Address 0x80047810.

Round 82, runner delta. Unit `src/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** not in the table; reached from elsewhere.
- **What:** if the +0x4C callback is set, call the +0x48 callback with +0x44 (sic: tests +0x4C, calls +0x48), then slot +0x48 on self.
- **Levers:** none needed. The +0x4C-test/+0x48-call asymmetry is retail's.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStream` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier B. `CdStream__OnStreamEnd` -- free function (not a slot), called only from GetNextFrame's end-of-stream branch. Evidence: retail tests `onStreamEnd` but calls `onFrameReady` (an asymmetry the report already flagged as retail's own behaviour, not a bug to fix), then closes the stream via the `close` slot. Tier B: the test/call asymmetry means the exact caller contract isn't fully pinned down, but "this runs when the stream ends" is solid.

## Source

```c
void CdStream__OnStreamEnd(CdStreamObj *self) {
    if (self->cb4C != NULL) {
        self->cb48(self->cbArg);
        self->methods->slot48(self);
    }
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__OnStreamEnd (tools/rename.py), the class rename only.
