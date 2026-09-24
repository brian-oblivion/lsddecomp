# Class65650__ApplyTodFrame -- MATCHED (33/33, round 75): lever = LOOP KIND (`for` with the increment at the body top) + `count` widened to `s32`

> Renamed from `func_800662BC` on 2026-09-24 (tools/rename.py). Address 0x800662bc.

REVISITED, round 75: MATCHED 33/33, whole-image `OK: build matches retail`; names/types used (`count` retyped `u16` -> `s32`; the incoming `hdr` parameter reused as the accumulator).

## Round 75 (charlie): matched

**Preserved body rebuilt first** (the 17/33 guard + `do/while` + `cont`
body with `u8 unused[8]`, which is also the `#ifdef NON_MATCHING` body that
stood in `src/`): 17/33, `insertions 2 / deletions 2`, positional skeleton
diffs 15 -- reproduces the recorded figure.

**Reading retail, not the old body.** The loop tail has the shape
`addiu $s0,$s0,1` BEFORE the loop label, the same `addiu` in the back
branch's delay slot, and an `addiu $s0,$s0,-1` after the loop. That is
reorg stealing the loop's FIRST instruction (the counter increment) from
the branch target into the delay slot and emitting the compensating
decrement on the fall-through path -- so the source loop body BEGINS with
`i++`, and the test compares the already-incremented `i`. That is the same
shape that closed `Class65650__SetDisplay` earlier this round.

| build | variant | result |
| --- | --- | --- |
| 1 | preserved body | 17/33, ins/del 2/2, skeleton 15 |
| 2 | `for (i = 0; i < count;) { i++; hdr = call(...); }`, `u16 count`, no filler, `hdr` reused | ins/del 2/2, 2 words LONG: loop exact, but `lhu $v0` + `andi $v0,0xffff` + `move $s2,$v0` instead of `lhu $s2` |
| 3 | build 2 with `s32 count` | **33/33, OK: build matches retail** |

```c
void *Class65650__ApplyTodFrame(Class65650 *self, void *hdr, void *extra)
{
    s32 count;
    u32 i;

    count = *(u16 *)((u8 *)hdr + 2);
    hdr = (u8 *)hdr + 8;
    for (i = 0; i < count;) {
        i++;
        hdr = self->methods->slot138(self, hdr, extra);
    }
    return hdr;
}
```

The recorded residue ("count's load routes through `$v0`", "`move $s3,$a1`
deferred past the `blez`") was two symptoms of (a) a `u16` local, whose
value 2.6.3 re-narrows with an `andi` and copies into its home register,
and (b) the wrong loop kind; the `u8 unused[8]` frame filler was
compensating for the latter, exactly as on `Class65650__SetDisplay`. The earlier
permuter campaigns (50,313 iterations in round 31 among others) searched
the do-while body and never saw this shape.

### Proposed learning

**`addiu sN,sN,1` just before a loop label, the same `addiu` in the back
branch's delay slot, and `addiu sN,sN,-1` right after the loop = the loop
body begins with `i++`.** reorg steals the first instruction of the target
into the delay slot and compensates on the fall-through. Write
`for (i = 0; i < n;) { i++; ... }`. And a narrow (`u16`/`u8`) counter
bound local costs an `andi` plus a `move` against retail's direct `lhu` into
the saved register: widen the local, keep the narrow load.

## History before round 75 (superseded title: "Class65650__ApplyTodFrame -- STALL: length EXACT (33/33 words, no drift); 17/33 raw word-match; first real diff at file 0x056ADC / vram 0x800662DC")

**Unit:** code_55dd4 · **Size:** 33 words (0x84 bytes) · **Status:** STALL —
**LENGTH exact (33/33 words, no drift); RAW WORD-MATCH 17/33; FIRST REAL DIFF
at file 0x056ADC / vram 0x800662DC** (`count`'s `lhu` load routes through
`$v0` instead of directly into `$s2`). Whole-image red. Restored to
`INCLUDE_ASM`.

## What it does (control flow and types are solid)

`Class65650Methods` slot `+0x134`. A fold/reduce: reads a `u16` count from
offset `+2` of a small header (`hdr`), then calls the class's own vtable
slot `+0x138` (`Class65650__ApplyTodPacket`, out of scope this round — 258 words,
flagged by the head for a dedicated assignment) once per item, threading
the accumulator (`hdr + 8` initially) through each call:

```c
void *Class65650__ApplyTodFrame(Class65650 *self, void *hdr, void *extra)
{
    void *acc;
    u16 count;
    u32 i;

    count = *(u16 *)((u8 *)hdr + 2);
    acc = (u8 *)hdr + 8;
    if (count != 0) {
        s32 cont;

        i = 1;
        do {
            acc = self->methods->slot138(self, acc, extra);
            cont = i < count;
            i++;
        } while (cont);
    }
    return acc;
}
```

Two structural findings here are solid — every attempt below reproduced
the loop's tail (words 24-32, epilogue and branch, byte-identical every
time) and the unsigned comparison, so these are not in question:

1. **The loop counter starts at 1, not 0**, and there is a redundant
   `i--` after the loop in retail that is never used again — the exact
   "increment unconditionally, back up at the merge point" idiom from the
   head's branch-target broadcast, just applied to a plain counter rather
   than a scan pointer. Modeled here with `i = 1;` before the loop and a
   `cont = i < count; i++;` inside it (equivalent to the `while (i++ <
   count)` shape, but written this way because it also fixed an unrelated
   residue — see below).
2. **The comparison must be unsigned.** `i` has to be `u32`, not `s32` —
   with a signed `i`, `i < count` compiles to `slt`, but retail's actual
   instruction is `sltu` (confirmed by decoding the raw word:
   `2b101202`, function code `0x2B` = `SLTU`). `count` alone being `u16`
   is not enough; C's usual arithmetic conversions promote a `u16` to
   plain (signed) `int`, so the *other* operand has to already be unsigned
   for the comparison itself to come out unsigned.

## The residue: `count`'s load routes through `$v0` instead of loading directly into its long-lived register

Retail's `lhu` loads the count field **directly into `$s2`** (decoded from
the raw word: `94b20002` = `lhu $s2, 2($a1)`), and independently, its
load-delay slot is filled with the pointer-advance
(`addiu $a1, $a1, 8`) — a genuinely independent instruction placed right
after a load, exploiting MIPS1's one-cycle load-delay slot. Every attempt
here instead loads into `$v0` first (`lhu $v0, 2($a1)`) and adds a
separate `move $s2, $v0` later, and leaves the load-delay slot as a bare
`nop`, moving the pointer-advance instruction later instead. Same overall
instruction *count* in the end (confirmed: words 24-32 always matched,
so nothing shifted past this section), but the specific sequence and
register routing in words 8-23 never matched.

| attempt | result |
| --- | --- |
| `for (i = 0; i < count; i++)`, no frame filler | address drift, 2/33-5/33 |
| add `u8 unused[8]` to fix the 8-byte frame (confirmed via `cc1`'s own `.frame` output matching retail's `$sp,0x30`) | still drifted — the `count`/`i` comparison needed an extra `move` before `slt` |
| `while (i++ < count)` (post-increment in the condition) instead of a separate `cont` local | one extra `move` per iteration for the pre-increment temporary — 8/33, still drifted |
| separate `s32 cont; cont = i < count; i++;` instead of `i++ < count` | removed the per-iteration `move`, loop body words 24-32 became exact, but the guard section (words 8-23) still differs |
| `i` as `s32` vs `u32` | `s32` gives `slt` (wrong); `u32` gives `sltu` (matches) — this was the fix for word 20 specifically |
| test the raw field expression directly (`if (*(u16*)(hdr+2) != 0)`) before assigning `count`, mirroring `Class65650__FindPartIndex`'s fix | worse — introduces a second, genuinely duplicate `lhu` (retail has only one) |
| `__asm__("")` in four different positions (before both assignments, between them, right after entering the `if`, before `i = 1`) | no improvement in any position — unlike `Class65650__FindPartIndex` and `Class65650__SetDisplay`, no barrier placement here moved this particular residue at all |
| reordering `count =` vs `acc =` (both orders tried) | no change either way |

Fourteen real build attempts, and the loop's tail (words 24-32) matched
byte-for-byte from partway through this process onward, which is why this
is filed as a stall rather than continuing to grind — the remaining
residue is concentrated in exactly the "direct-load-into-long-lived-register
vs. route-through-`$v0`" pattern that closed with a barrier in
`Class65650__FindPartIndex` but did not respond to the same lever here.

## Preserved body (best attempt, 17/33)

```c
#if 0
void *Class65650__ApplyTodFrame(Class65650 *self, void *hdr, void *extra)
{
    void *acc;
    u16 count;
    u32 i;
    u8 unused[8];

    count = *(u16 *)((u8 *)hdr + 2);
    acc = (u8 *)hdr + 8;
    if (count != 0) {
        s32 cont;

        i = 1;
        do {
            acc = self->methods->slot138(self, acc, extra);
            cont = i < count;
            i++;
        } while (cont);
    }
    return acc;
}
#endif
```

### Proposed learning

A `__asm__("")` scheduling barrier is not a universal fix for
"load routes through `$v0` instead of the long-lived register" — it closed
this exact shape in `Class65650__FindPartIndex`, but four different placements failed
to move it here. The difference may be that `Class65650__FindPartIndex`'s case had no
other live values competing for scheduling priority at that point, while
this function's guard sits right before a loop with its own register
pressure (accumulator, counter, count, `self`, `extra` all live
simultaneously). Worth escalating as a candidate toolchain/scheduler
question rather than assuming every instance of this pattern is
barrier-fixable.

## Round 19 (echo): one new axis tried (worse), re-verified claim

Re-verified 17/33 first (matches exactly). Read the disassembly closely
and noticed something the round-14 analysis hadn't stated explicitly:
retail's accumulator threading REUSES the `hdr` argument register (`$a1`)
directly as the accumulator for the whole function -- `hdr`'s only two
uses (`count = *(hdr+2)`, `acc-init = hdr+8`) both happen before the
value is ever needed again, so retail never materializes a SEPARATE
`acc` value at all; `$a1` just gets reassigned in place across the loop.
This suggested eliminating the separate `acc` local entirely and instead
reassigning `hdr` itself:

```c
void *Class65650__ApplyTodFrame(Class65650 *self, void *hdr, void *extra)
{
    u16 count;
    u32 i;
    u8 unused[8];

    count = *(u16 *)((u8 *)hdr + 2);
    hdr = (u8 *)hdr + 8;
    if (count != 0) {
        s32 cont;
        i = 1;
        do {
            hdr = self->methods->slot138(self, hdr, extra);
            cont = i < count;
            i++;
        } while (cont);
    }
    return hdr;
}
```

Result: **8/33, WORSE, WITH drift** (97701 bytes outside range) -- both
with and without the `unused[8]` frame filler (2/33 without it). Reverted
immediately; confirmed the reversion rebuilds green.

This is a genuine negative: despite retail's own register allocation
literally reusing `$a1` as both `hdr` and the accumulator (confirmed by
reading the disassembly, not guessed), writing the SOURCE to match that
reuse pattern produced WORSE code, not better -- GCC 2.6.3 apparently
allocates differently when the C source itself collapses two
conceptually-distinct roles (an input pointer being read, then an
accumulator being threaded) into one reassigned variable, versus keeping
them as two named locals where one happens to die before the other
starts. Filing unchanged as STALL at 17/33, `INCLUDE_ASM` restored.

### Proposed learning

**A register-reuse pattern confirmed from the disassembly (retail reuses
one hardware register for two conceptually-different source-level
roles) does not mean the SOURCE should merge those roles into one C
variable.** This is the same class of finding as this round's
`ClipSegmentToBox` result (a confirmed register-lifetime fact that, when
turned into a matching source restructure, made things worse) --
together they suggest that "make the source's variable structure mirror
retail's confirmed register reuse" is not a reliable lever in this
corpus's residue class, even though it is an intuitive and cheap thing to
try. Two independent instances now discourage reaching for it again
without a stronger reason.

## Round 20 (alpha): screened against the outgoing-arg dead-code lever -- confirmed inapplicable, via isolated cc1 (same finding as the unit's sibling)

Same numeric signature as `Class65650__SetDisplay` (frame `0x30`, outgoing area
`0x18`, zero live `jal` argument setup below it) -- one of the two
"0-`jal`" rows in round 19's live census. Tested the round-20 lever in
isolation before spending a real build attempt: took the preserved
17/33 body (`u8 unused[8]` frame filler) and built a variant replacing
that local with a dead 6-argument call (`if (0) { func_ZZZZ(self, hdr,
extra, 0, 0, 0); }`, unprototyped extern, matching `ApplyMatrixToLVArray`'s
construct), through `cpp | cc1` directly.

**Result: byte-identical instruction stream** (the only diff is `cc1`'s
own `.frame` comment, `vars=8/args=16` vs `vars=0/args=24`, plus internal
label numbers that don't reach machine code). This function's residue
(count loaded into `$v0` then moved, instead of loaded directly into its
long-lived register) is completely unaffected by which source construct
supplies the frame's extra 8 bytes -- consistent with the identical
finding on `Class65650__SetDisplay` (see that function's own report for the full
writeup; the two probes were run side by side this round). Did not cost
a real build attempt against this function's own 30-attempt budget.

Remains a STALL at 17/33, `INCLUDE_ASM` untouched in `src/` this round.

### Proposed learning

See `Class65650__SetDisplay.md`'s round-20 entry -- the same negative, independently
confirmed on this unit's second "0-jal, 0x18-outgoing" function: the
dead-code frame-sizing lever and a register ROUTING/ORDER residue are
decoupled GCC decisions, even under an identical numeric signature.

## Round 24 (echo): title corrected, two source axes (negative), first PERMUTER pass on this function (not closed)

Re-verified first: dropped the preserved 17/33 body in verbatim, rebuilt.
`funcdiff.py` reproduces **17/33** exactly, no drift. `tools/asm-differ/diff.py`
confirms the first real diff (realigned) is at file offset **0x056ADC** /
vram **0x800662DC** -- retail's `lhu $s2,2($a1)` (count loaded directly into
its long-lived register) versus ours `lhu $v0,2($a1)` plus a later
`move $s2,v0`. Corrected the title to the three-figure format.

**Axis 1: widen `count` from `u16` to `u32`** (round 23's "a local's declared
WIDTH is a codegen decision" lever, tried here since it had not been tried on
this specific local before -- prior width axes only covered `i`'s
signedness). Kept the field access as `*(u16 *)(...)` but declared the local
`u32`. **Result: WORSE, with an 8-figure drift (97677 bytes outside range)**
-- widening this particular local changes the function's compiled SIZE, not
just its register routing. Reverted immediately.

**Axis 2: early-return restructuring.** Rewrote
`if (count != 0) { ... loop ... } return acc;` as
`if (count == 0) { return acc; } ... loop ...; return acc;` (two return
sites for the same variable, testing whether an early exit changes delay-slot
scheduling around the guard). **Result: 17/33, BYTE-IDENTICAL residue**
(confirmed via `asm-differ`, not just the word count) -- this restructuring
is invisible to the compiler. Reverted.

**This function had never had a permuter pass, unlike its four siblings in
this unit's queue** (checked: no "Round 18 (permuter pass...)" section exists
above, and this unit's other four stalls all got one that round). Provisioned
one this round: `tools/setup-permuter.sh Class65650__ApplyTodFrame` against the preserved
17/33 body, confirmed the scaffold's `--debug --stack-diffs` base score (625)
matches this report's documented residue before any search (register
differences plus insertions/deletions consistent with the "routes through
`$v0`" description). Ran `-j 6 --stack-diffs --stop-on-zero --best-only` for
280 seconds under load: **41,561 iterations, best score improved from 625 to
280** (`permuter exit=124`, the bound firing, not a kill).

**The 280 candidate is a LEAD that fails the "judge what it returns" check
and was rejected without a real build attempt.** It reorders the loop body
to `i++; acc = self->methods->slot138(self, acc, extra); cont = i < count;`
(incrementing `i` BEFORE the call instead of after computing `cont`). Tracing
it by hand: with `i` starting at 1 and `count == 3`, the ORIGINAL body calls
`slot138` three times (cont computed from the PRE-increment `i`, matching the
already-confirmed "total calls == count" idiom); this candidate's reordering
calls it only TWICE for the same `count` (cont computed from the
POST-increment `i`) -- a genuine off-by-one behavioral change, not a
scheduling-only rewrite. Per CLAUDE.md's "judge what it returns" permuter
discipline, this was rejected on inspection rather than spent as a real
`src/` attempt; translating it would silently change how many times the fold
callback runs.

No zero reached in 41,561 iterations under contention. Per the project's
standing instruction this is **not marked permuter-exhausted** -- a single
280s run at load is a small fraction of an idle-box budget, and the one
below-base candidate found is disqualified on inspection rather than ruled
out by exhaustive search.

Remains a STALL at 17/33, `INCLUDE_ASM` restored, `src/code_55dd4.c`
confirmed clean.

### Proposed learning

**A permuter score improvement is not evidence of a usable candidate until
its behavior is traced by hand for at least one representative input,** not
just checked for compile errors. This candidate had 0 compile errors and a
real score improvement (625 -> 280) but changes the number of times a
per-element callback fires for a given `count` -- an easy thing to miss by
only reading the diff for "does this look like valid C", since both the
original and the mutated form ARE valid, differently-behaving C. Trace the
loop trip count by hand before accepting any permuter candidate that touches
a loop's increment/test ordering.

## Round 25 (delta): block-order check (negative), permuter scaffold re-provisioned

Per the head's round-25 block-order broadcast: grepped for a bare
unconditional `j` (not `beq`/`bne`/`bgez`) whose target is a join with real
work in its delay slot.

```
grep -nE '\*/\s+j\s' asm/nonmatchings/code_55dd4/Class65650__ApplyTodFrame.s
```

**No bare `j` mnemonic anywhere in this function's disassembly.** The only
control transfer is the loop's own `bnez`/`beqz`-family conditional
branches guarding the `do`/`while`. There is no candidate site for a
`CheckDreamAuxTriggerCondition`-style basic-block-layout residue — confirming (from the
opposite direction) round 24's finding that this function's residue is a
register-ROUTING question (`count` loaded into `$v0` then moved, instead
of loaded directly into `$s2`), which is a scheduling/allocation decision
with no corresponding C-level block to reorder.

Re-read echo's round-24 permuter run before repeating it: `-j 6
--stack-diffs --stop-on-zero --best-only`, 280s under load, 41,561
iterations, best score improved 625 -> 280 but the winning candidate
reordered `i++` to before the `slot138` call, which changes the loop's
trip count for a given `count` (traced by hand and rejected — see that
round's entry above). No zero was reached.

Re-provisioned the scaffold fresh this round (round-24's `permuter-work/`
is gitignored and does not survive between rounds/worktrees) against the
same preserved 17/33 body, inlined here as literal source with its
declarations so it travels with this report rather than living only in a
gitignored directory:

```c
#include "common.h"
#include "code_55dd4.h"

void *Class65650__ApplyTodFrame(Class65650 *self, void *hdr, void *extra)
{
    void *acc;
    u16 count;
    u32 i;
    u8 unused[8];

    count = *(u16 *)((u8 *)hdr + 2);
    acc = (u8 *)hdr + 8;
    if (count != 0) {
        s32 cont;

        i = 1;
        do {
            acc = self->methods->slot138(self, acc, extra);
            cont = i < count;
            i++;
        } while (cont);
    }
    return acc;
}
```

```
tools/setup-permuter.sh Class65650__ApplyTodFrame <this seed>.c
PATH=$PWD/permuter-work/bin:$PATH .venv/bin/python3 \
  tools/decomp-permuter/permuter.py --debug --stack-diffs permuter-work/Class65650__ApplyTodFrame
```

Scaffold's `--debug --stack-diffs` base score reproduces **625** exactly,
matching round 24's documented figure — confirms the scaffold is scoring
the same residue this report describes before spending any search time.
(Search launched separately once the unit's higher-priority `Class65650__SetDisplay`
search completed, per the one-search-at-a-time discipline; results appended
below.)

**Round 25's queued search was never run or its results never appended** — the
round ended with this line still saying "results appended below" and nothing
followed. Round 31 (alpha) picked this up fresh rather than trusting an
implied result that was never recorded.

## Round 31 (alpha): re-verified (no drift), three isolated-`cc1` probes (all negative), fresh permuter scaffold provisioned

Re-verified first, dropping the preserved 17/33 body in as LIVE code (not
`#if 0`) and rebuilding: `funcdiff.py` reproduces exactly **17/33**, range
`0x56ABC-0x56B40` (0x84 bytes = 33 words, no drift). `INCLUDE_ASM` restored
immediately after, `git diff --stat` confirmed clean.

**Re-ran the blocker screens** (both are current as of round 21's `addiu_at`
resolution): no `gp_rel` hit, no `mflo`/`mfhi` followed within two instructions
by `mult`/`div`. Blocker-clean, consistent with the head's round-31 assignment
note.

**Three new axes probed via the isolated `cc1` pipeline** (CLAUDE.md's
"escalate, do not experiment" reproducer recipe — no `src/` edit, no real
build attempt spent, cheap enough to try several):

1. `count = ((u16 *)hdr)[1];` instead of `*(u16 *)((u8 *)hdr + 2)` — same
   value, array-index spelling instead of cast-and-offset. **Byte-identical**
   to the existing residue (`lhu $v0,2($a1)` then a later `move $s2,$v0`).
2. `volatile u16 count;` — forces a full store-to-stack/reload-from-stack
   sequence (`sh`/`lhu` through a stack slot) instead of the direct-register
   routing question this residue is actually about. **Much worse and a
   structurally different shape** — not a viable lever, this qualifier
   answers a different question (cross-branch reload) than the one this
   residue poses (initial-load destination register).
3. `u8 *base = hdr; count = *(u16 *)(base + 2); acc = base + 8;` — alias
   `hdr` to a fresh local before reading through it, keeping `acc` a
   genuinely separate value (unlike round 19's negative, which collapsed
   `acc` and `hdr` into ONE variable). **Byte-identical** to the existing
   residue — the extra alias is invisible to the compiler.

None of the three isolated probes cost a real `src/` build attempt (per round
20's established convention); the manual-attempt count for this function
remains 14. **This residue continues to resist every source-shape axis
tried across three rounds** (14 manual attempts + these 3 cheap probes + the
41,561-iteration round-24 permuter run) — consistent with the report's
standing classification as a scheduler-internal "which register does the
first load target" decision.

Re-provisioned the permuter scaffold fresh (`permuter-work/` does not survive
between rounds/worktrees) against the same preserved 17/33 body. `--debug
--stack-diffs` reproduces base score **625** exactly, confirming the scaffold
targets the same residue before any search time is spent. Per the "one search
at a time" discipline, this function's search is queued to run after
`Class65650__SetLightMode`'s (already in flight when this entry was written — see that
function's own report for the live run). Results will be appended once that
search completes and this one runs.

### Round 31 permuter run: result

Ran after `Class65650__SetLightMode`'s campaign completed:

```
nohup timeout 600 env PATH=$PWD/permuter-work/bin:$PATH .venv/bin/python3 \
  tools/decomp-permuter/permuter.py -j 6 --stack-diffs --stop-on-zero \
  --best-only permuter-work/Class65650__ApplyTodFrame > /tmp/alpha_permuter_800662BC.log 2>&1 &
```

**50,313 iterations.** Exit inferred the same way as this round's other two
searches (nohup detached, no direct `$?`; wall-clock ~600-670s, log tail
carries the same graceful `resource_tracker` semaphore-cleanup warning): bound
firing on its own, **exit 124**, not an external kill.

Two below-base scores found, both inspected by hand before spending (or, in
one case, after spending) a real build attempt — per this project's
"judge what it returns" permuter discipline:

- **280 (best).** `void *new_var = hdr; ... i++; cont = i < count;` — this is
  **the exact same candidate round 24's search already found and rejected**
  (just with an inert `hdr`-to-`new_var` rename added on top): reordering
  `i++` before computing `cont` from the POST-increment value changes the
  loop's trip count for a given `count` (traced by hand again to confirm:
  `count=3` gives 3 calls in the original, 2 in this candidate). Not spent as
  a real attempt — this is round 24's own documented rejection, re-surfacing
  under a fresh random seed with cosmetic-only variation.
- **515.** `cont = count; cont = i < cont;` (splitting the single comparison
  statement into two, otherwise identical) — this one IS behavior-preserving,
  unlike the 280 candidate, so it was translated to `src/` and checked against
  the real oracle rather than dismissed on inspection alone. **Result: 19/33,
  WITH size drift (97,475 bytes outside range)** — worse in real terms than
  the trusted 17/33 no-drift baseline, despite scoring better under
  `--stack-diffs`. This is the same "permuter score improves, real length
  changes" trap `Class65650__SetLightMode`'s round-18 entry already documented for a
  DIFFERENT function in this unit — now confirmed a third time (once per
  function that owns a permuter history: `Class65650__SetLightMode`, and now this one).
  Reverted immediately; confirmed the reversion rebuilds `build exit=0`,
  whole-image green.

No score below 280 was found across the rest of the run, and no zero was
reached. **Combined with round 24's 41,561 iterations, this is now 91,874
iterations across two independent campaigns**, both converging on the
identical 280-score candidate (an off-by-one behavioral change) as the best
non-regressing lead, and both finding it inspection-disqualified. Per the
project's standing instruction this is **NOT marked permuter-exhausted** — the
machine ran under multi-runner contention throughout (up to 38
permuter-related processes visible in `ps aux` at points this round).
**Correct framing: not closed in 91,874 iterations under load across two
campaigns; the only candidates below base score are either a confirmed
behavioral change (280) or a confirmed real-oracle regression (515), not
untested leads.**

Remains a STALL at 17/33, `INCLUDE_ASM` restored, `src/code_55dd4.c`
confirmed clean (`git status --porcelain` empty for this function after the
515 real-build check was reverted).

### Proposed learning

**A "found new best score" permuter result needs two different checks
depending on whether it changes observable behavior, and conflating them
wastes either a trace or a build.** A candidate that changes control flow or
loop trip count (like this function's 280) is disqualified by HAND-TRACING
it for one representative input — spending a real build attempt on it proves
nothing new. A candidate that is behavior-PRESERVING (like this function's
515, a pure statement split) cannot be judged by inspection at all — only the
real oracle (`build-and-verify.sh` + `funcdiff.py`) can show that
`--stack-diffs`' linear penalty model missed a length/drift regression a
human reader has no way to see in the diff. Skipping the trace on a
control-flow-changing candidate wastes a build; skipping the build on a
behavior-preserving one risks reporting a false lead.


---

## HEAD NOTE, round 31 (2026-09-11) -- orphaned workers, negative UNAFFECTED

At 21:29:40 the head `kill -9`-ed every permuter process cwd-ed into this
runner's worktree (9 of them), during a sweep run on the mistaken belief that
this runner had died. **This did not truncate the campaign described above.**

This runner's searches were bounded at `timeout 600`; its last permuter output
artifact is timestamped 21:20, so the campaigns had already completed on their
own bounds roughly ten minutes before the kill. The 9 surviving processes were
**orphaned workers** -- the round-26 phenomenon recorded in PARALLEL-RUNS.md
2c, where `permuter.py -j N` runs a multiprocessing forkserver and killing or
exiting the root leaves workers reparented to init, still holding cores.

So this is a clean independent confirmation of that documented behaviour:
workers outlived a *normally completed* run, not just a killed one. The
iteration counts and exit codes recorded above stand.

## Round 33 (alpha): re-verified (no drift), round-32 levers screened — all inapplicable or already tried

Re-verified first: dropped the preserved 17/33 body in live, rebuilt.
`funcdiff.py` reproduces exactly **17/33**, same residue (`count`'s `lhu`
routing through `$v0` instead of directly into `$s2`) at `0x056ADC`/vram
`0x800662DC`. `INCLUDE_ASM` restored, `git diff --stat` clean.

**Round-32 lever 1 (volatile narrower than barrier):** round 31 already
tried `volatile u16 count;` directly and found it forces a full
store-to-stack/reload sequence — a structurally different and larger
regression, answering a different question (cross-branch reload) than
this residue (initial-load destination register). Not re-run; re-deriving
an already-negative result is not a new experiment. Screened instead for
an UNTRIED volatile placement: qualifying the `hdr` parameter's local
alias would require re-introducing round 19's already-rejected
role-merge (`hdr`/`acc` collapsed to one variable, 8/33 with drift) or
round 31's already-rejected `base` alias (byte-identical, no effect) —
both dead ends independent of volatile. No untried volatile placement
exists for this residue that isn't a re-run of an existing negative.

**Levers 2-5 screened:** (2) register-identity skepticism does not
apply — this residue was never classified as identity; it is a load-
destination ROUTING question (retail loads directly into `$s2`; every
attempt here loads into `$v0` then `move`s), confirmed structurally
different from an identity swap since the loop body (words 24-32)
matches byte-for-byte in every attempt. (3) not applicable this round.
(4) no new permuter run — round 31's own second campaign (50,313
iterations) already found and traced by hand the only two below-base
candidates (one behavior-changing, rejected on inspection; one real but
worse-with-drift when built), so a third un-hinted run is not queued.
(5) the residue is confirmed via raw operand bytes (`94b20002` = `lhu
$s2,2($a1)` vs `lhu $v0,...` + separate `move`), not a text-blind
addiu/ori-style artifact.

No new build attempt spent against `src/` this round — every lever
either does not apply to this residue's mechanism or has already been
tried and negatively confirmed in a prior round. This is now 14 manual
attempts + 6 isolated-`cc1` probes (rounds 20/31) + 2 permuter campaigns
(41,561 + 50,313 iterations) without closing the load-routing residue.
Remains a STALL at 17/33, `INCLUDE_ASM` restored, `src/code_55dd4.c`
confirmed clean before and after.

### Proposed learning

**Not every round-32 lever needs a fresh build to screen — for a residue
this well-characterized, the correct action can be recognizing that the
lever's only sensible application was already tried and rejected under a
different name in an earlier round** (`volatile u16 count`, round 31).
Screening honestly sometimes means concluding "already covered" rather
than manufacturing a new variant to report a fresh negative; padding the
attempt count with cosmetic restatements of an existing result would
understate, not strengthen, how thoroughly this residue has been checked.

## Round 49 (alpha): re-verified (no drift); third permuter campaign confirms the same two candidates, no improvement

Per the head's round-49 opening (this unit stale since round 33). Re-
verified first: dropped the preserved 17/33 body in live, rebuilt.
`funcdiff.py` reproduces exactly **17/33**, same load-routing residue at
`0x056ADC`/vram `0x800662DC`. `INCLUDE_ASM` restored, `git diff --stat`
confirmed clean.

Re-provisioned the permuter scaffold fresh (`permuter-work/` does not
survive between rounds). `--debug --stack-diffs` reproduced base score
**625** exactly, matching rounds 24/25/31's own documented figure —
**CHECK 3: AGREE**.

Launched under low contention (load average 3.85/32 at launch, well below
round 24/31's documented "up to 38 permuter-related processes"):

```
timeout 900 permuter.py -j 6 --stop-on-zero --best-only permuter-work/Class65650__ApplyTodFrame
```

**48,607 iterations, rc=124 (bound fired on its own — confirmed via its
own file, not inferred)**. Two sub-base candidates found, at 280 and 515
— the SAME two shapes rounds 24/31 already found and disqualified, not
new ground:

- **280**: reorders `i++` to happen BEFORE `cont = i < count;` is computed
  (retail/the preserved body compute `cont` from the pre-increment value,
  then increment). Same trip-count-changing bug round 31 traced by hand
  (`count=3` gives 3 calls in the original, 2 in this candidate) —
  disqualified by inspection again, not re-built.
- **515** (two variants, `515-1` rewriting `i < count` as `count > i` and
  `515-2` adding an inert `long long new_var` pass-through plus a
  `(*self->methods).slot138` parenthesization, both scoring identically):
  behaviorally identical to round 31's already-real-oracle-checked `cont =
  count; cont = i < cont;` split, which measured **19/33 WITH size drift
  (97,475 bytes)** when translated. Not re-built — retranslating an
  already oracle-confirmed regression is not a new experiment.

**This is now 140,481 combined iterations across three independent
campaigns** (41,561 + 50,313 + 48,607), all converging on the identical
two candidates and never finding a third. Combined with the 14 manual
attempts and 6 isolated-`cc1` probes from prior rounds, this residue
(a scheduler-internal choice of which register a `lhu` targets) shows the
narrowest, most repeatedly-confirmed search-space convergence of any
function in this unit — three separate random seeds, three different
iteration counts, zero new candidates. Remains a STALL at 17/33,
`INCLUDE_ASM` restored, `src/code_55dd4.c` confirmed clean before and
after.

### Proposed learning

**A third independent permuter campaign that finds only the same two
candidates a decade of prior search already characterized is itself
useful evidence, and is worth recording as such rather than skipped as
redundant.** Unlike `Class65650__SetDisplay`'s sibling case this same round (two
campaigns, 192,610 iterations, literally nothing below base score in
either direction — an EMPTY search space), this function's space has
exactly two paths below base, and three independently-seeded searches at
three different loads have now all found precisely those two and nothing
else. That is a qualitatively different kind of "exhausted" than
`Class65650__SetDisplay`'s: not an empty space, but a space with exactly two known
exits, both already closed. The distinction matters for the next round
deciding whether a fourth campaign is worth queuing — here the marginal
value is close to zero in a way that is actually demonstrated, not
assumed.

## Round 61 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. The preserved 17/33 body above (this report's
only `#if 0` block; hand-derived from the round-14 manual attempts, not a
permuter output -- the two permuter-found sub-base candidates, 280 and 515,
were both traced/built and rejected in rounds 24/31/49 and never touched
`src/`) is placed in `src/code_55dd4.c` as `#ifdef NON_MATCHING ... #else
INCLUDE_ASM ... #endif`. No new declarations needed; `slot138`'s signature
(`void *(*)(Class65650 *, void *, void *)`) already matches in
`include/code_55dd4.h`. `./build-and-verify.sh` stays green (no bytes
changed) and `tools/check-nonmatching.sh` compiles it clean.

NON_MATCHING body promoted, round 61.
