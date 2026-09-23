# Obj865C8__CheckTimeout — MATCHED (34/34 words)

> Renamed from `func_8004A364` on 2026-09-23 (tools/rename.py). Address 0x8004a364.

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s2, 0x18($sp)
addu  $s2, $a0, $zero        ; s2 = self
sw    $s0, 0x10($sp)
addu  $s0, $a1, $zero        ; s0 = arg1
sw    $s1, 0x14($sp)
sw    $ra, 0x1C($sp)
jal   Get_vtable_IntermediateBase
 addu $s1, $a2, $zero        ; s1 = arg2
addu  $a0, $s2, $zero
addu  $a1, $s0, $zero
lw    $v0, 0x5C($v0)         ; base slot +0x05C
jalr  $v0
 addu $a2, $s1, $zero
lw    $v1, 0x1C($s2)         ; self->unk1C
lw    $v0, 0x2C($s2)         ; self->unk2C
nop
sltu  $v0, $v0, $v1          ; v0 = (unk2C < unk1C) UNSIGNED
beqz  $v0, .L8004A3D0
 addu $a0, $s2, $zero
lw    $v0, 0x0($s2)          ; self->methods
nop
lw    $v0, 0x60($v0)         ; methods->onEventArg
jalr  $v0
 ori  $a1, $zero, 0x4
.L8004A3D0:
...
jr $ra
```

## Final C

```c
void Obj865C8__CheckTimeout(Obj865C8 *self, s32 arg1, s32 arg2) {
    Get_vtable_IntermediateBase()->slot5C(self, arg1, arg2);
    if ((u32)self->unk1C > (u32)self->unk2C) {
        self->methods->onEventArg(self, 4);
    }
}
```

## Residue and how it closed

First attempt (`if (self->unk2C < self->unk1C)`, plain signed `s32`
comparison) scored 31/34: the two `lw`s came out in the wrong order (built
loaded `unk2C` first, retail loads `unk1C` first) and the compare opcode
differed (`slt` built vs. `sltu` retail — 0x2a vs 0x2b in the top byte).
Flipping the comparison to `self->unk1C > self->unk2C` (loads the
left-hand/`unk1C` operand first, matching retail's load order) and casting
both operands `(u32)` (forces `sltu` instead of `slt`) fixed both at once —
one source change, two residue lines.

## New struct knowledge (`include/class_39e08.h`)

- `Obj865C8::unk1C` (s32) — compared unsigned against `unk2C`.
- `IntermediateBaseMethods::slot5C` typed `void (*)(void *self, s32 arg1,
  s32 arg2)` — same `Get_vtable_IntermediateBase()` base accessor as `slot44`/`slot48`/
  `slot60`, arguments just forwarded with no other evidence of type.

## Attempts

2. First: signed `<` comparison in source order, 31/34 (load-order + opcode
   residue). Second: flipped operand order + unsigned cast, 34/34.

### Proposed learning

A `sltu` (retail) vs `slt` (built) opcode residue with no other difference
means the comparison needs an explicit unsigned cast — plain relational
operators on `s32` fields default to signed `slt`. Separately, GCC 2.6.3
evaluates a relational comparison's operands in TEXTUAL left-to-right order
for which one it loads first; if retail's load order doesn't match your
comparison's source order, try swapping which side is written first (and
flipping the operator to preserve the same logical meaning) rather than
assuming a scheduling quirk.

## Naming

`Obj865C8__CheckTimeout` -- tier B. Occupies +0x05C. Forwards to the base's own +0x05C (`Obj86B60__IncrementFrameCounter`, code_2cc8c.h) then compares `frameCounter > timeoutFrames`, triggering `onEventArg(self, 4)` on overflow -- a timeout check by construction, though what the timeout gates in-game is not established.
