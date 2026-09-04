# func_8002CBF4 -- MATCHED (6/6 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
void func_8002CBF4(ObjDA34 *self, s32 arg1) {
    self->unk60 = arg1 * 12 - 0x18;
}
```

Byte-exact, 6/6 words (`sll`/`addu`/`sll`/`addiu`/`jr`/`sw` -- `arg1*12-24`
computed as `(arg1<<1 + arg1)<<2` then `-0x18`, matching GCC's usual
strength-reduction shape for a small constant multiply).

## Notes

`D_8006DA34`'s vtable slot +0x09C (last slot in this unit's typed range).
`self->unk60` is a genuine 32-bit field here (`sw`, not `sh`) -- see
`func_8002CA3C`'s report for the same field read back with an `(u16)` cast
(a direct `lhu`), which is why `ObjDA34::unk60` is declared `s32` rather than
`s16`/`u16` despite that narrower read.
