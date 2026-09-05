# func_8002AA6C -- STALL (202/223 words, compiled length EXACT at 223/223 -- improved from 119/223 in round 20, see below)

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
