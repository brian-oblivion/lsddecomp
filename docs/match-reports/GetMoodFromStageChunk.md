# GetMoodFromStageChunk

Unit: `StageGrid` · Size: 20 words (0x50 bytes) · Status: **MATCHED, byte-exact
(20/20, whole-image `build exit=0`)** · Round 23 (2026-09-07), head.

## What it is

Given a stage index and a `StageChunk` (an `s8` column/row pair), returns a
pointer to that chunk's entry in the stage's mood table.

```c
MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk) {
    return sStageChunkMoods[stage] + chunk->row * sStageGridDimensions[stage].columns + chunk->column;
}
```

The data was the only thing standing between this function and a one-line body,
which is why `src/StageGrid.c`'s header comment banked it as head work. Read out
of the executable:

- `sStageGridDimensions` (`0x800861D4`) is 14 entries of an **8-byte** struct.
  `s16 columns; s16 rows; bool isVertical;` with `bool` = `int` (`include/types.h`)
  gives exactly 8. Entry 0 is `{1, 5, 1}`, entry 1 `{3, 2, 0}`, entry 2 `{6, 6, 0}`,
  entry 3 `{16, 16, 0}` — the existing header struct is correct as written.
- `sStageChunkMoods` (`0x80086590`) is 14 pointers, to `sStage00ChunkMoods` …
  `sStage13ChunkMoods`. Row-major arrays of the 2-byte `MoodGraphPoint` union.
- `D_800861D6` is **not a separate object**: it is splat's symbol for
  `sStageGridDimensions[0].rows`, at base+2. `%hi`/`%lo(D_800861D6)` and a
  relocation against `sStageGridDimensions` with addend +2 assemble to the same
  two words (`lui $at,0x8008` / `addiu $at,$at,0x61D6`), so nothing needs
  renaming or re-segmenting. Both `.s` files reference it and both match.

## The one thing that was not obvious

The first attempt was the natural spelling and it came out **19 words, one short**:

```c
return &sStageChunkMoods[stage][chunk->row * sStageGridDimensions[stage].columns + chunk->column];
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

## Naming

**`GetMoodFromStageChunk`, tier A.** Inherited from lsddecomp, confirmed: the
body is exactly "given a stage and a chunk, return that chunk's mood point",
which is what the name says, and the one call site outside this unit,
`src/DreamSys.c:1460` inside `DreamSys__LogChunkMood` (`mood =
GetMoodFromStageChunk(this->currentStage, (StageChunk *)currentPos);`, whose
result is immediately logged via `LogMood`), passes the player's current
stage and position and reads back a mood value, agreeing with the name.
Confirmed, not renamed.

**Field `StageGridDimensions.isVertical`, tier B/unconfirmed.** This report's
own data read confirms the STRUCT is 8 bytes and that the third field exists
(entry 0's third word is `1`, entry 1's is `0`), but no code anywhere in
`src/` reads `isVertical` — `columns` and `rows` are the only fields any
function in this unit or `DreamSys.c` accesses, so there is no accessor to
check the name against. It stays an inherited hypothesis, recorded rather
than confirmed; see `## Proposed field names` below.

## Proposed field names

`StageGridDimensions.isVertical` (`include/StageGrid.h`): inherited name,
plausible (a grid can be laid out row-major or column-major, and the field
sits exactly where a 2-byte pad would otherwise go), but **unconfirmed** —
no function in the codebase reads it. Leaving it as-is rather than
inventing a replacement with no more evidence; flagging for whichever unit's
work ends up reading this field (likely wherever the grid is actually
rendered/laid out) to confirm or correct.

**Update (round 101, track 7): `isVertical` now has accessors.** The two
paragraphs above predate them. `src/class_3bb8c.c`, `src/class_3bb8c_b.c`
and `src/class_3bb8c_p.c` read `StageGridDimensions.isVertical` (tested
against 0 and 1) through a dimensions pointer, so the field is no longer
padding-by-rule; confirming or sharpening its name belongs to whoever
polishes those units, who can see what the two branches do.

**Globals `STAGE_CHUNK_MOODS` -> `sStageChunkMoods` and `STGnn_CHUNK_MOODS` ->
`sStageNnChunkMoods`, nn = 00..13 (round 101, track 7; fifteen
`tools/rename.py` runs).** `sStageChunkMoods` is named in C only by
`src/StageGrid.c`; the fourteen per-stage arrays are named by nothing but its
pointer initialisers in splat data. Unit-static data, so `sName`; the stage
number stays in the name because the arrays are indexed by stage and stage nn
is the disc's `STGnn` directory.
