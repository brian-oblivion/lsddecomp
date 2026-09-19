> Renamed from `func_8005DAAC` on 2026-09-19 (tools/rename.py). Address 0x8005daac.

# Entity__SetUnkF4

**Unit:** Entity · **Size:** 20 instructions · **Status:** MATCHED (20/20 words, whole-image build verified byte-exact)

## What it does

`Entity__SetUnkF4(Entity *this, s32 arg1)`: if `arg1` is nonzero, calls this
entity's own vtable slot `+0x030` with a literal `9`; unconditionally stores
`arg1` into `this->unkF4`.

## Derivation

```
move  $s1, $a1                ; s1 = arg1
beqz  $s1, .L8005DAE0          ; if (arg1 == 0) skip the call
 sw   $ra, 0x18($sp)            ; delay slot -- always runs regardless of branch
lw    $v0, 0x0($s0)
lw    $v0, 0x30($v0)
jalr  $v0                       ; this->methods->slot30(this, 9)
 ori  $a1, $zero, 0x9
.L8005DAE0:
sw    $s1, 0xF4($s0)            ; this->unkF4 = arg1
```

The `sw $ra,...` register save lands in the branch's delay slot (runs on
both paths), which is just ordinary callee-save scheduling, not something
the C needs to express explicitly.

## Final C

```c
void Entity__SetUnkF4(Entity *this, s32 arg1) {
    if (arg1 != 0) {
        this->methods->slot30(this, 9);
    }
    this->unkF4 = arg1;
}
```

## Attempt log

Matched on the first attempt. The single branch here never affects the
return value (the function is `void`, and the store after the branch runs
unconditionally either way) — not the `goto`-lever shape from
`func_80025B34`/`New_class_65650`, just a plain `if`.

## Proposed learning

None new.
