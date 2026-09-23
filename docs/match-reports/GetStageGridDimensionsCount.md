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

## Naming

**`GetStageGridDimensionsCount`, tier A.** A pure leaf returning a constant is
a getter, tier A by definition. Evidence: the constant it returns is the same
`0xE` the sibling `GetStageGridDimensionsTable` writes through its `count`
out-parameter, so the value IS the table's length, not an arbitrary number;
the name follows the `Get<X>Table` / `Get<X>Count` pairing this unit already
has, and the wider codebase's existing `Get*Count(void)` convention
(`GetFileTableCount`, `GetOpenVabCount`). No caller exists anywhere in the
extracted asm (checked: no `jal` to its address outside its own `.s`), so the
name rests on behaviour alone, not on a call site.
