# VabDriver__NoOpSlot40

> Renamed from `func_8002C3D0` on 2026-09-26 (tools/rename.py). Address 0x8002c3d0.

**Unit:** code_179d8_d · **Size:** 4 instructions (0x10 bytes) ·
**Status: MATCHED 4/4**, whole-image SHA1 green.

## Role

Empty function whose only visible effect is reserving and immediately
releasing a 0x40-byte stack frame -- no register other than `$sp` is
touched. Reproduced with an unused local array:

```c
void VabDriver__NoOpSlot40(void)
{
    char buf[0x40];
}
```

The unused-variable warning is expected and harmless (GCC still reserves
the stack slot for a declared array even though nothing reads/writes it).
No evidence of what the array's real size/type/purpose was beyond "0x40
bytes of frame" -- this is the minimal C that reproduces the observed
bytes, not a claim about the original source's intent.

## Naming (round 77, charlie -- track 3, no rename)

`tools/classtable.py 0x8006D9BC` confirms this is the `+0x040` slot of
`gVabDriverMethods` (29-slot FileResource-derived table, `code_179d8_d.c`,
the SPU/VAB driver base class) -- physically carved into code_179d8_d.c by
ROM address, semantically owned by that other unit. `code_179d8_d.c`'s own
`VabDriverMethods` struct comment leaves this and its four siblings
(`VabDriver__Open`/`VabDriver__Close`/`VabDriver__Seek`/`VabDriver__NoOpSlot50`) entirely opaque -- `u8 pad000[0x054]`
covers +0x000..+0x054 with no per-slot field even at the struct level, and
its own written rationale is "no call site in this unit" for the ones it
does name individually further down. Kept `func_` per that same
precedent: no purpose evidence beyond "reserved-and-unused stack frame,
occupies a FileResource extension slot the base class leaves for a subclass
to fill". Not renamed.

## Track 4 (2026-09-26, round 87, alpha): renamed `func_8002C3D0` -> `VabDriver__NoOpSlot40`

Named for its slot, FINISHING-PLAN track 4 step 6. `classtable.py
gVabDriverMethods --vs gFileResourceMethods` puts this function at `+0x040`, one of
FileResource's run-time-bound driver-interface slots (`include/FileResource.h`
names it `slot40`; the CD driver's occupant is `CdDriver__NoOpSlot40`).
`SetActiveDataSource` (code_171e0.c) copies the active driver's interface
slots into FileResource's table and every client table, and takes this table
(`GetVabDriverMethods()`) whenever `gActiveDataSource != DATASOURCE_CD`, so
when the SPU/VAB source is active every `methods->slot40(...)` in the game
reaches this body. The purpose evidence the tier-C verdict above lacked is
the slot's, not the body's: the body does nothing, which is what the VAB
driver does for that interface call. Unified into `include/VabDriver.h`.
