# FlagLargePolyForDivide — MATCHED (70/70, round 75): lever = LOOP KIND (guard + do/while with `yp` set inside the guard) + `end = self + (count << 2) + 0x5C`

> Renamed from `UpdatePolyBBoxAndCull` on 2026-09-26 (tools/rename.py). Address 0x8001a268.

> Renamed from `func_8001A268` on 2026-09-24 (tools/rename.py). Address 0x8001a268.

REVISITED, round 75: MATCHED; names/types not relevant (source shape only).

## ROUND 75 (bravo): MATCHED

Previous title: "FlagLargePolyForDivide — STALL: length EXACT (70/70 words, no drift); 53/70 raw word-match; first real diff at in-range word 16 (file 0xAAA8 / vram 0x8001A2A8), the stack-frame-adjustment placement residue".

**Preserved body rebuilt first** (the `#ifdef NON_MATCHING` body compiled
live): 53/70, `insertions 2 / deletions 2 (positional skeleton diffs 17)`.
asm-differ showed two moved instructions: `addiu $sp,$sp,-0x20` at word 0
instead of in the loop-skip `beqz`'s delay slot, and `addiu $a1,$a1,0x5c`
before the `addu` instead of after it.

**The "unreachable from C" diagnosis was wrong.** The frame adjustment is
an ordinary RTL insn that reorg moves into a delay slot like any other.
`fill_simple_delay_slots` scans BACKWARD from the branch and takes the
nearest insn that does not conflict with anything it has skipped. In the old
`for` body, `yp = self + 0x66` was computed before the loop test, so it was
the nearest movable insn and our build put `addiu $t0,$a3,0x66` in the slot.
Retail sets `yp` only in the loop preheader (after the branch). With it gone
from before the branch, everything between the branch and the prologue
conflicts through `$v0`/`$a1`/`$a2`, and the first movable insn reorg finds
is the prologue's sp adjust. The round-39/44/48 claim that the prologue is
placed "before any C statement is scheduled" and that a source search
"structurally cannot reach" this residue was a guess about the mechanism.
It had never been tested against a body whose preheader differed.

| body | score |
| --- | --- |
| preserved `for (; xp < end; xp += 2, yp += 2)` body | 53/70 (ins 2 / del 2) |
| `if (xp < end) { yp = self + 0x66; do { ...; xp += 2; yp += 2; } while (xp < end); }` | 62/70 (ins 1 / del 1): sp adjust now in the delay slot; remaining is `end`'s add order plus the `$a1`/`$t0` swap between `end` and `yp` that follows from it |
| same + `end = (s16 *)(self + (count << 2) + 0x5C)` | **70/70**, whole image `OK: build matches retail` |

The `end` spelling alone had been tried before (the "Attempts" section's attempt 3) and
recorded as a regression to 52/70. That measured it on the `for` body,
where it could not help. On the do/while body it is the last piece.
Equivalent spellings `self + count * 4 + 0x5C`,
`(s16 *)(self + (count << 2)) + 0x2E` and `(s16 *)((s32 *)self + count + 0x17)`
are also 70/70. Only `self + 0x5C + (count << 2)` puts the constant first
and misorders.

### Proposed learning

**An `addiu $sp,$sp,-N` in an early branch's delay slot is reorg, not the
prologue generator, and it is reachable from C.** Reorg fills a delay slot
with the NEAREST preceding insn that is free of conflicts. If your build puts
the sp adjust at word 0 and something else in that slot, find what the
something else is (usually a loop-preheader value such as a second cursor
that your loop kind computes before the test) and move it after the test:
guard + `do/while` with the value assigned inside the guard. Tell: compare
what occupies the delay slot in your build against retail's. Whatever is
there instead of the sp adjust is the insn to move.
`docs/DECOMPILATION_LEARNINGS.md`'s "SCOPE" paragraph that cites this
function as unreachable from C should be corrected. Also: a per-lever
negative measured on one loop shape (attempt 3's `end` order) does not carry
over once the loop shape changes.


NON_MATCHING body promoted, round 65.

## ROUND 65 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. This function had no standing `#if 0` block
in `src/code_8220_c.c` (noted in rounds 39/44 — it was only ever flipped in
live temporarily for reproduction). Took the "Best body reached (53/70
words)" snapshot below verbatim and placed it, wrapped in
`#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif`, in the unit at its
existing ROM position (immediately before `FillDivPolygonHeader`). The local
`Vec2s16` typedef was renamed `Vec2s16_268` to follow this unit's existing
per-function disambiguation convention (`Vec2s16_98`/`_C04`/`_EE4`/`_A64`
already exist for the sibling family) and avoid any future name collision
in the same translation unit — cosmetic only, no semantic change. Six
structurally distinct hand attempts plus an 8600+-iteration permuter search
(both on file below) make this hand-derived, not a permuter candidate.
`./build-and-verify.sh`: `build exit=0`, `OK: build matches retail`.
`tools/check-nonmatching.sh code_8220_c`: `OK`. No stale symbol references
(this function calls no Psy-Q SDK function).

## ROUND 48 (bravo): search provenance checked (this function DOES have its own real search, unlike 6 siblings), no new search staffed

My brief listed this function as having "none recorded" for prior search
depth, alongside `SubmitPolyG3`, and asked to verify whether either was
ever really searched. For this function that premise is WRONG and worth
correcting explicitly: this report's own "RUNNER PASS, permuter round"
section already records a REAL, dedicated, ~8600+-iteration blind search
(two bounded runs), not a cross-reference to a sibling's — it found a
genuine sub-baseline candidate (base 120 -> 60), verified it against the
live oracle, and it was FALSE (52/70, one word worse than the 53/70 this
report already carries). That is the opposite situation from
`SubmitPolyG4`/`SubmitPolyGT3`/`SubmitPolyFT4`/`SubmitPolyFT3`/
`SubmitPolyF4`/`SubmitPolyGT4`, whose "40000 iterations" in my brief's
table belongs to the family ROOT (`SubmitPolyF3`) only, per round 41's
own correction — see the broadcast post and those six functions' round 48
entries.

**No new search staffed this round, and here is why that is not
laziness.** This function's residue is NOT the family's shared
register-identity/OT-mask class at all — see the title line and the
"Residue" section below: it is a pure stack-frame-adjustment PLACEMENT
artifact (`grep -n '($sp)'` on the function's own `.s` returns nothing in
either version; the reserved 0x20 bytes are never used for storage).
`docs/DECOMPILATION_LEARNINGS.md`'s "SCOPE — measured, and it is narrower
than the paragraph above implies" section (search `FlagLargePolyForDivide` there)
is ABOUT this exact function and generalises the finding: **a raw
`$sp` adjustment with zero data dependency can be scheduled by GCC 2.6.3
into an early branch's delay slot, and this placement is emitted by the
function-PROLOGUE GENERATOR strictly before any C statement is scheduled
— including a barrier statement placed first.** A permuter search mutates
C source; it cannot reach an instruction GCC emits before it starts
scheduling C statements at all. That is a stronger claim than "not yet
closed by N iterations" — it is a mechanism-level reason no amount of
further blind search over this function's OWN C shape can move this
residue, independent of the 8600-iteration run already on file finding
nothing better than a falsified lead.

Re-verified the preserved body in-tree this round anyway (Check 3, full
oracle): `build exit=2`, zero compile-error hits, **53/70, identical diff
to every prior round** (`git diff --stat` empty after revert). Time went
to the six siblings that had never had a real per-function search instead
(`SubmitPolyG4`, `SubmitPolyGT3`, `SubmitPolyFT4`, `SubmitPolyFT3`,
`SubmitPolyF4`, `SubmitPolyGT4` — see their own round 48 entries) plus
`SubmitPolyG3` (see that report — DOES need a fresh full search, unlike
this one).

### Proposed learning

**"Has this function had a real search" and "would a real search be able
to reach this residue" are two different questions, and my brief's table
conflated them by using the same "none recorded" label for both.**
`FlagLargePolyForDivide` had a real search AND a residue a search structurally
cannot reach (prologue-generator placement, pre-C-statement). `SubmitPolyG3`
had NO real search and a residue nothing yet rules unreachable. Screening
"never searched" functions for search priority should also ask whether the
residue's own diagnosis (absent-instruction vs. substitution, prologue vs.
statement-level) makes a source-mutation search structurally pointless
before spending the 900s on it.

## ROUND 44 (alpha): rebuild-reconfirmed (fifth independent reproduction), not re-attempted

Inserted this report's own "Best body reached" snapshot live (over
`INCLUDE_ASM`, since this function has no standing `#if 0` block in
`src/code_8220_c.c`), ran the full oracle in isolation, reverted: `build
exit=2`, zero compile-error hits, **53/70 words, identical diff to every
prior round** (first diff word 16, `retail=22004010 built=2b10c500`).
`git status --porcelain` empty after revert. (`funcdiff.py` printed its
stock "still INCLUDE_ASM" warning because the file's `#else` branch still
contains that literal text after the revert-in-place edit was undone within
the same command sequence — a text-scan false positive, not a stale build;
the diff figures themselves are the live rebuild's own output and match
this report's history exactly.)

Given six prior structurally distinct attempts plus an 8600+-iteration
permuter search already on file (below), and this round's broadcast
confirming the `gp_rel`/`nop_mflo_mfhi` fixes are unrelated to this
function's residue (it carries neither construct), no new attempt was made.
Time went to the family's other six near-misses instead, all reconfirmed
this round at their recorded figures (see `SubmitPolyF3.md` and siblings).

Rebuilt this function's preserved body (the report's own "Best body reached"
snapshot, inserted live over `INCLUDE_ASM` since this function has no
standing `#if 0` block in `src/code_8220_c.c` — it was never left live
there) and ran the full oracle in isolation: `build exit=2`, zero
compile-error/`undefined reference` hits.

**Result: 53/70 words, byte-identical to every prior round's figure. First
diff at word 16 (file 0xAAA8, vram 0x8001A2A8): `retail=22004010
built=2b10c500`**, continuing at word 17 (`retail=e0ffbd27
built=21004010`) — the `addiu $sp,$sp,-0x20` scheduled into the
loop-skip branch's delay slot in retail vs. an ordinary prologue placement
here, exactly the residue rounds 20/21/32/38/39 already characterize.
`git diff --stat` empty after revert.

**Job 2 (attempt a match): assessed, not attempted.** This function's own
report already records 6 structurally distinct hand attempts and a dedicated
~8600+-iteration permuter search (round-alpha, "RUNNER PASS, permuter round"
section) that found and then oracle-FALSIFIED its one candidate improvement.
The residue's mechanism is pinned down precisely: `grep -n '($sp)'` on this
function's own `.s` returns nothing in either version (confirmed again this
round), meaning the reserved 0x20 bytes are never used for storage by
retail either — the frame adjustment is a pure, data-independent
prologue-generator emission that GCC 2.6.3 schedules into an early branch's
delay slot purely because nothing forces it to stay at word 0. Round 38
already established the `s32 unused[N]` lever's precondition ("build emits
NO `addiu $sp` at all") does not hold here (this build already reserves
`-0x20` unprompted) and that forcing it regresses (53/70 -> 52/70); round 21
already established the `__asm__("")` barrier cannot act on an instruction
the C-statement scheduler never sees (it is emitted by the prologue
generator, strictly before any C statement including a barrier placed
first). No axis remains identified, in three separate reports' own closing
sections, that has not already been tried and found either inert or
regressing. Re-running any of the above would reproduce an existing
negative, not test anything new — see the family-wide summary in
`SubmitPolyF3.md`'s round 40 section for why this is being called
explicitly rather than silently skipped.

**Title rebuilt to the three-figure CLAUDE.md format.** Classification
unchanged.

## ROUND 39 (bravo): rebuilt-verified only, unused-frame lever NOT retried per round 38's explicit measurement

Flipped the preserved body below to live, ran the full oracle, reverted:
**53/70, byte-identical diff to every prior round's report, no drift.**

Round 38 (head) already measured the "obvious" next step here — declaring
`s32 unused[N];` to reproduce retail's reserved-but-unused `0x20`-byte
frame — directly against this function and found it REGRESSES (53/70 ->
52/70), because this build already reserves `0x20` bytes with no C asking
for it; the array only stacks a second reservation on top. Confirmed that
finding still holds by checking the built object before touching anything:

```
$ tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/code_8220_c.c.o \
    | awk '/<FlagLargePolyForDivide>:/,/^$/' | grep 'addiu.*sp,sp'
addiu   sp,sp,-32
```

A line IS present (`-32` = `-0x20`), confirming the lever's precondition
("no `addiu $sp,$sp,-N`") does not hold and the lever is inert here, exactly
as round 38 found. Not retried. This round's two other candidate levers
(the new `gte.h` macro layer; hoist-both-before-either) don't apply to this
function's residue either, for reasons unrelated to the frame lever: this
function's residue is pure INSTRUCTION PLACEMENT of the stack-frame
adjustment itself (moved into a branch's delay slot), not a register
identity choice and not a missing/misordered pair of VALUE computations —
see "Residue" below, unchanged this round. No new attempt made.

Unit: `src/code_8220_c.c`. `void FlagLargePolyForDivide(void *arg0, s32 count)` —
computes `arg0`'s 2D bounding box over `count` vertices: seeds min/max
(fields `+0x70`/`+0x72`/`+0x74`/`+0x76`, X/Y min/max as `s16`) from a
2-`s16` value at `+0x60`, walks `count` vertices starting at `+0x64`
(X) / `+0x66` (Y), each 4 bytes apart, updating the running min/max, then
sets `arg0->0x78` (the same "culled" flag `TransformAndCullPoly`/`SubmitPolyF3`
use, this unit) to `1` if either axis's span is `>= 0x101`. This is the
`code = 3`/`code = 4` callee `ProjectTriFace`/`ProjectQuadFace`
(`code_8220_b`, matched round 13) call at the end of triangle/quad
submission.

## Best body reached (53/70 words — everything past the header matches)

```c
#if 0
/* A 2-s16 pair (alignment 2, not 4) -- forces the unaligned lwl/lwr whole-
 * struct copy retail uses for prim->0x60 -> prim->0x74 -> prim->0x70 even
 * though those particular offsets are accidentally 4-aligned; the compiler
 * only knows the DECLARED alignment of the type, not the runtime address.
 * Same idiom as FlashbackRotation (include/DreamSys.h) and Class866E8__SetTargetAndBuildRates. */
typedef struct {
    s16 x, y;
} Vec2s16;

void FlagLargePolyForDivide(void *arg0, s32 count)
{
    u8 *self = (u8 *)arg0;
    s16 *xp, *yp, *end;

    *(Vec2s16 *)(self + 0x74) = *(Vec2s16 *)(self + 0x60);
    *(Vec2s16 *)(self + 0x70) = *(Vec2s16 *)(self + 0x74);

    xp = (s16 *)(self + 0x64);
    yp = (s16 *)(self + 0x66);
    end = (s16 *)(self + 0x5C + (count << 2));

    for (; xp < end; xp += 2, yp += 2) {
        if (*xp < *(s16 *)(self + 0x70)) {
            *(s16 *)(self + 0x70) = *xp;
        }
        if (*yp < *(s16 *)(self + 0x72)) {
            *(s16 *)(self + 0x72) = *yp;
        }
        if (*(s16 *)(self + 0x74) < *xp) {
            *(s16 *)(self + 0x74) = *xp;
        }
        if (*(s16 *)(self + 0x76) < *yp) {
            *(s16 *)(self + 0x76) = *yp;
        }
    }

    if (*(s16 *)(self + 0x74) - *(s16 *)(self + 0x70) >= 0x101) {
        *(s32 *)(self + 0x78) = 1;
    }
    if (*(s16 *)(self + 0x76) - *(s16 *)(self + 0x72) >= 0x101) {
        *(s32 *)(self + 0x78) = 1;
    }
}
#endif
```

Build compiles clean, no address drift (in-range comparison, retail's
declared `0x118` bytes = 70 words, matched exactly). The diff is entirely
confined to words 0-17 (of 70) — everything from word 18 through the end,
including the ENTIRE bounding-box loop body and both post-loop flag checks,
is byte-identical.

## Residue: retail's stack-frame adjustment is scheduled one slot later, and never used for storage

Retail's `addiu $sp,$sp,-0x20` sits at word 17 — as the delay slot of the
`beqz $v0,.L8001A334` that skips the loop entirely when `count` is 0 — not
at word 0 where a normal prologue would put it. My best body puts it at
word 0 (an ordinary immediate prologue), which shifts every following
instruction earlier by exactly one slot until word 17, where the SAME
instruction reappears in mine and the two streams re-align (confirmed:
`grep -n '($sp)' asm/nonmatchings/code_8220_c/FlagLargePolyForDivide.s` returns
NOTHING — this function never reads or writes through `$sp` anywhere, in
EITHER version; the reserved 0x20 bytes are never used for storage in
retail either). This is a placement-only difference in an instruction with
literally no data dependency on anything before or after it — the purest
possible instance of a schedulable-anywhere instruction, and per CLAUDE.md's
own test (does moving it change WHICH REGISTER holds a value? No — it
changes nothing but timing) it is instruction-order-only, not register
identity.

## Attempts (6, well under the cap)

1. First cut, `if (xp < end) { do {...} while(...); }` — 53/70 immediately;
   the LOOP BODY matched byte-for-byte on the very first try (confirms the
   min/max update logic, the `Vec2s16` alignment-2 struct-copy idiom for
   the seed, and the two post-loop threshold checks were all read
   correctly from the disassembly). Only the prologue placement differed.
2. Converted to a `for` loop (`for (; xp < end; xp += 2, yp += 2) {...}`)
   instead of `if` + `do`/`while`, on the theory that a different loop
   lowering might relocate the frame adjustment — no change, 53/70,
   identical diff.
3. Reordered the `end` pointer's arithmetic to match retail's own
   instruction order (`self + (count<<2) + 0x5C` instead of
   `self + 0x5C + (count<<2)`) — regressed to 52/70 (introduced a NEW
   one-word mismatch in the address computation itself, on top of the
   unchanged prologue residue); reverted.
4. A bare `__asm__("")` as literally the function's first statement (the
   one documented lever for "prologue store order... not reachable from
   C" residues) — no change whatsoever, 53/70, identical diff. Expected in
   hindsight: that entry is about REORDERING GCC's own already-scheduled
   callee-save stores relative to each other, all of which are C-visible
   register saves; here the item being relocated is the raw `$sp`
   adjustment itself, emitted by GCC's function-level prologue generator
   BEFORE any of the function body (including a leading barrier statement)
   is scheduled — there is no C-level statement position that precedes
   prologue generation to place a barrier at.
5. Declared `end` first (moved its initializer ahead of the `Vec2s16`
   struct-copy statements, reducing the apparent number of live locals at
   any one point) — no change, 53/70.
6. Removed the `end` local's separate declaration/assignment split
   (combined into one declaration-with-initializer) — no change, 53/70.

### Proposed learning

**A new variant of "not reachable from C," distinct from the existing
prologue-callee-save-store-order entry: an entirely storage-free stack
frame (`-N`/`+N` bytes reserved and adjusted but never once read or written
through `$sp`) can be scheduled by GCC 2.6.3 at a point strictly LATER than
function entry — specifically, in the delay slot of an early branch — with
zero data dependency forcing that placement and zero register-identity
consequence.** The existing `__asm__("")`-as-first-statement lever does not
reach this: that lever operates on statements the C body already contains
(reordering scheduled-but-C-visible operations relative to each other), and
a raw frame-size adjustment is emitted by the function prologue generator
outside of and prior to any C statement, including one deliberately placed
first. The discriminator for this exact sub-class: `grep -n '($sp)'` on the
function's own `.s` file returns nothing at all — a real spill or local
array would produce hits and rule this out. When that grep is empty and a
`-N` frame adjustment is the only remaining diff, don't keep trying
loop-shape or declaration-order permutations; the loop body byte-matching
already on the first attempt (as it did here) is itself strong evidence the
C model is correct and the residue is purely this scheduling artifact.
(`FlagLargePolyForDivide`, 53/70 across all 6 structurally distinct attempts,
identical diff on 4 of them.)

## RUNNER PASS, permuter round (alpha): a real partial lead found, not yet closed

`--debug` on this report's preserved body: base score = **120**, decomposing
as 2 reorderings x 60 = 120, zero register diffs, zero insertions/deletions
-- confirms the report's own classification exactly (purely a scheduling-
placement residue, no register-identity component at all, so HARD RULE 6
does not bear on this function at all).

Ran two bounded single searches (no PERM macros, blind), combined ~8600+
iterations. **Found a genuine, non-degenerate improvement: rewriting the
`end` pointer's arithmetic from**
```c
end = (s16 *)(self + 0x5C + (count << 2));
```
**to the algebraically-identical**
```c
end = (s16 *)(self + 0x5C - (-(count << 2)));
```
**halves the score to 60** (one of the two reorderings resolves; the other
persists). Found independently twice (`output-60-1`, `output-60-2`,
identical diff both times) across the two search runs, reproducible, not a
fluke on the permuter's own metric.

**Oracle-verified, and it is FALSE.** Swapped the rewritten `end` expression
into `src/code_8220_c.c` in place of the preserved body, ran the real
oracle: `build-and-verify.sh` reports `build exit=2` (the whole-image
verification step fails -- SHA1 does not match), and `funcdiff.py
FlagLargePolyForDivide` reports **52/70**, one word WORSE than this report's
existing 53/70, with no drift warning (the in-range comparison is trusted,
it is simply wrong). Reverted immediately; `git diff --stat` confirmed
clean before continuing.

So this round now has TWO instances of the identical trap on two different
residues: a permuter-local score improvement (`SubmitPolyF3`'s cached-OT-
pointer lead, and this rewritten-`end`-pointer lead) that is not real
against the true oracle, for two different underlying reasons (address
drift there, an outright in-range regression here). **Do not trust a
permuter score reduction on this function's residue class without
re-verifying against the real oracle first** -- the permuter's own
comparison is evidently forgiving of something the strict positional
comparison the real check performs is not.

No zero reached in either run. Not permuter-exhausted -- a reproducible
sub-baseline score was found and then falsified, which is a stronger
statement than "the search ran out of iterations," but it is still not
the same as no path existing at all.

**Operational note:** a duplicate permuter process on this same directory
(a second, independently-started search, not spawned deliberately by this
session as far as could be reconstructed) was found running concurrently
mid-round and killed by PID (98285, plus its orphaned forkserver children)
per the head's instruction, scoped to specific PIDs rather than a
tool-name pattern that would have reached other runners' worktrees.

## ROUND 20 note

COP2/GTE-clobber-trap hypothesis tested against this function directly (its
own disassembly grepped for every GTE/COP2 mnemonic: zero hits) as part of
a whole-family screen — falsified for all ten functions in this unit's
work list, not just this one. Full method and family-wide result in
`SubmitPolyF3.md`'s "ROUND 20" section. This function's own residue
remains the register-identity + code-motion-filler class already
documented above, unaffected by this screen.

## ROUND 21 note

Checked `docs/DECOMPILATION_LEARNINGS.md` for any lever recorded since round
20 that could apply to this function's residue class (a storage-free stack
frame adjustment scheduled into an early branch's delay slot, with zero
`$sp`-relative reads/writes anywhere in the function). Nothing new applies:
the round-20 `__asm__("")` barrier discriminator ("genuine placement/ordering
residue" vs "register choice or cross-block decision wearing an order-shaped
appearance") was already checked against this function in attempt 4 of this
report's own log, and the reasoning there still holds — the frame adjustment
is emitted by the function-prologue generator strictly before any C statement
is scheduled, including a barrier placed first, so there is no C-level
statement position from which a barrier could reach it. Not re-attempted.
Score and residue unchanged (53/70).

## ROUND 32 note (runner charlie)

Re-verified this function's preserved body directly against the live oracle
this round (flipped to `#if 1`/inserted the report's own preserved source in
place of `INCLUDE_ASM`, ran `./build-and-verify.sh` + `funcdiff.py`, then
reverted and re-confirmed `git diff --stat` clean and `build exit=0` before
continuing) rather than trusting the title line. Result: **53/70, no
stale-build or drift warning, identical diff shape to every prior round's
report.** Confirmed clean, not drifted.

Read this family's full round-13/19/20/21 history (see `SubmitPolyF3.md`
for the shared root analysis) before attempting further reshaping this
round: the register-identity + code-motion-filler residue has already been
tested against six-plus independent axes (masking expression, reload/cache
count, per-reload local scoping, constant naming, signed/unsigned bitfield,
widened-copy intermediate, concrete struct typing for both operands, the
aggregate-assignment lever, the COP2/GTE-clobber-trap hypothesis, and a
40000-iteration blind permuter search on the root case) and confirmed a
genuine wall each time, most recently re-confirmed by the head in round 31.
Per CLAUDE.md's explicit instruction this round, the GTE/COP2-exception
hypothesis was NOT re-run a third time. No new mechanism found or attempted
against this specific residue this round; time went to the LEAD TASK
(`_card_clear`, `libcard_card.c`) instead, which surfaced a genuinely new
finding (a `li`-expansion ADDIU-vs-ORI encoding invisible to `asm-differ`/the
permuter's own scorer, see that function's own report) — checked whether
that class of hidden residue could explain any of this family's own diff
words: it does not, since every diff word in this family's own funcdiff
output changes a REGISTER FIELD (e.g. `$a1` vs `$a2` in an otherwise-identical
`lui`/`ori` instruction), not just an opcode's top bits with the same
register operand -- these are the genuine, already-diagnosed register-identity
residue, not a second hidden instance of the ADDIU/ORI blind spot.

## Naming (round 77, alpha)

`func_8001A268` -> `FlagLargePolyForDivide`, parameter `arg0` -> `ctx`.
**Tier A.** `ctx` is the same per-face draw context TransformAndCullPoly's
own extern comment documents (code_8220.h): its SXY0-2 cache at
`+0x60/+0x64/+0x68` and its culled flag at `+0x78` are exactly the fields
this function reads (`ctx+0x64`..`ctx+0x5C+count*4`) and writes
(`ctx+0x78 = 1`). The name states the two things the body does: update the
running 2D screen bounding box (`ctx+0x70..+0x76`) and set the cull flag
when either span reaches `0x101`. Purpose (why 0x101, i.e. what draw-time
constraint a >256px-wide/tall primitive violates) is not established.

## Round 91 polish (bravo)

Renamed from `UpdatePolyBBoxAndCull` (`python3 tools/rename.py
UpdatePolyBBoxAndCull FlagLargePolyForDivide`). **Tier A.** The body never
culls: it computes the screen bounding box and sets `ctx+0x78`, and that
flag's only readers are the eight SubmitPoly* wrappers, which route a
flagged face to Sony's RCpoly* subdivider instead of `addPrim`. The body now
reads through a unit-local view of the draw context (`PolyDrawCtx`:
`DVECTOR sxy[4]` +0x60, `bboxMin` +0x70, `bboxMax` +0x74, `divide` +0x78),
`0x101` is `> MAX_UNDIVIDED_SPAN` (256; a unit-local `#define`), and the two
load-bearing shapes keep one `MATCHING:` line each: the guarded do/while
with `yp` set inside the guard (round 75's lever, above), and the loop end,
now `(short *)((u8 *)ctx + count * sizeof(DVECTOR) + 0x5C)`. The natural
`&ctx->sxy[count - 1].vx` folds the 0x5C into the index first and scores
62/70; `&ctx->sxy[count].vx - 2` and `(short *)(ctx->sxy + count - 1)` score
69/70.

**Finding, recorded in the function comment: the loop stops one vertex
short.** `end` is `&sxy[count - 1]` and the loop runs while `xp < end`
starting from `sxy[1]`, so for a triangle only `sxy[0]`/`sxy[1]` enter the
box and for a quad `sxy[0..2]`. That is retail's behaviour; nothing here
changes it.

History moved from include/code_8220.h's prototype comment (rewritten there
as documentation): `count` is the face's vertex count (3 or 4), not a
primitive-kind code. An older "3 = triangle, 4 = quad" wording read the
right numbers off the call sites for the wrong reason, corrected round 51;
`ctx` was named round 77. From the old .c comment: the 0x20 frame is
unused, and retail's `addiu $sp,$sp,-0x20` sits in the loop-skip branch's
delay slot because nothing else before the branch is movable (round 75).
