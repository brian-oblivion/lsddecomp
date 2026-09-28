# WaitForReadyEvent

> Renamed from `FindFirstReadyEvent` on 2026-09-20 (tools/rename.py). Address 0x8004f4c8.

> Renamed from `func_8004F4C8` on 2026-09-20 (tools/rename.py). Address 0x8004f4c8.

**Unit:** TitleMenuTaskObjF · **Size:** 37 words (0x94) · **Status:** MATCH

## What it does

`s32 WaitForReadyEvent(s32 *arr, s32 count)`. An outer infinite retry loop
around an inner scan: for `i` in `[0, count)`, call `func_800390F4(arr[i])`
(the same validity-check callback `TaskObjF__ForEachEvent` also uses); the first
`i` for which it returns nonzero wins, and the function returns
`sCardEventSpecs[i]`. If no entry qualifies, the WHOLE array is rescanned from
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

- `extern s32 sCardEventSpecs[];` — the lookup table (`asm/data/76DC8.data.s`),
  left unsized (bound not established from this unit alone).
- `extern s32 WaitForReadyEvent(s32 *arr, s32 count);` (forward declaration,
  needed because `TaskObjF__WaitForReadyEvent`, earlier in ROM order, calls it).

See `TaskObjF__ForEachEvent`'s report for `func_800390F4` and the `TaskObjF` class
context (this function itself does not touch `TaskObjF` — its `arr`
parameter is a plain `s32*`, happens to be fed `TaskObjF::field14` by its
only caller in this unit).

## Naming (round 60, track 3)

`func_8004F4C8` -> `WaitForReadyEvent`. **Tier A.** Free function
(operates on a caller-supplied `s32 *arr`, not a `self`): forever-scans
up to `count` entries with `TestEvent`, returning `sCardEventSpecs[i]` for the
first ready one, re-scanning the whole array from the top if none
qualify yet. Mechanics and purpose (find which event fired) both
directly evident from the body.

**Head correction at merge (round 60).** The runner named this
`FindFirstReadyEvent`; the head renamed it to `WaitForReadyEvent` before
marking the unit passed. The evidence the runner recorded is right and the
tier is right -- what the old name got wrong is the control flow. The outer
`for (;;)` has NO exit other than the `return`, so the function cannot come
back without a ready event: it BLOCKS. "Find" invites a reader to expect a
search that can fail and return a sentinel, which is the one thing this body
never does, and "can this call hang?" is the question a caller most needs
answered. Byte-identical after `tools/rename.py` (a rename changes zero
bytes), same for the `TaskObjF__WaitForReadyEvent` forwarder.

## Round 95 (track 7, charlie)

### Naming

`D_80086E78` -> `sCardEventSpecs` (rename.py; **tier A**): TaskObjF__OpenEvents
passes entry i to `OpenEvent(0xF4000001, spec, 0x2000, 0)` -- SwCARD,
EvMdNOINTR -- and the four words, read from retail, are 0x0004, 0x8000,
0x0100, 0x2000: Sony's EvSpIOE, EvSpERROR, EvSpTIMOUT, EvSpNEW, the standard
card event set. This function returns the entry of the slot that fired,
and CardInfoStatus/CardLoadStatus compare it against 0x100/0x8000/0x2000.
Splat sizes the symbol at two words and starts `sCardIconNames` at
0x80086E80, over the last two specs (the head has the proposal). `arr` ->
`events`. Zero bytes.
