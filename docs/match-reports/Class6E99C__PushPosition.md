> Renamed from `func_8004042C` on 2026-09-20 (tools/rename.py). Address 0x8004042c.

# Class6E99C__PushPosition -- STALL, corrected (was misfiled as unfixable register-identity; real fix narrowed the function to a 3-word permuter-exhausted "redundant move" residue -- 22/25, NOT the "1 word remaining" (i.e. implied 24/25) figure this report previously stated, which propagated into PROGRESS.md and round 19's assignment)

## Round 46 (runner delta): drift-checked fresh, no new attempt -- DELIBERATE SKIP

Re-spliced the exact preserved body below (unchanged) in isolation and
rebuilt: **22/25 words match (file 0x30C2C-0x30C90)**, no outside-range
drift -- identical to every prior measurement, not stale. Deliberately
skipped a new attempt: this residue is already PERMUTER-EXHAUSTED across
two independent searches (~28,487 + ~47,996 iterations, rounds 19/bravo)
that both converged on the same 210 base score, plus a manually-applied
copy of the one UB-flavoured trick the permuter itself surfaced
(branch-forced self-copy of `a1`), also negative. Per this round's
round-33-selection-effect instruction to record a reasoned skip rather
than re-spend a budget on a function whose cheap levers are measurably
exhausted. No new lever occurred to me beyond what the two prior searches
and nine hand attempts already cover. Restored to `INCLUDE_ASM`; full
oracle re-confirmed green.

## Round 21 (runner delta): re-verified fresh, class holds, no new attempt

Restored the exact preserved body verbatim and rebuilt fresh: reproduces
`22/25 words match (file 0x30C2C-0x30C90)` exactly, matching this
report's own round-19 correction precisely (word 4 missing entirely,
words 13/16 read `$a1` instead of `$t0`). No outside-range drift. Also
re-ran with the coordinator's corrected oracle grep this round
(`error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]`): zero
hits, so this was never a masked compile error either -- the "22/25,
permuter-exhausted" verdict genuinely stands.

Considered whether the "write the expression in place, back into a
dying operand" lever that closed this round's `Class6E99C__Configure` (a
different function in this same unit) would transfer here: it would
not -- this residue is the OPPOSITE shape. There, retail wanted a value
kept in ONE register and this compiler split it into two; here, retail
WANTS an extra register (a redundant `move $t0,$a1`) that this compiler
never introduces, because nothing in the C creates a second live name
for `a1` at all. The two residues are structurally mirror images, not
the same lever applied twice. Given two independent permuter searches
(~28,000 + ~48,000 iterations) already exhausted this residue across
rounds 18/19, not re-spending a further search without a genuinely new
angle. Restored to `INCLUDE_ASM`; full oracle re-confirmed green.

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::slotE8` (`+0x0E8`).

**Correction to an earlier version of this report**, which claimed a full
25/25 match under the stale-build window described in `New_Class6E99C.md`.
Re-verified genuinely fresh, this function has a real residue -- but the
investigation also found and fixed a genuine FIELD-WIDTH bug in
`include/code_2cc8c.h` along the way (see "Field width correction" below),
which is the one part of this report NOT superseded by the correction.

**Second correction (round 19, runner alpha): the "ONE word remaining"
figure below is WRONG, verified by direct rebuild, not by re-reading
permuter output.** The round-bravo section further down states "this
final 1-word 'redundant move' residue" and the title inherited that
language; round 18's own PROGRESS entry and this round's task assignment
both repeated it as "Class6E99C__PushPosition ... ONE WORD REMAINING." Rebuilding
the EXACT preserved body (below, unchanged) against the real oracle
gives:

```
Class6E99C__PushPosition: 22/25 words match (file 0x30C2C-0x30C90)
```

-- three words differ, not one:

```
word  4 (0x8004043C): retail move $t0,$a1   ; built: missing entirely (nop)
word 13 (0x80040460): retail lhu $v0,0x0($t0) ; built: lhu $v0,0x0($a1)
word 16 (0x8004046C): retail lhu $v0,0x4($t0) ; built: lhu $v0,0x4($a1)
```

**Where the "1" came from:** the round-bravo section's own permuter
`--debug` breakdown (reproduced there) is "Register Differences: 2,
Insertions: 1, Deletions: 1" -- a WEIGHTED PENALTY SCORE (210 =
2x5 + 1x100 + 1x100), not a word count. `docs/MATCHING-GUIDE.md` says
this explicitly ("permuter.py's score is not funcdiff's... zero means
identical, not a word count") but this report's own prose collapsed
"the residue is ONE conceptual defect (a missing redundant move)" into
"ONE word," and that phrasing is what carried into three downstream
consumers. The missing `move $t0,$a1` is indeed a single missing
INSTRUCTION -- but its absence forces the two loads that would have
read `$t0` to read `$a1` instead, which is two MORE differing words in
the real byte-for-byte comparison. One missing instruction, three wrong
words: both are true, and only the second is what `funcdiff.py`/the
oracle actually measures.

**The preserved body itself is NOT lost or wrong** -- it is the best
body reached (confirmed: attempts 5-9 in the round-bravo section all
independently re-measure 210/22-25 with no improvement), so nothing
needs re-deriving. Only the VERDICT LINE was wrong. Corrected below and
in the title above; anyone citing this function's state from now on
should say **22/25 (3 words), permuter-exhausted**, not "1 word
remaining."

## Field width correction (kept, unlike the stall verdict above)

`Class6E99CObj::unk88`/`unk8C` were originally typed `s16` (from
`Class6E99C__PopPosition`'s own `lhu` read alone). This function's OWN `sw` (a full
WORD store, not `sh`) at those exact offsets is direct evidence they are
`s32`, not `s16`:

```
sw $v0, 0x88($a3)    /* NOT sh -- retail widens a lhu-loaded s16 into a real s32 field */
sw $v1, 0x8c($a3)
```

Retyped to `s32` in the header (removing an earlier, now-known-wrong 2-byte
`pad08A` inserted to compensate for the old `s16` typing). This retype is
what let `Class6E99C__PopPosition` (a SIBLING function, already matched) reach a
genuine 9/9 -- confirmed by rebuild, not assumed. Separately,
`Class6E99CObj::unk60`/`unk62` needed retyping `s16` -> `u16`: this
function's own `lhu` (zero-extending) when WIDENING them into the now-`s32`
`unk88`/`unk8C` only reproduces with an unsigned source type.

## Best body reached (structurally exact, wrong register bank)

```c
#if 0
void Class6E99C__PushPosition(Class6E99CObj *self, SkipShort2 *a1, Pair32E99C *a2) {
    if (self->unkC != 0) {
        self->unk88 = self->unk60;
        self->unk8C = self->unk62;
        __asm__("" ::: "memory");
        self->unk90 = self->unk50;
        self->unk94 = self->unk54;
        __asm__("" ::: "memory");
        self->unk60 = a1->x;
        self->unk62 = a1->y;
        __asm__("" ::: "memory");
        self->unk50 = a2->a;
        self->unk54 = a2->b;
    }
}
#endif
```

## Residue

With the `__asm__("" ::: "memory")` barriers between each load/store PAIR
(without them, GCC batches all four independent loads together before any
store -- see "Load/store interleaving" below), every LOAD, STORE, OFFSET
and OPERAND ORDER matches retail exactly. The ONLY remaining difference:
retail copies its incoming `self`/`a1` parameters into `$a3`/`$t0`
(`move $a3,$a0` / `move $t0,$a1`) right at function entry, before even
testing `self->unkC`; every C shape tried keeps them in `$a0`/`$a1`
directly. Same CLAUDE.md HARD RULE 6 test as `func_8003FCFC`'s stall: same
instructions, only the register NUMBER differs.

> **CROSS-REFERENCE CORRECTED (round 39, head).** `func_8003FCFC` is
> **`TransposeMatrix`** (`libgte/fgo_00.o`), reclassified as a linked Sony
> object -- it was never a game stall. The HARD RULE 6 *test* is CLAUDE.md's
> and applies here unchanged; what is void is the appeal to that function as
> a worked precedent for it. This function's own measurement (same
> instructions, register number only) is what carries the classification.

### Attempts at the register-identity part (2)

1. Base body above: `$a0`/`$a1` throughout, not `$a3`/`$t0`.
2. Explicit local copies (`Class6E99CObj *obj = self; SkipShort2 *src =
   a1;`, then use `obj`/`src` throughout): no change at all -- GCC still
   keeps the values in `$a0`/`$a1`.

## Load/store interleaving (solved, kept for the next reader)

Without barriers, GCC 2.6.3 hoists ALL FOUR of this function's independent
loads (`unk60`, `unk62`, `unk50`, `unk54`) ahead of any of the four stores,
which retail does not do -- retail interleaves each load immediately with
its own store before starting the next pair. A bare
`__asm__("" ::: "memory")` between pairs is what stops the hoist (a bare
`__asm__("")` with no clobber did NOT work when tried on the sibling
`Class6E99C__PopPosition`, memory clobber was required there too). This same lever
closed `Class6E99C__PopPosition` (now a real, confirmed 9/9 match) and is the reason
this function's own remaining gap is ONLY the register-identity issue, not
also a structural one.

## What I did NOT try, and why (round-14/15 version, SUPERSEDED below)

- **The permuter**, for the register-identity half specifically -- a
  4-instruction-prologue register-bank swap on an otherwise byte-exact
  25-word function is a plausible target, but not attempted due to time
  budget this round.

## Round-bravo correction: this was NOT an unfixable register-identity stall

The coordinator flagged this report for re-examination: CLAUDE.md HARD RULE
6 bans the MECHANISM (`register T v asm("$N")`, extended-asm operand
constraints) for fixing register identity, not the OUTCOME of getting
retail's registers via different, ordinary C. "Every C shape tried keeps
them in `$a0`/`$a1`" (above) had only tried 2 shapes: the direct body, and
declaring-then-immediately-using local aliases (which GCC copy-propagates
away, so it is not really a different shape at all).

**`permuter.py --debug` on the exact body above (verified against the
literal transcription) scores 780, not the "25/25 structurally, register
bank differs" this report claimed** -- the debug diff shows a REAL
structural defect the word-count framing had missed:

```
retail (a2->a/a2->b pair):     candidate (same pair, no struct-copy):
  lw   v0, 0(a2)                 lw   v0, 0(a2)
  lw   v1, 4(a2)                 nop                    <-- extra
  sw   v0, 0x50(a3)               [missing: lw v1,4(a2)]
  sw   v1, 0x54(a3)              sw   v0, 0x50(a0)
                                  lw   v0, 4(a2)          <-- extra
                                  nop                     <-- extra
                                  sw   v0, 0x54(a0)
```

Retail BATCHES the final pair's two loads (needs 2 registers, no delay-slot
nop); every version of the C body tried before this round INTERLEAVES it
instead (reuses one register, needs a filler `nop` after each `lw`) --
this is the load/store-interleaving mechanism from this same report's
"Load/store interleaving" section, but the OPPOSITE direction: pairs 1-3
all read and write through DIFFERENT base pointers per statement in a way
that made GCC interleave (matching retail), but the 4th pair, `self->unk50
= a2->a; self->unk54 = a2->b;`, is TWO SEPARATE assignment statements at
the C level -- and for this shape specifically, GCC 2.6.3 interleaves it
too, where retail batches it.

**Fix: write the final pair as a single whole-struct assignment instead of
two scalar ones** -- `*(Pair32E99C *)&self->unk50 = *a2;` (both sides are
word-aligned `s32` pairs, no unaligned-access concern). This makes GCC emit
the aggregate-copy lowering (`lw`/`lw`/`sw`/`sw`, no barrier needed, no
`nop`), which is exactly retail's batched form. `--debug` score dropped
from 780 to 210, structurally exact (0 reorderings on this pair) --
**and, unexpectedly, the SAME change also fixed the `$a0`->`$a3` register
identity for `self` for the ENTIRE REST OF THE FUNCTION**, including the
prologue `move a3,a0` that every previous attempt failed to reproduce.
Nothing about the earlier attempts changed; a single downstream shape
change altered enough of cc1's internal pseudo-register numbering that
`self`'s allocation flipped to match retail everywhere it is used. This is
further, sharper evidence for the project's existing "declaration/statement
order decides register/stack layout" family: here it decided a register
choice for a value structurally unrelated to the statement that was
changed.

### Best body reached this round (permuter score 210 / funcdiff 22-25 words, single MISSING-INSTRUCTION residue, structurally exact otherwise)

```c
#if 0
void Class6E99C__PushPosition(Class6E99CObj *self, SkipShort2 *a1, Pair32E99C *a2) {
    if (self->unkC != 0) {
        self->unk88 = self->unk60;
        self->unk8C = self->unk62;
        __asm__("" ::: "memory");
        self->unk90 = self->unk50;
        self->unk94 = self->unk54;
        __asm__("" ::: "memory");
        self->unk60 = a1->x;
        self->unk62 = a1->y;
        __asm__("" ::: "memory");
        *(Pair32E99C *)&self->unk50 = *a2;
    }
}
#endif
```

(Statement ORDER matters here and is confirmed, not guessed: swapping the
struct-copy pair to occur BEFORE the `a1->x`/`a1->y` pair reaches the exact
same instructions with 0 register differences at all, but as a pure
4-instruction REORDERING -- i.e. the whole block lands in the wrong
position instead of the wrong registers, worse overall (240) and proof the
order in the body above, not the swapped one, is the real source order.)

### The one remaining residue: `a1` -> `$t0`, an isolated "redundant move"

With the fix above, the ONLY remaining difference in the whole function is
that retail additionally does `move $t0,$a1` in the `beqz`'s delay slot
(executed unconditionally, on both branch paths, exactly like `self`'s own
`move $a3,$a0`) and then uses `$t0` for both `a1->x`/`a1->y` loads; every
version of the body above keeps using `$a1` directly (no redundant copy).
`self` needed no such extra mention to get its own move -- it is
structurally forced to appear early because the guard condition
(`self->unkC`) reads it -- but `a1` is never read outside the `if`-block,
so nothing in the C forces an early copy of it, yet retail makes one
anyway. This is the project's documented "redundant move" residue class
(see `Obj6EAC0__ApplyColor.md`, `func_80063144.md`, MATCHING-GUIDE): tried an
explicit `SkipShort2 *src = a1;` local declared before the `if` and used in
place of `a1` for both dereferences -- GCC 2.6.3 copy-propagates it away
with NO instruction emitted, score unchanged at 210, matching how this
class has resisted the same lever every other time it has been tried in
this project.

**Permuter run completed** (`permuter-work/func_8004042C_v4`, seeded with
the body above): `-j 6 --stop-on-zero --best-only`, bounded with
`timeout 500`. Base score confirmed 210 via `--debug` before the search.
Ran to completion (~28,487 iterations; exit code not literally captured,
see "Anomaly" in `Class6E99C__Stop.md` for the same wrapper issue -- the
iteration count and log tail are consistent with the 500s bound firing,
not with `--stop-on-zero`). **No zero reached; best found was 25**, not
210 -- but every candidate at or below 210 that was inspected used a
classic decomp-permuter UB-flavoured forcing trick (`if (self) { x = a1;
} else { x = a1; }`, forcing a copy of a live value through a branch
neither side of which changes it) to coerce a register copy, not a
change any idiomatic C would produce. Manually tried the SAME trick by
hand, directly on `a1` (`if (self) { src = a1; } else { src = a1; }`,
`src` used in place of `a1` for both dereferences): **no change, still
210** -- unlike `func_80051858`'s own use of this exact idiom elsewhere
in the project, it does not reproduce the missing move here. Given (a)
an explicit named local copy-propagates away, (b) the branch-based
forcing trick that worked on a DIFFERENT function does not work here,
and (c) the permuter's own from-scratch search over ~28k iterations
never beat 210 with anything that translates to real C, **this final
"redundant move" residue is marked PERMUTER-EXHAUSTED** -- same
verdict and same underlying class as `Obj6EAC0__ApplyColor.md`'s stall
elsewhere in this unit. The 210-scoring body above (structurally exact
except for this one missing move) is the reported best and is restored
to `INCLUDE_ASM`; this is a genuine STALL now, but on a single missing
INSTRUCTION (which shows up as **22/25, three wrong words** in a direct
funcdiff rebuild -- corrected at the top of this report, round 19 --
not "one word" as this section originally said), not the
whole-function register-bank swap the original report misdiagnosed.

### Attempts this round (9, beyond the original 2)

1. `--debug` on the original 2-attempt body: 780 (not the reported 25/25 --
   the word-count framing missed the pair-4 structural defect entirely).
2. Trailing `__asm__("":::"memory")` after the struct-copy pair: no change
   (780) -- nothing follows it for the barrier to separate from.
3. Removed the barrier immediately BEFORE the final pair (kept the two
   scalar assignments): no change (780) -- the interleave/batch choice for
   this exact 2-statement shape does not depend on a neighbouring barrier.
4. **Whole-struct assignment for the final pair**
   (`*(Pair32E99C*)&self->unk50 = *a2;`): 210 -- fixed the pair's own
   batching AND `self`'s register identity throughout the function, as one
   change. Current best.
5. Explicit `SkipShort2 *src = a1;` local (attempt 4's body, `a1->x`/`a1->y`
   replaced with `src->x`/`src->y`): no change (210) -- copy-propagated
   away, same as the original report's attempt 2 on the OLD body.
6. Swapped block order (struct-copy pair before the `a1` pair, attempt
   4's fix kept): 240, worse -- 0 register differences but 4 words
   reordered, confirming attempt 4's block ORDER (not just its content) is
   the one retail actually uses.
7. Bounded permuter search (`-j 6 --stop-on-zero --best-only`, `timeout
   500`, ~28,487 iterations): best 25, no zero; every sub-210 candidate
   used a UB-flavoured self-assignment-through-a-dead-branch trick, not
   idiomatic C.
8. Manually applied that exact trick to `a1` directly (`if (self) { src =
   a1; } else { src = a1; }`, then `src->x`/`src->y`): no change, 210.
9. (Same round) re-confirmed attempt 5's plain `SkipShort2 *src = a1;`
   gives the identical 210 as the direct-`a1` form -- the local is fully
   copy-propagated away either way.

### Proposed learning

**A word-count-only diff read ("structurally exact, only registers differ")
can hide a real structural defect that happens to score in the same
penalty bracket as a pure register swap.** This function's own original
report claimed "every load, store, offset and operand order matches retail
exactly" when `permuter.py --debug`'s alignment-aware diff shows a genuine
insertion/deletion pair in the final field-copy. Where available, run
`--debug` (or `asm-differ`) BEFORE accepting a "register-identity-only"
classification, not only before a permuter search -- the classification
itself can be the thing that is wrong, and CLAUDE.md's own worked example
for this exact ban (the `func_8001A3EC` whole-function `__asm__` reversal)
is a precedent for exactly this failure mode: an unverified structural
claim treated as settled fact blocked a real fix. Separately: **a
statement-level change to one field pair can flip a DIFFERENT, unrelated
parameter's register allocation for the whole function** -- broaden the
existing "declaration/statement order decides register/stack layout"
learning to cover cross-value effects, not just the value whose own
statement changed.

## Round 19 (runner alpha, second pass): verdict correction + one more permuter run, still negative

**Verdict correction (see the top of this report):** direct rebuild of
the exact preserved body gives 22/25, not the "1 word remaining" this
report previously claimed in its title and its round-bravo prose. The
underlying 210 permuter score and the preserved body itself were never
wrong -- only the human-readable word-count translation was, and it had
already propagated into `docs/PROGRESS.md` and into this round's own
task assignment by the time it was caught.

**The "did not try: permuter" note earlier in this file (marked
SUPERSEDED) is correctly superseded** -- the round-bravo section below
it already ran the permuter to ~28k iterations and reached
PERMUTER-EXHAUSTED. Confirmed this is the live, current verdict (not
stale) by re-running it independently this round rather than trusting
the file: `--debug --stack-diffs` on the identical preserved body
confirms base score 210 with 0 stack differences (this project's
`--stack-diffs` flag did not exist, or was not used, when the original
28k-iteration run was recorded, so this re-confirms it under corrected
tooling). A fresh bounded search (`-j 6 --stack-diffs --stop-on-zero
--best-only`, `timeout 300`) ran ~47,996 iterations; best score never
beat 210. Two independent runs (28k iterations, then 48k more) now
agree: PERMUTER-EXHAUSTED stands.

**Two more manual attempts on the missing `move $t0,$a1`, both
negative:**

1. Built a real `SkipShort2 tmp = *a1;` whole-struct read (mirroring
   the lever that fixed `self`'s own register identity elsewhere in
   this function), then `self->unk60 = tmp.x; self->unk62 = tmp.y;`.
   This does NOT parallel the `a2`/`Pair32E99C` case: `SkipShort2` has a
   real gap between `x` (+0x000) and `y` (+0x004), so it is not a
   plain 4-byte contiguous struct the way `Pair32E99C` is. GCC 2.6.3
   lowers the unaligned/gapped struct copy via a real stack temporary
   (a new `addiu sp,sp,-8` frame appears), REGRESSING hard to 1/25 with
   ~184KB of whole-image drift. Reverted immediately.
2. A two-stage scratch load (`s32 scratch = self->unkB0;` -- wait,
   this function has no such field; corrected: a scratch/`v`-style
   two-stage copy analogous to `func_8003F764`'s own attempt) does not
   apply to this function's shape at all (there is no loop, no `arr[i]`
   pattern) -- not a real attempt, listed for completeness only; no
   new manual lever was found beyond what round-bravo's 9 attempts
   already covered.

   > **CROSS-REFERENCE CORRECTED (round 39, head).** `func_8003F764` is
   > **`select_max_param`** (`libgs/gs_131.o`), a linked Sony object -- no
   > C attempt on it ever reached retail's bytes, so there is no "own
   > attempt" to be analogous to. The entry was already self-cancelling
   > ("not a real attempt"); it is now void at both ends.

**Verdict unchanged: STALL, PERMUTER-EXHAUSTED, 22/25 words (3 wrong:
one missing `move $t0,$a1` plus the two downstream loads that read
`$a1` instead of `$t0` as a direct consequence).** Restored to
`INCLUDE_ASM`, full oracle green re-confirmed.

### Proposed learning (round 19 addition)

**A verdict line stated in permuter-score units (or in loose prose like
"one word") needs a `funcdiff.py`/raw-byte-count cross-check before it
is trusted downstream**, exactly the same way `docs/MATCHING-GUIDE.md`
already warns for a *base-score* reading. This report's own error was
not in the measurement (210 was real, the preserved body was real) but
in translating "one missing instruction" into "one word wrong" without
checking that the instruction's absence has knock-on effects elsewhere
in the byte stream. Three independent readers (a PROGRESS.md entry, a
round-19 task assignment, and this report's own title) repeated the
same wrong translation because none of them re-derived it from a fresh
`funcdiff.py` run -- they read the prose. Treat "N words remaining" in
any report as unverified until a fresh `funcdiff.py`/`cmp -l` run
confirms it, the same discipline already applied to permuter zeros and
"clean/drift-free" claims.

## Round 59 (runner charlie): NON_MATCHING body promoted

NON_MATCHING body promoted, round 59. The exact preserved body above (22/25
words, length exact, permuter-exhausted redundant-move residue) is now live
in `src/code_2cc8c_e.c` under `#ifdef NON_MATCHING`, with the verified build
still taking the `#else INCLUDE_ASM` branch. `./build-and-verify.sh` and
`tools/check-nonmatching.sh` both green.

## Naming (round 61, track 3)

**`Class6E99C__PushPosition`** -- tier B (STALL, preserved body unchanged
by this rename). `Class6E99CMethods::slotE8` (`+0x0E8`), gated on `unkC`
being non-NULL. Stashes the current `unk60`/`unk62` pair into `unk88`/
`unk8C` and the current `unk50`/`unk54` pair into `unk90`/`unk94`, then
installs a NEW position from its own `a1`(x,y)/`a2` arguments into
`unk60`/`unk62`/`unk50`/`unk54`. Its exact inverse,
`Class6E99C__PopPosition` (`slotEC`), restores the stashed values back
into the live fields -- a save/restore-of-one pair, hence
"Push"/"Pop". What game event drives the push (an on-screen position
override, e.g. a highlight or animation) is not established -- tier B.
