# func_8002C0AC -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libc2/strncmp.o` (Psy-Q 3.3) AND IS `strncmp`.** It was
> matched C counted as game code; the object owns its bytes, so the C is gone from
> `src/` and the game-code count shrank by it -- the correction CLAUDE.md asks
> for, not a regression. The run `libcd/iso9660` + `libc2/strcmp` +
> `libc2/strncmp` tiles 0x1BE40..0x1C92C and crosses the code_179d8_g /
> code_179d8_d boundary; both units trimmed. Whole-image SHA1 green. Nothing
> here is assignable and there is no stall left to work. The text below is the
> pre-conversion record.

## Original report

# func_8002C0AC -- MATCHED (32/32 words), round 17 permuter pass

**Unit:** code_179d8_d · **Size:** 32 instructions (0x80 bytes) ·
**Status: MATCHED.** The report below documents a real stall at 30/32,
explicitly recommended in its own text as "a legitimate permuter
candidate" -- a round-17 permuter pass closed it in ~40s. Full history
kept below; the fix is described first.

## Round 17: what closed it

`tools/setup-permuter.sh`, seeded from this report's own preserved 30/32
body (unchanged). `--debug` base score 20, all register-difference
penalty (4 registers x 5), zero insertion/deletion/reordering -- matching
this report's own "pure instruction-order residue... register-identity
safe" description exactly. Search found a zero at iteration 95.

The winning mutation hoists the comparison itself into a named temp,
evaluated BETWEEN the two loads and the `s2++` increment (rather than
inline inside the `if`):

```c
s32 func_8002C0AC(char *s1, char *s2, s32 n)
{
    char c1;
    char c2;
    s32 mismatch_flag;

    if (s1 == NULL) {
        goto check_eq;
    }
    if (s2 != NULL) {
        goto loop_entry;
    }
check_eq:
    if (s1 != s2) {
        goto not_equal;
    }
    goto return_zero;
not_equal:
    if (s1 == NULL) {
        return -1;
    }
    return 1;

loop_entry:
    n--;
    if (n < 0) {
        return 0;
    }
loop_top:
    c1 = *s1;
    c2 = *s2;
    mismatch_flag = c1 != c2;
    s2++;
    if (mismatch_flag) {
        goto mismatch;
    }
    if (c1 == 0) {
        goto return_zero;
    }
    s1++;
    n--;
    __asm__("");
    if (n >= 0) {
        goto loop_top;
    }
mismatch:
    if (n < 0) {
        goto return_zero;
    }
    return *s1 - *(s2 - 1);
return_zero:
    return 0;
}
```

**This is the final source, committed as-is.** The permuter's own raw
output used `short new_var` for the temp; verified byte-identical against
`permuter-work/func_8002C0AC/target.o` (retail's own bytes, independently
assembled) via a hand rebuild through the pinned pipeline, THEN simplified
for commit -- retyped `short` -> `s32` (this project's usual boolean-flag
width) and renamed `new_var` -> `mismatch_flag`, re-verified byte-identical
after each change before committing either. `./build-and-verify.sh`
confirms the whole-image SHA1, not just this function.

### Proposed learning

**Hoisting an inline comparison (`if (a != b)`) into a named temp
assigned BETWEEN the two loads it depends on and a later unrelated
statement (here, `s2++`) is a real lever for the "two independent loads
compile in the opposite order, values register-safe" residue class** --
distinct from (and evidently more effective than) the bare
`__asm__("")` barrier already tried and rejected for this exact residue
shape in both this function and its sibling `func_8002C048`. Try this
BEFORE reaching for the permuter on the next instance of this residue
signature (base-score debug output: all register-difference penalty,
zero insertion/deletion/reordering) -- it may be cheap enough to attempt
by hand. `func_8002C048` (this same unit) still carries the identical
residue and is worth revisiting with this specific lever.

## Full history (original stall report)

## Role

The 3-arg `strncmp` sibling of `func_8002C048` (this unit, this round) --
same NULL-safe preamble, same loop shape with a trip count (`n`) added.
`func_8002C048`'s report documents the same underlying idioms this
function needed; this report focuses on what's specific to `n`.

## Best body reached (30/32, NOT byte-exact -- do not merge)

```c
#if 0
s32 func_8002C0AC(char *s1, char *s2, s32 n)
{
    char c1;
    char c2;

    if (s1 == NULL) {
        goto check_eq;
    }
    if (s2 != NULL) {
        goto loop_entry;
    }
check_eq:
    if (s1 != s2) {
        goto not_equal;
    }
    goto return_zero;
not_equal:
    if (s1 == NULL) {
        return -1;
    }
    return 1;

loop_entry:
    n--;
    if (n < 0) {
        return 0;
    }
loop_top:
    c1 = *s1;
    c2 = *s2;
    s2++;
    if (c1 != c2) {
        goto mismatch;
    }
    if (c1 == 0) {
        goto return_zero;
    }
    s1++;
    n--;
    __asm__("");
    if (n >= 0) {
        goto loop_top;
    }
mismatch:
    if (n < 0) {
        goto return_zero;
    }
    return *s1 - *(s2 - 1);
return_zero:
    return 0;
}
#endif
```

## Three residues found and fixed

1. **The shared `return 0` block sits at the OPPOSITE end of the function
   from `func_8002C048`'s.** In `func_8002C048`, the shared tail is
   positioned EARLY (right after the `s1==s2` check, reached by
   fallthrough from there and by `goto` from the loop). In THIS function,
   retail positions it at the very END (right before the epilogue,
   reached ONLY by explicit `goto` from three different sites: `s1==s2`,
   the loop's `c1==0` match, and the mismatch tail's `n<0` check -- none
   of them adjacent to it, none of them a fallthrough). Placing
   `return_zero: return 0;` early (mirroring `func_8002C048`'s layout,
   the natural first guess) put ONE of the three jump targets at the
   wrong address entirely (confirmed via `objdump`: `beq a0,a1` landed on
   the loop body instead of the shared tail). Moving the label to the end
   of the function body, with all three sites using an EXPLICIT `goto`
   (no fallthrough anywhere), reproduced retail's three-way share exactly.
   **Lesson: don't assume a sibling function's shared-tail layout
   transfers -- check where retail actually puts the block, every time.**
2. **A "wasted" `nop` after the loop-continuation branch, only reproduced
   with a scheduling barrier.** Before any fix, GCC found a genuinely
   MORE efficient schedule than retail's own: it filled the `bgez
   $a2,loop_top` branch's delay slot with `addiu $a0,$a0,1` (`s1++`)
   instead of leaving it as a `nop`, and moved `addiu $a2,$a2,-1` (`n--`)
   into the EARLIER `beqz $v1,return_zero` branch's delay slot instead
   (which retail fills with `s1++`, not `n--`). Both schedules are
   semantically identical and equally valid MIPS -- retail's is simply
   less aggressively packed. Source statement order (`s1++; n--;` vs
   `n--; s1++;`) had NO effect on which delay slot got which instruction
   (confirmed by testing both). A bare `__asm__("");` placed between
   `n--;` and the `if (n >= 0)` check DID force retail's less-packed
   schedule -- this is squarely the ALLOWED use of the barrier per
   CLAUDE.md's own test (order changed, not register identity: `n` and
   `s1` still end up correctly updated either way).
3. **The mismatch tail's re-cache into `c1`/`c2`, fixed the same way as
   `func_8002C048`'s** -- dereferencing `*s1`/`*(s2 - 1)` directly in the
   `return` expression rather than reassigning locals first, per the
   head's no-cache-across-CSE direction. No `volatile` needed here either.

## The remaining residue: identical to func_8002C048's

With all three fixes in place, the ONLY remaining difference is the exact
same class as `func_8002C048`'s own stall: the loop-top's two independent
`lbu`s (`c1 = *s1;` / `c2 = *s2;`) compile in the OPPOSITE order from
retail (`$v0`/c2 from `$a1` loaded first, `$v1`/c1 from `$a0` second) --
both registers end up holding the CORRECT values (register-identity safe,
confirmed via `objdump`), only their relative instruction order differs.

Tried, no effect (same levers as `func_8002C048`, not re-exhausted here
since the mechanism is identical): swapping the C statement order of the
two reads, a bare `__asm__("");` placed between them.

**Classification:** same pure instruction-order residue as
`func_8002C048`. Given it is the SAME two-instruction swap shape in a
SECOND function in this unit (same file, same day), this reads less like
an isolated fluke and more like a systematic scheduling-heuristic
difference the pinned toolchain has for this specific "two independent
`lbu`s feeding an immediate comparison" pattern -- worth flagging for
whoever next has a `-mips1`/`gcc-2.6.3` toolchain lead to investigate, though
that is an operator escalation, not something to experiment with further
here. Legitimate permuter candidate at 30/32, same as `func_8002C048` at
23/25.

### Proposed learning

A shared multi-way-`goto` return block's physical position in the
function body is NOT predictable from a sibling function's layout, even
when the sibling is otherwise structurally near-identical (same
preamble, same loop shape) -- check retail's actual block order via
`objdump` before assuming a shared tail belongs where a related function
put its own. Also: the same two-independent-loads instruction-order
residue recurring in two sibling functions in one unit is worth recording
as a pattern, not just two isolated near-misses -- see
`func_8002C048`'s report for the fuller catalogue of what was tried
against it.
