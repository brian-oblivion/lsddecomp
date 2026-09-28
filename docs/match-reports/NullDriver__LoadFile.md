# NullDriver__LoadFile -- MATCHED (2/2 words)

> Renamed from `VabDriver__LoadFile` on 2026-09-28 (tools/rename.py). Address 0x8002c410.

> Renamed from `func_8002C410` on 2026-09-26 (tools/rename.py). Address 0x8002c410.

Unit: `PlacementGridVabSound`. Report written round 52 (naming pass) -- this function
had no report before; it was matched (an empty `void` body, `jr $ra; nop`)
as part of the unit's original round-17 pass but never separately written
up. `tools/progress.py` counts a matched function like this without a
report as work nobody had to do -- see CLAUDE.md, "not every matched
function was work."

## Result

```c
void NullDriver__LoadFile(void) {
}
```

Byte-exact, 2/2 words (`jr $ra; nop`).

## Notes

`gNullDriverMethods`'s vtable slot +0x058 (confirmed with
`tools/classtable.py gNullDriverMethods`). One of five such empty-body slots
this unit defines for that table (the others are `NullDriver__RunRequestQueue`/
`NullDriver__RequestLoadFile`/`NullDriver__StopService`/`NullDriver__CancelRequests`, slots +0x068/+0x06C/+0x070/+0x074) -- the round-17/43 header
comment's claim that "only +0x054 is a slot this unit defines" for
`gNullDriverMethods` undercounted; this unit actually supplies six of that
table's slots. Corrected in the round-52 unit header comment.

## Naming

Kept `func_8002C410`, tier C (superseded 2026-09-26, Track 4 below). Same reasoning as `NullDriver__Read`: the class
(`gNullDriverMethods`, the driver-interface base `VabStreamObj` chains
through) is established, but this specific slot's role is not -- an empty
body called only through vtable data, no caller in this unit to read intent
from.

## Track 4 (2026-09-26, round 87, alpha): renamed `func_8002C410` -> `NullDriver__LoadFile`

Named for its slot, FINISHING-PLAN track 4 step 6. `classtable.py
gNullDriverMethods --vs gFileResourceMethods` puts this function at `+0x058`, one of
FileResource's run-time-bound driver-interface slots (`include/FileResource.h`
names it `loadFile`; the CD driver's occupant is `CdDriver__LoadFile (FileResource__LoadFile in the base table)`).
`SetActiveDataSource` (game_shell.c) copies the active driver's interface
slots into FileResource's table and every client table, and takes this table
(`GetNullDriverMethods()`) whenever `sActiveDataSource != DATASOURCE_CD`, so
when the SPU/VAB source is active every `methods->loadFile(...)` in the game
reaches this body. The purpose evidence the tier-C verdict above lacked is
the slot's, not the body's: the body does nothing, which is what the VAB
driver does for that interface call. Unified into `include/NullDriver.h`.
