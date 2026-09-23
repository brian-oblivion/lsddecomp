# SeqPlay -- MATCHED (round 69, runner bravo): 69/69, byte-exact, whole-image SHA1 green

> Renamed from `func_80034138` on 2026-09-23 (tools/rename.py). Address 0x80034138.

**REVISITED, round 69: MATCHED; names/types not relevant** (a one-token
semantic fix in the body, plus the frame pad removed).

Rebuilt the `#ifdef NON_MATCHING` body exactly as preserved: length exact
69/69, 66/69 raw, funcdiff `insertions 1 / deletions 1`, positional
skeleton diffs 3. Diffs at words 22/23 (the `lh 0x70`/`lw 0x88` pair
swapped) and 27 (retail `move $a2,$v0` in the `blez` delay slot, this
build `nop`). That matches every earlier round.

### The fresh re-read: the "dead" delay-slot filler is a VALUE

Every round since 24 classed word 27 as a dead computation placed in a
delay slot, which is scheduling-only and unreachable. It is not dead.
Follow `$a2` along the branch-taken path. `blez $a0` goes to the
`delta <= 0` block (no `$a2` read). The fall-through goes to `lh v1,0x6E`,
then `bnez v1` (remain != 0 exits), then **`sh $a2, 0x6E($s1)`**. So the
`remain == 0` arm stores the value `move a2,v0` put there, and that value
is `rec->unk70`, the `last` local. The parameter `a2` is never read.
Retail's register allocator reused the dead parameter's register for
`last`.

The preserved body wrote `rec->unk6E = a2;` (the parameter). That kept
`last` in `$v0` only and gave the store the incoming `$a2`, so there was no
copy and the slot got a `nop`. Because `last` then had one fewer use, the
load pair was also scheduled differently. **`rec->unk6E = last;` fixes
both residues at once**: 0 insertions / 0 deletions. The only remaining
difference was the frame, `-0x40` vs `-0x38`. Round 24's `s32 dead[2]` pad
had been compensating for the wrong body. Removing it gave **69/69,
`build exit=0`, whole-image SHA1 OK.**

Scorer-normalisation check (PARALLEL-RUNS §3.5 row 3): not needed. The
residue was a real semantic difference, not something the scorer
normalises. Builds this session on this function: 3. No permuter search.

### Proposed learning

A delay-slot `move $aN, $vM` counts as a "dead filler" only after you trace
`$aN` to its next READ on EVERY path, not just to the next call. Here it
was read by a store two blocks later, so the source had used the wrong
value (`param` for `local`). DECOMPILATION_LEARNINGS 3f's argument-register
rule covers call arguments; the same trace applies to a dead PARAMETER's
register, which 2.6.3 reuses freely. Four rounds, a head adjudication and a
55027-iteration permuter search missed this because they all assumed
"scheduling". A permuter cannot find it, because it never swaps which
variable a statement stores.

---

## History (superseded -- the stall verdict below is retired by the match above)

Old title: SeqPlay -- STALL: length EXACT 69/69; 66/69 raw word-match; first real diff at word 22

> **HEAD ADJUDICATION, round 24 (2026-09-08). Classification CONFIRMED. The
> one axis this report left "not attempted" was attempted by the head and is
> a CLEAN NEGATIVE.**
>
> The report ends the delay-slot discussion with *"No source-level construct
> is known to force a specific dead computation into a specific delay slot;
> not attempted, flagging as the class rather than guessing at a lever."*
> That was the right call to make and the wrong one to leave open, because
> there IS a construct the project permits for exactly this: a bare
> `__asm__("")`, which CLAUDE.md allows precisely because it changes
> instruction ORDER and not register identity. Runner echo closed a
> near-identical two-word deferral with one this same round.
>
> **Measured here: it does nothing.** A bare `__asm__("")` placed between the
> two loads -- with the initialisers split into declaration-plus-assignment
> so the barrier could sit between them -- produced output **byte-identical**
> to the body already preserved below: still 66/69, the load pair still
> swapped (`lw v1,0x88` before `lh v0,0x70`), and the `move a2,v0` delay slot
> still a `nop`. Both residues are unmoved.
>
> Two things follow, and the second is the more useful:
>
> - This function's 3 words are not reachable by the barrier lever. With
>   declaration order already ruled out by alpha, the remaining axes are the
>   permuter or a structurally different rendering of the loop -- not another
>   hand-placed barrier.
> - **The barrier lever's SCOPE is now bounded, and that matters because
>   echo's success with it this round could otherwise read as general.** It
>   closed a case where a value's computation was being DEFERRED past where
>   retail computed it. It did NOT reorder an adjacent load pair, and it did
>   NOT install a dead computation into a delay slot. Those are different
>   asks, and only the first is what the lever does.

`asm/nonmatchings/code_179d8_k/SeqPlay.s`, vram `0x80034138`, unit
`code_179d8_k`. Round 24, runner alpha. Continuation past this unit's
assigned six functions.

## What it is

A per-channel/slot sequencer "tick" function -- the caller of both
`ReadDeltaValue` (indirectly, via `rec->unk88`) and a sibling helper,
`GetSeqData` (still `INCLUDE_ASM` in this unit as of this report, so
its own signature/behaviour is an educated guess, not established fact).
Two new fields on this unit's `Entry90902E8` local struct view:

- `unk6E` (s16): a repeat/skip counter, decremented once per call while
  positive.
- `unk70` (s16): a scheduling threshold compared against `unk88`
  (the running tick position already established by the rest of this
  unit's family).

```c
void SeqPlay(s16 a0, s16 a1, s16 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    s32 dead[2];
    s16 last = rec->unk70;
    s32 elapsed = rec->unk88;
    s32 delta = elapsed - last;
    s16 remain;
    s32 sum;
    s32 step;
    s16 last2;

    if (0) {
        dead[0] = 0;
        dead[1] = 0;
    }
    if (delta > 0) {
        remain = rec->unk6E;
        if (remain > 0) {
            rec->unk6E = remain - 1;
            return;
        }
        if (remain == 0) {
            rec->unk6E = a2;
            rec->unk88 = rec->unk88 - 1;
            return;
        }
        rec->unk88 = delta;
        return;
    }
    if (last < elapsed) {
        return;
    }
    sum = elapsed;
    for (;;) {
        GetSeqData(a0, a1);
        step = rec->unk88;
        if (step != 0) {
            last2 = rec->unk70;
            sum += step;
            if (sum < last2) {
                continue;
            }
            rec->unk88 = sum - last2;
            break;
        }
    }
}
```

## Two things worth recording that a naive read of the disassembly gets wrong

1. **The third parameter (`a2`) is genuinely read, but only on ONE narrow
   path, and it is NOT the same value as the locally-computed `last`
   that momentarily shares its register.** Retail's delay slot at the top
   (`blez a0,...` / delay `addu a2,v0,zero`) reassigns the PHYSICAL
   register `$a2` to hold `last` (`rec->unk70`'s value) on the "behind
   schedule" path -- but that path never reads `$a2` again. The path that
   DOES read `$a2` (`remain == 0`) is only reachable through the OTHER
   branch (`delta > 0`), where `$a2` still holds the caller's original
   third argument, untouched. Reading the disassembly literally (as if
   `$a2` held one consistent value throughout) leads to writing
   `rec->unk6E = last;` in the `remain == 0` case, which is wrong -- it
   should be the caller's parameter, `rec->unk6E = a2;`. This is a
   register-reuse-across-unrelated-branches artifact, not a shared
   source-level variable.
2. **A second `if (cond)` that is provably unreachable, given the first
   branch's negation, is still compiled as a real branch.** After the
   `delta > 0` branch returns in every sub-case, the remaining code
   re-checks `last < elapsed` -- which cannot be true given `delta <= 0`
   (i.e. `elapsed <= last`) was the only way to reach this point. GCC
   2.6.3 does not eliminate this: it has no cross-branch knowledge that
   the two conditions are related, so it emits the redundant `slt`/`bnez`
   pair exactly as if the source had written two independent
   comparisons (which it very likely does -- see `SsUtSetDetVVol`'s
   caution in `DECOMPILATION_LEARNINGS.md` about not second-guessing
   what the compiler chooses to keep). Do not "simplify" this away when
   transcribing; the redundant check IS the retail shape.

## Levers that mattered

- **A dead 2-word stack array under `if (0)`** (this project's
  established idiom for forcing frame size onto an otherwise-undersized
  body) closed an 8-byte frame-size gap (0x30 vs retail's 0x38) that,
  left unfixed, shifted every `$sp`-relative offset in the whole function
  and made 22 words look wrong that were not. Declared genuinely dead
  (`if (0) { dead[0]=0; dead[1]=0; }`) so no side effect exists; C89
  requires it declared before the first statement, which is why it sits
  with the other locals rather than next to the `if (0)` block itself.
- **Which of two logically-related conditions is spelled as the
  POSITIVE `if` arm decides which code block falls straight through
  after the shared guard, and which is reached by an explicit jump.**
  The first attempt wrote `if (remain != 0) { ...; return; } rec->unk6E
  = a2; ...` (49/69, with an inverted branch polarity and the wrong
  block laid out first). Swapping to `if (remain == 0) { ...; return; }
  rec->unk88 = delta; return;` -- putting the SAME condition retail's
  fallthrough favors first -- moved the branch polarity and block layout
  to retail's exact shape in one rebuild (49/69 -> 66/69). Same family as
  the project's already-recorded "a shared return block's placement
  follows the FIRST return in source order" finding.

## The residue that did not close (3 words, both pure scheduling)

```
24990: TARGET lh v0,0x70(s1)      CURRENT lw v1,0x88(s1)      )  same two loads,
24994: TARGET lw v1,0x88(s1)      CURRENT lh v0,0x70(s1)      )  swapped order
249a4: TARGET move a2,v0          CURRENT nop                    delay-slot filler
```

The load-pair swap is the SAME two loads this function needs regardless
of source order (`rec->unk70` and `rec->unk88`); tried declaring `last`
before `elapsed` and `elapsed` before `last` -- both produced the
IDENTICAL swapped order relative to retail, so this is a scheduling
preference invariant to declaration order, not a source-order lever.
Not re-attempted further.

The `move a2,v0` is retail filling a delay slot (the `blez a0,...`
branch's slot) with a computation that, per the analysis above, is never
actually CONSUMED on that control-flow path -- it is dead-but-scheduled
filler, likely because the compiler had nothing better to put there and
this register happened to be free. No source-level construct is known to
force a specific dead computation into a specific delay slot; not
attempted, flagging as the class rather than guessing at a lever.

## Verification

`./build-and-verify.sh` build exit=0 with `SeqPlay` restored to
`INCLUDE_ASM` (near-miss body preserved above and in `src/` as `#if 0`).
`funcdiff.py SeqPlay` against the near-miss build (prior to
reverting): 66/69 words match, compiled length exact.

### Proposed learning

The "redundant re-derivation of a logically-implied condition" pattern
(item 2 above) may be worth a `DECOMPILATION_LEARNINGS.md` entry of its
own if it recurs: GCC 2.6.3 does not eliminate a second `if` whose
condition is the negation of an already-taken branch's guard, so an
apparently-dead check in the disassembly can be exactly what the source
wrote, not a sign of a misread branch.

## Round 31 update (runner bravo): re-verified, delay-slot filler confirmed genuinely inert

Rebuilt the preserved near-miss body and reproduced the title figures
exactly: 66/69, length exact, first real diff at word 22 (the swapped
`unk70`/`unk88` load pair), plus the `move $a2,$v0` delay-slot residue.

Checked this function's delay-slot filler against `ContNrpn2.md`'s
round-31 finding that a "dead" delay-slot filler can actually be a hidden
UNCONDITIONAL write the stalled C mis-scoped to one branch arm only.
**Does not apply here**: `addu $a2,$v0,$zero` (disassembly line 34,
`asm/nonmatchings/code_179d8_k/SeqPlay.s`) is a bare GPR-to-GPR
move with no memory effect. Unlike `ContNrpn2`'s `sb` store (which
persists past the branch and is observable from other code paths or a
later read of the same struct field), a register move that is never read
again cannot have a hidden effect on ANY path -- there is nothing further
downstream that could observe it, taken or not. This report's own
original derivation already established the same conclusion by testing
an equivalent explicit dead statement with zero effect. No new axis
found this round; not re-attempted with `decomp-permuter` given the
budget went to the higher-yield near-miss cluster instead (see
`ContPortaTime.md`, `ContModulation.md`, `ContPortamento.md`,
`SetPitchBend.md`, `ContNrpn2.md` for this round's five matches, and
`GetSeqData.md`/`NoteOn.md` for the round's other two
register-identity re-verifications). This remains a genuine
register-identity/scheduling STALL per `CLAUDE.md`'s explicit rule.

## Round 32 update (runner bravo): re-verified; volatile and permuter both negative

Rebuilt the preserved near-miss body and reproduced the title figures exactly:
66/69, length exact, first real diff at word 22 (the swapped `unk70`/`unk88`
load pair) plus the `move $a2,$v0` delay-slot filler.

**Lever 1 (narrow `volatile`), tried and NEGATIVE.** Marked only the `unk70`
read `volatile` (`s16 last = *(volatile s16 *)&rec->unk70;`), the scalpel
version of round 24's already-inert bare `__asm__("")` barrier. It is
*not* neutral the way the barrier was -- it is actively worse: `unk70` is a
sub-word (`s16`) field, so `volatile` turns retail's plain `lh` into an
`lhu` widen-pair the same way `DECOMPILATION_LEARNINGS.md`'s "narrow a
`volatile`" entry warns for exactly this shape, and it drops the score to
23/69 with 263815 bytes of whole-image drift. Reverted immediately. Combined
with round 24's barrier result, both the strong (barrier, fences scheduling
entirely) and narrow (volatile, fences one access) forms of this lever are
now confirmed inert-or-worse for this specific load-pair-swap residue.

**`tools/decomp-permuter`, never run against this function before, tried and
NEGATIVE against the real oracle.** `--debug --stack-diffs` reproduced this
report's own residue exactly (base score 260 = 1 reordering + 1 insertion + 1
deletion, matching "two pure scheduling residues"). A 240s `-j 6
--stop-on-zero --best-only` search found a local-best candidate at score 35
(down from 260) that introduced a redundant second pointer alias
(`Entry90902E8 *new_var = rec;`, used for every access after the guard) and
deleted the named `delta` local (recomputing `elapsed - last` inline
instead). Translated to real C and verified through the full oracle in two
separate steps:

- Deleting just `delta` (keeping one `rec` pointer): **inert**, still exactly
  66/69 -- confirms this component of the permuter's local improvement does
  not transfer.
- Adding the second pointer alias `rec2` on top: **regressed to 63/69**, a
  real oracle-verified WORSE result despite scoring better against the
  permuter's own local metric. Textbook instance of
  `docs/MATCHING-GUIDE.md`'s "a permuter score drop is a LEAD, never a
  RESULT" caution -- reverted immediately.

Both experiments left the function unchanged at 66/69; `INCLUDE_ASM`
restored, `build-and-verify.sh` re-confirmed byte-exact.

### Round 32 lever checklist (this function)

- **Lever 1 (narrow `volatile`)**: tried, **NEGATIVE** (actively worse than
  the already-inert barrier -- sub-word width makes it worse than neutral).
- **Lever 2 (register-identity verdict is a hypothesis)**: not directly
  applicable -- this residue was never filed as register-identity; it is a
  load-scheduling-order-plus-delay-slot-filler class, and remains so.
- **Lever 3 (emission order != source order)**: not implicated; declaration
  order of `last`/`elapsed` was already ruled out in round 24 (both orders
  give the identical swapped load order).
- **Lever 4 (permuter negative is evidence about one search)**: applied --
  ran a fresh search this round rather than trusting round 24's absence of
  one, and it did produce a real (if net-negative) lead worth recording.
- **Lever 5 (asm-differ/permuter compare text)**: not implicated; the two
  swapped loads (`lh`/`lw`) are different opcodes, not an `addiu`/`ori`
  ambiguity.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either lever explicitly checked, does NOT apply (already covered by round 24)

Rebuilt the preserved near-miss body: reproduces exactly, 66/69, length
exact, first real diff at word 22 (the swapped `unk70`/`unk88` load pair)
plus the `move $a2,$v0` delay-slot filler.

This round's headline lever is "hoist BOTH values before EITHER is
consumed, when retail's two loads are ADJACENT and their consumers come
LATER." Checking the diagnostic against this function's own residue:
retail's two loads (`lh $v0,0x70($s1)` / `lw $v1,0x88($s1)`) ARE adjacent
and their one shared consumer (`subu $a0,$v1,$v0`) DOES come later --
textbook shape for the lever. **But this function's C already hoists both
`last` (`unk70`) and `elapsed` (`unk88`) into named locals before either is
consumed** (`s16 last = rec->unk70; s32 elapsed = rec->unk88; s32 delta =
elapsed - last;`), which is exactly what the lever prescribes. Round 24
already tried BOTH declaration orders (`last` before `elapsed` and the
reverse) and got the IDENTICAL swapped load order either way -- i.e. this
function was already sitting in the "both hoisted" state the lever asks
for, from six rounds before the lever was named, and the specific
instruction ORDER within that hoisted pair is what resists (a scheduling
preference invariant to declaration order, confirmed independently by
round 24's own test). **The lever does not move this residue because
there is no un-hoisted state to fix here** -- the diagnostic's precondition
was already satisfied without closing the gap, which is itself informative
about the lever's limits: "adjacent loads, later consumer" describes the
SHAPE hoisting produces, not a guarantee hoisting produces retail's
specific instruction order once achieved.

No new experiment run this round; per the 30-attempt/round-40-attempt
history already on this function (rounds 24, 31, 32), continuing to press
manually on this specific 3-word residue without a new axis is not a good
use of this round's budget. `INCLUDE_ASM` unchanged, `build-and-verify.sh`
confirmed byte-exact.

## Round 47 (runner echo): re-verified 66/69 in isolation; this round's `do { return; } while (0)` lever tried, NEGATIVE (regresses hard, real drift)

Rebuilt the preserved body verbatim in isolation first: `build exit=2`, no
compile errors, `funcdiff.py` reports 66/69, no staleness warning, first
real diff at word 22 -- matches every prior round's figure exactly.

This same round's `NoteOn.md` found that wrapping a single-
statement early `return;` in `do { return; } while (0);` is not always a
no-op for GCC 2.6.3 -- it closed 1 real word there by perturbing register
allocation elsewhere in the function. Tried the same wrapper here, on all
four of this function's `return;` statements at once (the three inside
`if (delta > 0)`'s sub-branches plus the `if (last < elapsed) return;`
guard immediately after).

**Result: hard regression, not neutral.** `1/69` words match with
`build-and-verify.sh` showing **210365 bytes of whole-image drift** -- the
function's compiled LENGTH itself changed, unlike `NoteOn`'s clean
same-length improvement. Reverted immediately; `git status --porcelain`
confirmed clean, `build-and-verify.sh` re-confirmed byte-exact after
revert.

**Not a like-for-like test of the other function's lever, and worth
saying why**: `NoteOn` wrapped exactly ONE return, in a function
whose residue was a pure register rotation with no length change
possible. This function's four returns sit across three different
control-flow depths (nested inside `if (delta > 0)`, and a sibling
top-level `if`), and wrapping all four at once is a much bigger,
uncontrolled perturbation -- entirely plausible that ONE wrapped return
here would behave differently from all four together, but that variant
was not isolated this round (this function's residue -- a load-pair
scheduling order plus one delay-slot filler, both confirmed pure-
scheduling by rounds 24/31/32 -- sits in the function's PROLOGUE, before
any of the four returns are reached, so there is no obviously-adjacent
single return to target the way `NoteOn`'s residue sat right next
to its own early return).

**No new attempt beyond this one negative.** Given this function's
history (barrier negative round 24/confirmed by head, narrow-`volatile`
negative round 32, permuter negative round 32 with a rejected spurious
lead, hoist-both-before-either N/A round 39, and now this round's
do-while(0) wrap also negative), this residue has now had five
independent axes tried and failed. `INCLUDE_ASM` unchanged, still 66/69.

### Proposed learning

The `do { return; } while (0)` lever (`NoteOn.md`, this round) is
NOT a blanket "always worth trying on any early return" move -- applying
it to FOUR returns at once, in a function whose actual residue sits in
the prologue rather than adjacent to any of those returns, produced a
real length regression rather than a neutral-or-improving change. The
lever needs to be tried ONE return at a time, ideally the one nearest
(in control-flow terms) to the residue, not applied wholesale across
every return in a function on the strength of a single other success.

## Round 49 update (runner charlie): first genuine permuter search (55027 iterations, rc=124) -- no zero, best candidate real-oracle-verified NEGATIVE

This function was flagged in this round's work order as "NEVER
permuter-searched (0 iterations on file)". That is not quite accurate --
round 32 already ran one 240s search against an older seed framing and
found a net-negative lead -- but this round's search is the longest and
freshest one on file, so it is recorded in full per this round's
broadcast discipline (check 3 outcome, iteration count, rc, every
negative real-oracle-verified before being trusted).

Rebuilt the preserved near-miss body in isolation first: `build exit=2`,
no compile errors, `funcdiff.py` reports 66/69, no staleness warning,
length exact, first real diff at word 22 -- matches the title and every
prior round exactly.

**Check 3, run fresh:** built a new scaffold from this report's own
preserved body (`tools/setup-permuter.sh SeqPlay <seed>`); base
compiles. `--debug --stack-diffs`: **base score 260** -- `Stack
Differences: 0`, `Branch Differences: 0`, `Register Differences: 0`,
`Reorderings: 1`, `Insertions: 1`, `Deletions: 1`. This is an EXACT match
to round 32's own scaffold base score and penalty breakdown (1 reordering
+ 1 insertion + 1 deletion = "two pure scheduling residues"), i.e. the
AGREE case. Searched with confidence.

**Search: `timeout 900 ... -j 6 --stop-on-zero --best-only`, confirmed
exit via the 900s bound (`rc=124`, written to its own file per this
round's logging discipline). 55027 iterations.** Four candidates beat the
base score during the run (120, 135, 155, 200, all found within the first
five minutes; none improved further over the remaining ~10 minutes):

- **Score 120 (the best found)**: wraps `rec->unk6E = remain - 1; return;`
  in `if (a2) { ...same... } else { ...same... }` -- a behaviorally-inert
  dead branch (`a2` is read on this path, but both arms do the identical
  thing, so program behavior cannot change). **Translated to real C and
  verified through the full oracle: hard regression, 57/69 (down from
  66/69), with the mismatch starting at word 1 (the very top of the
  prologue), not confined to the target residue at word 22.** Reverted
  immediately; `build-and-verify.sh` reconfirmed byte-exact after revert.
- **Score 135 and 200**: both retype the `last` read (`s16 last =
  rec->unk70;`) -- one via an intermediate `unsigned char new_var = last;`
  used in the comparison, the other via `unsigned long last = rec->unk70;`
  outright. Both silently change the sign/width semantics of a genuinely
  signed 16-bit field (`unk70`, documented as a scheduling threshold that
  can plausibly take values that read differently unsigned or truncated
  to a byte) -- textbook instances of round 48/delta's forward-trace
  screen ("the scorer never executes candidates, so a value read under a
  changed meaning still scores as an improvement"). Not built; disqualified
  by inspection.
- **Score 155**: the same `if (a2) {X} else {X}` duplication as the
  score-120 candidate, PLUS a second, similarly-inert `if (a0) { sum =
  elapsed; } else { sum = elapsed; }` duplication near the `last <
  elapsed` guard. Given the single-duplication score-120 variant already
  regressed hard and the residue this function actually needs to move
  (the `unk70`/`unk88` load-pair order and the `move $a2,$v0` delay-slot
  filler) sits in the PROLOGUE, well before either duplication site, not
  built -- would only compound the same kind of unrelated perturbation.

**Disposition: not closed, and the search's own best candidate makes
things WORSE against the real oracle, not just "no better".** This is a
new, sharper instance of round 32's own permuter finding on this same
function (a rejected pointer-alias candidate that also scored well
locally and regressed 66/69 -> 63/69) -- the permuter's local byte-diff
metric and this function's real residue are apparently poorly correlated
for this specific prologue-scheduling residue, across two independent
searches now (round 32's 240s and this round's 900s). Not marking
permuter-exhausted (per this project's standing rule, a negative is
evidence about the searches run, not a certificate against a
differently-framed future one), but this residue has now had six
independent axes tried and failed: barrier (round 24), narrow `volatile`
(round 32), permuter (round 32, this round), hoist-both-before-either
N/A (round 39), do-while(0) wrap (round 47). `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact throughout (both regression
attempts reverted before this report was written).

### Proposed learning

A permuter candidate that only duplicates a branch into two identical
arms is BEHAVIORALLY safe (both arms are the same code, so no UB/logic-bug
risk the way a reassigned-then-read-under-old-meaning candidate carries),
but "behaviorally safe" is not the same as "codegen-neutral or better" --
this function's best-scoring safe candidate still regressed the real
build by 9 words with drift starting well outside the mutation site
itself. Local permuter score and real-oracle byte count can point in
opposite directions even for a mutation with zero semantic risk; the
"verify every candidate through the real oracle" rule needs to be applied
to inert-looking dead-branch duplications just as strictly as to
suspicious retype/widen candidates.

**NON_MATCHING body promoted, round 66** (runner charlie).
