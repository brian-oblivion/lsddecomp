# VabStreamObj__SetPitchOffset -- MATCHED (6/6 words)

> Renamed from `func_8002CBF4` on 2026-09-18 (tools/rename.py). Address 0x8002cbf4.

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
void VabStreamObj__SetPitchOffset(VabStreamObj *self, s32 arg1) {
    self->pitchOffset = arg1 * 12 - 0x18;
}
```

Byte-exact, 6/6 words (`sll`/`addu`/`sll`/`addiu`/`jr`/`sw` -- `arg1*12-24`
computed as `(arg1<<1 + arg1)<<2` then `-0x18`, matching GCC's usual
strength-reduction shape for a small constant multiply).

## Notes

`gVabStreamObjMethods`'s vtable slot +0x09C (last slot in this unit's typed range).
`self->pitchOffset` (`ObjDA34::unk60` before round 52) is a genuine 32-bit
field here (`sw`, not `sh`) -- see `VabStreamObj__PlayTone`'s report for
the same field read back with a narrower cast, which is why the field is
declared `s32` rather than `s16`/`u16` despite that narrower read.

## Naming

Renamed `func_8002CBF4` -> `VabStreamObj__SetPitchOffset`, tier B.
Confirmed `gVabStreamObjMethods`'s own +0x09C slot, called with `arg1 == 0`
right after construction (`VabStreamObj__VabStreamObj`'s own
`self->methods->slot9C(self, 0)`). Named from `VabStreamObj__PlayTone`'s
own use of the field it sets (`self->pitchOffset` added to a
`VagAtrView::center` note before dispatching a tone -- see that function's
report): the formula `arg1*12-24` (12 = semitones/octave) reads as an
octave-to-semitone-offset conversion, `arg1` an octave index. Tier B: the
arithmetic and its consumer are concrete, but nothing in this unit confirms
`arg1` really means "octave" rather than some other unit that happens to
use the same multiplier.
