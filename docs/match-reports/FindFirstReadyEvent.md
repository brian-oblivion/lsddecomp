> Renamed from `func_8004F4C8` on 2026-09-20 (tools/rename.py). Address 0x8004f4c8.

# FindFirstReadyEvent

**Unit:** class_3bb8c_f · **Size:** 37 words (0x94) · **Status:** MATCH

## What it does

`s32 FindFirstReadyEvent(s32 *arr, s32 count)`. An outer infinite retry loop
around an inner scan: for `i` in `[0, count)`, call `func_800390F4(arr[i])`
(the same validity-check callback `TaskObjF__ForEachEvent` also uses); the first
`i` for which it returns nonzero wins, and the function returns
`D_80086E78[i]`. If no entry qualifies, the WHOLE array is rescanned from
the top forever (retail's own `blez count,.L8004F4F8` sits at the loop's
own re-entry point, so `count <= 0` also spins here — no special case
needed in the C, a plain `for (;;) { for (i = 0; i < count; i++) ... }`
reproduces both).

Screened predicted-hard (6 distinct callee-saved registers per the round's
register census) but matched on the first structurally-faithful
transcription — the register pressure the census measured evidently comes
from a body shape this one doesn't have; the outer/inner double loop and
the single early `return` inside it turned out to need only `$s0`/`$s1`.

## Header additions

- `extern s32 D_80086E78[];` — the lookup table (`asm/data/76DC8.data.s`),
  left unsized (bound not established from this unit alone).
- `extern s32 FindFirstReadyEvent(s32 *arr, s32 count);` (forward declaration,
  needed because `TaskObjF__FindReadyEvent`, earlier in ROM order, calls it).

See `TaskObjF__ForEachEvent`'s report for `func_800390F4` and the `TaskObjF` class
context (this function itself does not touch `TaskObjF` — its `arr`
parameter is a plain `s32*`, happens to be fed `TaskObjF::field14` by its
only caller in this unit).
