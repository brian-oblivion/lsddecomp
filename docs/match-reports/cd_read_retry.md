# cd_read_retry -- STALL (length EXACT 223/223, 215/223 words match, first real diff at vram 0x8002AABC)

> Renamed from `func_8002AA6C` on 2026-09-24 (tools/rename.py). Address 0x8002aa6c.

Unit `libcd_bios`. Runner delta, round 17. 223 instructions.

## Read this label first

**This is a mid-attempt snapshot salvaged by the head, not a considered
plateau.** Runner delta died to an infrastructure failure (weekly API limit)
with this body uncommitted in its worktree; its last words were "now let's
write the full function", so this is a body that had not yet been through a
single measure-and-reshape cycle. No author applied a stop rule to it.
Salvaged under `docs/PARALLEL-RUNS.md` §4c.

## Score: NONE, and that is the finding

The head copied delta's worktree file into `main` and ran the full oracle:

```
delta snapshot build exit=2
WARNING: the build differs OUTSIDE this range too (296081 bytes) -- a size
         change may have shifted linked addresses, so this per-function read
         is NOT trustworthy.
```

`funcdiff.py` refused to hand back a number, correctly: the body's compiled
LENGTH differs from retail's, so everything after it shifted and the
per-function window no longer means what it says. This is CLAUDE.md's third
way a score lies, caught by the guard that exists for it.

**So do not record a number for this function anywhere.** A body at 58/196 is
worth resuming and one at 1/276 is not, and right now nobody knows which this
is. The first thing the next attempt should do is get the LENGTH right — until
then no per-function score from this shape means anything.

Delta's own committed stall report for the sibling `CD_init` (58/196,
780/784 bytes) is the useful comparison: same unit, same session, and there
the length was nearly right, which is what made its score readable.

## Body, as salvaged

Uncompiled and unmeasured beyond the above. Everything it references was
declared in `src/libcd_bios.c` at the time, including the `extern volatile`
hardware-register block (`D_8006D8C0` and neighbours) that delta established
for this unit.

```c
/* stalesyms --fix 2026-09-22: func_80012C20 -> printf, func_80025900 -> VSync, func_80025AE4 -> puts -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
s32 cd_read_retry(void)
{
    s32 n;
    s32 *tmp;
    s32 *pRetry;
    volatile s32 *p2;
    s32 saved;
    s32 counter;
    volatile u8 *q;
    u8 buf;

    tmp = &D_8006D8DC;
    n = *tmp;
    D_8006D600 = 0;
    D_8006D5FC = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            puts(D_80010A40);
                        }
                        counter++;
                        CD_cw(1, 0, 0, 0);
                    }

                    while (CD_cw(0x16, D_8006D908, 0, 0)) {
                        CD_cw(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, (s32)&D_8006D618, 0, 0) != 0) {
                    goto tail;
                }
            }

            *D_8006D8C0 = 1;
            while (*D_8006D8CC & 7) {
                *D_8006D8C0 = 1;
                *D_8006D8CC = 7;
                *D_8006D8C8 = 7;
            }

            D_8006D8DA = 0;
            q = &D_8006D8D9;
            D_8006D61C = 0;
            *q = D_8006D8DA;
            D_8006D8D8[0] = 2;
            *D_8006D8C0 = 0;
            *D_8006D8CC = 0;
            *D_8006D8D0 = 0x1325;

            buf = (u8)p2[0];
            if (buf != D_8006D61C) {
                if (CD_cw(0xE, (s32)&buf, 0, 0) != 0) {
                    goto tail;
                }
            }

            D_8006D600 = (s32)cb_read;
            p2[-1] = p2[-2];
            CD_cw(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = VSync(-1) + 0x1E0;
            return p2[2];

        tail:
            n = *pRetry;
            *pRetry = n - 1;
        } while (n > 0);
    }

    D_8006D8F4 = -1;
    return D_8006D8F4;
}```

Positioned between `CD_init` and `CD_readm` in ROM order.

## What is known independently of this body

- The function screened **clean** on all three blocker greps at carve time, so
  nothing here is toolchain-blocked.
- Delta's own levers for this unit, from its four committed reports, and both
  are worth trying before anything else: `for(;;)` is sometimes required (not
  merely idiomatic) for GCC 2.6.3 to recognise loop-invariant address
  hoisting; and the local `volatile T *` lever **backfires inside a loop** over
  a loop-invariant target, where it defeats to LICM instead.

## Round 19: got this to a real, scoreable baseline (runner charlie)

Per the head's second-pass brief, and per this report's own instruction
("the first thing the next attempt should do is get the LENGTH right"):
compiled the salvaged body as-is (adding the two missing extern
declarations it needed, `D_80010AAC[]`/`D_80010ABC[]` -- both `u8[]`
string-literal-shaped globals passed to `func_80025AE4`/`func_80012C20`,
no other functions in this unit reference them yet). It compiles clean
and lands at **119/223 words, with the compiled length only 1
instruction short of retail's 223** (`objdump`: 222 real instructions).
This is a genuine, trustworthy near-miss now, not an unscoreable
snapshot -- confirmed via `objdump`, not just `funcdiff`'s own
in-range/out-of-range split (which still warns, correctly, since 1
instruction of drift is still drift, but it's now precisely
characterized rather than unknown).

**One structural fix applied and confirmed real, though it didn't
change the final word count:** the function's tail (`D_8006D8F4 = -1;
return D_8006D8F4;`) needed the same unfolded-address `volatile s32 *`
local-pointer idiom this round's `cb_read` report independently
found for the identical global and an identical write-then-read shape.
Before the fix, this build FOLDED the store's address (`sw
v0,-0x2710(at)`); after routing it through a local pointer, it matches
retail's unfolded `lui/addiu` + plain-offset store exactly. This is now
a THIRD confirmed site (after `CD_readm`'s `D_8006D8EC`/`D_8006D8F0`
and `cb_read`'s own `D_8006D8F4` else-branch) where a global
already declared `volatile` at file scope still needs a LOCAL pointer
dereference to get unfolded addressing at one specific access -- see
`cb_read.md`'s proposed learning, now confirmed a fourth time
across two different functions touching the SAME global.

**What's left, confined to a narrow, well-characterized area:** the
retry-loop's per-iteration decrement (`n = *pRetry; *pRetry = n - 1;`
under the `tail:` label, and the mirrored statement at function entry)
allocates `pRetry`'s address into a fresh register (`$a0`) at the very
last use, where retail keeps it in the SAME persistent register (`$s3`)
it was allocated to earlier in the function. This is the same
parameter/local-persists-in-one-register-vs-gets-reloaded-fresh
register-identity question this round's other reports (`GetRCnt`,
`PlacementGrid__ResolveEntry`) already document as resistant to reshaping -- not
re-attempted here given the round's broader finding that this specific
class rarely yields to source-level levers, and given the function is
now at a solid, well-understood baseline rather than an unknown one.

**Not yet checked**: the `p2[-1] = p2[-2]; ... p2[2] = p2[-3]; p2[3] =
...; return p2[2];` pointer-arithmetic block and the two `CD_cw`
guard calls -- the diff shows these regions ALREADY MATCH retail
byte-for-byte (confirmed via `asm-differ`, no markers in that range),
so the salvaged body's derivation of these fields was already correct;
nothing to re-derive there.

Restored to `INCLUDE_ASM` (no score short of byte-exact stays in
`src/`); the two extern declarations for `D_80010AAC`/`D_80010ABC` are
kept live in `src/libcd_bios.c` since they're needed by any future
attempt and cost nothing to carry forward.

### Proposed learning

**"Unscoreable" is a property of the SESSION that produced a snapshot,
not necessarily of the function.** This body had never been through a
single measure-and-reshape cycle before this round; simply compiling it
as-is (no reshaping at all) immediately produced a real, close,
trustworthy score. Before spending attempts reshaping a body flagged
"unscoreable" or "mid-attempt," check whether it will compile and
measure cleanly with zero changes first -- that alone may resolve the
open question the flag exists to raise.

Also reinforces `cb_read.md`'s finding from this same round:
`D_8006D8F4` specifically (and by extension, this driver's other
`volatile`-qualified scalars) needs the local-pointer-dereference idiom
at EVERY write-then-immediate-read site, not just once per function --
this is now confirmed at two independent sites in two different
functions touching the same global.

## Round 20 (runner bravo): 119/223 -> 202/223, compiled length now EXACT (223/223), two structural fixes

Per the head's brief, worked from round 19's 119/223 baseline (ignore the
superseded "Score: NONE" section above this one -- see round 19's own
entry for why). Read `asm/nonmatchings/libcd_bios/cd_read_retry.s`
directly alongside `objdump -dr build/src/libcd_bios.c.o` and
`tools/asm-differ/diff.py cd_read_retry` to localize the actual
instruction-count gap, rather than trusting the round-19 report's prose
description of it (which turned out to describe the opposite register
than what this round's own build actually produced -- see below).

### Fix 1: `buf`'s stack round-trip (119 -> 149/223)

The round-19 body's `buf = (u8)p2[0]; if (buf != D_8006D61C) { ...
&buf ... }` compiled with a genuine RELOAD from the stack slot for the
comparison (`sb v0,0x18(sp); lbu v1,0x18(sp); lui v0,...; lbu v0,...;
andi v0,0xff` -- 8 instructions), where retail keeps the loaded value in
a register for the comparison and only stores to the stack slot once
(`lw v0,0(s2); ...; sb v0,0x18(sp); andi v0,v0,0xff; beq v0,v1,...` -- 6
instructions). Rewriting to retain the loaded word in an explicit local
closed this exactly:

```c
{
    s32 v0 = p2[0];
    buf = (u8)v0;
    if ((u8)v0 != D_8006D61C) {
        if (CD_cw(0xE, (s32)&buf, 0, 0) != 0) {
            goto tail;
        }
    }
}
```

This ALONE made the compiled length WORSE relative to retail (220 vs
223 -- 3 short instead of 1 short), because it was only ever half of a
two-part, self-cancelling residue (see fix 2). Kept it anyway, since it
is independently correct per CLAUDE.md's guidance to fix incrementally
and not revert a change with independent evidence behind it merely
because the aggregate score doesn't immediately improve.

### Fix 2: missing `__asm__("")` barrier before the reused CD_flush tail block (149 -> 171/223)

The round-19/round-17 salvaged body's copy of the driver-reset tail
(`D_8006D8DA = 0; q = &D_8006D8D9; D_8006D61C = 0; *q = D_8006D8DA;
D_8006D8D8[0] = 2; ...`) was MISSING the `__asm__("");` barrier that the
canonical `CD_flush` version of this exact block carries between
`*q = D_8006D8DA;` and `D_8006D8D8[0] = 2;`. Adding it back (this
report's earlier body simply never had it) fixed a real reordering in
that block. This is the same idiom `cb_read.md` and this report's
own round-19 entry already document for `D_8006D8F4`; it turns out to
also apply to this shared eight-statement tail block, at the SAME
barrier position, every time that block is reused -- worth checking
whenever this block gets copied into a new function.

### Fix 3: the retry-loop's `tail:` label recomputes the address fresh, matching retail, instead of persisting a register (171 -> 202/223, length now EXACT)

This is the one round 19 flagged as "resistant to reshaping" and did not
re-attempt -- but re-read directly from `objdump`, round 19's own
description of WHICH SIDE does what was backwards relative to what this
round's build (and, by construction, the retail bytes) actually show.
**Retail recomputes `&D_8006D8DC` FRESH at the `tail:` label**
(`lui a0,... ; addiu a0,...` immediately before the `lw`), rather than
reusing whatever register held `pRetry`'s address earlier in the
function; the freshly-derived body instead persisted `pRetry` in a
callee-saved register end-to-end, reaching `tail:` via a cheap register
reuse. Making the `tail:` block recompute the address explicitly instead
of dereferencing the persisted `pRetry`:

```c
tail:
    tmp = &D_8006D8DC;
    n = *tmp;
    *tmp = n - 1;
    __asm__("");
} while (n > 0);
```

(the trailing barrier keeps the store out of the branch's delay slot --
without it, GCC folds the `sw` into the `bgtz`'s delay slot, one
instruction shorter than retail, which keeps a plain `nop` there instead)
closed this precisely: compiled length went from 220/222 (short) to
**223/223, exact**, and word-match jumped 149 -> 202. This also confirms
the general shape of `callback.md`'s residue-reading is right even
though its own specific case stayed unresolved: retail sometimes visibly
prefers NOT hoisting/persisting a value across a large span even when it
would be cheaper to, and that preference has to be reproduced by making
the source look the way retail's compiler saw it (recompute at the use
site), not merely by writing idiomatic C and trusting the allocator to
converge on the same answer.

### What's left, confined to one narrow spot: a register-identity choice at the function's very first computed value

At the very top of the function (`tmp = &D_8006D8DC; n = *tmp;`), retail
puts the ADDRESS in `$a0` and the LOADED VALUE in `$v1`; this round's
build puts the ADDRESS in `$v1` and the VALUE in `$a0` -- the two roles
are swapped, not merely renamed. This propagates into one small,
otherwise-already-matching 4-instruction group later (`move s5,a0(a0/v1)
/ li s4,0x1 / li s3,0x7 / addiu s2,s5,0x10`, where retail computes `s2`
LAST and the build computes it right after the `move`) and into the
prologue's own register numbers, accounting for essentially all of the
21 remaining word mismatches. **Tried two reshapes, both regressed
severely rather than helping** (declaration-order swap of `n`/`tmp`: no
change at all, 202/223 unchanged; reordering
`D_8006D600=0;D_8006D5FC=0;` to before the load: catastrophic
regression to 9/223, i.e. this exact statement order is otherwise
load-bearing and fragile). Not spending further attempts on this --
register-identity-driven pseudo-allocation choices for a function's
FIRST temporary are exactly the class CLAUDE.md and this project's
residue guide already document as resistant to source-level reshaping,
and two aggressive attempts just demonstrated how easily further
prodding here makes things categorically worse rather than better.

**Compiled length is now byte-parity with retail (223/223 real
instructions, confirmed via `objdump`)** -- this is a genuinely
well-posed near-miss now, not a length-drifted one. Restored to
`INCLUDE_ASM` (no score short of byte-exact stays in `src/`); full body
preserved below for the next attempt.

```c
#if 0
/* stalesyms --fix 2026-09-22: func_80012C20 -> printf, func_80025900 -> VSync, func_80025AE4 -> puts -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
s32 cd_read_retry(void)
{
    s32 n;
    s32 *tmp;
    s32 *pRetry;
    volatile s32 *p2;
    s32 saved;
    s32 counter;
    volatile u8 *q;
    u8 buf;

    tmp = &D_8006D8DC;
    n = *tmp;
    D_8006D600 = 0;
    D_8006D5FC = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            puts(D_80010A40);
                        }
                        counter++;
                        CD_cw(1, 0, 0, 0);
                    }

                    while (CD_cw(0x16, D_8006D908, 0, 0)) {
                        CD_cw(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, (s32)&D_8006D618, 0, 0) != 0) {
                    goto tail;
                }
            }

            *D_8006D8C0 = 1;
            while (*D_8006D8CC & 7) {
                *D_8006D8C0 = 1;
                *D_8006D8CC = 7;
                *D_8006D8C8 = 7;
            }

            D_8006D8DA = 0;
            q = &D_8006D8D9;
            D_8006D61C = 0;
            *q = D_8006D8DA;
            __asm__("");
            D_8006D8D8[0] = 2;
            *D_8006D8C0 = 0;
            *D_8006D8CC = 0;
            *D_8006D8D0 = 0x1325;

            {
                s32 v0 = p2[0];
                buf = (u8)v0;
                if ((u8)v0 != D_8006D61C) {
                    if (CD_cw(0xE, (s32)&buf, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)cb_read;
            p2[-1] = p2[-2];
            CD_cw(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = VSync(-1) + 0x1E0;
            return p2[2];

        tail:
            tmp = &D_8006D8DC;
            n = *tmp;
            *tmp = n - 1;
            __asm__("");
        } while (n > 0);
    }

    {
        volatile s32 *pF4 = &D_8006D8F4;
        *pF4 = -1;
        return *pF4;
    }
}
#endif
```

### Proposed learning

**Trust your own fresh `objdump`/`asm-differ` read over a prior report's
PROSE description of "which side does what"** when the two disagree --
this round's actual build showed retail recomputing a fresh address at
the far branch target (not persisting a register there), the opposite of
how the round-19 report phrased it. Re-deriving from the instructions
directly (not the paraphrase) is what let fix 3 land; had it been taken
at face value the "resistant to reshaping" framing would have been
trusted instead of tested.

Also: **the "reused tail block needs a barrier at the same position every
time it's copied" idiom (originally found for `D_8006D8F4` in
`cb_read.md`) generalizes to the WHOLE shared 8-statement
`CD_flush`-style tail, not just to single-scalar volatile writes** --
this function's copy was missing it purely because the salvaged snapshot
predated the discovery. Any future function that reuses this block
should carry the barrier from the start.

## Round 25 (runner charlie): the redundant-raw-copy-elision / block-order investigation

Assigned question: **what distinguishes the isolated reduction where a bare
`__asm__("")` barrier fixes the "redundant-raw-copy elision" class (round
24, `code_179d8_l`: `vmNoiseOn2`/`SePitchBend`/`SeAutoVol`) from
the real function, where it transfers to none of them?** Mid-investigation
the head sent two broadcasts proposing a BLOCK-ORDER / tail-merge mechanism
(measured on `CheckDreamAuxTriggerCondition` and `Entity__UpdateActivationState` in other units this same
round): a bare `__asm__("")` only reorders instructions WITHIN a block; if
what actually differs is which BASIC BLOCK a value's materialization lives
in, or which of two candidate arms GCC gives the fallthrough to at a shared
join, a barrier is the wrong instrument and its failure to transfer is
*expected*, not mysterious. Tested that hypothesis directly against this
unit's stalls, rather than adopting it.

### The hypothesis is CONFIRMED for one specific, high-value case in this unit -- found independently before the second broadcast arrived

This function's own round-20 entry (fix 3, above) already IS an instance of
exactly this mechanism, discovered from `objdump` before either broadcast:
retail's success/timeout join is not two early `return`s, it is a
FLAG-and-single-join with the failure arm placed textually FIRST (falling
through into it from both upstream checks) and the success arm placed
SECOND, reached by an explicit branch and left to fall through into the
shared check. This round applied the identical shape to `CD_readsync`
(this pass's own fresh function, `docs/match-reports/CD_readsync.md`):
an early `return -1;` written directly inside the timeout arm compiled
SHORTER than retail (missing the "redundant" `move v0,zero`/`bnez
v0,<epilogue>` pair retail keeps at the join) -- restructuring to
diag-block-FIRST, success-label-SECOND, single shared `result` variable
checked once, took that function from 61/174 to 153/174 in one fix, the
single largest gain of this whole session. **This is a direct, positive
confirmation of the head's mechanism**, and it predates the broadcast that
named it -- the same shape, found twice independently in two different
functions in this unit, is stronger evidence than either instance alone.

### The hypothesis is CONFIRMED as the correct explanation for `cb_read`'s ALREADY-CLOSED delay-slot-sharing residue, and its remaining OPEN residue shows the naive fix does not always transfer

`cb_read`'s round-20 fix (57->60/91, the `code=2`/`code=5` dispatch)
is the SAME family: retail shares one `li $a0,0x5` via a delay slot across
both branches, with the fallthrough path overwriting it -- a compiler
choice about which value's write is shared vs duplicated, not a scheduling
question a barrier could touch (round 19 confirmed a bare `__asm__("")`
does nothing here; round 20 closed it by flipping the guard polarity
instead, changing which arm falls through).

Re-reading `cb_read`'s STILL-OPEN residue (the `elseBranch` pointer
landing in `$s0` instead of `$v1`) against the raw `.s`
(`asm/nonmatchings/libcd_bios/cb_read.s`, lines 34-45) confirms the
block-order PART of the mechanism is already satisfied here: the
if-branch's own tail ends with an explicit `j .L8002B56C` (line 39, jumping
OVER the elseBranch to the shared label), and `elseBranch` itself (line
41-45) has NO trailing jump at all -- it simply falls through into the
shared label. `elseBranch` is textually LAST in the current preserved C
(`docs/match-reports/cb_read.md`'s round-20 body) and correctly gets
the fallthrough; the if-branch is textually first and correctly carries the
explicit jump. **The block order matches the rule exactly already.**

What is LEFT (the `$s0`/`$v1` register split) is a downstream
REGISTER-ALLOCATION consequence, not a block-order question: the current C
reuses ONE pointer variable (`p`) for both the if-branch's
long-lived-across-a-call address and the elseBranch's short-lived one,
which drags the elseBranch's use into the SAME persistent callee-saved
register the if-branch needs. **The head's general prescription --
"give the duplicate its own separate copy" -- was tried here TWICE,
independently, in two different rounds (round 19 and round 20, the second
explicitly re-testing in case the first was a stale artifact), and BOTH
times it regressed** (17/91-scale and 60->50/91 respectively): a second
named pointer variable changes register allocation for the WHOLE function,
not just the one store, because the allocator's overall pressure budget
is shared across the entire body. **This is a clean, twice-reproduced
counter-example to the naive form of the head's fix**, not a failure to
try hard enough -- worth keeping next to the positive `CD_readsync`
result, because both are true at once: the MECHANISM (retail keeps two
independent materializations, block order decides which one earns the
fallthrough) is real and worth checking first, but "split it into two C
variables" is not itself the fix once the block order is already right --
that lever only helps while the C's block order is WRONG.

### The hypothesis does NOT explain this unit's remaining register-identity residues, and does not need to -- they are a different mechanism

- **This function's own remaining residue** (address vs. value swapped
  between `$a0`/`$v1` at the very first computed temporary, round 20's
  "what's left" section above) involves NO branch, NO shared join, and NO
  duplicated store -- it is a register-NAMING choice for the first two
  SSA-like values in a straight-line prologue. Two reshapes (declaration
  order, statement order) were tried and regressed, independent of this
  round's investigation. The block-order hypothesis has nothing to say
  about a residue with no candidate blocks to order.
- **`CD_datasync`'s remaining residue** ("what's left is register
  NUMBERING, not hoisting-or-not") and **this pass's own new instances in
  `CD_readsync`** (the `p6A0`/`p8D9` register swap, four independent
  reorder/rename attempts inert) **and `func_8002B640`** (the
  `copySrc`/`rec`/`off` register-numbering swap in its table-search loop)
  are all the SAME class: a straight-line or single-loop-body register
  NAME choice, not a fallthrough/block-order decision. None involve two
  competing arms at a merge point.

### The hypothesis does not explain the confirmed-PURE-scheduling residues either, and this is a genuine boundary, not a gap in testing

`CD_init`'s round-20 permuter run measured its residue's `--debug`
breakdown directly: **`Reorderings: 3` with `Register Differences: 0`,
`Insertions: 0`, `Deletions: 0`** -- the permuter's own scorer, which can
see block/CFG-shape mismatches as insertions or deletions, reports NONE.
This residue (a `jal`/`move $a3,zero` pair retail schedules ~90 bytes/26
instructions after where its arguments materialize, within a SINGLE
extended stretch of straight-line code with no intervening branch) is
confirmed pure intra-block scheduling, not a block-order question -- there
is only one block here, not two competing ones. **A bare `__asm__("")`
barrier was tried directly on this exact residue (round 19) and made no
difference at all.** This is the useful boundary case for the head's
hypothesis: it predicts a barrier fails BECAUSE the real difference is
which block a value lives in; here there is no second block, and the
barrier still fails. So barrier failure is not ALWAYS a symptom of a
hidden block-order problem -- sometimes GCC's list scheduler simply picks
a tie-break among orderings that a scheduling barrier, which only fixes
relative order across itself, cannot force from any source-level
phrasing tried (call/goto restructuring, named locals, `volatile`).

`callback`'s open residue is a different boundary again: it is an
address-FOLDING (selection) difference, not an ordering one, and its own
report already states this explicitly ("a barrier does not affect
instruction SELECTION... only ORDERING, and this residue is a selection
difference, not an ordering one") -- confirmed independently this round by
re-reading the same `.s`, not just trusted from the prior text.

This round's own new function, `func_8002B94C.md`, adds a FOURTH boundary:
a redundant-looking check retail keeps that this build's C could not even
get GCC to EMIT, because the direct reproduction (`if (provably-true-by-
construction)`) was dead-code-eliminated at compile time regardless of any
barrier -- a barrier cannot rescue a branch the optimizer proves unreachable
before scheduling ever runs.

### Answer to the assigned question, mechanism-level

**The barrier's failure to transfer from an isolated reduction to a real
function has (at least) three distinct causes in this unit, and the
block-order hypothesis correctly identifies exactly one of them:**

1. **Block-order / shared-join fallthrough choice** (the head's
   hypothesis, CONFIRMED): when retail's compiler had two candidate
   materializations of a value converging on one join point, it gave the
   fallthrough to whichever arm was TEXTUALLY LAST in the arm's own
   source, and the other arm carries an explicit jump. An isolated
   two-value reduction has only ONE natural pair of candidates and the
   "wrong" one is easy to make last by construction; a real function
   often has the SAME two candidates but embedded in a body with more
   surrounding live values and control flow, where simply "put it last"
   is not obviously available without restructuring the whole
   surrounding shape (which is exactly what closed `CD_readsync`, and
   exactly what does NOT further help `cb_read` once already
   correct). A barrier cannot fix this because there is nothing to
   reorder WITHIN a block -- the missing/extra instructions are a
   consequence of which block executes at all along a given path.
2. **Pure intra-block list-scheduling tie-breaks** (a genuine, DIFFERENT
   failure mode from (1), confirmed via permuter debug output showing
   zero insertions/deletions/register differences): GCC's scheduler picks
   an instruction order among several equally-valid ones based on
   internal heuristics (register-pressure estimates, pass ordering) that
   are not expressible from source at all in the forms tried. A barrier
   CAN in principle fix an ordering problem, and does, elsewhere in this
   project (e.g. `CD_readm`'s three-way case-store barrier) -- but
   only when the barrier's position happens to coincide with where the
   scheduler's tie-break needs breaking. When it does not (this
   function's `a3`/`jal` split, this pass's `func_8002B640` constant-load
   ordering), the barrier is the RIGHT category of instrument but the
   WRONG position, and no position tried this round or in round 19/20
   found the right one. This is why an isolated reduction (fewer
   competing values, shallower scheduling graph) can differ from the real
   function even with NO block-order difference at all.
3. **Dead-code elimination proving a redundant check false before
   scheduling runs** (this pass's `func_8002B94C`, a fourth, narrower
   case): when the redundant-looking check is directly reproduced as a
   literal-foldable condition, GCC's optimizer removes it outright,
   which is not a scheduling outcome a barrier operates on at all.

**None of the three explains why an isolated 2-4 value reduction of the
"preserve raw copy, narrow in place" shape (round 24's `SeAutoVol`)
closes with a barrier while the real function does not** -- that specific
class was not reproduced inside THIS unit's five assigned stalls (none of
them is a preserve-then-narrow parameter shape), so this investigation
cannot independently confirm or refute round 24's own characterization of
it. What this round adds is that the SAME SURFACE SYMPTOM ("barrier works
small, fails at scale") has at least three genuinely different underlying
causes across the seven functions examined here, and telling them apart
needs the SAME discipline in each case: read the real `.s`'s block/label
structure and the permuter's own debug breakdown (insertions/deletions vs.
reorderings vs. register differences) before deciding which of the three
applies, rather than reaching for the barrier as a first move.

### Proposed learning

**"The barrier doesn't transfer" is a symptom with three distinct,
independently-diagnosable causes, not one mechanism** -- block-order/
fallthrough choice (fixable by restructuring which arm is textually last,
confirmed twice this round), pure list-scheduling tie-break (a barrier is
the right TOOL but this round found no position that worked, confirmed via
permuter debug output showing zero insertion/deletion component), and
dead-code elimination (no scheduling-level tool applies at all). Before
spending an attempt on `__asm__("")` placement, check the permuter's own
`--debug` breakdown if one exists, or the raw `.s`'s label structure for a
duplicated store/two-candidate-join shape, to know which of the three is
in play.

## Round 33 (runner charlie): re-verified 202/223, title rebuilt with the three required figures -- unchanged

Read this report per the round-32 "check inherited bodies" rule before trusting
its title. Re-compiled the round-20 body verbatim (with symbol names updated
to what this unit now calls the same functions -- see note below) and
confirmed it independently: `build exit=2`, no compile errors, `funcdiff.py`
reports **202/223 words, no staleness warning, compiled length exact at
223/223** (0x37C bytes, matches retail's own `nonmatching cd_read_retry, 0x37C`
header). This matches the round-20 report's own claim exactly -- the
preserved body was NOT one of the "one in six" false claims this round.

**First real diff, read off `tools/asm-differ/diff.py cd_read_retry` on a
clean isolated build: file offset `0x1B270`, vram `0x8002AA70`** -- the very
first instruction after the prologue's `addiu sp,sp,-0x40`. This is exactly
the address/value register swap round 20's "what's left" section already
named (retail: address in `$a0`, value in `$v1`; this build: the reverse),
not a new residue. The title above had no location figure before this round;
it now carries all three required figures.

**Symbol-name note for the next attempt**: this report's preserved body and
round-17/19/20's prose both use the raw `func_80025AE4`/`func_80012C20`/
`func_80025900` names for the three external calls. This unit has since
renamed all three (matched siblings now call them by name): `func_80025AE4`
-> `puts`, `func_80012C20` -> `printf`, `func_80025900` -> `VSync`. The raw
names no longer have externs in this file and do not resolve -- translate
them before compiling, do not add a second stale extern.

**No new attempt made on the entry-point register swap.** Per HARD RULE 6 and
this project's own residue taxonomy, a register-IDENTITY mismatch (which
value ends up in which register, not merely instruction order) is explicitly
out of scope for source-level fixing once two independent reshapes have
already been tried and both regressed severely (round 20: declaration-order
swap, no change; statement-order swap, catastrophic regression to 9/223).
Re-deriving a third guess with no new idea would just spend attempt budget
confirming what is already established. Restored to `INCLUDE_ASM` (no score
short of byte-exact stays in `src/`); build re-verified clean after revert
(`./build-and-verify.sh` exit 0).

### Round-32 lever checklist
1. `volatile`-as-narrower-instrument: N/A -- residue is register identity, not
   an ordering question a qualifier of any granularity touches.
2. Register-identity verdict as hypothesis, not fact: re-examined: the
   residue provably constrains WHICH REGISTER a value lands in (address vs.
   value swapped, not merely renamed), matching the test in HARD RULE 6
   exactly. Verdict stands.
3. Emission-order vs. source-order: not applicable here -- no new reorder
   attempted this round.
4. Permuter negative is evidence about one search, not the function: not run
   here this round (budget went to `CD_init`'s extended search
   instead, which has a permuter-confirmed pure-scheduling residue --
   AA6C's is a confirmed register-identity one and is not a comparable
   target).
5. asm-differ/permuter compare text, `addiu`/`ori` render identically:
   checked the raw encoding at the diff site directly (`0780043c`/`dcd88424`
   vs `0780013c`/...) -- these are genuinely different registers encoded,
   not an `addiu`-vs-`ori` rendering artifact.

### Proposed learning
This unit's C source has, since these reports were first written, renamed
`func_80025AE4`/`func_80012C20`/`func_80025900` to `puts`/`printf`/`VSync`
respectively (matched sibling functions in this same file now call them by
name). Any preserved report body predating that rename needs its call sites
translated before it will compile -- worth checking for ALL of this unit's
still-open stalls, not just this one, since the rename evidently happened
unit-wide and no single report flagged it.

## Round 36 (runner bravo): re-verified 202/223 in ISOLATION with the rename actually applied -- confirms round 33's figure, no new attempt

Round 33's own entry above already correctly documented the rename
(`func_80025AE4`->`puts`, `func_80012C20`->`printf`, `func_80025900`->`VSync`)
in PROSE and translated its own build -- this report was NOT an instance of
this round's assigned trap (contrast `CD_readsync.md` and
`cb_read.md` in this same unit, both of which claimed "already
current" while their preserved code blocks still called the raw names).

Rebuilt the round-20 202/223 body verbatim (names already correct per round
33), **in isolation** (all five other stalled siblings in this unit reverted
to `INCLUDE_ASM` -- see `CD_init.md`'s round-36 entry: this unit's
known 1-word-short sibling, `callback`, shifts every later `.bss`
address project-wide when left live alongside anything else, which pollutes
a non-isolated `funcdiff.py` read even though the target function's own code
is unaffected). Confirmed: `build exit=2`, no compile errors, `funcdiff.py`
reports **202/223 words, no staleness warning, compiled length exact at
223/223** -- matches round 20/33's recorded figure exactly, no discrepancy
between claimed and measured.

**No new attempt.** This function's one remaining residue (the
address-vs-value register swap at the very first computed temporary, `$a0`
vs `$v1`) has been tried and rejected twice (round 20: declaration-order
swap, no change; statement-order swap, catastrophic regression to 9/223) and
is a confirmed register-IDENTITY question per HARD RULE 6's own test (WHICH
register holds a value, not merely instruction order). **SKIPPING as
exhausted for manual attempts**; this round's remaining permuter budget went
to `CD_readsync` (also in this unit, a much cleaner permuter debug
signature -- see that report) instead, per the same reasoning round 33 gave
for making the same choice.

### Corrected, linkable body (current best, 202/223 words, length EXACT -- unchanged, re-verified with names confirmed current)

```c
#if 0
s32 cd_read_retry(void)
{
    s32 n;
    s32 *tmp;
    s32 *pRetry;
    volatile s32 *p2;
    s32 saved;
    s32 counter;
    volatile u8 *q;
    u8 buf;

    tmp = &D_8006D8DC;
    n = *tmp;
    D_8006D600 = 0;
    D_8006D5FC = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            puts(D_80010A40);
                        }
                        counter++;
                        CD_cw(1, 0, 0, 0);
                    }

                    while (CD_cw(0x16, D_8006D908, 0, 0)) {
                        CD_cw(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, (s32)&D_8006D618, 0, 0) != 0) {
                    goto tail;
                }
            }

            *D_8006D8C0 = 1;
            while (*D_8006D8CC & 7) {
                *D_8006D8C0 = 1;
                *D_8006D8CC = 7;
                *D_8006D8C8 = 7;
            }

            D_8006D8DA = 0;
            q = &D_8006D8D9;
            D_8006D61C = 0;
            *q = D_8006D8DA;
            __asm__("");
            D_8006D8D8[0] = 2;
            *D_8006D8C0 = 0;
            *D_8006D8CC = 0;
            *D_8006D8D0 = 0x1325;

            {
                s32 v0 = p2[0];
                buf = (u8)v0;
                if ((u8)v0 != D_8006D61C) {
                    if (CD_cw(0xE, (s32)&buf, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)cb_read;
            p2[-1] = p2[-2];
            CD_cw(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = VSync(-1) + 0x1E0;
            return p2[2];

        tail:
            tmp = &D_8006D8DC;
            n = *tmp;
            *tmp = n - 1;
            __asm__("");
        } while (n > 0);
    }

    {
        volatile s32 *pF4 = &D_8006D8F4;
        *pF4 = -1;
        return *pF4;
    }
}
#endif
```

### Proposed learning
This unit's own `CD_init.md` round-36 entry has the full mechanism,
but the short version worth repeating here: `funcdiff.py` scores for ANY
stalled function in this unit are only trustworthy when EVERY OTHER stalled
sibling is also reverted to `INCLUDE_ASM` -- the unit's confirmed
1-word-short function (`callback`) shifts `.bss` addresses project-wide
whenever it is left live, and this shows up as spurious word-mismatch noise
in an unrelated function's own diff with no hint that the cause lies
elsewhere in the same file.

## Round 39 (runner delta): re-verified 202/223, one new lever tried and reverted -- unchanged

Rebuilt the round-20/33/36 202/223 body verbatim (names already current):
`build exit=2`, no compile errors, `funcdiff.py` reports **202/223 words, no
staleness warning, compiled length exact at 223/223** -- matches every prior
round's recorded figure exactly.

**Tried this round's `CD_readsync`-style "always-true either-branch"
trick** (which closed a register-numbering residue in this same unit's
`CD_readsync` after manual reorders had failed) against the one open
residue here (the `tmp`/`n` address-vs-value swap at the function's first
computed temporary). Wrapped the `D_8006D600 = 0;` statement immediately
after the load in a duplicated `if (tmp || n) {...} else {...}` referencing
both contested values:

```c
tmp = &D_8006D8DC;
n = *tmp;
if (tmp || n) {
    D_8006D600 = 0;
} else {
    D_8006D600 = 0;
}
D_8006D5FC = 0;
*tmp = n - 1;
__asm__("");
```

**Regressed catastrophically: 202/223 -> 17/223, with 296037 bytes of
outside-range drift** (the compiled length grew substantially). Reverted
immediately. Unlike `CD_readsync`'s case, this residue sits in the
function's very FIRST two instructions, before any other value has been
computed and before any register pressure has built up -- there is no
"downstream pressure" for a duplicated branch to influence, so forcing an
extra read of `tmp`/`n` at this specific point just adds real cost instead of
reshaping an existing allocation decision. This is consistent with HARD
RULE 6 and this report's own three prior verdicts (round 20 two reshapes,
round 33/36 no new attempts): this is confirmed register-IDENTITY, and nothing
tried across four rounds now (naming order, statement order, and this
round's forced-liveness trick) has been anything but inert-or-worse.

**No further attempt.** This function's remaining budget this round went to
the two closer siblings this unit's assignment specifically flagged
(`CD_readsync`, matched; `CD_datasync`, improved). Restored to
`INCLUDE_ASM`; body unchanged from round 36's preserved best.

### Proposed learning

The permuter-found "always-true either-branch" trick that closed two
residues elsewhere in this unit this round (`CD_readsync`, `CD_datasync`)
needs EXISTING register pressure/pipeline depth to redirect -- it works by
nudging an allocator's decision among several live candidates, not by
creating pressure from nothing. Applied to a residue at a function's very
first instructions, before any competing pressure exists, it has no lever to
pull and can make the allocator's job measurably harder instead (here: it
grew the frame and regressed the score by an order of magnitude). Screen a
candidate placement for "is there already meaningful register pressure at
this point" before trying this specific trick, rather than applying it
uniformly to every register-identity residue in a unit.

## Round 41 (runner charlie): FIRST-EVER permuter search on this function, 202/223 -> 215/223 -- STALL, better

This function was assigned as "the queue's best never-searched function":
223 words, length exact, 202/223 matching since round 17, and -- confirmed
by grepping every prior round's own text -- **no permuter search had ever
run against it**. Every prior round's work here (17, 19, 20, 24, 32, 33, 36,
39) was manual reordering against the one register-identity residue at the
function's first computed temporary (`$a0`/`$v1`); the permuter itself was
always spent on a sibling instead.

### Scaffold validated before trusting it

Rebuilt round 36's 202/223 body verbatim into `src/` in isolation (all other
stalled siblings reverted): `build exit=2`, no compile errors, `funcdiff.py`
confirms **202/223, no staleness warning, compiled length exact at
223/223** -- matches every prior round's recorded figure exactly, no
discrepancy.

Set up `tools/setup-permuter.sh cd_read_retry <seed>` from that body (seed
kept in `permuter-seeds/cd_read_retry.c` in this worktree; not committed,
mirrors this report's body). One adjustment was needed relative to the
literal round-36 text: `D_8006D8DC` is now (since round 40's data-model
correction elsewhere in this unit) declared `extern s32 D_8006D8DC[10];`,
not a scalar, so `tmp = &D_8006D8DC;` no longer type-checks as intended --
changed to `tmp = D_8006D8DC;` (array decay), which is address-identical
and preserves the 202/223 score exactly.

`permuter.py --debug --stack-diffs` on the scaffold: **base score 320**
(`Register Differences: 24, Reorderings: 0, Insertions: 1, Deletions: 1`,
Stack/Branch Differences both 0) -- a clean, almost-pure register-identity
residue, consistent with this report's own characterization across four
prior rounds. Scaffold trusted.

### Search: 1800s bound, ~194,000 iterations, rc=0 (ran to its own timeout, not killed)

```
timeout 1800 ... permuter.py -j 6 --stop-on-zero --best-only permuter-work/cd_read_retry
```

Ran the full 1800s to completion (the process's own timeout fired; verified
via the trailing `permuter rc=` line and iteration count in
`/tmp/charlie_permuter_aa6c.log`, not inferred). **No zero found** --
194,339 iterations, error rate climbed to ~3600/194339 (~1.9%, i.e. ~98%
of mutations still compiled) by the end. `--best-only` saved four
successive improvements over the 320 base: 320 -> 240 -> 240 -> 235 -> 215.
This is a genuine, non-trivial improvement queue, not a flat search.

### The winning candidates, read as STATEMENTS not as permuter diff noise, and checked for correctness before adoption

**`output-215-1` (permuter score 215) is UNSOUND and was rejected despite
scoring lower than the candidate actually adopted.** Its single mutation:
hoist `tmp = D_80010AAC;` to ONCE, immediately after `pRetry = tmp;`,
outside the `do {} while` loop, then call `puts(tmp)` inside the loop
instead of `puts(D_80010AAC)` directly. Verified against the real oracle:
**202 -> 208/223**, no drift, so the improvement is real. But `objdump -d`
on the resulting object shows GCC materializes `D_80010AAC`'s address into
`$a0` ONCE, before the loop label, and the loop body's `jal puts` relies on
`$a0` STILL holding it -- while retail's own disassembly
(`asm/nonmatchings/libcd_bios/cd_read_retry.s`, `.L8002AAC8:`) recomputes
the same `lui`/`addiu` pair FRESH INSIDE the loop, every iteration. Because
`$a0` is caller-saved and every iteration of this loop makes several calls
(`printf`, `CD_cw` more than once) that clobber it, **the hoisted
form is a real correctness bug**: on any iteration after the first where
the retry counter (`D_8006D8DC[0]`) is still `< 7` (a condition the loop
itself is built to make happen -- that is the entire point of the retry
counter), `puts()` would be called with whatever garbage was last left in
`$a0` by an intervening call, not the intended string. Confirmed by
re-placing the same assignment INSIDE the loop (each iteration, right
before the call, preserving correctness) -- this drops straight back to
202/223, proving the byte-improvement is entirely contingent on the unsound
one-time hoist, not on reusing `tmp`'s dead slot as such. **This is why a
permuter improvement must be read as a set of STATEMENTS and checked for
whether they remain correct across every path the C allows, not just typed
back in because the real oracle liked the resulting bytes** -- funcdiff and
`build-and-verify.sh` can only ever check compiled-byte identity for the
INPUTS the pinned toolchain happens to choose at compile time; they cannot
see that a hoisted register load stops being valid data on a second loop
iteration, because that is a runtime property, not a static one.

**`output-235-1` (permuter score 235) IS sound and is this round's adopted
fix.** Its mutation reuses two variables that are provably dead at that
exact program point for a completely different purpose, in the same
"already-hot register as sink" idiom this unit's `CD_datasync.md`
(round 36) and `CD_readsync.md` (round 39/40) already established:

```c
s32 v0 = p2[0];
buf = (u8)v0;
n = ((u8)v0 != D_8006D61C);      /* was: if ((u8)v0 != D_8006D61C) */
if (n) {
    saved = (s32)&buf;            /* was: CD_cw(0xE, (s32)&buf, 0, 0) */
    if (CD_cw(0xE, saved, 0, 0) != 0) {
        goto tail;
    }
}
```

`n` (the retry counter) is not read again until `tail:` overwrites it
outright; `saved` (used earlier to stash `D_8006D5FC` around the timeout
retry loop) is already restored and consumed by this point. Reusing both
as throwaway sinks for a boolean and a pointer is legal, has no
observable effect on ANY execution path, and unlike the `output-215-1`
candidate does not persist a stale value across a loop boundary --
`n` and `saved` are both freshly written on every pass through this code
before being read. Verified alone (without the `tmp` hoist):
**202 -> 215/223, length still exact, no drift** -- a bigger real
improvement than the unsound candidate, and free of its correctness
problem.

**Combining both candidates does not stack -- it REGRESSES to 208/223,
i.e. to the unsound candidate's own score.** Tried once, out of curiosity
given `CD_readsync`'s history of stacking independent fixes; reverted
immediately once the real-oracle score came back lower than the `n`/`saved`
fix alone. Two additional manual variations on the surviving fix's
neighbourhood, both negative and both cheap to rule out:

- Reordering `p2 = tmp + 4;` before `pRetry = tmp;` (matching a
  scheduling difference visible in the OTHER, discarded residue at
  `0x8002AABC`-`0x8002AAC4` -- retail computes that block's `$s2` from
  `$a0` before moving `$a0` into `$s5`) -- regressed by one word (215 ->
  214). Reverted.
- Replacing the final block's local `volatile s32 *pF4` idiom with a
  direct `D_8006D8F4 = -1; return D_8006D8F4;` (D_8006D8F4 is already
  `volatile` at file scope) -- catastrophic regression (215 -> 173/223)
  with 296022 bytes of outside-range drift, i.e. it re-folds the address
  and shortens the function. Confirms this unit's established
  "`volatile` on the global alone is not enough; a local `volatile T *`
  pointer is what forces retail's unfolded addressing mode" idiom
  (`CD_readm.md`, `CD_flush.md`) applies here too. Reverted.

### What's left at 215/223: two small, apparently unrelated register/scheduling clusters

Read off `funcdiff.py`'s own DIFF lines (isolated build, all siblings
`INCLUDE_ASM`), not inferred:

1. **`vram=0x8002AABC`-`0x8002AAC4` (3 words)**: a scheduling difference in
   the loop-setup prologue. Retail computes `$s2 = $a0 + 0x10` (i.e. `p2`
   from `tmp`) BEFORE moving `$a0` into `$s5` (`pRetry`); this build
   computes it from `$s5` (i.e. from `pRetry`) AFTER the move. The C
   currently reads `pRetry = tmp; p2 = pRetry + 4;` -- tried the source-order
   swap (`p2 = tmp + 4;` before `pRetry = tmp;`) per the above, regressed
   by one word rather than fixing it, so this is not a simple statement-order
   question; not investigated further this round.
2. **`vram=0x8002ADAC`-`0x8002ADBC` (5 words)**: a register-identity swap
   (consistent pattern across all 5 words, `v0`/`v1`-shaped) in the final
   `D_8006D8F4 = -1; return ...;` block. The `volatile s32 *pF4` local is
   confirmed load-bearing (removing it costs the whole unfolded-addressing
   shape, see above); no reorder or spelling variant of this 3-line block
   was found this round that touches the remaining swap without also
   losing the unfolded form.

Both are now genuinely SMALL, isolated residues on an otherwise
byte-identical 223-instruction function -- a considerably better jumping-off
point for the next round than the single big register-identity block this
report described for four rounds running.

**Restored to `INCLUDE_ASM`** (215/223 is short of byte-exact; full
`./build-and-verify.sh` re-confirmed `OK: build matches retail SLPS_015.56`,
exit 0, after reverting). Corrected, linkable 215/223 body below.

### Corrected, linkable body (current best, 215/223 words, length EXACT)

```c
#if 0
s32 cd_read_retry(void)
{
    s32 n;
    s32 *tmp;
    s32 *pRetry;
    volatile s32 *p2;
    s32 saved;
    s32 counter;
    volatile u8 *q;
    u8 buf;

    tmp = D_8006D8DC;
    n = *tmp;
    D_8006D600 = 0;
    D_8006D5FC = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            puts(D_80010A40);
                        }
                        counter++;
                        CD_cw(1, 0, 0, 0);
                    }

                    while (CD_cw(0x16, D_8006D908, 0, 0)) {
                        CD_cw(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, (s32)&D_8006D618, 0, 0) != 0) {
                    goto tail;
                }
            }

            *D_8006D8C0 = 1;
            while (*D_8006D8CC & 7) {
                *D_8006D8C0 = 1;
                *D_8006D8CC = 7;
                *D_8006D8C8 = 7;
            }

            D_8006D8DA = 0;
            q = &D_8006D8D9;
            D_8006D61C = 0;
            *q = D_8006D8DA;
            __asm__("");
            D_8006D8D8[0] = 2;
            *D_8006D8C0 = 0;
            *D_8006D8CC = 0;
            *D_8006D8D0 = 0x1325;

            {
                s32 v0 = p2[0];
                buf = (u8)v0;
                n = ((u8)v0 != D_8006D61C);
                if (n) {
                    saved = (s32)&buf;
                    if (CD_cw(0xE, saved, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)cb_read;
            p2[-1] = p2[-2];
            CD_cw(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = VSync(-1) + 0x1E0;
            return p2[2];

        tail:
            tmp = D_8006D8DC;
            n = *tmp;
            *tmp = n - 1;
            __asm__("");
        } while (n > 0);
    }

    {
        volatile s32 *pF4 = &D_8006D8F4;
        *pF4 = -1;
        return *pF4;
    }
}
#endif
```

### Proposed learning

- **A permuter improvement can be real (oracle-confirmed, byte-for-byte)
  and still be an unsound C program.** `funcdiff.py`/`build-and-verify.sh`
  only ever compile ONE path through the source once; they cannot detect
  that a hoisted load of an address into a caller-saved register stops
  being valid the moment a loop that contains function calls repeats.
  Before adopting ANY permuter candidate that hoists a value out of a
  loop, check by hand whether anything between the hoist point and every
  use of it (including on a SECOND pass through the loop) can clobber it
  -- "does this still compile to the claimed bytes" and "is this still
  the same program" are different questions, and only the second one
  determines whether a candidate belongs in `src/` even provisionally.
- **When a permuter search yields multiple independent single-mutation
  improvements from the same base, they do not necessarily stack** -- here
  the two candidates scoring lower individually on the permuter's own
  metric (208 and 215) combined to LAND BACK on the worse one (208), not
  on something better than either. Check every combination against the
  real oracle before assuming "two good candidates together must be at
  least as good as the better one alone."
- This function joins `CD_datasync`/`CD_readsync` in confirming the
  "reuse an already-dead variable's slot as a throwaway sink for an
  unrelated value at the same program point" idiom as a real, repeatable
  register-allocation lever in this unit -- three independent instances
  now, always found by the permuter first and never by manual reasoning
  about the source.

## Round 49 (runner echo): fresh permuter search against the 215/223 seed -- negative; both best-scoring candidates rely on undefined behavior, not a legitimate fix

Rebuilt round 41's preserved 215/223 body verbatim in isolation (all other
stalled siblings in this unit reverted to `INCLUDE_ASM`): `build exit=2`, no
compile errors, `funcdiff.py` reports **215/223 words, length exact,
first diffs at vram `0x8002AABC`-`0x8002AAC4` (3 words) and
`0x8002ADAC`-`0x8002ADBC` (5 words)** -- matches round 41's recorded figure
exactly, no discrepancy.

### Hand axes re-tried against the two known residues, both negative, both reconfirming prior findings

1. Declared `pF4` (the `volatile s32 *` for the final `D_8006D8F4 = -1;
   return *pF4;` block) at function scope instead of block scope, and also
   tried folding the assignment into the condition-style idiom that closed
   `cb_read`'s analogous residue this round (`*(pF4 = &D_8006D8F4) =
   -1; return *pF4;`) -- **both inert, identical 215/223 and identical
   byte diff** at `0x8002ADAC`-`0x8002ADBC`. Unlike `cb_read`'s case,
   this residue is not sensitive to `pF4`'s declaration placement or
   expression form.
2. Re-tried `p2 = &tmp[4];` (array-index spelling instead of `pRetry + 4`)
   for the OTHER residue -- inert, identical 215/223. Confirms round 41's
   own finding that this residue is not a spelling/expression-shape
   question.

### Fresh permuter search: check 3 passes (AGREE, with the residue's own reorder built in), ~76,160 + 82,965 = 159,125 iterations across two runs, no safe improvement

Set up `tools/setup-permuter.sh cd_read_retry permuter-seeds/cd_read_retry.c`
from the round-41 215/223 body. `--debug --stack-diffs`: base score **235**
(`Register Differences: 7, Insertions: 1, Deletions: 1`). Read the `--debug`
disassembly diff directly rather than trusting the summary numbers alone:
the insertion/deletion pair is `retail: move s5,a0 / addiu s2,a0,0x10` vs
`this build: move s5,a0 / <nothing> ... addiu s2,s5,0x10` (the SAME
instruction, displaced by the alignment algorithm because of the
`0x8002AABC` scheduling residue, not a genuine length difference) --
consistent with the real build's own length-exact status. AGREE per check 3.

Ran two searches this round (the second because the first's rc was not
captured -- see below -- so a bounded second run gives a trustworthy
iteration count and rc for the record):

```
timeout 900 ... permuter.py -j 6 --stop-on-zero --best-only permuter-work/cd_read_retry
```

First run: ~76,160 iterations (background job, rc not captured to its own
file -- a process-compliance gap, noted rather than hidden, same as this
round's `callback` entry). Second run against the identical seed:
~82,965 iterations, also no rc file captured (both were launched as plain
backgrounded `&` jobs rather than the `timeout ...; rc=$?; printf ...`
form this round's broadcast specifies -- corrected for any further searches
this session). Both runs' logs end with the process's own multiprocessing
resource-tracker shutdown message, consistent with the 900s bound firing
naturally in each case. `--best-only` saved improvements to **200**, then
**95** (both runs converged on the same 95, from two different candidate
source files, `output-95-1` and `output-95-2`).

### The 95-candidates are UNDEFINED BEHAVIOR, not a legitimate fix -- rejected without building either into the real oracle

Both `output-95-1` and `output-95-2`'s only substantive mutation reorders
the two loop-setup statements so that `p2 = pRetry + 4;` executes BEFORE
`pRetry = tmp;` -- i.e. `p2` is computed from `pRetry`'s UNINITIALIZED
value:

```c
if (n > 0) {
    p2 = pRetry + 4;   /* pRetry has never been assigned yet here */
    pRetry = tmp;
    do { ... } while (n > 0);
}
```

This is reading an uninitialized local, which is undefined behavior in C.
The permuter's scorer never executes the candidate -- it only diffs
compiled bytes -- so it cannot tell that the "improvement" depends on
whatever garbage value cc1 happens to leave in `pRetry`'s register at that
program point (here, apparently the same register `tmp` was already
computed into moments earlier, which is why it happens to produce
plausible-looking bytes for THIS specific compile). This is the second
confirmed instance this round of the exact trap this round's broadcast
names (delta's round-48 lever 3, generalized): a mutation that is a real
correctness bug, not a semantically-inert store, scoring as an improvement
because the scorer cannot execute the code. **Rejected outright, without
building either candidate into the real oracle** -- reading the mutation
against the variable's own declaration already proves it unsound.

**No safe candidate found in either search.** Restored to `INCLUDE_ASM`
(215/223 unchanged from round 41); full `./build-and-verify.sh`
re-confirmed `OK: build matches retail SLPS_015.56`, exit 0, after
reverting.

### Proposed learning

Extends this round's `callback` finding (a reused-but-still-live
variable) with a second, distinct shape of the same underlying hazard: a
**reordered-but-uninitialized** variable. Both are invisible to the
permuter's byte-only scorer and both are cheap to catch by inspection
once suspected -- the common tell is a mutation that touches the ORDER or
IDENTITY of a variable's assignment/use pair rather than introducing a
genuinely new, freshly-initialized value. Before adopting any permuter
candidate that reorders two adjacent assignment statements, check that
neither statement now reads a variable that has not yet been assigned on
that path -- this is a strictly cheaper check than a full build-and-verify
round trip and catches this class before it costs one.

## Round 68 (runner charlie): NON_MATCHING body promoted

Track 1b. This report's title-region "salvaged mid-attempt snapshot,
never measured" label describes the STARTING point (round 17-19), not the
body being promoted here. What is promoted is round 41's 215/223-word
body (length exact, 223/223), reached through nine subsequent rounds of
measure-and-reshape (19, 20, 24, 25, 32, 33, 36, 39, 41) after the salvage
-- by round 20 the length was already exact (202/223) and every round
since re-verified the figure fresh rather than trusting a stale one. This
is not the unmeasured snapshot; judged on the body actually in front of
me, not on the round-17 label.

**Hybrid, both halves reviewed:**
- The overall control-flow/loop structure, the retry-counter decrement
  idiom, the reused `CD_flush`/link-wait blocks and the final
  `volatile s32 *pF4` unfolded-addressing idiom are hand-derived across
  rounds 17-39.
- The `n`/`saved` throwaway-sink reuse inside the `{ s32 v0 = p2[0]; ... }`
  block is a permuter find (round 41, candidate `output-235-1`). Reviewed
  here against the report's own correctness argument and re-confirmed:
  `n` (the retry counter) is not read again until `tail:` unconditionally
  overwrites it, and `saved` (holding a stashed `D_8006D5FC`) is already
  consumed earlier on this same path before being reused as `(s32)&buf` --
  both are freshly written on every pass through this code before their
  next read, with no loop-carried path back to a stale value. This is the
  SAME idiom already reviewed and promoted in `CD_datasync.md`.
- A second, lower-scoring permuter candidate from the same round
  (`output-215-1`, which hoists a `D_80010AAC` address load out of the
  retry loop) was reviewed and rejected as UNSOUND: `$a0` is caller-saved
  and the loop body makes calls that clobber it, so the hoisted form reads
  garbage on any iteration after the first where the retry counter is
  still under 7. Correctly never adopted. A third permuter search (round
  49) found two candidates at score 95 that both reorder `p2 = pRetry + 4;`
  before `pRetry`'s own assignment -- reading an uninitialized local,
  undefined behavior -- and both were rejected on inspection without ever
  being built. Only `output-235-1` was ever incorporated.

Residue at 215/223: two small isolated clusters (a loop-setup scheduling
swap and a register-identity swap in the final block), both confirmed
inert to every reorder/spelling axis tried across rounds 41 and 49.
Compiles clean under `-DNON_MATCHING` (one pre-existing `CD_cw`
int-from-pointer warning, no errors).

NON_MATCHING body promoted, round 68.

## Round 75 (runner delta): NOT GAME CODE -- this is libcd/bios.c's static `cd_read_retry`; not matched

REVISITED, round 75: stopped at the pre-work provenance check, no build made;
names/types not relevant.

`progress.py` currently counts this function as GAME (it is not in
`config/sdk-in-game.txt`, has no `config/psyq-objects.ld` pin and no
`identified` comment in the symbols file). That is only because no fingerprint
fires: retail's libcd is the December 1995 build that matches no disc in
`sdk/`, so the exact-masked screen in `sdkname.py --game` cannot hit. The
evidence that it is Sony's is otherwise conclusive:

1. **Position.** It lies between `CD_init` (0x8002A75C) and `CD_readm`
   (0x8002ADE8), both already identified as libcd/bios and pinned in
   `config/psyq-objects.ld`. In libcd 3.3's `bios.o`
   (`sdk/work/3.3/elf/libcd/bios.o`) the function in that exact slot, between
   `CD_init` (0x1220) and `CD_readm` (0x1740), is the weak/static
   `cd_read_retry` (0x1474).
2. **Its own strings.** `D_80010AAC` = `"CD read retry:"` and `D_80010ABC` =
   `"%d,pos=(%02x:%02x:%02x)\n"`. 3.3's bios.o carries
   `"CD read retry %2d(%02x:%02x:%02x)"`, the same message in an older form.
   The rodata block they sit in (`asm/data/120C.rodata.s`) also holds
   `"$Id: bios.c,v 1.71 1995/12/01 08:36:19 makoto Exp $"`, `"CD_init:"` and
   `"CD opening...\n"` / `"CD closing...\n"` (this function's `D_80010A40` /
   `D_80010A50`).
3. **Shape.** 3.3's `cd_read_retry` uses the same distinctive sequence as
   this body: the `0x1325` register write, `CD_cw(2, ...)`, `CD_cw(0xE, ...)`,
   `CD_cw(6, 0, 0, 1)`, `VSync(-1)` and a final `-1` return. Retail is 223 words
   against 3.3's 179 because it is a later build: `CD_init` grew the same way
   (196 words retail against 149 in 3.3).

So the right place for this function is track 2 (name it `cd_read_retry`,
libcd/bios, Dec-1995 build, identified by position and strings) and
`progress.py`'s library count, not the matching queue. **Proposed for the head,
not acted on** (the symbols file is off-limits to runners): add an
`identified` entry `cd_read_retry = 0x8002AA6C; // type:func` citing the
evidence above. After that, `progress.py` drops it from the game queue. The
round-68 NON_MATCHING body stays as it is (readable, 215/223, length exact)
until the head decides what to do with it.

### Proposed learning

**Where the build matches no SDK disc, the fingerprint screen goes quiet and
position plus rodata strings become the evidence.** `sdkname.py --game` needs
an exact masked match, and a libcd build that matches no disc produces none.
But a function sandwiched between two identified functions of one module,
whose string literals sit in that module's `$Id:` rodata block, is that
module's code. Before revisiting any stall in a `code_179d8_*` unit, look at
its neighbours in `config/psyq-objects.ld` and grep its `D_` strings'
rodata block for a `$Id:` line.
