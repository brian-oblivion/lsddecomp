# CdStreamObj__Finalize -- MATCHED (exact length, 21/21 words), round 82

> Renamed from `func_80047074` on 2026-09-25 (tools/rename.py). Address 0x80047074.

Round 82, runner delta. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** slot +0x00C (finalize) of gCdStreamObjMethods.
- **What:** call own slot +0x48 (`CdStreamObj__Close`, stop the stream), then chain to `Get_vtable_BasicClass()->finalize`.
- **Levers:** none needed.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `gActiveCdStreamObj` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `CdStreamObj__Finalize` -- slot +0x00C, overrides BasicClass's finalize (`Class__Finalize` convention, BasicClass.h's model). Evidence: calls `close(self)` then chains to `Get_vtable_BasicClass()->finalize`.

## Source

```c
void CdStreamObj__Finalize(CdStreamObj *self) {
    self->methods->slot48(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
```
