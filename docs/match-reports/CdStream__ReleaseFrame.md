# CdStream__ReleaseFrame -- MATCHED (exact length, 24/24 words), round 82

> Renamed from `CdStreamObj__ReleaseFrame` on 2026-09-26 (tools/rename.py). Address 0x800477b0.

> Renamed from `func_800477B0` on 2026-09-25 (tools/rename.py). Address 0x800477b0.

Round 82, runner delta. Unit `src/cd/CdStream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** not in the table (between slot +0x06C `CdStream__GetNextFrame` and +0x070); reached from elsewhere.
- **What:** if the +0x48 callback is set, call it with +0x44, then `freeRing(self, base)` through slot +0x70.
- **Levers:** none needed.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `sActiveCdStream` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier B. `CdStream__ReleaseFrame` -- free function (not a slot), called from both the normal and the end-of-stream paths of GetNextFrame. Evidence: if `onFrameReady` is set, calls it, then frees the ring buffer via the `freeRing` slot -- releasing a consumed frame's buffer back to the ring. Tier B: mechanics are clear, the caller-facing purpose of `onFrameReady` itself is not established from this unit alone.

## Source

```c
void CdStream__ReleaseFrame(CdStreamObj *self, u32 *base) {
    if (self->cb48 != NULL) {
        self->cb48(self->cbArg);
        self->methods->freeRing(self, base);
    }
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__ReleaseFrame (tools/rename.py), the class rename only.
