# func_8002AA6C -- STALL (length EXACT 223/223, 202/223 words match, first real diff at vram 0x8002AA70)

Unit `code_179d8_g`. Runner delta, round 17. 223 instructions.

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

Delta's own committed stall report for the sibling `func_8002A75C` (58/196,
780/784 bytes) is the useful comparison: same unit, same session, and there
the length was nearly right, which is what made its score readable.

## Body, as salvaged

Uncompiled and unmeasured beyond the above. Everything it references was
declared in `src/code_179d8_g.c` at the time, including the `extern volatile`
hardware-register block (`D_8006D8C0` and neighbours) that delta established
for this unit.

```c
s32 func_8002AA6C(void)
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
                func_80025AE4(D_80010AAC);
                func_80012C20(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            func_80025AE4(D_80010A40);
                        }
                        counter++;
                        func_80029F10(1, 0, 0, 0);
                    }

                    while (func_80029F10(0x16, D_8006D908, 0, 0)) {
                        func_80029F10(1, 0, 0, 0);
                        func_80025AE4(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (func_80029F10(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (func_80029F10(2, (s32)&D_8006D618, 0, 0) != 0) {
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
                if (func_80029F10(0xE, (s32)&buf, 0, 0) != 0) {
                    goto tail;
                }
            }

            D_8006D600 = (s32)func_8002B4D4;
            p2[-1] = p2[-2];
            func_80029F10(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = func_80025900(-1) + 0x1E0;
            return p2[2];

        tail:
            n = *pRetry;
            *pRetry = n - 1;
        } while (n > 0);
    }

    D_8006D8F4 = -1;
    return D_8006D8F4;
}```

Positioned between `func_8002A75C` and `func_8002ADE8` in ROM order.

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
local-pointer idiom this round's `func_8002B4D4` report independently
found for the identical global and an identical write-then-read shape.
Before the fix, this build FOLDED the store's address (`sw
v0,-0x2710(at)`); after routing it through a local pointer, it matches
retail's unfolded `lui/addiu` + plain-offset store exactly. This is now
a THIRD confirmed site (after `func_8002ADE8`'s `D_8006D8EC`/`D_8006D8F0`
and `func_8002B4D4`'s own `D_8006D8F4` else-branch) where a global
already declared `volatile` at file scope still needs a LOCAL pointer
dereference to get unfolded addressing at one specific access -- see
`func_8002B4D4.md`'s proposed learning, now confirmed a fourth time
across two different functions touching the SAME global.

**What's left, confined to a narrow, well-characterized area:** the
retry-loop's per-iteration decrement (`n = *pRetry; *pRetry = n - 1;`
under the `tail:` label, and the mirrored statement at function entry)
allocates `pRetry`'s address into a fresh register (`$a0`) at the very
last use, where retail keeps it in the SAME persistent register (`$s3`)
it was allocated to earlier in the function. This is the same
parameter/local-persists-in-one-register-vs-gets-reloaded-fresh
register-identity question this round's other reports (`func_80032BB8`,
`func_8002C278`) already document as resistant to reshaping -- not
re-attempted here given the round's broader finding that this specific
class rarely yields to source-level levers, and given the function is
now at a solid, well-understood baseline rather than an unknown one.

**Not yet checked**: the `p2[-1] = p2[-2]; ... p2[2] = p2[-3]; p2[3] =
...; return p2[2];` pointer-arithmetic block and the two `func_80029F10`
guard calls -- the diff shows these regions ALREADY MATCH retail
byte-for-byte (confirmed via `asm-differ`, no markers in that range),
so the salvaged body's derivation of these fields was already correct;
nothing to re-derive there.

Restored to `INCLUDE_ASM` (no score short of byte-exact stays in
`src/`); the two extern declarations for `D_80010AAC`/`D_80010ABC` are
kept live in `src/code_179d8_g.c` since they're needed by any future
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

Also reinforces `func_8002B4D4.md`'s finding from this same round:
`D_8006D8F4` specifically (and by extension, this driver's other
`volatile`-qualified scalars) needs the local-pointer-dereference idiom
at EVERY write-then-immediate-read site, not just once per function --
this is now confirmed at two independent sites in two different
functions touching the same global.

## Round 20 (runner bravo): 119/223 -> 202/223, compiled length now EXACT (223/223), two structural fixes

Per the head's brief, worked from round 19's 119/223 baseline (ignore the
superseded "Score: NONE" section above this one -- see round 19's own
entry for why). Read `asm/nonmatchings/code_179d8_g/func_8002AA6C.s`
directly alongside `objdump -dr build/src/code_179d8_g.c.o` and
`tools/asm-differ/diff.py func_8002AA6C` to localize the actual
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
        if (func_80029F10(0xE, (s32)&buf, 0, 0) != 0) {
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

### Fix 2: missing `__asm__("")` barrier before the reused func_8002A510 tail block (149 -> 171/223)

The round-19/round-17 salvaged body's copy of the driver-reset tail
(`D_8006D8DA = 0; q = &D_8006D8D9; D_8006D61C = 0; *q = D_8006D8DA;
D_8006D8D8[0] = 2; ...`) was MISSING the `__asm__("");` barrier that the
canonical `func_8002A510` version of this exact block carries between
`*q = D_8006D8DA;` and `D_8006D8D8[0] = 2;`. Adding it back (this
report's earlier body simply never had it) fixed a real reordering in
that block. This is the same idiom `func_8002B4D4.md` and this report's
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
the general shape of `func_8002B3F4.md`'s residue-reading is right even
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
s32 func_8002AA6C(void)
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
                func_80025AE4(D_80010AAC);
                func_80012C20(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            func_80025AE4(D_80010A40);
                        }
                        counter++;
                        func_80029F10(1, 0, 0, 0);
                    }

                    while (func_80029F10(0x16, D_8006D908, 0, 0)) {
                        func_80029F10(1, 0, 0, 0);
                        func_80025AE4(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (func_80029F10(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (func_80029F10(2, (s32)&D_8006D618, 0, 0) != 0) {
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
                    if (func_80029F10(0xE, (s32)&buf, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)func_8002B4D4;
            p2[-1] = p2[-2];
            func_80029F10(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = func_80025900(-1) + 0x1E0;
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
`func_8002B4D4.md`) generalizes to the WHOLE shared 8-statement
`func_8002A510`-style tail, not just to single-scalar volatile writes** --
this function's copy was missing it purely because the salvaged snapshot
predated the discovery. Any future function that reuses this block
should carry the barrier from the start.

## Round 25 (runner charlie): the redundant-raw-copy-elision / block-order investigation

Assigned question: **what distinguishes the isolated reduction where a bare
`__asm__("")` barrier fixes the "redundant-raw-copy elision" class (round
24, `code_179d8_l`: `func_8002DDBC`/`func_8002E138`/`func_8002E308`) from
the real function, where it transfers to none of them?** Mid-investigation
the head sent two broadcasts proposing a BLOCK-ORDER / tail-merge mechanism
(measured on `func_8005CBC8` and `func_8005DBF0` in other units this same
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
shared check. This round applied the identical shape to `func_8002AEE0`
(this pass's own fresh function, `docs/match-reports/func_8002AEE0.md`):
an early `return -1;` written directly inside the timeout arm compiled
SHORTER than retail (missing the "redundant" `move v0,zero`/`bnez
v0,<epilogue>` pair retail keeps at the join) -- restructuring to
diag-block-FIRST, success-label-SECOND, single shared `result` variable
checked once, took that function from 61/174 to 153/174 in one fix, the
single largest gain of this whole session. **This is a direct, positive
confirmation of the head's mechanism**, and it predates the broadcast that
named it -- the same shape, found twice independently in two different
functions in this unit, is stronger evidence than either instance alone.

### The hypothesis is CONFIRMED as the correct explanation for `func_8002B4D4`'s ALREADY-CLOSED delay-slot-sharing residue, and its remaining OPEN residue shows the naive fix does not always transfer

`func_8002B4D4`'s round-20 fix (57->60/91, the `code=2`/`code=5` dispatch)
is the SAME family: retail shares one `li $a0,0x5` via a delay slot across
both branches, with the fallthrough path overwriting it -- a compiler
choice about which value's write is shared vs duplicated, not a scheduling
question a barrier could touch (round 19 confirmed a bare `__asm__("")`
does nothing here; round 20 closed it by flipping the guard polarity
instead, changing which arm falls through).

Re-reading `func_8002B4D4`'s STILL-OPEN residue (the `elseBranch` pointer
landing in `$s0` instead of `$v1`) against the raw `.s`
(`asm/nonmatchings/code_179d8_g/func_8002B4D4.s`, lines 34-45) confirms the
block-order PART of the mechanism is already satisfied here: the
if-branch's own tail ends with an explicit `j .L8002B56C` (line 39, jumping
OVER the elseBranch to the shared label), and `elseBranch` itself (line
41-45) has NO trailing jump at all -- it simply falls through into the
shared label. `elseBranch` is textually LAST in the current preserved C
(`docs/match-reports/func_8002B4D4.md`'s round-20 body) and correctly gets
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
try hard enough -- worth keeping next to the positive `func_8002AEE0`
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
- **`func_8002B198`'s remaining residue** ("what's left is register
  NUMBERING, not hoisting-or-not") and **this pass's own new instances in
  `func_8002AEE0`** (the `p6A0`/`p8D9` register swap, four independent
  reorder/rename attempts inert) **and `func_8002B640`** (the
  `copySrc`/`rec`/`off` register-numbering swap in its table-search loop)
  are all the SAME class: a straight-line or single-loop-body register
  NAME choice, not a fallthrough/block-order decision. None involve two
  competing arms at a merge point.

### The hypothesis does not explain the confirmed-PURE-scheduling residues either, and this is a genuine boundary, not a gap in testing

`func_8002A75C`'s round-20 permuter run measured its residue's `--debug`
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

`func_8002B3F4`'s open residue is a different boundary again: it is an
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
   surrounding shape (which is exactly what closed `func_8002AEE0`, and
   exactly what does NOT further help `func_8002B4D4` once already
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
   project (e.g. `func_8002ADE8`'s three-way case-store barrier) -- but
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
"preserve raw copy, narrow in place" shape (round 24's `func_8002E308`)
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
223/223** (0x37C bytes, matches retail's own `nonmatching func_8002AA6C, 0x37C`
header). This matches the round-20 report's own claim exactly -- the
preserved body was NOT one of the "one in six" false claims this round.

**First real diff, read off `tools/asm-differ/diff.py func_8002AA6C` on a
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
   here this round (budget went to `func_8002A75C`'s extended search
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
this round's assigned trap (contrast `func_8002AEE0.md` and
`func_8002B4D4.md` in this same unit, both of which claimed "already
current" while their preserved code blocks still called the raw names).

Rebuilt the round-20 202/223 body verbatim (names already correct per round
33), **in isolation** (all five other stalled siblings in this unit reverted
to `INCLUDE_ASM` -- see `func_8002A75C.md`'s round-36 entry: this unit's
known 1-word-short sibling, `func_8002B3F4`, shifts every later `.bss`
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
to `func_8002AEE0` (also in this unit, a much cleaner permuter debug
signature -- see that report) instead, per the same reasoning round 33 gave
for making the same choice.

### Corrected, linkable body (current best, 202/223 words, length EXACT -- unchanged, re-verified with names confirmed current)

```c
#if 0
s32 func_8002AA6C(void)
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
                        func_80029F10(1, 0, 0, 0);
                    }

                    while (func_80029F10(0x16, D_8006D908, 0, 0)) {
                        func_80029F10(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (func_80029F10(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (func_80029F10(2, (s32)&D_8006D618, 0, 0) != 0) {
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
                    if (func_80029F10(0xE, (s32)&buf, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)func_8002B4D4;
            p2[-1] = p2[-2];
            func_80029F10(6, 0, 0, 1);
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
This unit's own `func_8002A75C.md` round-36 entry has the full mechanism,
but the short version worth repeating here: `funcdiff.py` scores for ANY
stalled function in this unit are only trustworthy when EVERY OTHER stalled
sibling is also reverted to `INCLUDE_ASM` -- the unit's confirmed
1-word-short function (`func_8002B3F4`) shifts `.bss` addresses project-wide
whenever it is left live, and this shows up as spurious word-mismatch noise
in an unrelated function's own diff with no hint that the cause lies
elsewhere in the same file.

## Round 39 (runner delta): re-verified 202/223, one new lever tried and reverted -- unchanged

Rebuilt the round-20/33/36 202/223 body verbatim (names already current):
`build exit=2`, no compile errors, `funcdiff.py` reports **202/223 words, no
staleness warning, compiled length exact at 223/223** -- matches every prior
round's recorded figure exactly.

**Tried this round's `func_8002AEE0`-style "always-true either-branch"
trick** (which closed a register-numbering residue in this same unit's
`func_8002AEE0` after manual reorders had failed) against the one open
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
immediately. Unlike `func_8002AEE0`'s case, this residue sits in the
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
(`func_8002AEE0`, matched; `func_8002B198`, improved). Restored to
`INCLUDE_ASM`; body unchanged from round 36's preserved best.

### Proposed learning

The permuter-found "always-true either-branch" trick that closed two
residues elsewhere in this unit this round (`func_8002AEE0`, `func_8002B198`)
needs EXISTING register pressure/pipeline depth to redirect -- it works by
nudging an allocator's decision among several live candidates, not by
creating pressure from nothing. Applied to a residue at a function's very
first instructions, before any competing pressure exists, it has no lever to
pull and can make the allocator's job measurably harder instead (here: it
grew the frame and regressed the score by an order of magnitude). Screen a
candidate placement for "is there already meaningful register pressure at
this point" before trying this specific trick, rather than applying it
uniformly to every register-identity residue in a unit.
