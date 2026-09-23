# StreamTaskObj__Reset

> Renamed from `func_8003BA38` on 2026-09-23 (tools/rename.py). Address 0x8003ba38.

**Unit:** code_2c054 · **Size:** 8 instructions (0x20 bytes) · **Status:** MATCHED (8/8 words, whole-image SHA1 green), first attempt

## What it does

Resets the five fields the `StreamTaskObj__SetUnkC4`.."`7C`" setters (see that
report) write individually: `self->unkC4=0; unkC8=-1; unkCC=1; unkD0=0;
unkD4=1;`. Slot `+0x040` of `gStreamTaskObjMethods` (`Get_vtable_StreamTaskObj`'s report), so
presumably an "init"/"reset" method for this `StreamTaskObj` class.

## Derivation

```
addiu $v0, $zero, -0x1
sw    $v0, 0xC8($a0)
ori   $v0, $zero, 0x1
sw    $zero, 0xC4($a0)
sw    $v0, 0xCC($a0)
sw    $zero, 0xD0($a0)
jr    $ra
 sw   $v0, 0xD4($a0)
```

```c
void StreamTaskObj__Reset(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}
```

Matched first attempt, source order following the store order exactly (the
compiler reuses one temp register, `$v0`, first for `-1` then for `1`,
which falls out naturally from writing the five assignments straight-line in
this order -- no reshaping needed).

## New struct/header knowledge

Named the five fields in `include/code_2c054.h`'s `StreamTaskObj` (shared
with the setters' report).

## Proposed learning

None new beyond `StreamTaskObj__SetUnkC4`'s.
