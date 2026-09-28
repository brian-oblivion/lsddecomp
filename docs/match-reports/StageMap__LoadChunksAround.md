# StageMap__LoadChunksAround -- MATCH (140/140 words, ins 0 / del 0, exact length)

> Renamed from `StageMap__BuildRateEntries` on 2026-09-26 (tools/rename.py). Address 0x8004b700.

> Renamed from `Class866E8__BuildRateEntries` on 2026-09-26 (tools/rename.py). Address 0x8004b700.

> Renamed from `func_8004B700` on 2026-09-24 (tools/rename.py). Address 0x8004b700.

REVISITED, round 63: MATCHED 140/140, whole-image SHA1 green; names/types not
relevant (no header, symbol or type change -- the fix removed a local).

> **ROUND 63 (delta): MATCHED, 137/140 -> 140/140.** Stalled at 137/140 since
> round 40, re-verified in rounds 41 and 47, probed with five structural
> variants across rounds 39/41 and searched by the permuter for 37,155
> iterations from a `--debug`-validated scaffold.
>
> ### Step (a): figure AND cause both confirmed
>
> `stalesyms.py` first: no stale callee names in this report.
>
> Rebuilt the preserved `#if 0` body verbatim:
>
> ```
> StageMap__LoadChunksAround: 137/140 words match (file 0x3BF00-0x3C130)
> StageMap__LoadChunksAround: insertions 0 / deletions 0
> ```
>
> Exact length, no drift, and `asm-differ` shows zero `<`/`>` markers with
> three `r` rows -- so like `StageMap__ComputeChunkLoadEntry` and unlike `StageMap__ComputeFootprintDescriptor` this
> round, **the inherited "pure register identity" verdict is CONFIRMED.**
> Residue, all three words in the second loop:
>
> ```
> retail  3c0c0  addu a2,s3,a0   3c0c4  lw v1,4(a2)   3c0c8  lhu v0,2(a2)
> built   3c0c0  addu v0,s3,a0   3c0c4  lw v1,4(v0)   3c0c8  lhu v0,2(v0)
> ```
>
> ### The fix: DELETE a local
>
> The second loop had its own element pointer `e2`; the first loop uses `e`.
> Retail uses **one** variable for both. Making the second loop reuse `e`:
>
> ```c
> for (i = 0; i < 7; i++) {
>     e = &self->arr[i];
>     e->unk4->unk32 = e->unk2;
> }
> ```
>
> **140/140, ins 0 / del 0, whole-image SHA1 green.** Nothing else changed.
>
> Sixteen variants were built to get there, one build each. Eight on the
> loop's shape -- all inert at 137/140 except two regressions:
>
> | loop-shape variant | score |
> | --- | --- |
> | split `tgt = e2->unk4;` out | 137/140 |
> | `k = e2->unk2;` value local | 136/140 |
> | `e2 = self->arr + i;` | 137/140 |
> | `i = 0;` hoisted out of the `for` header | 137/140 |
> | `__asm__("")` barrier inside the loop | 137/140 |
> | split both `tgt` and `k` | 137/140 |
> | `while` form | 137/140 |
> | hoist `arr = self->arr;` before the loop | 107/140, DRIFT |
>
> Then eight on the function's LOCALS rather than the loop's shape:
>
> | locals variant | score |
> | --- | --- |
> | declaration order (`e2` first / last / `i` last / `count` after `i`) | 137/140 (all four) |
> | separate counter `j` for the second loop | 105/140 |
> | reuse `e` **and** separate counter `j` | 107/140 |
> | **reuse `e`, delete `e2`** | **140/140** |
> | reuse `e`, declared after `stackBuf` | **140/140** |
>
> ### Why 37,155 permuter iterations could not find this
>
> The permuter mutates the body it is given; **it does not merge two of that
> body's locals into one.** Every prior attempt on this function -- five
> structural variants across rounds 39 and 41, plus the whole search -- moved
> statements, changed operand order, changed declaration order and changed the
> addressing form, all with `e` and `e2` both present. The answer was one
> deleted declaration away the entire time and outside the search space.
>
> Round 41's own two negatives are worth re-reading with this in mind: a
> pointer-increment walk and dropping `e2` to index `self->arr[i]` twice both
> regressed to 107/140 with identical drift, and the report concluded retail
> "genuinely needs a single already-computed element pointer reused twice per
> iteration". That conclusion was exactly right, and it was one step short:
> retail needs a single already-computed element pointer **that is also the
> first loop's pointer.**
>
> ### The `__asm__("")` barrier is retired
>
> The body carried a bare `__asm__("")` before `u14 = e->unkC->unk14;` in the
> first loop. With `e` merged it is no longer needed: removed, and the
> whole-image rebuild stays green. It was a crutch for the two-variable shape.
> (Contrast `StageMap__ComputeChunkLoadEntry`'s `do {} while (0);` this same round, which was
> tested the same way and IS still load-bearing -- test, do not assume,
> in either direction.)
>
> Oracle: `build exit=0`, `OK: build matches retail SLPS_015.56`, funcdiff
> 140/140 ins 0/del 0, no drift. Unit `class_39e08` INCLUDE_ASM count 4 -> 3
> (6 -> 3 across delta's three functions this round).
>
> ### Proposed learning
>
> **Two sibling loops over the same array in one function share ONE pointer
> variable in retail more often than they have their own.** When the residue
> is a register-colour difference on a loop's walking pointer and another loop
> in the same function walks something similar, try reusing that loop's
> variable before trying anything else. Measured twice this round on the same
> axis (`StageMap__ComputeChunkLoadEntry`: four locals to two; `StageMap__LoadChunksAround`: `e2` deleted),
> and bravo's `_SsInit` is a third instance from the other direction.
>
> **And the operational form of it: "the permuter found nothing" bounds the
> search space, not the function.** The permuter's space is the body's
> statements and expressions; the number of LOCALS is a parameter of that
> space, not a point in it. So a clean, validated, high-iteration negative --
> which round 40's was -- says nothing about a body with one fewer variable.
> Before spending a second search on a function with a standing negative,
> **spend eight builds varying the local count first.** That is what closed
> both of this unit's long-standing register-identity stalls this round, at a
> cost of about fifteen seconds each.


> **ROUND 47 (charlie): Gate 1b re-verified 137/140, no drift** (rebuild
> via the standard `#if 0`->`#if 1` swap; this was the one function of
> the six whose Gate-1b rebuild ran cleanly on the first try, before a
> batch-script regex bug affecting the other five was found and fixed --
> `make clean && make extract` then re-ran to recover). Identical `$a2`
> vs `$v0` row-pointer residue in the second loop, unchanged from round
> 41.
>
> **Permuter check 3:** round 40's scaffold recorded base score 75, 15
> register differences, ZERO reorderings/insertions/deletions --
> matching this function's own documented residue (pure register
> identity) exactly. Not one of the five confirmed family
> scaffold-mismatch cases; the 37,155-iteration search stands as a real
> negative.
>
> Checked the 12th lever (hoist a field pair used on every path into
> locals PER `if`) against the remaining residue: does not apply --
> the second loop's row pointer (`self->arr[i]`) is a plain walking
> pointer with no field-pair-shared-across-condition-and-body shape
> nearby. **Disposition unchanged: 137/140.** No new attempt made;
> `INCLUDE_ASM` untouched throughout.

> **ROUND 41 (alpha): Gate 1b re-verified 137/140, no drift; two new
> structural variants tried on the remaining 3-word residue (the second
> loop's row pointer), both REGRESSED hard.** Rebuilt the exact preserved
> body from a clean `INCLUDE_ASM` baseline first: confirmed 137/140, exact
> length, identical residue (retail `$a2`, built `$v0`), unchanged from
> round 40.
>
> This function was staffed as the highest-priority "close the residue the
> lever did NOT touch" target. The remaining residue is entirely in the
> second loop:
> ```c
> for (i = 0; i < 7; i++) {
>     e2 = &self->arr[i];
>     e2->unk4->unk32 = e2->unk2;
> }
> ```
> Two variants tried, both against the whole-image oracle:
> 1. **Pointer-increment walk instead of indexed addressing** (`e2 =
>    self->arr; for (...) { e2->unk4->unk32 = e2->unk2; e2++; }`) --
>    **REGRESSED to 107/140 with 185767 bytes of drift** (the compiled
>    length itself shrank by a word elsewhere in the loop, not just a
>    register reshuffle).
> 2. **Drop the `e2` local entirely, index `self->arr[i]` directly at both
>    sites** (`self->arr[i].unk4->unk32 = self->arr[i].unk2;`) --
>    **IDENTICAL regression, 107/140, same 185767-byte drift.** Both
>    variants hit the exact same wrong length, confirming retail's shape
>    genuinely needs a single already-computed element pointer reused
>    twice per iteration (the current preserved body's own shape), not a
>    walking increment or a doubly-indexed expression.
>
> Both reverted immediately; restored to the confirmed 137/140 body
> (`e2 = &self->arr[i];` then two `e2->` uses), matching round 40's own
> committed form exactly (`git diff` against it is empty). **Verdict
> unchanged: this is a genuine register-identity residue** (same opcodes,
> same operands, only `$a2` vs `$v0`), confirmed inert against two more
> structural axes this round on top of round 39's three (statement order,
> commutative operand order, declaration order) and round 27's
> callee-saved-register check. Per project rule 6, not to be forced with a
> register pin. Not attempted further; time went to `StageMap__FindSlotForPosition`'s
> fresh permuter search instead, per this round's own closer-target
> priority -- a one-word residue with a real prior signal (score 10 from
> base 20, never 0) is a better use of a bounded permuter run than a third
> structural probe of an already twice-confirmed pure register swap.

> **ROUND 40 (bravo): 125/140 -> 137/140, first-ever permuter search on
> this function.** Staffed here specifically because the function had
> never been searched (this report's own history is five rounds of manual
> confirmation that the residue is pure register identity, with zero
> permuter attempts).
>
> **Gate 1b:** rebuilt the exact preserved 125/140 body from a clean
> `INCLUDE_ASM` baseline first -- confirmed 125/140, no drift, identical
> residue. Honest.
>
> **Scaffold:** `--debug --stack-diffs` reported base score 75, 15 register
> differences, 0 reorderings/insertions/deletions -- matching this report's
> own description exactly (pure register identity, no instruction-shape
> difference). Trusted.
>
> **Search:** `timeout 900 ... -j 4 --stack-diffs --stop-on-zero
> --best-only`, backgrounded. **37155 iterations. No `rc` was captured** --
> the trailing `echo "permuter rc=$?"` never reached the log, the same
> "wrapping shell torn down before the echo runs" trap round 17 documented
> for `StageMap__ApplyChunkLoads`'s own permuter invocation. The process list showed no
> surviving permuter workers when checked after the 900s bound should have
> elapsed, consistent with the `timeout` bound firing rather than an
> external kill, but this is inferred from absence, not read off an exit
> code -- recording it as uncaptured, not as `124`.
>
> **The lead:** best candidate found at iteration ~282 dropped the permuter
> score from base 75 to 15 and never improved further across the remaining
> ~36,900 iterations (a flat plateau, not a stalled-but-still-searching
> run). The change: the second `u14 = e->unkC->unk14; u14->unk0 = 0;`
> reload/store pair (a named-local reassignment immediately consumed once)
> replaced with a direct `e->unkC->unk14->unk0 = 0;` -- re-deriving the
> pointer chain inline instead of caching it in `u14` a second time.
>
> **Translated as found and re-verified through the full oracle:** isolated
> single-function test (the sibling `StageMap__PopulateSlotCells` restored to
> `INCLUDE_ASM` while measuring) gives **137/140, no drift** --
> `funcdiff.py` reports no "differs outside range" warning, confirming the
> compiled length is still exactly retail's. `asm-differ` confirms every
> instruction from this fix's site through the end of the loop now matches
> retail one-for-one; the entire remaining 3-word residue is the row
> pointer in the SECOND loop (`self->arr[i]` walk), retail `$a2` vs built
> `$v0`, unchanged from what this report already documented -- same class,
> not touched by this fix, no instruction-shape difference (confirmed via
> `asm-differ`, pure register substitution).
>
> This closes 12 of the residue's 15 words with ONE source change on a
> class this report's own history describes as pure register identity
> across five rounds -- the permuter found an axis (a redundant reload's
> exact phrasing) that none of the five rounds' manual attempts had
> targeted, because nothing in the residue's own description as "register
> identity" pointed at a reload site rather than the loads/stores the
> manual attempts focused on. **Disposition: 137/140, exact length,
> `INCLUDE_ASM` restored, preserved body updated in `src/class_39e08.c`.**
> The remaining 3-word residue (second loop's row pointer) is unchanged
> register-identity, consistent with everything else already confirmed
> inert for this class in this unit.

### Proposed learning

**The same source-level lever (drop a redundant reload-into-named-local,
re-derive the pointer chain inline at the point of use) closed real residue
on TWO different functions in the same unit this round**
(`StageMap__LoadChunksAround` here, `StageMap__PopulateSlotCells` below) -- both permuter-found, both
translating directly to idiomatic C with no cleanup needed. Worth adding to
the standard lever list: when a value is loaded into a local, used once
immediately, then the SAME expression is re-evaluated a second time into
the SAME local for a second single use a few statements later, try
dropping the second reassignment and dereferencing the chain directly at
its use site instead of caching it.

> **ROUND 39 (charlie): re-verified 125/140, headline lever checked and
> already-applied; three new combinatorial variants tried, all inert or
> worse.** Rebuilt per Gate 1b: confirmed 125/140, identical residue
> (`tbl` a0-here/a1-retail, `u14` a3-here/a0-retail, second-loop row
> pointer v0-here/a2-retail). Checked this round's headline lever against
> retail's own disassembly: `tbl` (`0x8004B7E8`) and `u14`
> (`0x8004B7F8`) ARE adjacent loads with later shared consumers (both used
> starting at `0x8004B804`), exactly the diagnostic shape -- **but this
> function's preserved body already hoists both, with a `__asm__("")`
> barrier between them, from round 13's own fix.** The lever is already
> in place and the residue survives it, so this is a case where the lever
> APPLIES (retail's shape matches the diagnostic) but is not SUFFICIENT on
> its own -- consistent with `StageMap__FindSlotForPosition` needing a second axis
> combined with its hoist.
>
> Three combinatorial variants tried, targeting what that second axis
> might be here:
> 1. **Swap `tbl`/`u14` computation order** (compute `u14` first, `tbl`
>    second, barrier kept between them) -- **REGRESSED to 117/140**, and
>    changed which instructions differ entirely (the `tbl` address
>    computation itself came out wrong-shaped, not just wrong-register).
>    Reverted.
> 2. **Flip the commutative operand order** in both `u14->unk18.w =
>    arg2->unk0 + tbl->unk0` and the `unk20.w` sibling (to `tbl->unk0 +
>    arg2->unk0`) -- **REGRESSED to 123/140**. Reverted.
> 3. **Declaration-order swap** (`tbl` declared before `u14`, matching
>    assignment order) -- **IDENTICAL 125/140**, confirming (again, on a
>    third function) that declaration order is inert for this compiler.
>    Reverted for a clean diff.
>
> None of the three axes tried this round (statement order, operand order,
> declaration order) combines productively with the already-applied hoist.
> Given round 27's callee-saved-register check already confirmed identical
> frame/save-order and this round's own confirmation that the hoist lever
> is already in place and insufficient, this reads as a genuine
> multi-register allocation-ordering choice with no combinatorial axis
> found yet. Not re-attempted further this round.

> **ROUND 32 (bravo2): re-verified, no new attempt.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline: confirmed 125/140, no
> drift, identical residue (`tbl`/`u14`/second-loop-row-pointer register
> swaps only, no instruction shape differences). Given three prior rounds'
> confirmation including a callee-saved-register-order check, no new
> variant was attempted this round; time went to `StageMap__FindSlotForPosition` (matched)
> and a permuter run on `StageMap__ComputeChunkLoadEntry` instead.

> **ROUND 27 (delta): callee-saved-register check per the head's broadcast
> (a parameter that must survive a call sometimes gets promoted to a
> callee-saved register where retail instead spills it to its own home
> slot -- 2 extra words per promoted parameter, and worth checking before
> trusting a register-identity verdict).** Rebuilt the exact preserved body
> from a clean `INCLUDE_ASM` baseline (125/140 confirmed, no drift) and
> disassembled the compiled `.o`'s prologue directly
> (`mipsel-linux-gnu-objdump -d build/src/class_39e08.c.o`): saves
> `s3,s5,s2,s1,ra,fp(s8),s7,s6,s4,s0` at offsets `0x8C,0x94,0x88,0x84,0xA4,
> 0xA0,0x9C,0x98,0x90,0x80` off a `-0xA8` frame -- **the IDENTICAL set, at
> the IDENTICAL offsets, in the IDENTICAL order**, to retail's own prologue
> (`asm/nonmatchings/class_39e08/StageMap__LoadChunksAround.s`, lines 1-16). Same total
> frame size too (`0xA8` both). **The lever does NOT apply: retail does not
> save fewer callee-saved registers than this body does, so the
> register-identity verdict for this function is CONFIRMED, not merely
> plausible.** Not re-attempted further this round.
> **ROUND 20 (charlie): re-verified, no new attempt.** Rebuilt the exact
> preserved body below from a clean `INCLUDE_ASM` baseline: `funcdiff.py`
> reports 125/140 words, no drift warning -- **claim confirmed accurate**,
> unchanged since this report was last updated. Not re-attempted further
> this round: this function's residue is pure register identity (`tbl`,
> `u14`, the second loop's row pointer -- three separate registers, no
> instruction shape differences anywhere), the same class independently
> re-confirmed as unfixable-by-reshaping on three OTHER functions in this
> unit this round (`StageMap__FindSlotForPosition`, `ComputeCellWorldOffsets`, `StageMap__PopulateSlotCells`'s
> `info` residue). Time this round went to `StageMap__PopulateSlotCells` instead, per
> the staffing guidance to move to differently-shaped ground once a
> register-identity wall is this well established.

Unit: `class_39e08`. Not toolchain-blocked: no `gp_rel` hit, no
`addiu $at,$at,%lo` hit, no dense-`switch`/`jr $v0` table dispatch in
`asm/nonmatchings/class_39e08/StageMap__LoadChunksAround.s`. Has a genuine `div`
(integer divide by a non-constant, `self->unk68->divisor`) via the standard
maspsx-expanded zero/overflow-check sequence -- matched cleanly from the
first attempt, no issue there.

## What it does

`void StageMap__LoadChunksAround(Obj866E8 *self, s32 val, Unk54Struct *arg2, ChunkSlotSpec *arg3)`.
If `arg3 == 0`, does nothing at all (whole body skipped). Otherwise:

1. `divisor = self->unk68->divisor` (s16); `flag = (val / divisor) & 1`
   (the quotient's low bit).
2. `savedResult = StageMap__ComputeNeighbourMask(self, val, flag)` (already-matched sibling,
   spilled to the stack and reused later).
3. Loop `i = 0..6`: resolve `e = self->methods->slot118(self, i)`, copy
   `arg3[i].key` into `Elem::unk2` (new field). If `arg3[i].flag != 0`:
   look up `tbl = &sNeighbourOffsets[arg3[i].key]` (new 0xC-stride `Unk54Struct`
   data table, reused type), fill `e->unkC->unk14`'s `unk18`/`unk1C`/
   `unk20` from `arg2` combined with either `tbl` (when
   `self->unk68->unk4 == 0`) or a flat `-0x5000` adjustment (otherwise),
   zero the SAME `SplitCoord2`'s new `unk0` field (a genuine reload, not dead
   code), call the still-raw sibling `StageMap__ComputeChunkLoadEntry` (7 args, 2 on the
   stack) to fill one slot of a 7-entry `ChunkLoadEntry` stack buffer,
   increment `count`.
4. A SECOND loop, `i = 0..6` again unconditionally: copies
   `self->arr[i].unk2` into `self->arr[i].unk4->unk32` (field
   `StageMap__ComputeFootprintDescriptor`/`StageMap__FindSlotByNeighbour` already established).
5. `self->methods->slotFC(self, stackBuf, count)` -- dispatches into
   `StageMap__ApplyChunkLoads` (this unit, also stalled this round).

New header additions (all committed, additive; unchanged from the previous
draft of this report): `Elem::unk2` (u16 @+0x002), `SplitCoord2::unk0` (s32
@+0x000), `ChunkSlotSpec` (new: `u8 key`@0, `u8 flag`@1, size 2),
`extern Unk54Struct sNeighbourOffsets[]` (reuses the existing 3-`s32`-word shape),
`extern void StageMap__ComputeChunkLoadEntry(...)` (7-arg prototype, established from this
call site only), and `Obj866E8Methods::slotFC` fixed to its real signature
(see `StageMap__ApplyChunkLoads`'s report for the mixup that entry had).

## Progress this round: 52/140 -> 125/140, and the size-drift bug is FIXED

An earlier pass on this function stalled at 52/140 with a `funcdiff`
DRIFT WARNING, which turned out to matter: direct `.o` inspection
(`mipsel-linux-gnu-objdump -d build/src/class_39e08.c.o`, function
`StageMap__LoadChunksAround`, subtracting its start address from the next function's
start) showed the compiled body was **138 words, 2 words (8 bytes) SHORT**
of retail's 140 -- `funcdiff`'s own reported byte RANGE is not proof of
correct length; it can look plausible while a function is short, and the
tell was an absolute data-symbol reference (`sNeighbourOffsets`) resolving 8
bytes low in the FINAL LINKED image, because everything downstream of a
short function shifts, including unrelated data in another file entirely.
**Always cross-check a stalled function's true compiled length against
`.o` output before trusting `funcdiff`'s window on a large residue.**

Both missing instructions were closed this round:

1. **One `nop` for a load-delay slot.** In `tbl = &sNeighbourOffsets[arg3[i].key];
   u14 = e->unkC->unk14;`, GCC 2.6.3 scheduled `u14`'s independent load
   RIGHT AFTER the tbl-address computation, incidentally filling the
   load-delay slot that would otherwise follow the very next `lbu`
   (rereading `arg3[i].key` for the `*12` multiply) -- retail needs an
   EXPLICIT `nop` there because nothing fills it. A source-level statement
   reorder (moving `u14 = ...;` before `tbl = ...;`) did NOT change the
   compiler's scheduling at all. **A bare `__asm__("");` scheduling
   barrier placed BETWEEN the two statements did** -- confirmed it only
   changes instruction ORDER (forces the explicit `nop` to appear, matching
   retail), not WHICH REGISTER holds `tbl` or `u14`, so it is the
   permitted form per CLAUDE.md's test. This single barrier closed one of
   the two missing words AND, combined with re-testing the if/else branch
   polarity (below), the polarity fix that had previously made things
   WORSE now made things dramatically better once the barrier was in
   place -- the two residues were entangled the whole time.

2. **The second loop's addressing strategy.** Retail computes the row
   pointer via an explicit `$s3 + $a0` ADD every iteration (`$a0` is a
   plain running BYTE OFFSET starting at literal `0xEC`, not a pointer),
   giving it a uniform small per-field offset (`+4`, `+2`) off the freshly
   computed row pointer. Writing the loop as `self->arr[i].unk4->unk32 =
   self->arr[i].unk2;` (repeated array indexing) let GCC fold everything
   into ONE incrementing pointer with LARGE per-field constant offsets
   (`+0xF0`, `+0xEE`) baked in, needing no extra ADD -- one instruction
   shorter, and wrong. Applying this project's documented "explicit
   intermediate element pointer" idiom --
   `Elem *e2 = &self->arr[i]; e2->unk4->unk32 = e2->unk2;` -- closed this
   exactly. (Note this is the SAME idiom that, in `StageMap__ApplyChunkLoads`'s stall
   this round, conspicuously did NOT help an analogous-looking situation;
   see that report. The difference: here the loop genuinely walks ONE
   array via ONE index for BOTH field accesses, so nothing prevents GCC
   from choosing the intermediate-pointer strategy once asked; there, two
   DIFFERENT provably-related expressions were involved and the compiler
   collapsed them regardless of phrasing.)

Also required: the if/else branch LAYOUT for the `self->unk68->unk4`
test had to be written as `if (... == 0) { ADD } else { SUBTRACT }`
(inverted from the "natural" `!= 0` reading) to match retail's actual
branch/fallthrough assignment (`bnez unk4, SUBTRACT-as-target`,
`ADD`-as-fallthrough). Tried in isolation (without the barrier) this
inversion made the score WORSE (49 vs 52) -- it was only with the barrier
ALSO in place that it helped, taking the score from ~83 to 125. The two
fixes are not independently separable in this function; whoever iterates
further should keep BOTH in place together, not re-test them one at a
time expecting monotonic improvement.

## SUPERSEDED by round 63 -- the matching body

The round-63 match is live in `src/class_39e08.c`. Against the 137/140 body it
differs in exactly two places: the `Elem *e2;` declaration is gone and the
second loop uses `e`, and the first loop's `__asm__("")` barrier is gone (no
longer needed once `e` is merged; verified by whole-image rebuild).

```c
        for (i = 0; i < 7; i++) {
            e = &self->arr[i];
            e->unk4->unk32 = e->unk2;
        }
```

## HISTORICAL -- best body reached in round 40 (125/140, NO drift warning)

```c
void StageMap__LoadChunksAround(Obj866E8 *self, s32 val, Unk54Struct *arg2, ChunkSlotSpec *arg3) {
    s32 divisor;
    s32 flag;
    s32 savedResult;
    s32 count;
    s32 i;
    Elem *e;
    Elem *e2;
    SplitCoord2 *u14;
    Unk54Struct *tbl;
    ChunkLoadEntry stackBuf[7];

    if (arg3 != 0) {
        divisor = self->unk68->divisor;
        flag = (val / divisor) & 1;
        savedResult = StageMap__ComputeNeighbourMask(self, val, flag);

        count = 0;
        for (i = 0; i < 7; i++) {
            e = self->methods->slot118(self, i);
            e->unk2 = arg3[i].key;
            if (arg3[i].flag != 0) {
                tbl = &sNeighbourOffsets[arg3[i].key];
                __asm__("");
                u14 = e->unkC->unk14;
                if (self->unk68->unk4 == 0) {
                    u14->unk18.w = arg2->unk0 + tbl->unk0;
                    u14->unk1C = arg2->unk4;
                    u14->unk20.w = arg2->unk8 + tbl->unk8;
                } else {
                    u14->unk18.w = arg2->unk0 - 0x5000;
                    u14->unk1C = arg2->unk4 + tbl->unk4;
                    u14->unk20.w = arg2->unk8 - 0x5000;
                }
                u14 = e->unkC->unk14;
                u14->unk0 = 0;
                StageMap__ComputeChunkLoadEntry(self, &stackBuf[count], divisor, flag, val, savedResult, arg3[i].key);
                count++;
            }
        }

        for (i = 0; i < 7; i++) {
            e2 = &self->arr[i];
            e2->unk4->unk32 = e2->unk2;
        }

        self->methods->slotFC(self, stackBuf, count);
    }
}
```

## Remaining residue: PURE register identity, no instruction differences at all

Every one of the 15 remaining word mismatches is the identical opcode/
operand SHAPE with a different register: `tbl` lives in `$a1` in retail,
`$a0` here; `u14` (both loads) lives in `$a0` in retail, `$a3` here; the
second loop's row pointer lives in `$a2` in retail, `$v0` here. No
instruction is missing, added, or reordered anywhere in the function --
confirmed via full instruction-by-instruction diff, not just the summary
score. Per CLAUDE.md and MATCHING-GUIDE, this is squarely the
register-identity residue class: **do not** reach for
`register T v asm("$a1")` or an operand constraint, both banned. Tried and
found NOT to move it: swapping `tbl`/`u14` statement order (with the
barrier), moving the barrier before both statements instead of between
them (regressed, reintroduced drift), and declaring `e2` differently.

## Attempts

~12 iterations total across two sessions (initial derivation, if/else
polarity swap alone -- worse, `.o`-length investigation that found the
real 2-word shortfall, a scheduling-barrier fix for the `nop`, an
intermediate-element-pointer fix for the second loop, the polarity swap
RE-tried in combination with the barrier -- this is what actually worked,
plus a few barrier-placement and statement-order variations on the
residual register mismatches that did not help). Current best: 125/140,
zero drift, 15 pure register-renames remaining.

### Proposed learnings

- **`funcdiff.py`'s reported byte range is not proof of a stalled
  function's correct compiled length.** When a large residue includes an
  absolute address constant (a `%hi`/`%lo` pair, especially against a DATA
  symbol in an unrelated file) that's off by a small multiple of 4 from
  the expected value, suspect the CURRENT function is that many
  bytes short/long before suspecting anything else. Confirm directly:
  `mipsel-linux-gnu-objdump -d build/src/<unit>.c.o`, find the function,
  subtract its start from the next symbol's start, compare against the
  `.s` file's own `nonmatching <func>, 0x<LEN>` header.
- **A missing `nop` in a load-delay slot can be a genuine, fixable
  residue, not just an artifact to accept** -- when two SOURCE statements
  are data-independent, GCC 2.6.3's scheduler may interleave a LATER
  statement's load early enough to fill a delay slot the EARLIER
  statement's own next dependent use would otherwise need a `nop` for.
  Reordering the SOURCE statements does not change this (the scheduler
  reorders regardless); a bare `__asm__("");` between them does, and is
  the permitted form (changes order only, not register identity) -- test
  by checking whether removing it changes which register a value ends up
  in, not just whether it changes the byte count.
- **Two residues that each look like isolated failures in separate
  A/B tests can be entangled, and testing them one at a time gives an
  actively misleading signal.** Here, an if/else branch-layout inversion
  that matched retail's actual polarity scored WORSE in isolation (49 vs
  52) and would read as "wrong direction, don't pursue" -- but combined
  with an unrelated scheduling-barrier fix elsewhere in the same function,
  the SAME inversion took the score from ~83 to 125. When a change you
  have strong independent evidence for (here: direct `.o`-level
  confirmation the branch polarity matches retail exactly) scores worse
  in isolation, consider whether a SECOND fix is a precondition for the
  first one's benefit to show, rather than discarding the first change.

## ROUND 38 (alpha, salvaged by head): preserved body REBUILT and confirmed at 125/140 words

Runner alpha was staffed onto this unit and died to an infrastructure error
(org API 403) with nothing committed. The head restored the worktree and
re-measured this report's preserved body directly, by flipping its
`#if 0` guard to `#if 1` and commenting out the matching `INCLUDE_ASM`,
one body at a time against the whole-image oracle.

**Result: `125/140 words`, compiling and LINKING cleanly (zero hits on the
compile-error grep).** The recorded figure is accurate and the body is real.

This is the Gate 1b "rebuild before trusting" check, and it matters here
because round 37 measured roughly one inherited body in six carrying a false
drift-free claim, plus one body that could never have linked at all (it
called a symbol since renamed, so its figure had measured nothing). **All
four preserved bodies in `class_39e08` were rebuilt this round and all four
are honest** — `StageMap__FindSlotForPosition` 68/70, `ComputeCellWorldOffsets` 58/73,
`StageMap__LoadChunksAround` 125/140, `StageMap__PopulateSlotCells` 130/150. No stale figure and no
never-linked body in this unit.

No new lever was tried — alpha died before attempting one. This is a
confirmation, not a negative result, and the function's permuter status is
unchanged.


## ROUND 40 (HEAD, independent corroboration of the entry above)

**Read this as a second measurement, not a second account.** While runner
bravo was between turns, the head independently read its saved permuter
candidate, translated it, and measured the result -- without access to
bravo's reasoning, which had not been written down yet. **It reached the same
137/140 and identified the same single statement as the cause.** Bravo's own
entry above is the fuller record and is authoritative; this section is kept
for the two things it adds: an independent reproduction of the headline
figure, and two negative variants bravo did not try.

The head's provisional note that the `StageMap__PopulateSlotCells` sibling lever was
"UNTESTED" was correct when written and is now superseded -- bravo tested it
and reached 142/150. See that report.

### The search

| field | value |
| --- | --- |
| iterations | **37155** |
| base score | 75 |
| best score reached | **15** (`output-15-1`) |
| zero found | no |
| stop reason | own `timeout 900` bound |

### The lead, and why it was worth translating even though it was not a zero

Gate 3 says a permuter zero is a LEAD, not an answer. **This was not even a
zero** -- 15 against a base of 75 -- and it was still worth 12 words. The
saved candidate's diff is almost entirely the permuter's own reformatting
(brace style, added parentheses, `asm` for `__asm__`); reduced to statements,
**exactly one thing changed** -- a redundant reload of a pointer already held
in a local, replaced by a write through the full chain:

```c
/* before */
u14 = e->unkC->unk14;
u14->unk0 = 0;

/* after */
e->unkC->unk14->unk0 = 0;
```

`u14` already holds that same value from earlier in the block, so the reload
is semantically dead -- and eliminating it re-shaped allocation across the
whole loop. **Already idiomatic C: no UB, no duplicate-arm artifact, nothing
to clean up.** Translated verbatim and re-verified through the full oracle:
**137/140**.

### What remains: 3 words, register identity

Three consecutive words at vram `0x8004B8C0`, in the second loop (`e2 =
&self->arr[i];` then `e2->unk4->unk32 = e2->unk2;`). Retail computes the
element address into **`$a2`** and reads through it (`addu $a2,$s3,$a0` /
`lw $v1,4($a2)` / `lhu $v0,2($a2)`); we compute it into **`$v0`**. Same
instructions, same offsets, same order -- only the register number differs.
That is CLAUDE.md HARD RULE 6's exact test, so it is a STALL and not
something to pin.

Two source shapes tried on it this round, both negative, so the next reader
does not re-run them:

| variant | score |
| --- | --- |
| candidate as translated | **137/140** |
| drop the local, index directly | 107/140 |
| read the value into a `u16` temp first | 136/140 |

### Proposed learning (round 40)

**A sub-base permuter candidate is worth translating even when the search
never reached zero, and the cost of finding out is one diff.** The prevailing
reading of Gate 3 is about zeros; this function went 125 -> 137 on a candidate
that scored 15, not 0. The screening question is not "did it reach zero" but
**"reduced to statements, what did it actually change?"** -- here, one line,
and the rest was reformatting noise that makes the raw diff look far too big
to be worth reading.

### Preserved body (137/140, the best reached)

```c
#if 0
void StageMap__LoadChunksAround(Obj866E8 *self, s32 val, Unk54Struct *arg2, ChunkSlotSpec *arg3) {
    s32 divisor;
    s32 flag;
    s32 savedResult;
    s32 count;
    s32 i;
    Elem *e;
    Elem *e2;
    SplitCoord2 *u14;
    Unk54Struct *tbl;
    ChunkLoadEntry stackBuf[7];

    if (arg3 != 0) {
        divisor = self->unk68->divisor;
        flag = (val / divisor) & 1;
        savedResult = StageMap__ComputeNeighbourMask(self, val, flag);

        count = 0;
        for (i = 0; i < 7; i++) {
            e = self->methods->slot118(self, i);
            e->unk2 = arg3[i].key;
            if (arg3[i].flag != 0) {
                tbl = &sNeighbourOffsets[arg3[i].key];
                __asm__("");
                u14 = e->unkC->unk14;
                if (self->unk68->unk4 == 0) {
                    u14->unk18.w = arg2->unk0 + tbl->unk0;
                    u14->unk1C = arg2->unk4;
                    u14->unk20.w = arg2->unk8 + tbl->unk8;
                } else {
                    u14->unk18.w = arg2->unk0 - 0x5000;
                    u14->unk1C = arg2->unk4 + tbl->unk4;
                    u14->unk20.w = arg2->unk8 - 0x5000;
                }
                e->unkC->unk14->unk0 = 0;
                StageMap__ComputeChunkLoadEntry(self, &stackBuf[count], divisor, flag, val, savedResult, arg3[i].key);
                count++;
            }
        }

        for (i = 0; i < 7; i++) {
            e2 = &self->arr[i];
            e2->unk4->unk32 = e2->unk2;
        }

        self->methods->slotFC(self, stackBuf, count);
    }
}
#endif
```

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B700` | `StageMap__LoadChunksAround` | B | Occupant of `gStageMapMethods` +0x0F8 (`slotF8`, verified its own identity via classtable -- see the corrected slot comment in `include/class_3bb8c.h`). Iterates a `ChunkSlotSpec[7]` (`sDefaultTargetSpecs` at its one known call site), calling `StageMap__ComputeNeighbourMask` once and `StageMap__ComputeChunkLoadEntry` per enabled entry to fill a 7-slot `ChunkLoadEntry` stack buffer, then dispatches the filled count through `slotFC` (`StageMap__ApplyChunkLoads`). "Build...RateEntries" names the mechanic (assembling the entry array that the next slot applies), consistent with the sibling already-matched functions in this same vtable region (`StageMap__StartScaleRamp`, `StageMap__StepScaleRamp`, `StageMap__EndScaleRamp`). |

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

This function: `StageMap__BuildRateEntries` -> `StageMap__LoadChunksAround` (`python3 tools/rename.py StageMap__BuildRateEntries StageMap__LoadChunksAround`, tier B): assigns every slot its neighbour key; each slot marked `load` moves its cellParent to centre + `sNeighbourOffsets[key]` and gets a ChunkLoadEntry; then applyChunkLoads.

## Round 94 (track 6, charlie)

`Unk14Obj` is `SplitCoord2` (include/class_3bb8c.h): the slot's
cellParent->coord2, a GsCOORDINATE2 with tx/tz as word-or-halfword unions.
Its fields are named for the GsCOORDINATE2 words they overlay: `unk18` ->
`tx` (coord.t[0]), `unk1C` -> `ty`, `unk20` -> `tz`; `unk0` (flg) had no
accessor through this view and is padding. Zero bytes changed.

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals, tier A: `val` -> `centreChunk`, `arg2` -> `centrePos`, `arg3` -> `specs` (the slot declaration's `specs`), `divisor` -> `columns`, `flag` -> `oddRow` (`(centreChunk / columns) & 1`), `savedResult` -> `onGridMask` (ComputeNeighbourMask's result), `e` -> `slot`, `u14` -> `origin` (the slot's cellParent coord2 as SplitCoord2), `tbl` -> `offset` (the sNeighbourOffsets entry), `stackBuf` -> `loads`.

Constants: the key loop runs to `CHUNK_NEIGHBOUR_COUNT` (enum ChunkNeighbour), the slot loop to `ARRAY_COUNT(self->slots)`, `loads` is `[CHUNK_NEIGHBOUR_COUNT]`, 0x5000 -> `STAGE_CHUNK_SIZE / 2`. The reuse of `slot` in the second loop keeps a one-line `/* MATCHING */`.

The comment that stood above the function in `src/class_39e08.c`, moved here verbatim (its local names are the pre-track-7 ones):

```c
/* MATCH, round 63 (delta): closed a 137/140 stall that had stood since round
 * 40 across four re-verifications, ten inert structural variants and a
 * 37,155-iteration permuter search -- see docs/match-reports/StageMap__LoadChunksAround.md.
 * The 3-word residue was a genuine pure register-identity difference (funcdiff
 * ins 0 / del 0, no asm-differ markers): retail held the second loop's element
 * pointer in $a2, the build in $v0. The fix was to DELETE a local -- the
 * second loop reuses `e`, the same variable the first loop walks, instead of a
 * separate `e2`. Nothing else in the body changed.
 * That axis is exactly the one a permuter cannot reach: it mutates a body, it
 * does not merge two of its locals into one. Same lever as StageMap__ComputeChunkLoadEntry this
 * round.
 * The `__asm__("")` barrier this body used to carry before `u14 = ...` is gone:
 * with `e` merged it is no longer needed, verified by whole-image rebuild. */
```
