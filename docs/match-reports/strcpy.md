# strcpy -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libc2/strcpy.o` (Psy-Q 3.3) AND IS NAMED `strcpy` (0x80028B78).** It was
> matched C counted as game code; the object owns its bytes, so the C is gone from
> `src/` and the game-code count shrank by it -- the correction CLAUDE.md asks
> for, not a regression. The run `libc2/strcpy` + `libc2/strstr` + `libcd/sys`
> tiles 0x19378..0x19C78 and crosses the code_179d8_h / libcd_bios boundary;
> both units trimmed. Whole-image SHA1 green. Nothing here is assignable and
> there is no stall left to work. The text below is the pre-conversion record.

## Original report

# strcpy -- MATCHED 17/17 (round 18, echo, via permuter lead + reuse-both-parameters idiom)

Unit: `code_179d8_h`.

## Class: register identity (banned to fix with `register T v asm("$N")`)

Screened clean on both documented blockers (no `gp_rel`, no `addiu $at`
hits). This is a NULL-safe `strcpy` (returns `NULL` if either argument is
`NULL`, not the textbook libc version), confirmed structurally via
`tools/m2ctx.py code_179d8_h --sig 'char *strcpy(char *dest, char *src)'
--run`, which independently reconstructs the same control flow: two early
NULL guards, a "peeled" first-byte copy (write unconditionally, THEN test
whether to enter the copy loop), and a tight `do`/`while` for the remaining
bytes.

## What's right

Every attempt (6 total, all build-verified) reproduced retail's exact
INSTRUCTION COUNT, INSTRUCTION TYPES, and BRANCH TARGETS -- the control-flow
skeleton is settled. What never converged is which physical register holds
which of the four live values (the saved `dest`, the incrementing write
pointer, the incrementing read pointer, and the current byte).

Best attempt (7/17, `build exit=0`, no out-of-range drift):

```c
#if 0
char *strcpy(char *dest, char *src) {
    char *destWrite;
    char *srcRead;
    char *result;
    char c;
    char c2;

    result = NULL;
    if (dest != NULL && src != NULL) {
        c = *src;
        srcRead = src + 1;
        destWrite = dest + 1;
        *dest = c;
        if (c != 0) {
            do {
                c2 = *srcRead;
                srcRead++;
                *destWrite = c2;
                destWrite++;
            } while (c2 != 0);
        }
        result = dest;
    }
    return result;
}
#endif
```

Diff at that attempt (asm-differ, target left / current right):

```
19384:    move    v1,a0        |    19388:    addiu   a2,a1,1
19388:    lbu     v0,0(a1)          1938c:    lbu     v0,0(a1)
1938c:    addiu   a1,a1,1      <
19390:    addiu   a0,v1,1      r    19390:    addiu   v1,a0,1
19398:    sb      v0,0(v1)     r    19398:    sb      v0,0(a0)
1939c:    lbu     v0,0(a1)     r    1939c:    lbu     v0,0(a2)
193a0:    addiu   a1,a1,1      r    193a0:    addiu   a2,a2,1
193a4:    sb      v0,0(a0)     r    193a4:    sb      v0,0(v1)
193ac:    addiu   a0,a0,1      r    193ac:    addiu   v1,v1,1
193b0:    move    v0,v1        r    193b0:    move    v0,a0
```

Retail saves the ORIGINAL `dest` into `$v1` via an explicit `move`, then
reuses the ORIGINAL parameter register `$a0` itself as the incrementing
write cursor. Every attempt that instead introduced a separate
"destWrite"-style local (leaving `dest`/`$a0` untouched) put the saved copy
and the write cursor in the opposite registers from retail, or (when the
local was eliminated to let the compiler reuse `$a0` as the cursor directly)
dropped the shared-epilogue "already terminated" branch target entirely,
producing a DIFFERENT residue (an extra/missing jump) rather than a cleaner
match.

## Attempts (6, all build-verified, all on the SAME axis)

1. Plain `do { c = *src++; *d++ = c; } while (c);` -- 4/17, wrong SHAPE
   entirely (no first-byte peel at all; retail's is fundamentally not this
   simple tail-test loop).
2. Peeled first byte + plain `while` for the rest, still with a separate
   local for the write cursor -- 0/17, restructured completely differently
   (compiler reordered the two NULL checks).
3. m2c's own reconstruction, transcribed close to verbatim (`var_a0` renamed
   to a local) -- 5/17, correct shape, registers swapped.
4. Same shape, renamed locals (`destWrite`/`srcRead`) -- 7/17 (BEST),
   registers still swapped, only ONE nop-vs-real-instruction difference at
   the very top.
5. `dest`/`src` PARAMETERS reused directly as the incrementing cursors (no
   separate write-cursor local), with an explicit `orig` local saving the
   pre-increment `dest` -- 6/17: eliminated the save-copy `move`
   ENTIRELY (closer to retail's INTENT of reusing `$a0`) but also
   eliminated the branch target retail keeps at the "already terminated"
   early-exit, landing on the function's final `jr` instead of the
   dedicated `move v0,v1` label -- a different residue, not strictly worse
   or better.
6. Attempt 4's exact shape with plain `while` instead of `if + do/while` --
   5/17, worse than attempt 4.

**Every attempt varies WHICH C-LEVEL VARIABLE gets reused vs. introduced
fresh (the untested axis CLAUDE.md's guide warns about is elsewhere: I did
not try forcing the loop itself into a different shape, e.g. a `for` loop,
or moving the two NULL guards into a single combined `if` with an `||`
early-return instead of `&&` continue -- those remain unexplored, though
attempt 2 suggests the compiler is happy to reorder the guards on its own).**
This is squarely CLAUDE.md's register-identity class: "if removing it
changes WHICH REGISTER holds a value, it is banned" -- no reshaping tried
changed WHETHER the value is computed, only which register ends up holding
which of the four live pointers, and 6 different variable-naming/shape
combinations produced 6 different (never fully matching) register
assignments.

### Proposed learning

A NULL-safe libc-style function that reuses one of its OWN incoming
parameters as a mutating cursor (rather than introducing a fresh local for
the write/read pointer) is worth trying BOTH ways early -- "increment the
parameter directly, save a copy for the return value" vs "leave the
parameter alone, introduce a separate cursor local" -- since GCC 2.6.3's
register allocator treats them differently enough that one shape can
eliminate an instruction the other needs (see attempt 5's disappearing
`move`), and getting BOTH the shape and the reused-vs-fresh choice right
simultaneously is where the residue actually lives, not in either alone.

## RESOLUTION (round 18) — permuter found a real lead at score 100 (not zero); hand-translation closed it

Ran the permuter bounded (`timeout 600`, `-j 6 --stop-on-zero`) against the
round-17 7/17 near-miss body. `--debug` base score 315 (11 register
differences + 1 reordering + 1 insertion/deletion, matching the report's
"register identity, widespread" reading). Ran **52148 iterations**; never
reached zero, but DID find a real improvement partway through, saved by
`--best-only` at `permuter-work/strcpy/output-100-1` (score **100**, down
from 315). `timeout`'s own exit code line was not captured in the log
(same outer-Bash/inner-`timeout` race documented in `StageMap__ConfigureRateEntry`'s
report this round) — treated as an ordinary self-stop given the clean
`iteration 52148` count and non-crash `multiprocessing` shutdown warning,
not independently verified.

**The permuter's raw 100-score diff** (`output-100-1/diff.txt`):

```diff
 char *strcpy(char *dest, char *src)
 {
   char *destWrite;
+  char *new_var;
   char *srcRead;
   char *result;
-  char c;
+  unsigned char c;
   char c2;
   result = 0;
-  if ((dest != 0) && (src != 0))
+  destWrite = dest;
+  if ((destWrite != 0) && (src != 0))
   {
     c = *src;
     srcRead = src + 1;
+    new_var = dest;
     destWrite = dest + 1;
     *dest = c;
     ...
-    result = dest;
+    result = new_var;
   }
+  if (1) { ...
   return result;
 }
+}
```

Not committable verbatim (dead reassignments of `destWrite`, an inert
`if (1) { }` wrapper, a spurious `unsigned char` retype) — but the
underlying idea (introduce an EXTRA copy of `dest`, distinct from the one
used for the guard, distinct again from the one used as the write cursor)
scored strictly better than every one of round 17's 6 hand attempts, none
of which had tried more than ONE copy at a time.

**What actually closed it, hand-derived from that lead in 3 real-oracle
iterations (round 18's attempts 7-9, all `build-and-verify.sh`-verified):**

1. Literal translation of the permuter's two-copy idea, cleaned up
   (`destWrite = dest;` hoisted before the guard, a second `newVar = dest;`
   right before the mutating `destWrite = dest + 1;`, `result = newVar;`)
   -- **9/17**, better than 7/17 but not there.
2. Realizing the lead's real content is "the compiler wants A DIFFERENT
   pointer identity for the RETURNED copy than for the WRITE cursor, and
   letting BOTH `dest` and `src` themselves be reused as mutating cursors
   (eliminating `destWrite`/`srcRead` as separate locals entirely) rather
   than introducing yet another named cursor" -- combined with keeping
   ONE extra copy (`newVar = dest;`) purely for the return value, taken
   right after the guard:

```c
char *strcpy(char *dest, char *src) {
    char *newVar;
    char *result;
    char c;
    char c2;

    result = NULL;
    if (dest != NULL && src != NULL) {
        newVar = dest;
        c = *src;
        src = src + 1;
        dest = dest + 1;
        *newVar = c;
        if (c != 0) {
            do {
                c2 = *src;
                src++;
                *dest = c2;
                dest++;
            } while (c2 != 0);
        }
        result = newVar;
    }
    return result;
}
```

**17/17, `build exit=0`, whole-image SHA1 verified** (`./build-and-verify.sh`
-> `OK: build matches retail SLPS_015.56`).

This is round 17's own attempt 5 (`dest`/`src` parameters reused directly
as cursors, an explicit `orig`-style local for the pre-increment `dest`)
-- which THAT round scored 6/17 and rejected for dropping a branch target
-- but with `newVar`'s assignment moved to happen BEFORE `c = *src;`
instead of after the parameters are reused, which is the detail that
actually recovers the missing branch target. Round 17 never tried that
exact ordering; the permuter's noisy two-copy lead was what pointed at
"copy timing relative to the parameter reuse" as the remaining axis, even
though its own literal diff didn't survive translation as-is.

**Verdict on the `strcat` precedent, per this round's brief: it did NOT
repeat in the "permuter zero is dead code" sense (the permuter never
reached zero here at all), but the shape of what generalized is the same
one `strstr` demonstrated this round: a permuter score that IMPROVES but
doesn't zero out can still be a correct, translatable lead if you read
WHAT AXIS it moved (here: copy timing/count relative to a guard and a
reuse), not just WHETHER it reached zero.** Three data points now:
`strcat` (zero via dead code, translated to idiomatic same-path reuse),
`strstr` (zero via ordinary hoisting, committed verbatim), `strcpy`
(never zeroed, but a mid-search improvement pointed at the right axis by
example rather than by being the answer itself).

### Proposed learning

**A permuter run that never reaches zero is not automatically a wasted
attempt if `--best-only` improved on the seed at all** -- check
`permuter-work/<func>/output-*` for anything below the base score even
after a `--stop-on-zero` run times out without finding one. Here a
score-100 (down from 315) intermediate result, itself not committable,
was the difference between round 17's 6 attempts plateauing at 7/17 and
round 18 closing at 17/17 in 2 more hand attempts once its AXIS (not its
literal diff) was identified.

---

## Prior state (round 17) -- STALL at 7/17, since resolved above

Runner: echo, round 17 (second assignment). Restored to `INCLUDE_ASM` at
the time; kept below for the correct algorithm derivation and the six
ruled-out attempts that establish why this is squarely the register-
identity class.
