# SsUtVibrateOff -- Sony's libsnd/vmanager, identified round 99 (2/2 words, splat-generated)

> Renamed from `func_8002E300` on 2026-09-27 (tools/rename.py). Address 0x8002e300.

Unit: `src/libsnd_vmanager.c`. Address `0x8002E300`.

Trivially matched: the empty C body compiles to retail's own `jr $ra; nop`
-- splat emitted this body itself when the unit was carved (round 24,
2026-09-08). Not decompilation work. The definition now takes LIBSND.H's
prototype, `void SsUtVibrateOff(short vc)`; still `jr $ra; nop`,
byte-identical.

## Naming

**`SsUtVibrateOff`** -- Sony's name, identified by position: it is the
second of the two 8-byte stubs that `libsnd/vmanager.o` (discs 3.3 and 3.5)
places between `SePitchBend` and `SeAutoVol`, and retail has exactly two
such stubs there. The full argument is in `SsUtVibrateOn.md`. Recorded in
the symbols file with an `identified` comment.

## History: the round 75 naming pass (superseded)

Round 75 (runner alpha, track 3) kept the placeholder, tier C: no caller in
`src/` and no rodata table reference (same checks as its neighbour). Two
adjacent unreferenced stubs looked like vtable filler, but nothing tied
either to a table entry.
