# GetMoodFromStageChunk

Unit: `StageGrid` · Size: 20 words (0x50 bytes) · Status: **MATCHED, byte-exact
(20/20, whole-image `build exit=0`)** · Round 23 (2026-09-07), head.

## What it is

Given a stage index and a `StageChunk` (an `s8` column/row pair), returns a
pointer to that chunk's entry in the stage's mood table.

```c
MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk) {
    return STAGE_CHUNK_MOODS[stage] + chunk->row * STAGE_GRID_DIMENSIONS[stage].columns + chunk->column;
}
```

The data was the only thing standing between this function and a one-line body,
which is why `src/StageGrid.c`'s header comment banked it as head work. Read out
of the executable:

- `STAGE_GRID_DIMENSIONS` (`0x800861D4`) is 14 entries of an **8-byte** struct.
  `s16 columns; s16 rows; bool isVertical;` with `bool` = `int` (`include/types.h`)
  gives exactly 8. Entry 0 is `{1, 5, 1}`, entry 1 `{3, 2, 0}`, entry 2 `{6, 6, 0}`,
  entry 3 `{16, 16, 0}` — the existing header struct is correct as written.
- `STAGE_CHUNK_MOODS` (`0x80086590`) is 14 pointers, to `STG00_CHUNK_MOODS` …
  `STG13_CHUNK_MOODS`. Row-major arrays of the 2-byte `MoodGraphPoint` union.
- `D_800861D6` is **not a separate object**: it is splat's symbol for
  `STAGE_GRID_DIMENSIONS[0].rows`, at base+2. `%hi`/`%lo(D_800861D6)` and a
  relocation against `STAGE_GRID_DIMENSIONS` with addend +2 assemble to the same
  two words (`lui $at,0x8008` / `addiu $at,$at,0x61D6`), so nothing needs
  renaming or re-segmenting. Both `.s` files reference it and both match.

## The one thing that was not obvious

The first attempt was the natural spelling and it came out **19 words, one short**:

```c
return &STAGE_CHUNK_MOODS[stage][chunk->row * STAGE_GRID_DIMENSIONS[stage].columns + chunk->column];
```

That form builds ONE index expression, so GCC 2.6.3 sums `row*cols + col` as
integers and scales the sum once:

```
mflo v0            # row*cols
addu v0,v0,v1      # + col        <- summed BEFORE scaling
sll  v0,v0,0x1     # *2 once
addu v0,v0,v1      # + base
```

Retail scales the two offsets **separately**:

```
sll  v1,v1,0x1     # col*2
mflo v0
sll  v0,v0,0x1     # (row*cols)*2
addu v0,v0,a0      # + base
addu v0,v0,v1      # + col*2
```

which is what plain pointer arithmetic produces, because each `+` on a pointer
is its own scaled addition rather than a term in one index. Rewriting the
subscript as `base + a + b` closed it with no other change.

### Proposed learning

**`&arr[i + j]` and `arr + i + j` are not the same codegen in GCC 2.6.3, and the
difference is exactly one instruction.** The subscript form sums the indices as
integers and scales once; the pointer form scales each addend separately. So a
one-word-short residue on a pointer-returning accessor, with an `addu` sitting
on the wrong side of an `sll`, is a spelling question and not a scheduling one —
try the other form before reaching for anything else. (Measured here: 19/20 vs
20/20, no other change.)
