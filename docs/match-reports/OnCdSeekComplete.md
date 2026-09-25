# OnCdSeekComplete -- MATCHED (exact length, 23/23 words), round 82

> Renamed from `func_80047388` on 2026-09-25 (tools/rename.py). Address 0x80047388.

Round 82, runner delta. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** not a slot: a libcd `CdSyncCallback` handler installed by `CdStreamObj__Seek`.
- **What:** when the active object exists and the sync status is 2 (CdlComplete), uninstall the callback and call the active object's +0x54 callback with its +0x44 argument.
- **Levers:** `u8 status` parameter gives the `andi 0xFF`; re-read the global `gActiveCdStreamObj` after the call (no local).
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStreamObj` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Kept `func_`. Not renamed this round (brief: no renames).

## Source

```c
void OnCdSeekComplete(u8 status, u8 *result) {
    if (gActiveCdStreamObj != NULL && status == 2) {
        CdSyncCallback(NULL);
        if (gActiveCdStreamObj->cb54 != NULL) {
            gActiveCdStreamObj->cb54(gActiveCdStreamObj->cbArg);
        }
    }
}
```
