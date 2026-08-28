# GetStageGridDimensionsTable

**Unit:** StageGrid · **Size:** 7 instructions · **Status:** MATCHED (7/7 words)

## What it does

Returns the stage-dimensions table, optionally writing its length through an
out-parameter. The classic "give me the table and tell me how big it is" shape.

## Derivation

```
beqz  $a0, .L800494C8
 ori  $v0, $zero, 0xE
sw    $v0, 0x0($a0)
.L800494C8:
lui   $v0, %hi(STAGE_GRID_DIMENSIONS)
addiu $v0, $v0, %lo(STAGE_GRID_DIMENSIONS)
jr    $ra
 nop
```

`$a0` is a nullable `s32 *`: tested against zero, and on the non-null path a
full word is stored, so the pointee is 4 bytes. lsddecomp's header named the
parameter `unknown`; `count` is better supported by what the code does with it,
and the length is the same `0xE` `func_800494B4` returns.

Note the `ori` sits in the branch's DELAY SLOT, so it executes on both paths —
which is why the value is computed before the null test in the assembly but
written as an ordinary `if` body in C.

```c
StageGridDimensions *GetStageGridDimensionsTable(s32 *count) {
    if (count != NULL) {
        *count = STAGE_GRID_DIMENSIONS_COUNT;
    }
    return STAGE_GRID_DIMENSIONS;
}
```

Returning the bare array name (not `&STAGE_GRID_DIMENSIONS`, not a cast) is what
produces the plain `lui`/`addiu` pair.
