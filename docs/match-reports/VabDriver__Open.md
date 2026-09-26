# VabDriver__Open

> Renamed from `func_8002C3E0` on 2026-09-26 (tools/rename.py). Address 0x8002c3e0.

**Unit:** code_179d8_d · **Size:** 4 instructions (0x10 bytes) ·
**Status: MATCHED 4/4**, whole-image SHA1 green. Identical shape to
`VabDriver__NoOpSlot40` immediately preceding it (this unit, matched the same
round) -- same 0x40-byte reserved-and-unused stack frame.

```c
void VabDriver__Open(void)
{
    char buf[0x40];
}
```

## Naming (round 77, charlie -- track 3, no rename)

Same finding as `VabDriver__NoOpSlot40`'s report: `+0x044` slot of
`gVabDriverMethods` (code_179d8_e.c), left opaque even at that unit's own
struct-comment level. Kept `func_`, not renamed.

## Track 4 (2026-09-26, round 87, alpha): renamed `func_8002C3E0` -> `VabDriver__Open`

Named for its slot, FINISHING-PLAN track 4 step 6. `classtable.py
gVabDriverMethods --vs D_8006D430` puts this function at `+0x044`, one of
Class6D430's run-time-bound driver-interface slots (`include/Class6D430.h`
names it `open`; the CD driver's occupant is `Class6D4E8__Open`).
`SetActiveDataSource` (code_171e0.c) copies the active driver's interface
slots into Class6D430's table and every client table, and takes this table
(`GetVabDriverMethods()`) whenever `gActiveDataSource != DATASOURCE_CD`, so
when the SPU/VAB source is active every `methods->open(...)` in the game
reaches this body. The purpose evidence the tier-C verdict above lacked is
the slot's, not the body's: the body does nothing, which is what the VAB
driver does for that interface call. Unified into `include/VabDriver.h`.
