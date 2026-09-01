# func_8003C1DC

**Unit:** code_2c054 · **Size:** 23 instructions · **Status:** MATCHED (23/23 words)

## What it does

The base task class's own (unoverridden) implementation of method-table
slot `+0x044` -- the function StreamTask's own override (func_8003BA58,
first pass) chains into via `func_8003DFBC()`. Chains ONE level further
into a THIRD, grandparent class's own copy of the same slot (its result
discarded), then returns this object's own "state" field (`unk38`, also
touched by func_8003BD10).

## The C

```c
s32 func_8003C1DC(StreamTask *self, s32 a1, s32 a2) {
    func_8003E5C8()->slot44(self, a1, a2);
    return self->unk38;
}
```

## A third class discovered: the grandparent

`func_8003E5C8()` (defined in the still-uncarved `asm/code_2cc8c.s`, right
next to `func_8003DFBC()`) has the identical "always returns the same
fixed table address, ignores its argument" shape: `lui $v0,%hi(D_8006E878);
addiu $v0,$v0,%lo(D_8006E878); jr $ra`. This is a class table ONE LEVEL
ABOVE the base task class (`D_8006E730`), reached only from this one
function in this unit. Nothing about `D_8006E878` beyond this single
slot's signature (`self, a1, a2`) is derived here -- typed in
`include/code_2c054.h` as `TaskGrandBaseMethods`, deliberately minimal.

This also means the class chain this unit has now confirmed spans THREE
levels: `D_8006E878` (grandparent) -> `D_8006E730` (base task,
`func_8003DFBC()`) -> `D_8006E5F8` (StreamTask, `func_8003BE84()`).

## How the return type was derived

Unlike most of this unit's chain-to-base slots, this one does NOT simply
forward the base's own return value -- it explicitly reloads
`self->unk38` AFTER the grandparent call and returns THAT instead,
discarding whatever the grandparent's slot `+0x044` produced. This is
positive evidence (not just an unproven guess) that this slot's real
return type is `s32`, since the value returned is a genuine object field,
not leftover register noise.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (second pass).
