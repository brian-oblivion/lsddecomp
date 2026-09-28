# GetStageGridDimensions

**Unit:** StageGrid · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## What it does

Returns the dimensions entry for one stage: `table[index]`.

## Derivation

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
addu  $s0, $a0, $zero      ; save index across the call
sw    $ra, 0x14($sp)
jal   GetStageGridDimensionsTable
 addu $a0, $zero, $zero    ; NULL — delay slot
sll   $s0, $s0, 3          ; index * 8
addu  $v0, $v0, $s0
...
```

**The `sll $s0, $s0, 3` fixes `sizeof(StageGridDimensions) == 8`**, which
corroborates the layout in `include/StageGrid.h` (`s16 columns; s16 rows;
bool isVertical;` = 2 + 2 + 4). That is the useful finding here: a scaled index
is a direct measurement of a struct's size, and this one agrees with a layout
that was otherwise only a guess.

Written as pointer arithmetic rather than `&table[index]` — both compile the
same here, but the pointer form matches how the result is used.

```c
StageGridDimensions *GetStageGridDimensions(s32 stage) {
    return GetStageGridDimensionsTable(NULL) + stage;
}
```

## Proposed learning

An `sll` by N applied to an index before an `addu` onto a table base is a struct
size of `1 << N`. Cheapest struct-size evidence available, and it needs no
context beyond the one function.

## Naming

**`GetStageGridDimensions`, tier A.** An indexed table accessor, tier A by
definition (a getter). The body is literally `table[index]` over the same
table `GetStageGridDimensionsTable` returns, and the one confirmed caller
(`src/class_3bb8c_l.c:346`, `unk14->methods->slotE0(unk14,
GetStageGridDimensions((s32)self->unk38))`) is consistent with an index
lookup, agreeing with the name from the body alone.

**Parameter `index` -> `stage` (round 101, track 7).** The table has one entry
per stage (`STAGE_COUNT`), and the value is used only to index it, so the index
IS a stage index. The header prototype still says `index` (proposal to the
head).
