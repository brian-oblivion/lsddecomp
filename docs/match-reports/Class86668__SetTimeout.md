# Class86668__SetTimeout

> Renamed from `Obj865C8__SetTimeout` on 2026-09-23 (tools/rename.py). Address 0x8004a458.

> Renamed from `func_8004A458` on 2026-09-23 (tools/rename.py). Address 0x8004a458.

**Unit:** class_39e08 · **Size:** 8 words (0x20 bytes) · **Status:** MATCHED (8/8 words)

## What it does

Method-table slot +0x06C, shared verbatim between `D_800865C8` and
`gClass86668Methods` (same address in both tables). Sets `self->unk2C` to `arg1`
unconditionally, then overwrites it with `arg1 * 20` if `arg1` is
non-negative.

## Derivation

```
bltz  $a1, .L8004A470
 sw   $a1, 0x2C($a0)      ; ALWAYS runs (delay slot): self->unk2C = arg1
sll   $v0, $a1, 2          ; v0 = arg1*4
addu  $v0, $v0, $a1         ; v0 = arg1*5
sll   $v0, $v0, 2            ; v0 = arg1*20
sw    $v0, 0x2C($a0)          ; only reached if arg1 >= 0
.L8004A470:
jr    $ra
 nop
```

Written as:

```c
void Class86668__SetTimeout(Obj865C8 *self, s32 arg1) {
    self->unk2C = (arg1 < 0) ? arg1 : arg1 * 20;
}
```

The ternary reproduces retail's "always store, conditionally overwrite"
shape exactly -- GCC 2.6.3 compiles the `arg1 < 0` branch to keep the
original value (no-op past the initial store) and the else branch to the
`*20` recomputation, matching the delay-slot-store-then-maybe-overwrite
structure byte for byte.

## Proposed learning

None beyond what's already documented.

## Naming

`Class86668__SetTimeout` -- tier A. Pure setter/converter: stores its argument verbatim if negative, else multiplied by 20 (a units-to-frames conversion) into `timeoutFrames`; mechanics are its purpose.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/Class86668.h`. Not renamed. Slot +0x06C, `setTimeout`, the first of this class's own slots; parameter `timeout`, stored as `timeout * 20` frames or kept when negative (Class86668__CancelTimeout's -1 never fires, CheckTimeout comparing unsigned). Image byte-identical.
