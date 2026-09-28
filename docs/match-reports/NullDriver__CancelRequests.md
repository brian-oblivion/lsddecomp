# NullDriver__CancelRequests -- MATCHED (2/2 words)

> Renamed from `VabDriver__CancelRequests` on 2026-09-28 (tools/rename.py). Address 0x8002c430.

> Renamed from `func_8002C430` on 2026-09-26 (tools/rename.py). Address 0x8002c430.

Unit: `PlacementGridVabSound`. Report written round 52 (naming pass) -- no report
before; matched (empty `void` body) as part of the unit's original
round-17 pass. See `NullDriver__LoadFile.md` for the shared context (this is one
of five identical-shape empty slots this unit defines for
`gNullDriverMethods`).

## Result

```c
void NullDriver__CancelRequests(void) {
}
```

Byte-exact, 2/2 words (`jr $ra; nop`).

## Notes

`gNullDriverMethods`'s vtable slot +0x074 -- the last slot before the table
boundary this unit types (`pad058[0x078 - 0x058]` inside `NullDriverMethods`
after slot54; +0x078 onward is untyped in this unit's own view).

## Naming

Kept `func_8002C430`, tier C (superseded 2026-09-26, Track 4 below) -- same reasoning as `NullDriver__LoadFile`.

## Track 4 (2026-09-26, round 87, alpha): renamed `func_8002C430` -> `NullDriver__CancelRequests`

Named for its slot, FINISHING-PLAN track 4 step 6. `classtable.py
gNullDriverMethods --vs gFileResourceMethods` puts this function at `+0x074`, one of
FileResource's run-time-bound driver-interface slots (`include/FileResource.h`
names it `cancelRequests`; the CD driver's occupant is `CdDriver__CancelRequests`).
`SetActiveDataSource` (GameApplicationFileResource.c) copies the active driver's interface
slots into FileResource's table and every client table, and takes this table
(`GetNullDriverMethods()`) whenever `sActiveDataSource != DATASOURCE_CD`, so
when the SPU/VAB source is active every `methods->cancelRequests(...)` in the game
reaches this body. The purpose evidence the tier-C verdict above lacked is
the slot's, not the body's: the body does nothing, which is what the VAB
driver does for that interface call. Unified into `include/NullDriver.h`.
