# VabStreamObj__NoOpSlot94 -- MATCHED (2/2 words)

> Renamed from `VabStreamObj__func_2cbe4` on 2026-09-26 (tools/rename.py). Address 0x8002cbe4.

Unit: `vab_sound`. Report written round 52 (naming pass) -- no report
before; matched (empty `void` body, `jr $ra; nop`) as part of the unit's
original round-17 pass, renamed this round from `func_8002CBE4`. See
`VabStreamObj__NoOpSlot90.md` for the shared context.

## Result

```c
void VabStreamObj__NoOpSlot94(void) {
}
```

Byte-exact, 2/2 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x094.

## Naming

Kept the tier-C `Class__func_xxxxx` form -- same reasoning as
`VabStreamObj__NoOpSlot90`.

## Track 4 (2026-09-26, round 87)

Renamed to `VabStreamObj__NoOpSlot94` with `rename.py`: it was
the old `Class__func_xxxxx` name, a tier-C placeholder. The name now says what the
body is, an empty body in the class's own slot +0x094 (`slot94` in
`include/vab_stream_obj.h`). This follows `NullDriver__NoOpSlot40`. No caller
reaches the slot, so the slot's purpose is still unknown. The "Kept the
tier-C form" sentence above describes the old name.
