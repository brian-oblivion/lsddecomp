# VabDriver__RunRequestQueue -- MATCHED (2/2 words)

> Renamed from `func_8002C418` on 2026-09-26 (tools/rename.py). Address 0x8002c418.

Unit: `code_179d8_d`. Report written round 52 (naming pass) -- no report
before; matched (empty `void` body) as part of the unit's original
round-17 pass. See `VabDriver__LoadFile.md` for the shared context (this is one
of five identical-shape empty slots this unit defines for
`gVabDriverMethods`).

## Result

```c
void VabDriver__RunRequestQueue(void) {
}
```

Byte-exact, 2/2 words (`jr $ra; nop`).

## Notes

`gVabDriverMethods`'s vtable slot +0x068.

## Naming

Kept `func_8002C418`, tier C (superseded 2026-09-26, Track 4 below) -- same reasoning as `VabDriver__LoadFile`.

## Track 4 (2026-09-26, round 87, alpha): renamed `func_8002C418` -> `VabDriver__RunRequestQueue`

Named for its slot, FINISHING-PLAN track 4 step 6. `classtable.py
gVabDriverMethods --vs gFileResourceMethods` puts this function at `+0x068`, one of
FileResource's run-time-bound driver-interface slots (`include/FileResource.h`
names it `runRequestQueue`; the CD driver's occupant is `CdDriver__RunRequestQueue`).
`SetActiveDataSource` (code_171e0.c) copies the active driver's interface
slots into FileResource's table and every client table, and takes this table
(`GetVabDriverMethods()`) whenever `gActiveDataSource != DATASOURCE_CD`, so
when the SPU/VAB source is active every `methods->runRequestQueue(...)` in the game
reaches this body. The purpose evidence the tier-C verdict above lacked is
the slot's, not the body's: the body does nothing, which is what the VAB
driver does for that interface call. Unified into `include/VabDriver.h`.
