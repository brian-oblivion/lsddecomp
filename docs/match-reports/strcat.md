# strcat

**Unit:** code_171e0 · **Size:** 42 instructions (0xA8 bytes) · **Status:** STALLED, best 16/42 in-range, class DELAY-SLOT-FILLER DUPLICATION

## What it does

Not the textbook libc `strcat`. It has a guard textbook `strcat` doesn't:
after computing `strlen(dest)` and `strlen(src)` (via a still-uncarved helper,
`func_80013348`, address only), it bails returning `NULL` if
`dest + strlen(dest) == src + strlen(src)` — i.e. the two strings' *end*
pointers coincide. It also returns `NULL` (not `dest`) if either input is
`NULL`. Otherwise it behaves like ordinary `strcat`: scans `dest` for its
terminator, then copies `src` (including the terminator) onto that point,
returning the original `dest`.

This is exactly the case CLAUDE.md's task brief calls out: a named
libc-shaped routine whose real source may be Sony's rather than the game's.
The overlap/end-pointer guard is not something an ordinary game-code
`strcat` reimplementation would have any reason to add, and reads as
Psy-Q-library-shaped defensive code.

## Derivation (control flow, confirmed correct — see Residue for the actual gap)

```
if (dest == NULL) return NULL;
if (src == NULL) return NULL;
if (dest + strlen(dest) == src + strlen(src)) return NULL;
d = <pointer to dest's terminator>;      /* while(*d) d++; idiom */
do { *d++ = c = *src++; } while (c);     /* while((*d++ = *src++)) idiom */
return dest;                              /* the ORIGINAL dest, preserved */
```

Confirmed instruction-for-instruction correct at the control-flow level: with
the C below, every non-delay-slot-filler instruction in the function matches
retail exactly, in the same order, using the same registers. The residue is
two isolated missing/differently-filled delay slots, not a structural or
register-identity problem.

## Residue — two delay-slot-filler gaps, not register or control-flow bugs

Read with `tools/asm-differ/diff.py strcat` at the best (16/42) attempt:

**Gap 1 — a redundant register reload retail keeps, mine elides:**

```
retail:  jal 13348 / move a0,s1     (redundant: a0 already == s1 here)
mine:    jal 13348 / nop
```

`$a0` already holds `dest` unchanged since function entry at this point (the
first call, `func_80013348(dest)`); nothing between entry and here writes
`$a0`. Retail's compiler re-established it anyway with an explicit `move`,
filling the branch-delay slot that would otherwise be a `nop`. My build's
compiler recognized the value was already correct and left the slot empty.
Both are semantically identical — same register, same value — this is a
pure delay-slot-filler *choice*, not a register-identity change (nothing here
is fixable or bannable per CLAUDE.md rule 6's register-pin test, since no
register identity differs).

**Gap 2 — a delay slot retail fills, mine leaves empty, plus an operand-source choice:**

```
retail:  beqz v0,X / addiu s1,v1,1     (fills the delay slot, reads FROM v1)
mine:    beqz v0,X / nop  ...  addiu s1,s1,1   (as a separate, later instruction, reads FROM s1)
```

Both compute the identical result (`v1 == s1` at this point — `v1` was just
copied from `s1` two instructions earlier), but retail's scheduler filled the
branch's delay slot with the increment (sourcing it from the freshly-copied
`v1`, which has no dependency on the just-tested branch condition), while
mine left the slot as `nop` and emitted the increment separately afterward
(sourcing it from `s1` itself). This costs exactly one extra word, which is
also why Gap 1 and Gap 2 together shift everything after them by 2 words
total in a naive size count, though after Gap 1 alone the size differential
is 1 word and after both it is netted against retail's own count correctly
(42/42 either way once both close — this was cross-checked with
`build-and-verify.sh`'s whole-image diff, not just the isolated window).

## Why this is classified TOOLCHAIN/COMPILER-INTERNAL, not a reshaping target

Both gaps are the same phenomenon documented in
`docs/match-reports/new_class_6d3c8.md` (a different unit, `code_1677c`,
found independently by a different runner): GCC 2.6.3's `-O2` delay-slot
filler (`fill_eager_delay_slots`/`fill_slots_from_thread` in reorg.c-era
GCC) sometimes duplicates an already-live value into a delay slot and
sometimes doesn't, for reasons that report's author could not find a
source-level lever to control after 14+ attempts on an even simpler
function (a 24-word allocator wrapper, one delay-slot residue, single
merge point). This function has the *same* residue class, twice, in a much
larger (42-word) body with three-way branching, guard clauses, and two
independent loops — strictly harder terrain for the same problem.

## Attempts tried (did not change the residue)

1. Named `s32 lenDest, lenSrc;` locals for both `strlen` results, then
   `if (dest + lenDest == src + lenSrc)` — worse (11/42): forces GCC to spill
   `lenDest` into a callee-saved register across the second `strlen` call
   (retail never does this — it reuses `$v0` directly in the *second* call's
   delay slot, before that call's own return overwrites it).
2. Inline compound expression, no named length locals:
   `if ((dest + func_80013348(dest)) == (src + func_80013348(src)))` —
   **16/42, the best result**, matches retail's delay-slot reuse of `$v0`
   across the two `strlen` calls exactly. All further attempts start from
   this shape.
3. Swapped addition operand order (`func_80013348(dest) + dest` instead of
   `dest + func_80013348(dest)`) — no change, 16/42.
4. Named `s32 lenDest` for *only* `dest`'s length, left `src`'s inline —
   no change, 16/42.
5. `char *d;` as a genuinely separate scan variable (not mutating the `dest`
   parameter itself), returning the untouched `dest` parameter directly —
   **worse (7/42)**: this flips which physical register holds the
   "preserved original pointer" vs. "advancing scan pointer" in a way that
   diverges further from retail's choice (retail mutates the *parameter's
   own* register in place for the scan, and copies the original into a
   fresh temp — the opposite of what this attempt does).
6. Bare `__asm__("");` as the very first statement of the function (per the
   head's `func_80025D10` broadcast, Lever 1) — **worse (6/42)**. Confirms
   this residue is not the prologue-store-order class that lever addresses;
   it perturbed unrelated scheduling instead.
7. Bare `__asm__("");` immediately before the overlap-check expression
   (attempt #2's shape) — no change, 16/42. Neither helps nor hurts; the
   barrier has no effect on this specific filler choice, same as
   `new_class_6d3c8`'s finding that an `asm("")` barrier didn't touch its
   analogous residue either.

None of these are register-identity changes (CLAUDE.md rule 6's test: would
removing/changing the attempt move a value to a DIFFERENT register? No —
every attempt either matches retail's registers or fails outright by
choosing different ones for unrelated reasons, e.g. attempt 5). This is
consistent with the finding in `new_class_6d3c8.md` that this residue class
does not yield to `if`/`goto`/`return` spelling, temp-variable placement, or
scheduling barriers.

## Preserved body

```c
#if 0
char *strcat(char *dest, char *src) {
    char *origDest;

    if (dest == NULL) {
        goto fail;
    }
    if (src == NULL) {
        goto fail;
    }
    if ((dest + func_80013348(dest)) == (src + func_80013348(src))) {
        goto fail;
    }
    origDest = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++) != 0) {
    }
    return origDest;
fail:
    return NULL;
}
#endif
```

(`func_80013348` is declared `extern s32 func_80013348(char *s);` in
`include/code_171e0.h`; it's a still-uncarved helper, address only, shaped
like `strlen`.)

## Head broadcast levers — applicability

- **goto-vs-return (func_80025B34 lever):** **applied, and it is the reason
  this function reached 16/42 rather than something much worse.** All three
  early exits return a value (`NULL`) different from the main path's
  (`origDest`) — exactly the shape the lever describes. Using `goto fail;`
  for all three, landing on a single shared `return NULL;`, matches retail's
  single shared tail (`move v0,zero` immediately before the common epilogue)
  with a single `move v0,v1`-shaped return on the success path — no
  duplicate epilogues, no extra `j`. This part of the function is fully
  correct; the residue is entirely within the delay-slot-filler class above,
  unrelated to how the early exits are spelled.
- **loop-invariant hoisting (func_80025D10 lever 2):** **checked, not
  applicable in the form described.** Neither loop here walks a named array
  against a hoisted base/end pointer — both are simple forward pointer scans
  (`while (*d) d++;` and `while ((*d++ = *s++))`) with no bound/array
  identifier for GCC to hoist in the first place. No `base`/`end` temp was
  introduced in any attempt, consistent with the lever's advice; it simply
  doesn't have a target to apply to here.
- **prologue store-order barrier (Lever 1):** **tried (attempts 6–7 above),
  did not close either gap.** Confirmed by checking register allocation
  before/after: removing the barrier changed nothing (attempt 7) and adding
  it at the top changed unrelated scheduling for the worse (attempt 6) without
  touching the two delay slots this report is about — this residue is a
  delay-slot **filler** choice (which independent instruction gets moved into
  an already-existing slot), not a prologue callee-save **store order**
  choice (which stack slot gets written first), so the lever's mechanism
  doesn't reach it either way.

## Proposed learning

This is the **second** independent instance (after `new_class_6d3c8` in
`code_1677c`) of GCC 2.6.3's delay-slot filler duplicating (or failing to
duplicate) an already-live register value, with no source-level lever found
across a combined 20+ attempts between the two functions. Promoting this to
a named residue class is worth doing project-wide: **"redundant delay-slot
value duplication"** — same value, same register, retail either restates it
or doesn't, and neither restating it manually (impossible without banned
register pins) nor barriers nor control-flow reshaping moves it. Per
`new_class_6d3c8.md`'s own conclusion, this is the strongest candidate for
the project's first permuter target once set up — and now has two known
occurrences to validate any candidate fix against, in two different units,
found by two different runners.
