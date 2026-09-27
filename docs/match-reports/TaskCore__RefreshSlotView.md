# TaskCore__RefreshSlotView -- MATCHED 145/145 (round 75): struct assignment for the position pair, and the unk68 slot4C call takes the pair as a THIRD argument

> Renamed from `Obj86B60__RefreshSlotView` on 2026-09-25 (tools/rename.py). Address 0x8003d73c.

> Renamed from `func_8003D73C` on 2026-09-24 (tools/rename.py). Address 0x8003d73c.

REVISITED, round 75: MATCHED; names/types used (local `SlotPos pos` view of `Unk24Elem`'s +0x10/+0x14 pair; a three-argument call type for `Unk68ObjMethods.slot4C`)

## Round 75 (runner charlie): MATCHED -- two semantic features were missing, not a register or scheduling residue

**Baseline first.** The unit's `#if 0` body (round 49's, verbatim) was compiled
live in place of the INCLUDE_ASM: **136/145, `insertions 0 / deletions 0`,
positional skeleton diffs 9** -- the inherited figure, honest.

### Lever 1: the `local[2]` pair is a whole-struct copy (shared with TaskCore__CommitElementScroll)

```c
typedef struct { s32 x; s32 y; } SlotPos;
#define SLOT_POS(target) (*(SlotPos *)&(target)->unk10)
    pos = SLOT_POS((Unk24Elem *)self->unk4C->unk24[idx]);
    pos.y -= counter * 10;
```

in place of the `t0`/`t1` temps, the `__asm__("" ::: "memory")`, the named
`delta` and the second bare `__asm__("")`. Measured alone: **the
`target->unk14`-in-`$v0` residue and the six-word "register-role swap" both
vanish** (retail's `lw v1`/`lw a0` are the `movstrsi` block move's scratch
registers, and the reload of `.y` is what a BLKmode copy forces -- no barrier
needed). That left ONE instruction: retail's `addiu $a2,$sp,0x10` in the
reload's load-delay slot, with the `sll`/`lw` pair ordered around it. (Score
reads 52/145 in that state only because the function is one word short and
everything after it shifts: insertions 2 / deletions 2 in funcdiff, and
`asm-differ` shows exactly one `<` plus the one `|`.)

### Lever 2: `unk68->methods->slot4C` is called with `&pos` as a third argument

The "early materialization" was never early and never a filler: `$a2` is the
third-argument register, and the value in it is `&pos`. Retail's call in the
`if (a2 != 0)` arm is `slot4C(self->unk68, self->unk14, &pos)`; the header had
typed the slot with two arguments, so the body never passed it. With the third
argument the address is computed before the `beqz` (the block copy's
destination-address pseudo is the same value, so it is live into the arm and
allocated straight into `$a2` -- the arm itself then needs no instruction for
it, which is why nothing in the arm looked wrong), and the scheduler fills the
reload's delay slot with it. **145/145 on the first build, `build exit=0`,
`OK: build matches retail`.** The same edit on the else arm's `slot50`
(`slot50(unk68, 0, &pos)`) measured 88/145 -- only the `slot4C` arm takes it.

Both levers are needed. The final body also drops round 19's inner-block `i`
declarations for one function-level `i` (no effect on the bytes).

### Header note, for the head

`Unk68ObjMethods.slot4C` in `include/TaskViewport.h` is declared
`(Unk68Obj *self, s32 a1)`; its only observed caller is this function, which
passes three arguments. The shared header was left untouched (additive-only
rule); the unit carries a local `Unk68Slot4CFn` call type and casts at the
call site. Retyping the slot to `(Unk68Obj *self, s32 a1, void *pos)` and
dropping the cast is a zero-byte cleanup. Likewise `Unk24Elem`'s +0x10/+0x14
could become one struct member (only `TaskViewport.c` uses `Unk24Elem`).

### Proposed learning

**A "delay-slot filler" that loads an ARGUMENT register (`$a0`-`$a3`) with a
value the next call does not take is a missing argument, not a filler.** This
residue was filed for six rounds as "early materialization of `&local`,
redundant move, never reproduced by any C-level lever" and classed with
`FadeBox__PushPosition`'s permuter-exhausted word. The value was the third
argument of the call immediately after the branch; the prototype had one
argument too few, so no body could produce it. Check the register class
before the word: a stray write to `$aN` right before a call site means read
the call's arity first. This is the prototype-arity lever from the other
direction (round 75's other revisit had one argument too MANY).

## Earlier title: TaskCore__RefreshSlotView — STALL, FURTHER PROGRESS: length EXACT (145/145, no drift); 136/145 raw word-match; two residues remain, both register-identity/delay-slot-filler, not scheduling

## Round 49 (runner delta): the sll/lw scheduling swap that survived 5 hand attempts and a 27k-iteration permuter search is CLOSED -- a named temp PLUS a bare barrier, not either alone

Re-verified the inherited stall first: the round-19/46 preserved body
(unchanged) reproduces exactly **50/145 raw, with the expected
`WARNING: differs OUTSIDE this range too (232311 bytes)`** -- honest,
matching this report's own figure precisely (this is the "144/145
compiled, ONE short" body, misleadingly low on the raw window compare
per round-20's own note).

### The lever: name the multiply's result, THEN add a bare scheduling barrier before consuming it

Round 20's attempt 1 already tried naming the multiply
(`s32 delta = counter * 10; local[1] -= delta;`) alone -- **no change**.
This round adds ONE bare `__asm__("");` immediately after that
declaration, before the subtraction:

```c
__asm__("" ::: "memory");
{
    s32 delta = counter * 10;
    __asm__("");
    local[1] -= delta;
}
```

Neither half alone does anything (confirmed separately this round: the
named temp with NO added barrier reproduces the exact 50/145-with-drift
baseline, byte-for-byte identical to the existing round-20 record).
**Together, they close the length gap outright and fix the instruction
ORDER**: `asm-differ` (realigned) now shows the full `sll`/`addu`/`sll`
multiply sequence completing BEFORE the reload of `local[1]`, exactly
retail's own order -- the swap that resisted every prior lever is gone.
**Length is now exactly 145/145 words with NO outside-range drift**,
where every previous body (14 hand attempts + one permuter search) was
one word (4 bytes) short.

### What's left: two SEPARATE, more tractable residues, not the swap

With the swap fixed, the raw score is 136/145 (9 words), all inside one
15-word window (vram `0x8003D9F8`-`0x8003DA20`) and all in one of three
buckets:

1. **The `target->unk14` register choice (2 words, vram `0x8003D9F8`/
   `0x8003DA00`)**: `a0` in retail vs `v0` in this body -- confirmed via
   `asm-differ` to be the IDENTICAL residue this unit's own
   `TaskCore__CommitElementScroll` report already carries as its unfixable register-
   identity stall (same struct, same field, same load/store pair). Not
   new; already resistant to every C-level lever across two functions
   and 30+ combined rounds.
2. **A register-ROLE swap in the multiply/reload pair itself (6 words,
   vram `0x8003DA04`-`0x8003DA20`)**: retail computes the product into
   `v0` and reloads `local[1]` into `v1`; this body swaps which of the
   two lands in which register (product in `v1`, reload in `v0`). Every
   individual INSTRUCTION is now in the right ORDER (see above) -- only
   the register each one uses differs, symmetrically, as if the
   allocator picked the opposite assignment for this one pair. Untested
   this round whether this cascades FROM residue 1 (freeing `a0` instead
   of `v0` for the unk14 temp could plausibly perturb what's free for
   this pair) or is independent; flagging as the next thing to check
   before assuming it needs its own separate lever.
3. **One delay-slot filler (1 word, vram `0x8003DA08`)**: retail issues
   `addiu $a2,sp,0x10` (the early materialization of `&local`, needed
   only much later at the second loop's `slot4C` call) in the reload's
   own load-delay slot; this body leaves it a `nop`. This is the SAME
   residue CLASS already on file for `FadeBox__PushPosition`'s own permuter-
   exhausted final word and (per round-19's own note on this exact
   function) an earlier form of this function's own stall -- an
   early-materialized, delay-slot-filling redundant move that has never
   been reproduced by any C-level lever in this project's history.
   **Not re-attempted this round**: round 19's own explicit early-alias
   attempt (`s32 *buf = local;` used at the second loop's call site)
   regressed hard (4/145, ~180KB drift) under the OLD (swapped) ordering
   baseline; re-testing it under this round's NEW (order-fixed) baseline
   is the natural next attempt but was not reached this round.

### Confirmed negative: this lever does NOT transfer to `TaskCore__CommitElementScroll`

`TaskCore__CommitElementScroll` has the textually IDENTICAL `local[1] -= counter * 10;`
line (same struct, same idiom) but the plain baseline there is already
114/118 with the swap NOT among its residues (its own two residues are a
delay-slot placement for an unrelated `i = 0` and the same `target-
>unk14` register choice as above). Applying the identical named-temp+
barrier change there **regresses sharply to 14/118 with ~232KB drift**
-- confirmed and reverted immediately. This is not a contradiction: the
lever targets a scheduling swap that only exists in `TaskCore__RefreshSlotView`'s
compiled form in the first place; forcing the same barrier into a
function whose compiler-chosen order was already correct just injects
an unwanted ordering constraint. **A lever closing one residue class in
one function is not evidence it is safe to try in a textually identical
line elsewhere** -- verify per-function, not per-idiom.

Restored to `INCLUDE_ASM` (per-project rule -- no score short of
byte-exact stays in `src/`); full oracle re-confirmed green (`build
exit=0`, `OK: build matches retail`) before moving on.

### Proposed learning (round 49)

**A residue described as "the compiler's own instruction scheduler
choosing to finish the multiply's last op before or after an unrelated
reload" (round 46's own characterization, offered as a reason NOT to
try a `PERM_LINESWAP` search) is not necessarily below the reach of a
named-temp-plus-barrier combination, even when naming alone and a
barrier placed elsewhere (round 7's mid-expression and first-statement
positions, tried on the SIBLING residue in `Viewport__InitOt`) both fail.**
The combination that worked here is narrow and specific: name the
independent sub-computation, THEN place the barrier directly between
that declaration and the statement that consumes it -- not before the
whole expression, not after the whole statement. Round 46's own
conclusion that "no amount of statement splitting reaches the
scheduler's internal choice" was half right (splitting alone, round 20's
attempt 1, really doesn't) but the ADDITIONAL barrier is not "more
statement splitting," it's an ordering constraint the split alone
doesn't supply. Worth re-testing on any other "adjacent independent
sub-computation" residue in this project's queue before concluding no
lever exists.

**And: fixing a scheduling residue does not imply fixing the DIFFERENT
residues it was drift-obscuring.** This function's length-short state
made every OTHER residue invisible to a straight window compare; now
that the length is exact, the remaining two residues are cleanly
separable and one of them (register choice) is immediately recognizable
as already-known, cross-function evidence rather than a fresh mystery.

## Round 46 (runner delta): drift-checked fresh, no new attempt -- DELIBERATE SKIP, reason recorded

Re-spliced the exact preserved body below (unchanged) into
`src/TaskViewport.c` in isolation (every other INCLUDE_ASM in all four of
this runner's units confirmed still wrapped first) and rebuilt through
the full oracle: reproduces **50/145 words, with the expected
`WARNING: differs OUTSIDE this range too (232312 bytes)`** -- exactly the
documented consequence of this body compiling 4 bytes (one instruction)
short, per this report's own round-20 section above. Not stale, not a
new regression.

**Deliberately skipped a new attempt, per this round's own "the
round-33 selection effect" instruction to say so with a reason rather
than burn attempts.** The single remaining residue (the `sll`/`lw`
scheduling swap inside `local[1] -= counter * 10;`) lives BELOW the
granularity of a C statement: it is the compiler's own instruction
scheduler choosing to finish the `counter * 10` multiply's last `sll`
before or after reloading `local[1]`, both of which come from ONE
already-atomic C expression. Round 20's own attempts 1 and 2 (naming the
multiply as an explicit temp; writing the subtraction out explicitly
instead of `-=`) already tested exactly the two ways this expression
could be split into separate statements, and both reproduced the
identical baseline byte-for-byte. A `PERM_LINESWAP`-guided permuter
search (floated as the next lever in round 20's own write-up) needs two
independent C *lines* to swap, and there are none here to hand it --
the swap retail made is internal to how cc1 schedules a single
expression's sub-computations, not a reordering of statements. Spending
a guided search here would be searching for a line boundary that does
not exist in the source; not attempted for that reason.

No new lever found this round. Restored to `INCLUDE_ASM`; full oracle
re-confirmed green (`build exit=0`, `OK: build matches retail`).

### Proposed learning (round 46)

**A residue described as "instruction A and B are swapped" is only a
`PERM_LINESWAP` candidate if A and B are separable at the C-statement
level.** When both come from evaluating one arithmetic expression (here,
`counter * 10` finishing before or after an unrelated reload), no amount
of statement splitting reaches the scheduler's internal choice, and this
was already established by hand in round 20 -- worth checking a
residue's OWN statement boundary before recommending a guided macro
search as "the next lever," since the recommendation can look plausible
and still target a swap the C source has no seam for.

## Round 20 (runner delta): four more attempts on the one-word residue, all negative; mechanism pinned down precisely

**Read the title carefully before staffing, per the coordinator's own
warning**: "50/145 raw" and "1-word residue" describe the SAME body --
`objdump` confirms 144 of 145 words compiled correctly (one instruction
missing), and that one missing word is what shifts every subsequent word
in the raw `funcdiff` count, producing the misleadingly low 50/145. Do
not read this as "95 words wrong."

**A tooling scare, resolved, worth recording so nobody repeats the
detour:** rebuilding this body threw a much larger-looking failure than
expected -- the whole-image `cmp -l` first divergence landed at file
offset `0x1908`, inside a DIFFERENT unit's rodata
(`code_2cc8c_e.c.o`'s `jtbl_80011108`), nowhere near this function's own
address range. This is NOT a new bug and NOT evidence of cross-unit
corruption -- it is the ordinary, fully-expected consequence of this
function compiling 4 bytes (one instruction) short: with `section_order`
concatenating every file's output in a fixed order, a 4-byte shrink in
this unit's `.text` shifts the START address of every later file's
sections by 4 bytes, which is exactly what showed up as a mismatch deep
inside `code_2cc8c_e`'s already-matched rodata. Confirmed by direct
section-size diffing (`objdump -h` on `TaskViewport.c.o`, INCLUDE_ASM
vs. this body: `.text` `0x1258` -> `0x1254`, exactly 4 bytes) --
CLAUDE.md's own "one instruction short, everything after it shifted"
category, not a new failure mode. **Also confirmed, separately: splat's
`make extract` is match-status-aware of `src/*.c`** -- while this
function was defined as real C (mid-investigation), a fresh `make
clean && make extract` did not regenerate
`asm/nonmatchings/TaskViewport/TaskCore__RefreshSlotView.s` at all, because splat
saw the symbol already implemented in C and skipped generating a
nonmatching stub for it. Restoring `INCLUDE_ASM` and re-running `make
extract` regenerated it correctly. Recording this because it means **an
extract run while mid-investigation on a stalled function can silently
delete that function's own `.s` file** if the C body happens to still be
in the source tree at that moment -- always restore `INCLUDE_ASM` before
running `make extract` (a HARD-RULE-compliant command, but only safe for
this purpose in that state). No tracked file was affected either way
(`asm/` is gitignored), and the final state re-verified clean.

**Located the exact scheduling shape precisely via `asm-differ`**
(not just described abstractly, as the round-19 report's own residue
description was): retail's instruction sequence at the "`local[1] -=
counter * 10`" computation is

```
sll v0,s5,0x2 / addu v0,v0,s5 / sll v0,v0,0x1 / lw v1,0x14(sp) / addiu a2,sp,0x10 / subu v1,v1,v0
```

i.e. finish computing `counter*10` (the third `sll`) BEFORE reloading
`local[1]`, then fill the reload's OWN load-delay slot with the early
materialization of `&local` (needed later, in the second loop's
`slot4C` call, but with nothing else scheduled to need it yet). This
body's compiled form does:

```
sll v0,s5,0x2 / addu v0,v0,s5 / lw v1,0x14(sp) / sll v0,v0,0x1 / subu v1,v1,v0
```

-- i.e. reloads `local[1]` BEFORE finishing the multiply (the two
instructions are swapped relative to retail), and consequently has no
load-delay slot in the same place to fill, so the `&local`
materialization never gets invited there and instead happens later, at
its natural use site. **This is the SAME delay-slot-filler mechanism
this round's other reports name precisely** (`func_8003FCFC.md`,
`func_8003F848.md`) -- a real hardware hazard creates an idle slot, and
the scheduler opportunistically fills it with unrelated-but-safe,
already-computable work. The residue is not "one instruction is simply
missing"; it is "the instruction ordering choice that CREATES the slot
retail fills differs by one swap."

### Attempts (4)

1. **Named the multiply as an explicit temp** (`s32 delta = counter *
   10; local[1] -= delta;` instead of the inline expression): no change
   -- byte-identical to baseline. GCC 2.6.3 still reloads before
   finishing the multiply regardless of whether the multiply's result
   has its own name.
2. **Explicit subtraction instead of `-=`** (`local[1] = local[1] -
   counter * 10;`): no change -- identical output to attempt 1 and the
   baseline. The compound-assignment operator is not what is driving the
   evaluation order.
3. **Different pointer spelling at the use site** (`&local[0]` instead
   of bare `local` in the `slot4C(*arr, a1, local)` call): no change --
   confirms the LATE materialization site is not what needs adjusting;
   the ordering choice happens far earlier, at the reload/multiply pair
   itself, independent of how the pointer is later spelled.
4. **Reversed declaration order of the two loaded temps** (`t1 =
   target->unk14;` before `t0 = target->unk10;`, targeting residue #2 --
   the `target->unk14` register-choice mismatch shared with
   `TaskCore__CommitElementScroll`): no change -- identical output. Declaration order of
   these two temps does not influence either residue.

None of these four reached the swap; all four produced byte-identical
output to the existing 50/145 baseline. Combined with round 19's
~27,000-iteration unguided permuter search (also unable to beat the
base score), this residue has now resisted five independent source-level
variations plus one bounded permuter run.

### What I did NOT try, and why

- **A guided (`PERM_LINESWAP`) permuter search** targeting specifically
  the `sll`/`lw` swap: blocked by the same `setup-permuter.sh`
  cc1-sanity-check tooling boundary documented in `func_8003F764.md`
  (round 19) and confirmed again this round in `func_8003FCFC.md` --
  PERM macros are not real C, so the scaffold-validation step rejects
  them before any search runs. An unguided search already ran (round
  19); a guided one needs an operator-level tooling fix.
- **Re-running the unguided permuter fresh** this round: round 19's own
  search already covered ~27,000 iterations against the identical base
  score (170, confirmed matching this round's own re-derivation of the
  residue), so a repeat run would not add information without a new
  seed or a guided macro -- not repeated per this round's own "don't
  re-confirm an already-negative permuter search" discipline.

### Proposed learning

**A "missing instruction" residue can really be "the wrong ordering of
its TWO NEIGHBORS," not a gap to fill directly** -- the addiu that
retail places is not absent from consideration, it is simply never
INVITED, because this body's compiled form never opens the specific
load-delay slot retail's own instruction order creates. Four source-level
levers that all target the SYMPTOM (naming the multiply, respelling the
subtraction, respelling the later pointer use, reordering unrelated
declarations) left the actual CAUSE (which of two independent,
commutative sub-computations gets scheduled first) untouched. This
strengthens the round-19 report's own tentative note that this class
resists direct C-level manipulation -- worth checking whether a
`PERM_LINESWAP` guided search (once the tooling permits it) can find the
swap, since this looks close to the ideal case for that specific macro
(two adjacent, provably-independent statements whose ORDER alone
differs).

Restored to `INCLUDE_ASM`. `asm/nonmatchings/TaskViewport/TaskCore__RefreshSlotView.s`
regenerated via `make extract` in that state. Full oracle re-confirmed
green (`build exit=0`, `OK: build matches retail`) before moving on to
`TaskCore__CommitElementScroll`.

## Round 19: drift-checked the inherited body, applied the round-12 head's own suggested lever, closed the register-pressure problem

**Drift check first (coordinator broadcast, round 19):** rebuilt the
LITERAL preserved body below verbatim before touching anything. It
compiles to exactly 145 words with NO outside-range drift warning --
the round-12 report's "40/145 with matching total length" claim is
CONFIRMED accurate, unlike the false "clean" claims found this round in
`TextRow__DetachFromParent.md` (and reported elsewhere by other runners for
`DreamSys__AdvanceMoveCycle`/`Entity__MoodCue81`). Recording the negative explicitly per
the coordinator's request: this one checked out.

**The round-12 head's reclassification named the exact fix, and it
works.** That note says: *"go through the preserved body counting
values live across each of the four call sites and re-dereferencing the
cheap ones -- targeting a budget of 8, not chasing the register
numbers."* The preserved body's SECOND loop caches `*arr` into a named
`elem` local and uses it across TWO calls (`slot4C`, `slot60`) --
exactly the "value cached across a call it doesn't need to survive"
shape the round-12 report's own first proposed learning already named.
Removing the cache -- calling through `(*arr)->methods->slot4C(*arr, ...)`
and `(*arr)->methods->slot60(*arr, ...)` directly, re-dereferencing `arr`
for the second call instead of reusing `elem` -- closes the
register-pressure problem OUTRIGHT: **retail's callee-saved file
saturates at exactly 8 (`$s0`-`$s7`), and so does this fix.** No `$s8`/
`$fp` spill, matching retail's own register count for the first time.

**Result: 50/145 words, `objdump`-confirmed 144 words compiled (ONE word
short of retail's 145), no other drift.** Progress from the round-12
report's 40/145-at-9-registers to 50/145-at-8-registers-matching in one
change.

### The one remaining residue

1. **One missing instruction**, an early materialization of `&local`
   into `$a2` (retail: `addiu $a2,$sp,0x10`) that fills what would
   otherwise be the load-delay slot after `lw $v1,0x14($sp)` (the
   `local[1] -= counter*10` reload) -- retail computes this address far
   ahead of its actual first use (inside the SECOND loop's `slot4C`
   call) simply because `$a2` is free at that point (the incoming `a2`
   parameter was cached into `$s6` at function entry) and the scheduler
   had an empty delay slot to fill. This is the exact "redundant
   move"/early-materialization residue class documented for
   `FadeBox__PushPosition`'s own still-open final word (see that report) --
   there, the SAME class of residue was marked PERMUTER-EXHAUSTED after
   an explicit branch-forced-copy trick and a 28k-iteration permuter
   search both failed to reproduce it.
2. **One temp register choice**: `target->unk14`'s value lands in `$v0`
   in this body vs `$a0` in retail (the exact same "which caller-saved
   scratch register holds the second loaded field" residue as
   `TaskCore__CommitElementScroll`'s own still-open residue #2 -- same unit, same
   `Unk24Elem` struct, same `unk10`/`unk14` field pair).

**Attempted a fix for residue 1:** an explicit `s32 *buf = local;`
declared right after building `local[]`, used in place of `local` at the
second loop's `slot4C` call site (to force an early, separately-named
materialization of the pointer). This REGRESSED badly (4/145, ~180KB
drift, `objdump`-confirmed far longer than retail) -- the explicit early
alias changed far more than intended, evidently disturbing the whole
function's register allocation rather than just adding the one missing
instruction. Reverted immediately.

**Permuter run** (`--debug --stack-diffs` confirmed base 170: 1
deletion, 1 reordering, 2 register differences -- an exact match for the
"one missing instruction plus one register choice" diagnosis above).
Bounded search (`-j 6 --stack-diffs --stop-on-zero --best-only`,
`timeout 300`) ran ~27,000+ iterations with no PERM macros; best score
never beat the 170 base. Not phrased as permuter-exhausted (no PERM
macros written, so the search space actually covered is narrow) --
phrased precisely: not closed in ~27,000 iterations under load.

**Verdict: STALL, but reclassified and substantially improved.** This is
no longer the round-12 "9-registers-vs-8, saturated file" problem -- that
is fixed. What remains is the SAME "redundant early move" class
`FadeBox__PushPosition` already carries as its own permuter-exhausted final
residue, plus the SAME "loaded-field temp register choice" class
`TaskCore__CommitElementScroll` carries as its own open residue. Both are now confirmed
present in at least 3 functions across this unit's `Unk24Elem`-touching
family, which is worth treating as a real recurring class rather than
three unrelated coincidences.

### Best body reached this round (144/145 compiled, 8 registers matching retail)

```c
void TaskCore__RefreshSlotView(Obj86B60 *self, void *a1, s32 a2)
{
    s32 idx;
    Unk64Elem **arr;
    s32 count;
    s32 counter;
    s32 local[2];

    idx = self->unk58;
    arr = (Unk64Elem **)self->unk64[idx];
    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];

        count = self->unk5C[idx];
        counter = target->unk4;
    }

    {
        s32 i;

        for (i = 0; i < count; i++) {
            (*arr)->methods->slot50(*arr);
            arr++;
        }
    }

    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];
        s32 t0 = target->unk10;
        s32 t1 = target->unk14;

        local[0] = t0;
        local[1] = t1;
    }
    __asm__("" ::: "memory");
    local[1] -= counter * 10;

    if (a2 != 0) {
        s32 local2[2];

        self->unk68->methods->slot4C(self->unk68, self->unk14);
        local2[0] = 0x28;
        local2[1] = count * 12;
        self->unk68->methods->slotC0(self->unk68, local2);
    } else {
        self->unk68->methods->slot50(self->unk68);
    }

    arr = (Unk64Elem **)self->unk64[idx];
    {
        s32 i;

        for (i = 0; i < count; i++) {
            (*arr)->methods->slot4C(*arr, a1, local);
            (*arr)->methods->slot60(*arr, a2);
            local[1] += 10;
            arr++;
        }
    }

    arr = (Unk64Elem **)self->unk64[idx];
    {
        Unk64Elem *elem = arr[counter];

        elem->methods->slot60(elem, 1);
    }
}
```

Restored to `INCLUDE_ASM` (clean build re-confirmed, `build exit=0`).
The NEXT attempt should start from THIS body, not the round-12 original
below -- it is a strict improvement (register class closed) and the
remaining residue is narrower and better-characterized.

### Proposed learning (round 19)

A THIRD instance (with `FadeBox__PushPosition`, `TaskCore__CommitElementScroll`) of an
early-materialized "redundant move" filling a delay slot that resists
every C-level lever tried (explicit early alias regressed hard here;
`FadeBox__PushPosition`'s branch-forced-copy trick and permuter both failed on
its own instance). Worth naming as a standing residue class in
`docs/DECOMPILATION_LEARNINGS.md` once a 4th instance turns up --
three is close to the threshold this project already uses elsewhere
("three would justify opening a new named residue class",
`func_8003FCFC.md`'s own words) for the sibling "temp register choice"
class.

## Round-12 history (context for the body above; original 40/145 stall)

**Unit:** code_2cc8c_b · round 12 straggler — the largest and last of the
five, 145 words.

## What it does

Takes `(self, a1, a2)`. Walks the current slot's element array pinging
each element (`slot50`), rebuilds the same 2-word local buffer
`TaskCore__CommitElementScroll` builds (from the SAME `Unk24Elem` record at
`self->unk4C->unk24[idx]`, using the SAME `unk10`/`unk14`/`counter*10`
shape — cross-confirms that struct), then either builds a size descriptor
and forwards it through `self->unk68` (when `a2 != 0`) or just pings
`unk68` directly, then walks the array a SECOND time forwarding `a1`/`a2`
and the running buffer to each element via two more slots, and finally
pings the element at the target's own stashed `counter` index.

## New struct knowledge (all confirmed, safe regardless of stall status)

- Corroborates `Unk24Elem` (`unk4`/`unk10`/`unk14`) from `TaskCore__CommitElementScroll`.
- New `Unk64ElemMethods` slot `+0x050 slot50(self)` (also independently
  needed by `TaskCore__ReleaseTarget`, already matched).
- New `Unk68ObjMethods` slots: `+0x04C slot4C(self, s32 a1)` and
  `+0x0C0 slotC0(self, void *buf)` (buf = address of a 2-word
  `{0x28, count*12}` local pair — a size/descriptor of some kind, unit
  never dereferences it further).

## Where it stalled

Best body reaches **40/145 words**, but — and this is the important part —
**the function's TOTAL LENGTH is exactly correct**: `funcdiff.py` raises
no "differs outside this range" warning at all, meaning every function
after this one in the image lands at its correct address. Every one of
the 105 non-matching words is a same-instruction, different-register-number
diff (asm-differ shows zero inserted/deleted instructions across the
entire function body, only `r`-tagged register renames) — the
SAME whole-function s-register renumbering shift documented in
`TaskCore__CommitElementScroll`'s report, one register wider throughout (this function has
3 params — `self`/`a1`/`a2` — vs. `TaskCore__CommitElementScroll`'s 1, so the same
one-register-too-many problem shows up as `s0..s8` where retail uses
`s0..s7`).

Applying every fix from `TaskCore__CommitElementScroll`'s report (memory-clobber barrier
around the raw-store-then-reload local-buffer build; no named `elem`
cached across a call boundary in either loop; a fresh, narrowly-scoped
variable for the post-loop `self->unk64[idx]` re-derivation) got the FIRST
loop's body and the local-buffer build to match byte-for-byte, and got the
whole-function LENGTH to line up exactly — but did not close the
remaining register-count gap. Two further residues, neither resolved
within budget:

1. A second `sll`/reload pair (part of the `counter*10` computation, same
   shape as the one the memory barrier fixed) schedules one slot later
   than retail in the build's current form, despite being structurally
   identical C to the ALREADY-matching occurrence in `TaskCore__CommitElementScroll`.
   Every variant tried (an extra named temp for the multiply, consolidating
   vs. splitting the `target`/`counter` computation into one vs. two
   blocks, an un-named `(*arr)->methods->slot50(*arr)` first loop matching
   `TaskCore__CommitElementScroll`'s successful pattern) left this specific swap
   unchanged — meaning whatever is pinning it is upstream of anything
   these particular restructurings touch, likely interacting with the
   SECOND loop's or the `a2`-branch's own register needs in a way that
   isn't visible by inspecting this local buffer's own code in isolation.
2. The whole-function register count itself: even with the first loop and
   local-buffer build both individually matching, the function as a whole
   still needs one MORE callee-saved register than retail (9 vs. 8, same
   diagnostic as `TaskCore__CommitElementScroll`: epilogue restores `s0..s8` not
   `s0..s7`). Given `TaskCore__CommitElementScroll`'s fixes did NOT fully transfer here
   despite being the closest available template, the extra register is
   evidently tied to something in the SECOND loop or the `a2`-branch
   (neither of which `TaskCore__CommitElementScroll` has an analogue for) rather than to
   anything already isolated.

Per CLAUDE.md, this is a register-identity mismatch, not a logic gap —
every individual instruction the function DOES emit is either an exact
match or an exact match on a DIFFERENT but equally valid register, and no
banned technique (`register T v asm("$N")`, an extended-asm operand
constraint) is available to force it further. STALL.

## Head reclassification (round 12) — this is NOT the same class as `TaskCore__CommitElementScroll`

The stall itself stands, and the runner's measurements are sound. **The
CLASSIFICATION above is wrong, and correcting it changes what the next attempt
should do.**

Measured on the retail side, which settles it:

```sh
grep -oE 'sw +\$s[0-9]' asm/nonmatchings/TaskViewport/TaskCore__RefreshSlotView.s | sort -u | wc -l   # 8  ($s0..$s7)
grep -oE 'sw +\$s[0-9]' asm/nonmatchings/TaskViewport/TaskCore__CommitElementScroll.s | sort -u | wc -l   # 6  ($s0..$s5)
```

- **`TaskCore__CommitElementScroll` has no register-count problem at all.** Its own report
  documents two independent *cosmetic* residues totalling 4 words — a
  delay-slot placement for a register-only `i = 0`, and one temp register
  (`a1` vs `v0`). Retail uses 6 callee-saved registers there, with two s-regs
  and `$fp` still spare. Nothing is saturated.
- **`TaskCore__RefreshSlotView` is a register-PRESSURE stall, which is a different thing.**
  Retail uses `$s0..$s7` — *the entire callee-saved s-register file*, with zero
  headroom. The body reached here needs a 9th live callee-saved value and
  therefore spills into `$s8`/`$fp`, which is why the epilogue restores one
  register too many and why every register number downstream shifts. Hence
  40/145 despite exactly correct length and zero inserted/deleted
  instructions.

So the report's own observation that "`TaskCore__CommitElementScroll`'s fixes did NOT fully
transfer here despite being the closest available template" is not a puzzle —
it is the expected result of the two stalls having different causes. DAD4's
fixes target instruction scheduling; this one needs **fewer values live across
a call**.

**And the lever is already in this round's findings — the runner filed it as an
unrelated observation.** Its own first proposed learning is: *a value read from
memory, used, then re-read for a second use should be written as two separate
dereferences, not a named local, because caching forces cross-call register
survival.* That is precisely the operation that frees a callee-saved register.
Whoever resumes this should go through the preserved body below counting values
live across each of the four call sites and re-dereferencing the cheap ones —
targeting a budget of 8, not chasing the register numbers.

**Generalised stall class, new this round: "retail saturates the callee-saved
file."** Discriminator — count distinct `sw $sN` in retail's prologue; if it is
8, there is no spare callee-saved register and any C shape needing a 9th live
cross-call value cannot match, no matter how the instructions are ordered. This
is diagnosable in one command before spending an attempt budget, and it points
at a specific, permitted fix (reduce live values) rather than at register
identity, which is unfixable by any allowed technique.

## Preserved near-miss body

```c
void TaskCore__RefreshSlotView(Obj86B60 *self, void *a1, s32 a2)
{
    s32 idx;
    Unk64Elem **arr;
    s32 count;
    s32 counter;
    s32 local[2];

    idx = self->unk58;
    arr = (Unk64Elem **)self->unk64[idx];
    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];

        count = self->unk5C[idx];
        counter = target->unk4;
    }

    {
        s32 i;

        for (i = 0; i < count; i++) {
            (*arr)->methods->slot50(*arr);
            arr++;
        }
    }

    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];
        s32 t0 = target->unk10;
        s32 t1 = target->unk14;

        local[0] = t0;
        local[1] = t1;
    }
    __asm__("" ::: "memory");
    local[1] -= counter * 10;

    if (a2 != 0) {
        s32 local2[2];

        self->unk68->methods->slot4C(self->unk68, self->unk14);
        local2[0] = 0x28;
        local2[1] = count * 12;
        self->unk68->methods->slotC0(self->unk68, local2);
    } else {
        self->unk68->methods->slot50(self->unk68);
    }

    arr = (Unk64Elem **)self->unk64[idx];
    {
        s32 i;

        for (i = 0; i < count; i++) {
            Unk64Elem *elem = *arr;

            elem->methods->slot4C(elem, a1, local);
            elem->methods->slot60(elem, a2);
            local[1] += 10;
            arr++;
        }
    }

    arr = (Unk64Elem **)self->unk64[idx];
    {
        Unk64Elem *elem = arr[counter];

        elem->methods->slot60(elem, 1);
    }
}
```

Requires (already committed to `include/TaskViewport.h`): `Unk24Elem`,
`Unk64ElemMethods` with `slot4C`/`slot50`/`slot60`, `Unk68ObjMethods` with
`slot4C`/`slot50`/`slotC0`, `Obj86B60.unk68`/`unk4C`/`unk58`/`unk14`.

### Proposed learning

A whole-function register-numbering shift that survives applying every
fix from a SIMILAR already-matched (or already-STALLED-closer) sibling
function is evidence the extra register lives in a part of the function
the sibling doesn't share (here: the second loop, or the `a2`-conditional
branch) — don't keep re-testing fixes against the part that's already
byte-exact; the diff tool's lack of insert/delete markers across the
WHOLE function (not just the part you're staring at) is what confirms
you're chasing a real, localized-but-not-yet-found register, not a
misunderstanding.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__RefreshSlotView`. **Tier B**: Pings every element of the current slot's item list, rebuilds the position buffer (`SlotPos`, the same shape TaskCore__CommitElementScroll builds from the SAME `SlotEntry` record -- cross-confirms that struct), then either shows the list through `self->listView` with a size descriptor or hides it, re-walks the list installing the position on each element, and finally highlights the element at the target's own stashed cursor. The clearest single function establishing `listView`'s role (show/position/hide the item list).

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__RefreshSlotView (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

listView is a BoxFill (include/BoxFill.h); the deleted `Unk68Obj` view's slots are BoxFill's: slot4C = attachToParent (cast to BoxFillAttachToParentFn, arguments cast), slotC0 = setSize({0x28, count * 12}), slot50 = detachFromParent. Zero bytes.

## Track 7 (2026-09-27, round 98, bravo)

The layout literals are unit-local constants: SLOT_LIST_ROW_PITCH 10 (item rows' spacing, also CommitElementScroll's), SLOT_LIST_FRAME_WIDTH 40 and SLOT_LIST_FRAME_ROW_HEIGHT 12 (listView's setSize: 40 by count * 12, BoxFill.h's banner reads the same). `pos` is now `entry->pos` (SlotEntry::pos, see CommitElementScroll's report). Locals: a1 -> parent, a2 -> show, idx -> slot, arr -> item, counter -> cursor, buf -> size, target -> entry.
