# VabStreamObj__func_2cbec -- MATCHED (2/2 words)

Unit: `code_179d8_e`. Report written round 52 (naming pass) -- no report
before; matched (empty `void` body, `jr $ra; nop`) as part of the unit's
original round-17 pass, renamed this round from `func_8002CBEC`. See
`VabStreamObj__func_2cbdc.md` for the shared context.

## Result

```c
void VabStreamObj__func_2cbec(void) {
}
```

Byte-exact, 2/2 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x098 -- the last of the three
consecutive empty slots, immediately before `VabStreamObj__SetPitchOffset`
(+0x09C).

## Naming

Kept the tier-C `Class__func_xxxxx` form -- same reasoning as
`VabStreamObj__func_2cbdc`.
