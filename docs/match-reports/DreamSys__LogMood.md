# DreamSys__LogMood — STALL

**Unit:** DreamSys · **Size:** 14 instructions · **Best reached:** 8/14 words (with a scheduling barrier that does NOT fully close it -- see below; word-count-correct variants topped out lower)

## What it does

Vtable `+0x208`. "Logs" a mood point into a contributor: copies
`mood->value` into `layer->lastMood.value`, adds `mood->axis.dynamic` into
`layer->sumMoods.dynamic`, increments `layer->amountMoods`, and adds
`mood->axis.upper` into `layer->sumMoods.upper`. Called by
`DreamSys__LogInstanceMood` (matched this round) as `this->vt->LogMood(this,
&this->entityMoods, source)`.

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
void DreamSys__LogMood(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *mood)
{
	layer->lastMood.value = mood->value;
	layer->sumMoods.dynamic = mood->axis.dynamic + layer->sumMoods.dynamic;
	layer->amountMoods = layer->amountMoods + 1;
	layer->sumMoods.upper = mood->axis.upper + layer->sumMoods.upper;
}
#endif
```

## Residue: retail's instruction SCHEDULE hoists different loads than GCC 2.6.3 naturally chooses for this source, and no reshape found the right one

This is a straight-line function -- four independent field updates, no
branches, no calls -- so there is no register-identity or missing-argument
question (the general lever this round otherwise applied twice
successfully; see `New_DreamSys.md` and `func_80059E3C.md`). Retail's own
instruction order (target column) vs. the direct-translation body's (built
column), via `tools/asm-differ/diff.py DreamSys__LogMood`:

```
TARGET                          BUILT (direct translation)
lhu   v0,0(a2)                  lhu   v0,0(a2)
lw    v1,4(a1)     [dyn old]    lw    a0,4(a1)      [dyn old, different reg]
sh    v0,0(a1)                  sh    v0,0(a1)
lb    v0,0(a2)                                      lw    v0,0xc(a1)  [amountMoods hoisted HERE instead]
                                 lb    v1,0(a2)
lw    a0,8(a1)     [upper old]  addiu v0,v0,1
addu  v0,v0,v1                  sw    v0,0xc(a1)
sw    v0,4(a1)                  lb    v0,1(a2)
lw    v0,0xc(a1)                nop
lb    v1,1(a2)                  addu  v0,v0,v1
addiu v0,v0,1                   sw    v0,0xc(a1)   [DUPLICATE store, wrong]
addu  v1,v1,a0
sw    v0,0xc(a1)
jr    ra
sw    v1,8(a1)
```

Retail hoists `layer->sumMoods.upper`'s OLD value (into `$a0`) very early --
right after `mood->axis.dynamic`'s load, well before `amountMoods` is
touched at all -- then interleaves `mood->axis.upper`'s read into the
load-delay slot right after `amountMoods`'s load. The direct-translation
body instead hoists `amountMoods`'s load early (GCC 2.6.3's scheduler
picking a different, but equally valid, independent instruction to hoist)
and leaves no useful filler for the load-delay slot later, forcing an
explicit `nop`. **The size mismatch (15 words built vs 14 retail) traces
entirely to this one extra `nop`.**

Reshapes tried (9 total), none reaching 14/14:

1. Direct translation, `mood->axis.X + layer->sumMoods.X` order (shown
   above) -- 15 words, wrong schedule as diagrammed.
2. Operand order swapped (`layer->sumMoods.X + mood->axis.X`) -- IDENTICAL
   output to #1. GCC normalizes commutative-add operand order; this lever
   does nothing here.
3. Two named temps for the NEW sums, computed up front
   (`newDynamic`/`newUpper`), stored at the end -- worse: 15 words,
   different wrong schedule.
4. Two named temps for the OLD sums, computed up front
   (`oldDynamic`/`oldUpper`) -- reproduces #1's exact schedule (GCC
   apparently treats the inline and hoisted-temp forms identically here).
5. `__asm__("")` between the `dynamic` and `amountMoods` statements --
   8/14, but still 15 words total (a *different* extra nop appears
   elsewhere). Best score reached.
6. `__asm__("")` between `lastMood` and `dynamic` -- worse, 3/14.
7. `__asm__("")` between `amountMoods` and `upper` -- worse, 2/14, back to
   the #1 schedule.
8. `+=`/`++` compound-assignment forms instead of `x = x + y` -- byte-
   identical to #1. No effect, as expected (they desugar the same way).
9. Two named temps for `mood->axis.dynamic`/`mood->axis.upper` (the DELTA
   reads, not the old sums), computed up front -- worst result, 0/14 and a
   still-wrong instruction count.

None of the 9 reached retail's specific interleaving. Per
`DECOMPILATION_LEARNINGS.md`'s residue guide, a scheduling-only residue
(same final values, different instruction ORDER) is the one class where a
bare `__asm__("")` barrier is the legitimate lever -- tried in three
positions (5-7), one of which measurably helped (8/14) without fully
closing it. Filing as a stall rather than continuing to guess barrier
placements; a fourth or fifth position might close it, but the search space
of "where among 4 statements to place how many barriers" is large enough
that this is better handed to the permuter than iterated further by hand.

### Proposed learning

Not every residue that "looks like" register identity or a missing
argument actually is either -- this one is neither (no branches, no calls,
so the two general levers this round otherwise used successfully don't
apply). When a straight-line multi-field-update function's compiled
instruction COUNT is off by exactly one `nop`, and `asm-differ` shows
retail hoisting a DIFFERENT independent load than your direct translation
does, that is scheduling, not logic -- but do not assume a single
`__asm__("")` barrier will find the right schedule by trial and error
quickly. Nine reshapes here (four pure statement-order/temp variations, one
compound-operator check, three barrier placements, one delta-hoist variant)
did not find it; this function is now a well-posed permuter target (a small
4-statement basic block, verified-correct final field values, single
instruction-count-and-nop-position residue) rather than a good use of
further manual attempts.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
Restored to `INCLUDE_ASM`.
