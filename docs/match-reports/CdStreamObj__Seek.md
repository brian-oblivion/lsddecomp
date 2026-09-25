# CdStreamObj__Seek -- MATCHED (exact length, 39/39 words), round 82

> Renamed from `func_800472EC` on 2026-09-25 (tools/rename.py). Address 0x800472ec.

Round 82, runner delta. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** slot +0x04C (seek) of gCdStreamObjMethods.
- **What:** if state is not 2 and active: with a +0x54 callback, install `OnCdSeekComplete` via `CdSyncCallback` and issue the non-blocking `CdControlF(0x15 /*CdlSeekL*/, loc)`; otherwise busy-wait the blocking `CdControl(0x15, loc, 0)`. Either way state = 1.
- **Levers:** one `self->unk2C = 1;` after the if/else; cc1 emits the `li v0,1` in both arms and shares the store.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStreamObj` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `CdStreamObj__Seek` -- slot +0x04C (the struct's own pre-existing field name, confirmed by the body: installs the async seek callback or busy-waits `CdControl(CdlSeekL, ...)`, then sets state = seeking).

## Source

```c
void CdStreamObj__Seek(CdStreamObj *self, u8 *loc) {
    if (self->unk2C != 2 && gActiveCdStreamObj == self) {
        if (self->cb54 != NULL) {
            CdSyncCallback(OnCdSeekComplete);
            CdControlF(0x15, loc);
        } else {
            while (CdControl(0x15, loc, 0) == 0) {
            }
        }
        self->unk2C = 1;
    }
}
```
