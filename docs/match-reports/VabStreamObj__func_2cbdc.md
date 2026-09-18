# VabStreamObj__func_2cbdc -- MATCHED (2/2 words)

Unit: `code_179d8_e`. Report written round 52 (naming pass) -- no report
before; matched (empty `void` body, `jr $ra; nop`) as part of the unit's
original round-17 pass, renamed this round from `func_8002CBDC` once the
class was established (`tools/rename.py func_8002CBDC
VabStreamObj__func_2cbdc`).

## Result

```c
void VabStreamObj__func_2cbdc(void) {
}
```

Byte-exact, 2/2 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x090. One of three consecutive empty
slots (with `VabStreamObj__func_2cbe4` at +0x094 and `VabStreamObj__func_2cbec`
at +0x098) sandwiched between `VabStreamObj__StopVoice` (+0x084, well
evidenced) and `VabStreamObj__SetPitchOffset` (+0x09C, well evidenced) --
no call site anywhere in this unit dispatches through any of the three, so
there is nothing to read purpose from beyond "an empty override slot this
class's own vtable happens to fill."

## Naming

Kept the tier-C `Class__func_xxxxx` form (`VabStreamObj__func_2cbdc`) per
FINISHING-PLAN track 3: the CLASS is known (`VabStreamObj`, the SPU/VAB
stream object -- see the unit header comment) but this slot's own purpose
is not. Bare `func_8002CBDC` would have thrown away the one thing that IS
established; a guessed name (e.g. "OnPause") would assert purpose from
nothing.
