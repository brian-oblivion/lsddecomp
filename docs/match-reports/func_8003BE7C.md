# func_8003BE7C

**Unit:** code_2c054 · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkD4 = value;`. Fifth (last) of the run of five
described in `func_8003BE5C`'s report (same class, table slot `+0x134`); see
that report for the shared context.

## Derivation

```
jr   $ra
 sw  $a1, 0xD4($a0)
```

```c
void func_8003BE7C(StreamTaskObj *self, s32 a1) {
    self->unkD4 = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `func_8003BE5C`'s report — same header, `include/code_2c054.h`.

## Proposed learning

None beyond `func_8003BE5C`'s.
