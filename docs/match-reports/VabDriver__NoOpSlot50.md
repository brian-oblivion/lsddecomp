# VabDriver__NoOpSlot50

> Renamed from `func_8002C400` on 2026-09-26 (tools/rename.py). Address 0x8002c400.

**Unit:** PlacementGridVabSound · **Size:** 1 instruction (`jr $ra; nop`, 0x8 bytes) ·
**Status: MATCHED**, whole-image SHA1 green. Splat matched this itself
(empty body); no derivation was spent, and it never had a report until
this naming pass touched it. Last function in the unit (0x8002C400..0x8002C408,
the unit's own upper bound).

## Role

Empty function, no visible effect.

```c
void VabDriver__NoOpSlot50(void)
{
}
```

## Naming (round 77, charlie -- track 3, no rename)

`tools/classtable.py 0x8006D9BC` confirms this is the `+0x050` slot of
`gVabDriverMethods` (29-slot FileResource-derived table, `PlacementGridVabSound.c`,
the SPU/VAB driver base class) -- physically carved into PlacementGridVabSound.c by
ROM address, semantically owned by that other unit. `PlacementGridVabSound.c`'s own
`VabDriverMethods` struct comment leaves this slot and its siblings
(`VabDriver__NoOpSlot40`/`VabDriver__Open`/`VabDriver__Close`/`VabDriver__Seek`) entirely opaque (`u8 pad000[0x054]`
covers +0x000..+0x054 with no per-slot field). Kept `func_`, not renamed.

## Track 4 (2026-09-26, round 87, alpha): renamed `func_8002C400` -> `VabDriver__NoOpSlot50`

Named for its slot, FINISHING-PLAN track 4 step 6. `classtable.py
gVabDriverMethods --vs gFileResourceMethods` puts this function at `+0x050`, one of
FileResource's run-time-bound driver-interface slots (`include/FileResource.h`
names it `slot50`; the CD driver's occupant is `CdDriver__NoOpSlot50`).
`SetActiveDataSource` (GameApplicationFileResource.c) copies the active driver's interface
slots into FileResource's table and every client table, and takes this table
(`GetVabDriverMethods()`) whenever `gActiveDataSource != DATASOURCE_CD`, so
when the SPU/VAB source is active every `methods->slot50(...)` in the game
reaches this body. The purpose evidence the tier-C verdict above lacked is
the slot's, not the body's: the body does nothing, which is what the VAB
driver does for that interface call. Unified into `include/VabDriver.h`.
