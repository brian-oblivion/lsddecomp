# GetStageGridDimensionsCount

> Renamed from `func_800494B4` on 2026-09-23 (tools/rename.py). Address 0x800494b4.

**Unit:** StageGrid · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Returns the number of entries in the stage-dimensions table: 14.

## Derivation

```
jr    $ra
 ori  $v0, $zero, 0xE
```

A leaf returning a constant in `$v0`. The return type is `s32` on the weak
evidence that `ori` against `$zero` is how 2.6.3 materialises a small positive
int; nothing in two instructions distinguishes `s32` from `u32` or an enum.

The same `0xE` appears in `GetStageGridDimensionsTable` immediately below, which
is what makes the constant a table LENGTH rather than an arbitrary return value.
Written as a shared `STAGE_GRID_DIMENSIONS_COUNT` define for that reason.

## Proposed learning

Two functions materialising the same magic number four bytes apart is evidence
about what the number MEANS, not just what it is — worth grepping for a
constant's other uses before naming it.
