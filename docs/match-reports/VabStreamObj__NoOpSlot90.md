# VabStreamObj__NoOpSlot90 -- MATCHED (2/2 words)

> Renamed from `VabStreamObj__func_2cbdc` on 2026-09-26 (tools/rename.py). Address 0x8002cbdc.

Unit: `code_179d8_e`. Report written round 52 (naming pass) -- no report
before; matched (empty `void` body, `jr $ra; nop`) as part of the unit's
original round-17 pass, renamed this round from `func_8002CBDC` once the
class was established (`tools/rename.py func_8002CBDC
VabStreamObj__NoOpSlot90`).

## Result

```c
void VabStreamObj__NoOpSlot90(void) {
}
```

Byte-exact, 2/2 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x090. One of three consecutive empty
slots (with `VabStreamObj__NoOpSlot94` at +0x094 and `VabStreamObj__NoOpSlot98`
at +0x098) sandwiched between `VabStreamObj__StopVoice` (+0x084, well
evidenced) and `VabStreamObj__SetPitchOffset` (+0x09C, well evidenced) --
no call site anywhere in this unit dispatches through any of the three, so
there is nothing to read purpose from beyond "an empty override slot this
class's own vtable happens to fill."

## Naming

Kept the tier-C `Class__func_xxxxx` form (then `VabStreamObj__func_2cbdc`) per
FINISHING-PLAN track 3: the CLASS is known (`VabStreamObj`, the SPU/VAB
stream object -- see the unit header comment) but this slot's own purpose
is not. Bare `func_8002CBDC` would have thrown away the one thing that IS
established; a guessed name (e.g. "OnPause") would assert purpose from
nothing.

## Track 4 (2026-09-26, round 87)

Renamed to `VabStreamObj__NoOpSlot90` with `rename.py`: it was
the old `Class__func_xxxxx` name, a tier-C placeholder. The name now says what the
body is, an empty body in the class's own slot +0x090 (`slot90` in
`include/VabStreamObj.h`). This follows `VabDriver__NoOpSlot40`. No caller
reaches the slot, so the slot's purpose is still unknown. The "Kept the
tier-C form" sentence above describes the old name.
