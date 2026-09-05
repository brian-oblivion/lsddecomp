# func_8002AA6C -- STALL (119/223 words, drift confined to 1 real instruction -- established a real baseline in round 19, see below)

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
