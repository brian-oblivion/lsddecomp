# GetStageChunkFromMood

Unit: `StageGrid` · Size: 43 words (0xAC bytes) · Status: **MATCHED, byte-exact
(43/43, whole-image `build exit=0`)** · Round 23 (2026-09-07), head.

## What it is

The inverse of `GetMoodFromStageChunk`: a linear search over all 14 stages'
mood tables for the first entry equal to `*mood`, writing the found chunk
coordinates through `chunk` and returning the stage index, or `-1`.

```c
s32 GetStageChunkFromMood(StageChunk *chunk, MoodGraphPoint *mood) {
    u32 stage;
    s32 row;
    s32 column;
    MoodGraphPoint *chunkMood;
    s32 rows;
    s32 columns;

    for (stage = 0; stage < STAGE_COUNT; stage++) {
        chunkMood = sStageChunkMoods[stage];
        rows = sStageGridDimensions[stage].rows;
        columns = sStageGridDimensions[stage].columns;
        for (row = 0; row < rows; row++) {
            for (column = 0; column < columns; column++) {
                if (mood->value == chunkMood->value) {
                    chunk->column = column;
                    chunk->row = row;
                    return stage;
                }
                chunkMood++;
            }
        }
    }
    return -1;
}
```

`chunkMood` walks each stage's table row-major and is advanced once per inner iteration
(retail puts the `addiu $t0, $t0, 2` in the backward branch's delay slot, which
executes unconditionally, so it is an ordinary `chunkMood++` and not a conditional one).
The data layout is documented in `GetMoodFromStageChunk.md`.

## Two local-type facts did all the work — the structure was right first try

The first attempt had the exact control flow, the exact register roles for the
walkers, and the exact three induction variables, and still came out **five
words long at 8/43**. Both residues were declared types of locals, not shape:

- **`s16 rows` / `s16 columns` cost four words.** Held as 16-bit quantities,
  GCC 2.6.3 loaded `columns` with `lhu` and then re-materialised the sign
  extension (`sll ...,0x10` / `sra ...,0x10`) at **each** of its two uses — once
  for the inner loop's `blez` guard and once inside the row loop, i.e. inside a
  loop, every row. Retail loads `lh` once and keeps the value. Declaring both
  locals `s32` makes the load itself the sign extension and the duplicates
  vanish. Note the asymmetry that made this confusing to read: `rows`, written
  identically, *did* come out as a single `lh` — so seeing one of two
  same-typed locals behave correctly is not evidence the type is right.
- **`s32 stage` cost one word.** Retail's stage bound is `sltiu $v0, $t2, 0xE`,
  unsigned. A signed `s32` counter gives `slti`. `u32 stage` makes the
  comparison against the `int` constant unsigned by the usual promotions, and
  `return stage;` still compiles to the bare `move $v0, $t2` retail has.

Nothing needed a scheduling barrier and no register had to be named.

## The unused stack frame is not a residue

Retail brackets the body with `addiu $sp, $sp, -0x10` / `addiu $sp, $sp, 0x10`
and never touches the frame — no store, no load, no `$ra` save, and the function
is a leaf. It reproduces **automatically** from the C above with no prompting.
So on this compiler an allocated-but-unused frame is just what a leaf function
with enough simultaneously-live locals emits; it is not a signal that the source
declared an array that got optimised away, and it is not something to try to
reproduce deliberately.

### Proposed learning

**A local's DECLARED WIDTH is a codegen decision, not a documentation choice,
and `s16` is the expensive default.** An `s16` local that is compared or used as
an index gets re-sign-extended at each use — inside loops included — where the
`s32` spelling folds the extension into the `lh` at the point of load. So when a
correct-shape body is a few words long and the extra words are `sll 0x10` /
`sra 0x10` pairs, widen the locals before touching anything structural: the
struct FIELD stays `s16` (it is 2 bytes in the data), only the local copy
widens. Corollary worth its own line, because it is what delayed the diagnosis
here: **two locals of the same declared type can come out differently**, so one
of them compiling to a clean `lh` does not clear the type.

Second, smaller: **`sltiu` on a loop bound means an UNSIGNED counter.** Retail's
`sltiu $v0, $t2, 0xE` against a signed `s32` counter's `slti` is a one-word
residue with a one-word fix (`u32`), and it survives `return stage;` from an
`s32` function unchanged.

## Naming

**`GetStageChunkFromMood`, tier A.** Inherited from lsddecomp, confirmed:
the body's own name (`Get<output>From<input>`) matches what it does — search
every stage's mood table for a value equal to `*mood` and return the owning
chunk — and the one call site outside this unit, `src/world/DreamSys.c:2003`
inside `GenerateInitialSpawn` (`stage = GetStageChunkFromMood(&chunk,
mood);`), passes a `MoodGraphPoint *mood` and uses the returned stage to
index `sStageTimeLimits`/`sStageSpawnPoints` and the returned `chunk` to
match a spawn point's own chunk, agreeing with "find the (stage, chunk) a
mood value belongs to". Confirmed, not renamed. (`include/DreamSys.h:43`'s
comment attributing `StageChunk`/`GetMoodFromStageChunk` usage to
`DreamSys__LogChunkMood` is about the *other* function in this unit, not this
one — see `GetMoodFromStageChunk.md`.)

**Parameter and locals (round 101, track 7).** `ret` -> `chunk`: it is an
out-parameter written exactly once, with the found cell's `column` and `row`,
and DreamSys's one caller passes `&chunk`. `p` -> `chunkMood`: it points at the
`MoodGraphPoint` of the chunk the loops are currently at, compared against
`*mood` and advanced once per chunk. `col` -> `column`, matching the
`StageChunk.column` field it is stored into and the `columns` bound beside it.
Zero bytes: local names are not in the object. The header prototype still
says `ret` (see the round 101 proposal to the head).
