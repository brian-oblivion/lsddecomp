# CdStream__Stop -- MATCHED (exact length, 43/43 words), round 82

> Renamed from `CdStreamObj__Stop` on 2026-09-26 (tools/rename.py). Address 0x800474c8.

> Renamed from `func_800474C8` on 2026-09-25 (tools/rename.py). Address 0x800474c8.

Round 82, runner delta (second session). Unit `src/cd/CdStream.c`. Fresh
ground, no prior attempt. Byte-exact on the first build; whole-image SHA1
green.

- **Where:** slot +0x054 of gCdStreamMethods.
- **What:** if state is 2 and this is the active stream (`gActiveCdStream`): call
  slot +0x64 (mute), +0x78 (clearRing, StClearRing wrapper), +0x74
  (unsetRing, StUnSetRing wrapper), busy-wait `CdControl(9 /*CdlPause*/, 0, 0)`,
  then state = 4.
- **Levers:** none. Direct `gActiveCdStream == self` compare (no local) matches.
- **Context:** extended the local `CdStreamObjMethods` view with slots
  +0x074 `unsetRing`, +0x078 `clearRing`, +0x07C `slot7C` (additive; the
  table is 31 slots per `tools/classtable.py`).

## Naming

Tier A. `CdStream__Stop` -- slot +0x054. Evidence: only fires from the reading state; mutes, clears and unsets the ring, busy-waits `CdControl(CdlPause)`, transitions to the stopped state.

## Source

```c
void CdStream__Stop(CdStreamObj *self) {
    if (self->unk2C == 2 && gActiveCdStream == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdControl(9, 0, 0) == 0) {
        }
        self->unk2C = 4;
    }
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__Stop (tools/rename.py), the class rename only.
