# strstr -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libc2/strstr.o` (Psy-Q 3.3) AND IS NAMED `strstr` (0x80028BBC).** It was
> matched C counted as game code; the object owns its bytes, so the C is gone from
> `src/` and the game-code count shrank by it -- the correction CLAUDE.md asks
> for, not a regression. The run `libc2/strcpy` + `libc2/strstr` + `libcd/sys`
> tiles 0x19378..0x19C78 and crosses the code_179d8_h / code_179d8_b boundary;
> both units trimmed. Whole-image SHA1 green. Nothing here is assignable and
> there is no stall left to work. The text below is the pre-conversion record.

## Original report

# strstr -- MATCHED 30/30 (round 18, echo, via permuter)

Unit: `code_179d8_h`.

## RESOLUTION (round 18) — permuter zero, and it was ALREADY idiomatic

Ran the permuter bounded (`timeout 600`, `-j 6 --stop-on-zero`) against the
round-17 26/30 near-miss body. `--debug` base score 210 (1 insertion + 1
deletion penalty, the documented one-instruction-residue signature),
matching the report's claimed residue class. Reached score 0 at iteration
1068. `timeout` exit code: **0** (the run stopped itself on `--stop-on-zero`
finding a match; it did not hit the 600s bound).

**The winning diff, verbatim from `permuter-work/strstr/output-0-1/diff.txt`:**

```diff
   matching = 0;
   cursor = needle;
+  matchStart = haystack;
   if ((*haystack) != 0)
   {
-    matchStart = haystack;
     do
```

I.e. hoist `matchStart = haystack;` out of the `if` block to sit
unconditionally right after `cursor = needle;`, leaving everything else
(including the `if`'s condition and the loop) untouched.

**Verdict on the `strcat` precedent (the question this round's brief asked
directly): it did NOT repeat, and the reason is worth recording precisely.**
`strcat`'s permuter zero was a *provably dead* store (`origDest = dest;`
inside the branch that returns `dest`, on a path where `origDest` is never
read again) — committing it verbatim would have been dead code, and the
idiomatic translation (`return dest;` instead of `return NULL;`) had to be
derived separately, then verified to score identically. Here, `matchStart =
haystack;` is dead ONLY on the empty-`haystack` early-exit path (where the
function returns `NULL` without ever reading `matchStart`) — but it is very
much NOT dead on the path that matters, where `*haystack != 0` and the loop
runs: it is `matchStart`'s only initializer, read on every subsequent
iteration and returned on a match. Moving a variable's initialization above
an unrelated guard, when the variable's value doesn't depend on that guard,
is ordinary, unremarkable C style — nobody would flag `matchStart =
haystack;` sitting next to `cursor = needle;` as a hack. **The permuter's
raw output needed NO translation here; it went into `src/` verbatim.**

So: two data points, not yet three, and they diverge on the one thing that
matters (does the winning form need a rewrite before it's committable) —
`strcat` did, `strstr` did not. The common thread across both is narrower
than "expect a dead store": it is "the residue is a MENTION-COUNT/ORDERING
question, not a value question" — both were one-instruction residues where
the retail instruction in question was an initialization whose exact
placement (which guard it sits before/after) was the entire gap. Whether
that placement change reads as dead code or as ordinary hoisting depends on
whether the variable is used elsewhere on the same path, which has to be
checked per-instance -- it is not something the permuter's score tells you.

**Committed body (identical to the one above, in `src/code_179d8_h.c`):**

```c
char *strstr(char *haystack, char *needle) {
    char *cursor;
    char *matchStart;
    s32 matching;

    matching = 0;
    cursor = needle;
    matchStart = haystack;
    if (*haystack != 0) {
        do {
            if (*haystack == *cursor) {
                cursor++;
                if (*cursor == 0) {
                    return matchStart;
                }
                if (matching == 0) {
                    matchStart = haystack;
                    matching = 1;
                }
            } else {
                cursor = needle;
                matching = 0;
            }
            haystack++;
        } while (*haystack != 0);
    }
    return NULL;
}
```

`build exit=0`, `funcdiff`: `strstr: 30/30 words match`, whole-image SHA1
verified (`./build-and-verify.sh` -> `OK: build matches retail
SLPS_015.56`).

### Proposed learning

**A one-instruction "wrong side of a guard" residue is worth a permuter run
even when 7 hand attempts on the SAME axis (as round 17's were, all
varying `cursor`'s placement) failed** — the permuter found the fix on the
adjacent, untried axis (`matchStart`, not `cursor`) in closer to 1000
iterations than the ~7 manual tries, and the result required zero
translation. Contrast with `strcat`: always re-derive whether a winning
diff is dead code or ordinary hoisting by checking whether the touched
variable is read on the path where the change is "unnecessary" — do not
assume either answer from the `strcat` precedent alone.

---

## Prior state (round 17) -- STALL at 26/30, since resolved above

Runner: echo, round 17 (second assignment). Restored to `INCLUDE_ASM` at
the time; kept below for the correct algorithm derivation and the seven
ruled-out attempts, all on the (as it turned out, wrong) axis.

## Class: instruction order (one instruction sunk into a branch by GCC, not
retail) -- NOT register identity, and a scheduling barrier did not fix it

Screened clean on both documented blockers. Confirmed structurally via
`tools/m2ctx.py code_179d8_h --sig 'char *strstr(char *haystack, char
*needle)' --run`, which independently reconstructs the same algorithm: a
single forward scan over `haystack` with a persistent needle cursor that
resets to `needle` on any mismatch, and a "match started" flag so the
FIRST matching position (not every position re-matched) is what gets
returned.

## Result (best, 26/30, `build exit=0`, no out-of-range drift)

```c
#if 0
char *strstr(char *haystack, char *needle) {
    char *cursor;
    char *matchStart;
    s32 matching;

    matching = 0;
    cursor = needle;
    if (*haystack != 0) {
        matchStart = haystack;
        do {
            if (*haystack == *cursor) {
                cursor++;
                if (*cursor == 0) {
                    return matchStart;
                }
                if (matching == 0) {
                    matchStart = haystack;
                    matching = 1;
                }
            } else {
                cursor = needle;
                matching = 0;
            }
            haystack++;
        } while (*haystack != 0);
    }
    return NULL;
}
#endif
```

## The residue

Every single instruction in the function's body matches retail -- same
count, same opcodes, same operands, same branch targets -- EXCEPT ONE:
retail's `move $a2,$a1` (`cursor = needle`) is the function's SECOND
instruction, unconditionally, before the `lbu`/`beqz` that tests whether
`haystack` is empty. My compiled output places the exact same instruction
INSIDE the `if` block instead (after the branch), because `cursor` is
provably dead in the "haystack is empty, return NULL" path and GCC 2.6.3
sinks it there. `matching = 0` (`move $a3,$zero`), equally dead on that
path, is NOT sunk by GCC -- it stays as the function's first instruction in
every attempt, matching retail exactly. This asymmetry (one dead
initialization sunk, an equally-dead one not) is the entire residue.

```
TARGET (retail)                          CURRENT (mine)
193bc: move a3,zero                      193bc: move a3,zero
193c0: move a2,a1        <-- MISSING HERE
193c4: lbu   v0,0(a0)                    193c0: lbu   v0,0(a0)
193c8: nop                               193c4: nop
193cc: beqz  v0,19428                    193c8: beqz v0,19428
                                          193cc: move a2,a1      <-- SUNK HERE
193d0: move t0,a0 (delay slot)           193d0: move t0,a0 (delay slot)
[... rest of the function is byte-identical from here on ...]
```

## Attempts (7, all build-verified, all on the SAME axis: WHERE `cursor`'s
first assignment lands relative to the branch)

1. `cursor = needle;` written before the `if`, everything else as above --
   26/30 (BEST). This is the natural, most literal translation.
2. Same, but with `matching = 0;` written AFTER `cursor = needle;` (swapped
   order) -- 25/30, worse (a second instruction also drifted).
3. `cursor = needle;` written explicitly INSIDE the `if` block (matching
   what the compiler seems to "want") -- 25/30, worse, not better: the
   compiler placed it at a DIFFERENT position within the block than where
   it lands when left to sink on its own.
4. A bare `__asm__("");` scheduling barrier inserted between `cursor =
   needle;` and the `if` -- introduced a genuine extra `nop` (everything
   after shifted by one word), not just a reorder; CLAUDE.md's own bracket-
   real-branches note is the likely mechanism (the barrier sits adjacent to
   a `noreorder`-sensitive branch and triggers defensive padding rather
   than simply pinning order). Worse (7/30) and a different failure mode
   than the other attempts.
5. Same barrier, moved inside the `if` block instead (after the branch) --
   no effect either way (still 26/30) since it no longer touches the
   ordering decision that matters.
6. `if (*haystack == 0) { return NULL; } { ... rest of function ... }`
   (structured as an explicit early return instead of a wrapping `if`) --
   4/30, much worse: the function's shared "found nothing" tail (retail's
   `.L80028C28`/`.L80028C2C`, reached from BOTH the empty-haystack case and
   the loop-exhausted case) does not get shared this way, producing a
   substantially different, longer function.
7. `haystack` used directly as the mutating scan pointer (no separate
   `scan` local, matching the parameter-reuse idiom that fixed `GetCdFileSize`
   and was explored for `strcpy`) -- no change to this specific residue
   (still 26/30); ruled out as the relevant axis for this particular
   instruction.

**All 7 attempts vary WHEN/WHERE `cursor`'s initial assignment is placed.
The untested axis: whether `matching`'s assignment is what should be
DELAYED instead (matching retail by NOT pre-computing `matching=0`
unconditionally, forcing IT into the branch instead of `cursor` -- I did
not find a C form that produces this inversion; every attempt either kept
both dead assignments hoisted or sank `cursor` specifically).** This is
CLOSE to CLAUDE.md's "prologue callee-save stores in the wrong order" class
(same registers, same offsets/values, order differs, permitted lever is a
scheduling barrier) but is NOT that exact case -- there is no frame/callee-
save here, and the one scheduling-barrier attempt made things measurably
worse rather than better, which is itself useful negative evidence: this
residue is NOT a simple delay-slot-filler choice the barrier idiom
resolves.

### Proposed learning

A scheduling barrier (`__asm__("")`) is not a safe default lever for "an
instruction is in the wrong position relative to a branch" residues --
CLAUDE.md's own bracket-real-branches warning (about `noreorder` and
maspsx's defensive nop insertion) applies even to code that LOOKS like a
harmless ordinary branch (a plain `beqz`, no explicit `__asm__` block
containing the branch itself): placing a bare `__asm__("")` NEAR one can
still trigger padding. Test it on a copy/rebuild before trusting it, the
same as any other reshape -- it is not risk-free just because CLAUDE.md
calls it "allowed".
