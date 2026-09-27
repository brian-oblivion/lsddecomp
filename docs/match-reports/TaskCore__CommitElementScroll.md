# TaskCore__CommitElementScroll -- MATCHED 118/118 (round 75): the `local[2]` pair was a WHOLE-STRUCT COPY

> Renamed from `Obj86B60__CommitElementScroll` on 2026-09-25 (tools/rename.py). Address 0x8003dad4.

> Renamed from `func_8003DAD4` on 2026-09-24 (tools/rename.py). Address 0x8003dad4.

REVISITED, round 75: MATCHED; names/types used (the +0x10/+0x14 pair is now a local `SlotPos` struct view, `pos`)

## Round 75 (runner charlie): MATCHED -- struct assignment, no barrier, no temps

**Baseline first.** The preserved body below (field names updated to the
current header: `unk58` -> `activeSlot`, `unk60` -> `slotCounts`) was
compiled live in place of the INCLUDE_ASM and rebuilt through the oracle:
**114/118, `insertions 0 / deletions 0`, positional skeleton diffs 4** --
the report's inherited figure, honest. The four differing words were the two
residues rounds 19-49 describe (the `i = 0` delay-slot placement and the
`target->unk14` temp in `$v0` instead of `$a1`).

**The lever: the stack pair is a struct, and it is filled by struct
assignment.**

```c
typedef struct { s32 x; s32 y; } SlotPos;
#define SLOT_POS(target) (*(SlotPos *)&(target)->unk10)
...
    SlotPos pos;
...
    pos = SLOT_POS((Unk24Elem *)self->unk4C->unk24[idx]);
    pos.y -= counter * 10;
...
        (*arr)->methods->slotBC(*arr, &pos);
        pos.y += 10;
```

First build: **118/118, `build exit=0`, `OK: build matches retail`.** Both
residues went at once, and so did the two compensations the old body needed:

- GCC 2.6.3 expands an 8-byte, 4-aligned struct assignment through the MIPS
  `movstrsi` pattern, whose output is `lw`/`lw` into two FRESH scratch
  registers, then `sw`/`sw`. Those scratches are why retail's second word
  lands in `$a1`: it was never the allocator choosing a register for a named
  `t1`, it was a clobbered scratch of a block move. That was the whole
  "register-identity" residue B.
- A block move is opaque (BLKmode) to later passes, so `pos.y -= ...` must
  RELOAD `.y` from the stack. The old body's `__asm__("" ::: "memory")` existed
  only to force that reload (round 39 measured it as "a LENGTH lever"); the
  struct copy produces it naturally.
- With neither the barrier nor the named temps, reorg puts `i = 0` back into
  the loop guard's `blez` slot and leaves the early-return `bne` slot a `nop`
  -- residue A was a side effect of the barrier.

Tell, for the next function: **two adjacent `lw` of consecutive words into
distinct fresh registers, two `sw` to consecutive stack words, then an
immediate reload of one of those stack words.** That is a struct copy, not
two scalar assignments plus a barrier.

The same lever transfers to the sibling `TaskCore__RefreshSlotView` (see its report).

### Proposed learning

**An `__asm__("" ::: "memory")` that exists only to force a reload of a
just-stored stack word is a symptom of a missing struct assignment.** Round
39 correctly measured the barrier as load-bearing (removing it lost two
words) and correctly classified it as a length lever, and that made it look
like a finished part of the body. It was compensation: a whole-struct copy
is BLKmode, so the following field access must go back to memory, and the
block-move pattern's scratch registers set the register identity the
allocator was blamed for. The shared header still spells `Unk24Elem`'s
+0x10/+0x14 as two scalars; retyping them as one struct member is a header
change for the head (only `code_2cc8c_b.c` references `Unk24Elem`).

## Earlier title: TaskCore__CommitElementScroll -- STALL: length EXACT (118/118 words, no drift); 114/118 raw word-match; first real diff at in-range word 11 (file 0x2E304 / vram 0x8003DB04), the `bne $v1, $v0` delay slot

## ROUND 49 (runner delta): confirmed negative -- the named-temp+barrier lever that closed a scheduling swap in the sibling `TaskCore__RefreshSlotView` does NOT transfer here

`TaskCore__RefreshSlotView` (same unit, same `Unk24Elem` struct, textually IDENTICAL
`local[1] -= counter * 10;` line) had a `sll`/`lw` scheduling swap
(product-vs-reload instruction order) that survived 5 hand attempts and
a 27k-iteration permuter search until this round found that naming the
multiply's result AND adding a bare `__asm__("")` barrier immediately
after the declaration together (neither alone) fixes it. Since this
function shares the identical line, tried the identical change here on
the baseline preserved body:

```c
__asm__("" ::: "memory");
{
    s32 delta = counter * 10;
    __asm__("");
    local[1] -= delta;
}
```

**Result: regresses sharply, 14/118 with ~232KB outside-range drift.**
Reverted immediately; baseline 114/118 re-confirmed byte-identical
before moving on. This is not a contradiction of the sibling result --
this function's compiled form was never doing the swap in the first
place (its own two residues are the unrelated `i = 0` delay-slot
placement and the shared `target->unk14` register choice), so the extra
barrier here just injects an unwanted ordering constraint into an
already-correctly-scheduled pair. **A lever verified against one
residue's specific compiled shape is not evidence it transfers to a
textually identical line in a sibling function whose compiled shape
differs** -- confirmed per-function, not assumed per-idiom.

### Proposed learning (round 49)

Companion negative to `TaskCore__RefreshSlotView.md`'s round-49 positive: the same
two functions, the same struct, the same literal C statement, and the
lever helps one and badly hurts the other. The discriminator is not
visible in the SOURCE (both start from identical text) -- it is in
which of the two THIS function's compiler already schedules correctly.
Before trying a lever "because it worked on the textually identical line
in a sibling," check whether the sibling's PROBLEM (a specific wrong
instruction order) is actually present here; if the baseline doesn't
exhibit the symptom the lever targets, the lever has nothing to fix and
only adds constraint.

## ROUND 46 (runner delta): FIRST PERMUTER SEARCH on this function -- 130 base score, ~68,672 iterations under a 600s bound, no zero, verdict unchanged

This function had never been permuter-searched despite 27 rounds of hand
attempts (per this round's own assignment brief, which flagged it as the
best-posed never-searched target in the whole brief). Ran all three
mandated checks before spending the search:

1. **Correctness:** the preserved body below (byte-identical to the copy
   already on file) was spliced into `src/code_2cc8c_b.c` in place of the
   `INCLUDE_ASM` and rebuilt through the full oracle in isolation (every
   other INCLUDE_ASM in all four of this runner's units confirmed still
   wrapped first). Reproduces **exactly 114/118, zero outside-range
   drift** -- matches this report's own inherited figure precisely, so
   the body is not stale and links cleanly.
2. **Cost (`--debug --stack-diffs`):** base score **130** = 0 stack
   differences, 0 branch differences, 2 register differences (5 each),
   2 reorderings (60 each), 0 insertions, 0 deletions. This is NOT the
   0/0-insertion/deletion signature that round 45's `ServiceSoundCueSet`
   correctly flagged as outside a source-mutation search's reach --
   there ARE two real reorderings in the score, meaning at least part of
   the residue (residue A, the `i = 0` delay-slot placement this report
   already names) is in principle addressable by a source-level
   mutation, even though 27 rounds of hand attempts have not found the
   mutation. The two register differences correspond to residue B
   (the `target->unk14` temp landing in `$v0` vs retail's `$a1`), which
   this report's own catalogue already treats as a hard register-identity
   stall.
3. **Base-score agreement with the real build:** the scaffold's 130 score
   decomposes exactly into this report's own two named residues (A: one
   moved `addu $s1,$zero,$zero`; B: one register-choice pair) with no
   extra insertion/deletion noise, i.e. the isolated scaffold is scoring
   the SAME thing the in-tree build measures as 114/118. No disagreement
   -- proceeded to search.

**Search:** `-j 6 --stack-diffs --stop-on-zero --best-only`, bounded with
`timeout 600`. Ran the full 600-second wall-clock bound (the process
ended via the timeout, not `--stop-on-zero`) for **~68,672 iterations**.
**No candidate ever beat the 130 base score** -- the log shows the
search repeatedly rediscovering 130 itself (i.e. semantically-equivalent
rewrites of the baseline) alongside a wide scatter of worse scores, never
anything lower.

**Phrased precisely per this round's search-hygiene rule: not closed in
~68,672 iterations under load (600s bound), NOT permuter-exhausted** --
one bounded run at `-j 6` is not an exhaustive search of the space, only
a negative data point. Given residue A has already resisted 27 rounds of
targeted hand mutation (documented in the "Ten variants, all inert or
worse" table above) AND a broad unguided ~68k-iteration randomized
search, and residue B is a register-identity mismatch outside any
C-source lever's reach by CLAUDE.md's own test, a further unguided
search is unlikely to add information without a `PERM_LINESWAP`- or
`PERM_VAR`-guided macro specifically targeting the `i = 0` hoist
placement -- flagging that as the concrete next lever rather than a
repeat of this round's unguided run.

Restored to `INCLUDE_ASM` (the preserved body below, unchanged); full
oracle re-confirmed green (`build exit=0`, `OK: build matches retail`)
before moving on.

### Proposed learning (round 46)

**A base score with nonzero reorderings but zero insertions/deletions is
a different animal from round 45's `ServiceSoundCueSet` (0/0, pure register
identity, correctly unsearchable) even though both eventually resist a
search** -- the PRESENCE of addressable reordering penalty in the base
score is what justifies spending the search budget at all (check 2 is
about whether there is anything for a source mutation to reach, not
merely whether the search succeeds). This function is the useful negative
case: check 2 said "worth searching," the search ran a real 68k
iterations, and it still came back empty -- which is meaningfully
different evidence than a search that was never justified in the first
place.

## ROUND 39 (head): baseline re-verified from scratch, ten source-shape variants, all negative -- but the round-38 hoist lever is CONFIRMED REQUIRED here, and the barrier is a LENGTH lever, not a scheduling one

The preserved body below was spliced fresh against `main` at dc88d61 and
**rebuilds to exactly 114/118 with zero outside-range drift** -- this
report's inherited figure is honest and was re-measured, not carried.

### The four differing words are TWO residues, not four

| word | file | retail | built | residue |
| --- | --- | --- | --- | --- |
| 11 | 0x2E304 | `nop` | `addu $s1,$zero,$zero` | **A: `i = 0` placement** |
| 23 | 0x2E330 | `lw $a1, 0x14($v0)` | `lw $v0, 0x14($v0)` | **B: register identity** |
| 25 | 0x2E338 | `sw $a1, 0x14($sp)` | `sw $v0, 0x14($sp)` | B (same value) |
| 39 | 0x2E370 | `addu $s1,$zero,$zero` | `nop` | A (same instruction) |

**Residue A is ONE instruction in two places, not two diffs.** Retail's
delay-slot pass (`reorg`) fills the loop guard's `blez $s3` slot with the
counter zero-init and leaves the early-return `bne` slot a `nop`; our build
does the opposite. Counting these as two words overstates the gap -- the
function is **two instructions from matching, not four**.

Residue B is the second hoisted temp landing in `$v0` (reusing the dying
base pointer) where retail takes `$a1`. At that point `$a0` holds `idx*4`
and is live later, so `$a1` is simply the next free caller-saved register
-- retail's allocator declined to reuse the base, ours did.

### The round-38 hoist lever is ALREADY APPLIED here, and it is REQUIRED

Retail loads both fields back-to-back with the consumers after:

```
lw $v1, 0x10($v0)     # t0 = target->unk10
lw $a1, 0x14($v0)     # t1 = target->unk14   <- ADJACENT
sw $v1, 0x10($sp)     # local[0] = t0
sw $a1, 0x14($sp)     # local[1] = t1        <- consumers LATER
```

That is exactly the diagnostic in DECOMPILATION_LEARNINGS' "Hoist BOTH
values before EITHER is consumed", and the preserved body already does it.
**Measured here, which is what makes it a datapoint rather than a
restatement: dropping the second temp alone** (`local[1] = target->unk14;`
in place of `s32 t1 = ...; local[1] = t1;`) **collapses the function from
114/118 to 24/118 and makes it SHORTER.** So this function is a positive
instance of the lever, reached by hand in round 12 before the lever had a
name -- and the lever is *necessary but not sufficient*: it buys the whole
body and leaves residues A and B.

**This is the useful shape of the result for the next round.** Round 38
named the lever from five functions it CLOSED. This is the first measured
case where it is required and still leaves residue, which bounds the
lever's claim: it reproduces the load/store *scheduling*, not necessarily
the *register identity* of the values it hoists.

### The `__asm__("" ::: "memory")` barrier is a LENGTH lever

Removing it drops the function to **27/118 and two words SHORT**, with
outside-range drift. Without it GCC keeps `t1` live in a register and
elides retail's reload of `local[1]` from the stack
(`lw $v1, 0x14($sp)` at 0x2E344). The barrier is what forces the reload,
i.e. it buys two *instructions*, not a schedule.

That distinction matters against this report's own round-20 finding that
barriers here are "presumptively harmful on a hoist-distance residue".
Both are true and they are about different barriers: a barrier placed to
move a schedule regressed; this one exists to defeat a load elision. It is
a memory clobber, not a register constraint, so it is within CLAUDE.md
rule 6 (removing it changes which INSTRUCTIONS exist, not which register
holds a value -- verified by the measurement above).

### Ten variants, all inert or worse

Each was spliced, built through the full oracle (`build exit=`, zero
compile-error hits) and scored:

| variant | score |
| --- | --- |
| baseline (preserved body as filed) | 114/118 |
| `i` declared FIRST among locals | 114/118 |
| `for (i = 0; count > i; i++)` (comparison operands swapped) | 114/118 |
| `i = 0;` as its own statement, `for (; i < count; i++)` | 114/118 |
| no `target` local -- chain spelled out twice | 114/118 |
| `target`/`t0`/`t1` declared at function top, no inner block | 114/118 |
| bare `__asm__("")` as the FIRST statement (the documented prologue lever) | 114/118 |
| `count` assigned before `arr` | **110/118** |
| bare `__asm__("")` immediately before the `for` | **1/118** |
| second temp dropped (`local[1] = target->unk14;`) | **24/118**, 2 words short |
| memory barrier removed | **27/118**, 2 words short |

**Six independent declaration- and statement-order permutations produce one
identical score.** Combined with rounds 12, 19 and 20, residue B has now
survived every source-level lever in the project's catalogue, which is the
definition of a register-identity stall. Residue A has survived every
scheduling lever including both barrier positions.

### Proposed learning (round 39)

**A function can REQUIRE the hoist-both-before-either lever and still not
match.** Round 38 named the lever from five closes; this is the first
measured instance where it is load-bearing (removing it costs 90 words and
two instructions) and the function still stalls. The lever reproduces
retail's load/store *scheduling*; it does not by itself settle which
register each hoisted value lands in. When a report says "the hoist is
already there", that is not evidence the lever was tried and failed -- it
may be evidence the lever already paid and the remaining residue is a
different class.

**And: count an instruction that MOVED as one residue, not two.** Residue A
shows as two differing words at opposite ends of the function because one
`addu $s1,$zero,$zero` swapped delay slots with a `nop`. A raw word-match
figure charges it twice. This function's honest distance is two
instructions; "114/118" reads as four.

## Round 20 (runner delta): two more attempts, both negative; residues confirmed identical to round 19's description

Drift-checked the preserved body fresh (`git diff` shows no changes had
crept in): rebuilds to exactly 114/118 with no outside-range drift,
matching this report's own baseline precisely. Both residues confirmed
via `tools/asm-differ` at the same two spots this report already names:

1. `move $s1,zero` (the loop counter's zero-init) lands in retail's
   `blez $s3,.L2e3d0` delay slot (the LOOP GUARD, right where the loop
   itself begins); this body's compiled form hoists it all the way to
   the function's FIRST branch instead (`bne $v1,$v0,.L2e484`, the
   `self->unk3C != 2` early-return guard) -- a much larger hoist distance
   than "one delay slot over," and the same mechanism this round's other
   reports name precisely (a scheduler filling an available slot with
   independent, already-computable work): `func_8003FCFC.md`,
   `func_8003F848.md`, and this unit's own `TaskCore__RefreshSlotView.md` (companion
   function, same round).
2. `target->unk14`'s temp lands in `$v0` here vs `$a1` in retail --
   IDENTICAL residue and IDENTICAL surrounding code shape to
   `TaskCore__RefreshSlotView`'s own residue 2 (same `Unk24Elem` struct, same
   `unk10`/`unk14` field pair, same local-buffer-build idiom). Confirmed
   side-by-side this round rather than assumed from the two reports'
   separate descriptions.

### Attempts (2)

1. **A bare `__asm__("")` scheduling barrier placed immediately after the
   `self->unk3C != 2` guard** (a position round 12's own attempts did not
   try -- that report tried a barrier "immediately before the loop,"
   this one targets the OTHER end of the hoist, where the value ends up
   landing): **regressed sharply to 23/118** with a large outside-range
   drift warning, the same failure mode `func_8003FCFC.md` and
   `func_8003F848.md` hit this round with barriers placed elsewhere in
   their own functions. **A fifth confirmed instance (with those two,
   plus round-12's own two barrier attempts on THIS function) of
   `__asm__("")` failing to generalize past its one documented use**
   (callee-save prologue ordering) -- this is now a large enough count
   that it should be read as "presumptively harmful on a hoist-distance
   residue," not merely "sometimes doesn't help."
2. **A fresh, block-scoped loop variable** (`{ s32 j; for (j = 0; j <
   count; j++) {...} }` instead of the pre-declared `i`, isolating the
   loop's own scope from the rest of the function's declarations): no
   change at all -- byte-identical to the baseline 114/118. Confirms
   (again, as this report's own round-12 "what did NOT work" section
   already established for OTHER declaration-order questions) that GCC
   2.6.3's hoist-distance choice for a zero-initialization is not
   influenced by the variable's C-level name or declaration scope, only
   by its data-dependency graph -- which has nothing forcing it to stay
   local to the loop guard's own delay slot.

Both reverted immediately; final state re-verified byte-identical to the
114/118 baseline before restoring `INCLUDE_ASM`.

### Proposed learning

**`__asm__("")` failing to generalize past its one documented use is no
longer a occasional caution -- it is now a 5-for-5 negative record this
round alone** (this function's two new attempts, plus
`func_8003FCFC`'s barrier, plus `func_8003F848`'s implicit confirmation
via the same class, plus round-12's own two attempts on this same
function). Every barrier tried on a hoist-distance/delay-slot-placement
residue this round either did nothing or actively regressed by breaking
an already-correct DIFFERENT delay-slot filler elsewhere in the same
function. The one documented working case (callee-save prologue
ordering) remains the only confirmed positive instance in the project's
history. Worth treating "try a barrier" as a LOW-PRIORITY, likely-harmful
lever for this residue class specifically, not a cheap first thing to
reach for.

**This function's residue 1 (hoist DISTANCE, not just delay-slot choice)
is a variant worth distinguishing from residue 2 (register CHOICE) and
from `TaskCore__RefreshSlotView`'s residue 1 (instruction ORDER swap that opens/
closes an available slot)** -- three related-looking but mechanically
distinct sub-classes now confirmed within this one unit's `Unk24Elem`-
touching family:
- a wrong-slot hoist that travels ACROSS multiple branches (this
  function, residue 1),
- a swap between two adjacent, commutative-order instructions that
  changes which slot even EXISTS to be filled (`TaskCore__RefreshSlotView`,
  residue 1),
- a same-instruction, different-register temp choice with no
  scheduling component at all (`TaskCore__RefreshSlotView` residue 2 and this
  function's residue 2 -- confirmed identical this round).

Restored to `INCLUDE_ASM`. Full oracle re-confirmed green
(`build exit=0`, `OK: build matches retail`) before moving on.

## Round 19: two more attempts on the delay-slot residue, both negative

Re-examined per this round's brief (register-shaped verdicts are the
least reliable class). The two remaining residues are unchanged from
the round-12 description below: (1) `i = 0`'s move schedules into the
function's very FIRST branch's delay slot (the early `self->unk3C != 2`
guard) instead of the loop-guard `blez`'s delay slot where retail puts
it, and (2) `target->unk14`'s temp reuses `$v0` where retail keeps it in
a separate `$a1`.

1. **`for (i = 0; ...)` -> explicit `i = 0;` followed by a `while`
   loop** (removing the for-loop's combined init/test/increment
   clause, in case the for-loop's specific lowering was pinning the
   delay-slot choice): byte-identical output to the `for` version, no
   change at all (114/118, same two residues).
2. **Wrapped the `arr`/`count`/loop block in its own
   `do { ... } while (0)`** (the lever that closed `BoxFill__AttachAbsolute` this
   round for an almost identical symptom -- a delay-slot filler placed
   in the wrong of two available slots): this REGRESSED badly, to
   18/118 with a whole-function length change (the "differs outside
   range" warning fired). Unlike `BoxFill__AttachAbsolute`'s case, the `do/while`
   wrapper here doesn't just re-route one delay-slot filler; it changes
   the compiled length of the loop-adjacent code entirely. Reverted
   immediately.

Both closed as firm negatives for this instance. The `do/while(0)`
lever is confirmed NOT a general delay-slot-residue fix -- it worked for
`BoxFill__AttachAbsolute`'s specific shape (a single unconditional call-plus-two-
stores block) and actively hurts this considerably larger, branch-and-
loop-containing function. Restored to `INCLUDE_ASM`, build re-confirmed
clean (`build exit=0`) before moving on.

### Proposed learning (round 19 addition)

Narrows `BoxFill__AttachAbsolute`'s new `do/while(0)`-wrapper learning: it is a
lever to TRY on a delay-slot-filler-placement residue, not a lever that
generalises to every such residue regardless of the surrounding
function's size or control-flow complexity. Confirmed here as an active
regression on a larger, loop-containing function where it helped a much
simpler call-plus-stores block elsewhere in this same round. Try it
cheaply, verify immediately, and revert without hesitation if it makes
things worse -- do not assume it will help just because the SYMPTOM
(wrong delay slot for an otherwise-correct instruction) looks the same.

**Unit:** code_2cc8c_b · round 12 straggler · slot `+0x10C` (`slot10C`,
per `Obj86B60Methods`).

## What it does

Gated on `self->unk3C == 2` (a twin of the already-matched
`TaskCore__CancelElementScroll`, which is gated on the same value and undoes this
function's `unk3C = 1` at the end — the two form a state-machine pair).
Builds a 2-word local buffer from a target-descriptor record
(`self->unk4C->unk24[idx]`, typed here as the new `Unk24Elem`), walks the
current slot's element array calling two per-element slots with that
buffer, then re-derives the "current" element by `counter` (the target
record's own stashed index) and finishes by writing `counter` back into
the target record's `unk4` field — which is exactly the field
`TaskCore__CancelElementScroll` reads back out as `newVal`, confirming the two share this
record shape.

## New struct knowledge (all confirmed, not part of the stall)

- New type `Unk24Elem` — the pointee of `self->unk4C->unk24[idx]`:
  `+0x004 s32 unk4` (the counter slot `TaskCore__CancelElementScroll` reads as `newVal`,
  written here), `+0x010 s32 unk10`, `+0x014 s32 unk14` (combined with a
  per-slot counter into the 2-word local buffer passed to `slotBC`).
- New `Unk64ElemMethods` slot `+0x0BC slotBC(self, void *buf)`.
- `self->unk68->methods->slot50(self)` — new `Unk68ObjMethods` slot
  `+0x050` (no args beyond self).

All of this is corroborated independently by `TaskCore__RefreshSlotView` (the other
stall this round, which touches the SAME `Unk24Elem`/`slotBC`/`unk68.slot50`
surface) and is safe to keep in `include/code_2cc8c.h` regardless of this
function's own stall status.

## Where it stalled

Best body reaches **114/118 words**, with the whole-image build otherwise
green (this body was reverted to `INCLUDE_ASM` before merge — CLAUDE.md is
explicit that no score short of byte-exact stays in `src/`). The remaining
4 words are TWO independent, purely cosmetic mismatches with no semantic
difference:

1. **One `i = 0` loop-counter initialization schedules into a different
   (but equally valid) delay slot than retail chose** — `move sN, zero`
   lands 4 bytes earlier or later depending on unrelated register-pressure
   decisions elsewhere in the function. Every restructuring tried (moving
   the statement, wrapping it in its own block, an explicit `for(i=0;...)`
   vs. a pre-loop `i=0;`, a bare `__asm__("")` scheduling barrier
   immediately before the loop) either left it unchanged or actively
   regressed the rest of the function's byte count. A bare scheduling
   barrier is the CLAUDE.md-sanctioned tool for exactly this (order-only,
   no register pinned) but a barrier only orders MEMORY-touching
   operations relative to itself — a pure register move like `i = 0` has
   no memory side effect for the barrier to anchor, so it can still float
   across.
2. **One temp register choice for the raw `target->unk14` load differs**
   (`a1` in retail vs. `v0` in the best body) at the exact point building
   the local buffer's second word. Every variant tried (separate named
   temp, direct field access, reordering relative to the first field's
   load) either left this unchanged or (twice) triggered a much bigger
   regression elsewhere, confirming the two are linked through the
   compiler's own register allocation in a way this project's permitted
   toolset (source restructuring, bare scheduling barriers) can't
   independently steer.

This is the class of thing CLAUDE.md calls out directly: *"if removing it
changes WHICH REGISTER holds a value, it is banned; if it only changes
instruction ORDER, it is allowed... A register-identity mismatch is a
STALL."* Both remaining diffs are exactly that — a different but
equally-valid register/scheduling choice, not a misunderstanding of the
function's logic (confirmed: every OTHER instruction in the function,
including all four call sites, both loops, and the final field write,
matches byte-for-byte).

### Head note (round 12) — this stall is NOT the same class as `TaskCore__RefreshSlotView`

Accepted as written; the classification above is correct for THIS function.
Flagging only because the companion report filed the same round
(`TaskCore__RefreshSlotView.md`) described itself as "the same whole-function s-register
renumbering shift documented in `TaskCore__CommitElementScroll`'s report". It is not, and this
report never claimed such a shift:

- Retail here saves **6** callee-saved registers (`$s0..$s5`), leaving two
  s-registers and `$fp` spare — no pressure, and the 114/118 residue is purely
  scheduling and one temp choice.
- Retail in `TaskCore__RefreshSlotView` saves **8** (`$s0..$s7`), saturating the file, and
  that body's 40/145 comes from needing a 9th live cross-call value and
  spilling into `$fp`.

```sh
grep -oE 'sw +\$s[0-9]' asm/nonmatchings/code_2cc8c_b/TaskCore__CommitElementScroll.s | sort -u | wc -l   # 6
grep -oE 'sw +\$s[0-9]' asm/nonmatchings/code_2cc8c_b/TaskCore__RefreshSlotView.s | sort -u | wc -l   # 8
```

Do not carry this function's fixes to that one expecting them to transfer —
the companion report observed that they did not, and the register census above
is why. See the reclassification in `TaskCore__RefreshSlotView.md`.

## What DID work, for the next attempt

Getting from a naive first draft (~10/118, ~180KB of image-wide address
drift from oversized code) to 114/118 took several real fixes, in this
order:

1. **`local[1] = target->unk14 - counter * 10;` as ONE expression silently
   drops a real memory round-trip.** Retail stores the RAW `target->unk14`
   to the stack slot first, then reloads it before subtracting — GCC's
   own optimizer, given the compound form, keeps the value in a register
   and skips the redundant store/reload entirely (verified in isolation
   with the pinned toolchain: `local[1] -= counter*10;` written as either
   one statement or two adjacent statements both compile to a single
   store, no matter how the two are shaped in C). The fix that actually
   reproduces retail's double-store: split the two field reads into named
   temps declared TOGETHER (`s32 t0 = ...; s32 t1 = ...;` both assigned to
   `local[]` before either is used further), THEN a
   `__asm__("" ::: "memory")` barrier, THEN the subtraction as its own
   statement. The memory-clobber barrier forces the two stores to actually
   happen (no register pinned — it's a barrier, not an asm operand), and
   using two named temps for the two loads (rather than the array elements
   directly) makes them land in two DIFFERENT scratch registers matching
   retail's `v1`/`a1` split instead of reusing one register twice.
2. **The element-array loop must NOT cache `*arr` across both per-element
   calls.** First cut: `Unk64Elem *elem = *arr; ...slot60(elem,0);
   ...slotBC(elem,local);`. Retail RELOADS `*arr` independently for the
   SECOND call too (two separate `lw`s from the same not-yet-incremented
   address) rather than keeping the first load's value alive across the
   first call. Rewriting the loop body as `(*arr)->methods->slot60(*arr,
   0); (*arr)->methods->slotBC(*arr, local);` (no named `elem` at all)
   dropped the function from 9 live callee-saved registers to 8 and fixed
   the entire loop body to a byte-for-byte match — this was the single
   biggest jump (43 → 108/118 words in one edit).
3. **The final `arr[counter]` re-derivation needs its OWN fresh variable,
   not the loop's `arr`.** Reusing the loop's `arr` name for
   `self->unk64[idx]` a second time (after the loop) kept it tied to the
   same persistent register the loop needed, forcing the intermediate
   `self->unk64[idx]` value into a saved register too. Naming it `arr2`
   (a genuinely fresh local, scoped to just that one use) let the
   compiler put it in a scratch register instead, closing another 2-word
   gap (108 → 112).
4. **Declaration/statement ORDER inside a single expression (e.g. which
   of `target`/`counter` is computed first, which of `t0`/`t1` is
   declared first) has NO effect on the compiled output** — verified
   repeatedly by swapping and rebuilding; GCC 2.6.3's scheduler here is
   driven by data dependencies, not source order, for anything within one
   basic block. Don't spend attempts reordering independent statements
   hoping to influence register choice; it doesn't.

## Preserved near-miss body

```c
void TaskCore__CommitElementScroll(Obj86B60 *self)
{
    s32 idx;
    s32 counter;
    s32 local[2];
    Unk64Elem **arr;
    s32 count;
    s32 i;

    if (self->unk3C != 2) {
        return;
    }
    idx = self->unk58;
    counter = self->unk60[idx];
    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];
        s32 t0 = target->unk10;
        s32 t1 = target->unk14;

        local[0] = t0;
        local[1] = t1;
    }
    __asm__("" ::: "memory");
    local[1] -= counter * 10;

    arr = (Unk64Elem **)self->unk64[idx];
    count = self->unk5C[idx];
    for (i = 0; i < count; i++) {
        (*arr)->methods->slot60(*arr, 0);
        (*arr)->methods->slotBC(*arr, local);
        local[1] += 10;
        arr++;
    }

    {
        Unk64Elem **arr2 = (Unk64Elem **)self->unk64[idx];
        Unk64Elem *elem = arr2[counter];

        elem->methods->slot60(elem, 1);
        elem->methods->slotB8(elem, self->unk4C->unk10);
    }

    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];

        target->unk4 = counter;
    }

    self->unk68->methods->slot50(self->unk68);

    self->unk3C = 1;
    self->methods->slot60(self, 0x10);
}
```

Requires (already committed to `include/code_2cc8c.h`, so this body
compiles as-is against current `main`): `Unk24Elem`, `Unk64ElemMethods`
with `slot60`/`slotBC`/`slotB8`, `Unk68ObjMethods` with `slot50`,
`Obj86B60Methods` with `slot60`, `Obj86B60.unk68`/`unk4C`/`unk58`/`unk3C`,
`Unk4CObj.unk10`/`unk24`/`unk5C`/`unk60`/`unk64`.

### Proposed learning

A CACHED single-load-reused-across-two-calls pattern and a
DOUBLE-RELOAD-per-call pattern are BOTH real, observed shapes in this
codebase (see `TaskCore__UpdateSlotElements`'s report for the tail-merge angle) — which
one retail used is NOT guessable from the C alone; it shows up ONLY as a
register-count difference in the diff (one extra callee-saved register
needed = the value is being kept alive across a call it doesn't need to
survive). When funcdiff/asm-differ shows a uniform s-register renumbering
shift across an ENTIRE function with every individual instruction
otherwise correct, look for exactly this: a local variable whose value is
consumed by TWO OR MORE calls where retail re-derives it via a second
cheap memory read instead of holding it in a register across the first
call.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__CommitElementScroll`. **Tier B**: Gated on `self->unk3C == 2` (state 2 -> 1, the counterpart of TaskCore__BeginElementScroll). Repositions every item, highlights the one at the slot's current ring cursor (`slotCounts[idx]`), and writes that cursor value into `SlotEntry::savedCursor` -- persisting the value the interactive scroll landed on. 'Commit' is the mechanics: the currently-scrolled-to position becomes the new persisted one.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__CommitElementScroll (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

listView is a BoxFill (include/BoxFill.h); the `Unk68Obj` slot50 call is detachFromParent. Zero bytes.

## Track 7 (2026-09-27, round 98, bravo)

`SlotEntry` and `SrcDesc` moved out of `include/code_2cc8c.h` into
`src/code_2cc8c_b.c` (only this unit uses them). `SlotEntry`'s `unk10`/`unk14`
pair is now a `SlotPos pos` field, so the `SLOT_POS()` macro and its
`*(SlotPos *)&target->unk10` cast are gone; the source still reads
`pos = entry->pos;`, the same whole-struct copy round 75 found, byte-exact.
The field carries one `MATCHING:` line saying so.

History that lived in the header's comments on `SlotEntry` (moved here, not
deleted): it was derived in round 12 from this function and
`TaskCore__RefreshSlotView`, cross-checked against
`TaskCore__CancelElementScroll`'s `((s32 *)self->unk4C->unk24[idx])[1]` read
at the same +0x004; round 78 renamed it from `Unk24Elem` and `unk4` to
`savedCursor` (tier B). Round 12 left the `(u8 *)...unk24[idx] + 8` buffer in
BeginElementScroll/SetSlotCursor as a raw cast; round 98 names it
`cursorColor` (+0x008, a `SpriteRgb`), and CancelElementScroll's `[1]` read is
now `->savedCursor`. Retail data for the one record the game has,
TitleMenu's `D_80086CA8`: savedCursor 0, cursorColor (128, 128, 0), pos
(53, 57), item names `D_80086C9C` (two strings).
