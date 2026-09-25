# CdStreamObj__Stop -- MATCHED (exact length, 43/43 words), round 82

> Renamed from `func_800474C8` on 2026-09-25 (tools/rename.py). Address 0x800474c8.

Round 82, runner delta (second session). Unit `src/code_3770c.c`. Fresh
ground, no prior attempt. Byte-exact on the first build; whole-image SHA1
green.

- **Where:** slot +0x054 of gCdStreamObjMethods.
- **What:** if state is 2 and this is the active stream (`gActiveCdStreamObj`): call
  slot +0x64 (mute), +0x78 (clearRing, StClearRing wrapper), +0x74
  (unsetRing, StUnSetRing wrapper), busy-wait `CdControl(9 /*CdlPause*/, 0, 0)`,
  then state = 4.
- **Levers:** none. Direct `gActiveCdStreamObj == self` compare (no local) matches.
- **Context:** extended the local `CdStreamObjMethods` view with slots
  +0x074 `unsetRing`, +0x078 `clearRing`, +0x07C `slot7C` (additive; the
  table is 31 slots per `tools/classtable.py`).

## Naming

Kept `func_`. Not renamed this round (brief: no renames).

## Source

```c
void CdStreamObj__Stop(CdStreamObj *self) {
    if (self->unk2C == 2 && gActiveCdStreamObj == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdControl(9, 0, 0) == 0) {
        }
        self->unk2C = 4;
    }
}
```
