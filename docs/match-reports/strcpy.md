# strcpy -- STALL (best: 7/17 words, register identity)

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment). Restored to
`INCLUDE_ASM`.

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
