# Entity__MoodCue26 -- MATCHED (49/49, closed by runner alpha)

> Renamed from `func_8005F544` on 2026-09-24 (tools/rename.py). Address 0x8005f544.

Unit: `Entity_c`. Originally staffed to runner bravo (stalled at 30/49,
~7 attempts). Reopened for runner alpha per HEAD BROADCAST (round following
alpha's `Entity_c` batch of six) on the strength of the permuter setup and
the cross-jump-control levers alpha's own round had just established.
Closed by alpha in ~10 further attempts (17/30 total spent across both
runners).

## Final matched form

```c
void Entity__MoodCue26(Entity *this, EntityMoodHandlerArg *out) {
    s32 v1;
    s32 arg1;
    void (**slotD0)(Entity *self, s32 arg1, s32 arg2);

    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * register allocation only -- see "Attempt 10" below. Without it GCC
     * swaps which callee-saved register holds `this` vs `out` for the
     * whole function. */
    do {
        if (out->unk4 % this->unk80 == 0) {
            out->unk10 = this->methods->slot148(this);
            out->unk1C = 0x1A;
            __asm__("");
            v1 = 0x6E;
            goto compare;
        }
    } while (0);
    v1 = 0x6E;
compare:
    slotD0 = &this->methods->slotD0;
    arg1 = -0x180;
    if (this->unkFC == v1) {
        arg1 = -0x2D00;
    }
    (*slotD0)(this, arg1, 0);
}
```

Whole-image build verified byte-exact (`./build-and-verify.sh` green,
`funcdiff.py Entity__MoodCue26` reports 49/49, no address drift).

## Shape (bravo's finding, confirmed, matches 21/21 words up to the residue)

```c
void Entity__MoodCue26(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % this->unk80 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0x1A;
    }
    if (this->unkFC == 0x6E) {
        this->methods->slotD0(this, -0x2D00, 0);
    } else {
        this->methods->slotD0(this, -0x180, 0);
    }
}
```

This is the best-scoring flat form (30/49) and was structurally correct:
the runtime `div`/`mfhi` remainder-gate (divisor `this->unk80`, a runtime
field, not a compile-time constant -- hence a genuine `div` instruction, not
the mult-magic idiom) matched byte-for-byte, as did the final `slotD0` call
and its two literal arguments, and the branch TARGETS all agreed (no CFG
mismatch -- ruled out per CLAUDE.md's discriminator).

## Residue (bravo's finding, confirmed)

Retail materializes the literal `0x6E` into `$v1` **twice**:

1. Unconditionally, in the delay slot of the `bnez $v1, .L8005F5C8` that
   tests the div remainder (`ori $v1, $zero, 0x6E` at `0x8005F59C`) -- this
   value is needed by BOTH paths at the shared merge point below.
2. **Redundantly**, at `0x8005F5C4`, right after the `slot148` vtable call
   inside the `remainder == 0` branch -- because `$v1` is caller-saved and
   the intervening `jalr` through `slot148` clobbers it, so the same literal
   has to be re-established before falling into the shared
   `this->unkFC == 0x6E` comparison.

## Attempts and results

Bravo's six (kept verbatim):

| # | Shape | Score | Notes |
|---|-------|-------|-------|
| 1 | Flat: `if (mod==0) {call}` then `if (unkFC==0x6E) A else B` | 30/49 | Best of bravo's round. Missing exactly the redundant post-call `li`. |
| 2 | Named local `s32 target = 0x6E;` set before the mod-gate, compared after | 6/49, worse | GCC promoted `target` to a NEW callee-saved register (`$s2`), growing the frame (extra `sw $s2`/`lw $s2` in prologue/epilogue) -- retail's frame only saves `$s0`/`$s1`/`$ra`. A value that must survive an indirect call through a genuine local does NOT get the caller-saved-rematerialize treatment; it gets promoted, which is the wrong shape entirely. |
| 3 | Duplicated nested `if`/`else` inside EACH arm of the outer mod-gate (so each predecessor textually repeats its own `unkFC==0x6E` check) | 31/49, still wrong | GCC 2.6.3 did NOT tail-merge the two textually-identical `if`/`else` bodies back into one shared call site here -- it kept them fully separate (extra `j` instructions, duplicated call sequence), unlike the cross-jump precedent in DECOMPILATION_LEARNINGS (`EnableTeleportsForKind`). Not a viable lever for THIS shape as bravo wrote it (see attempt 9 below, which succeeds with the same idea plus alpha's `&slot` lever). |
| 4 | "Default value, then conditionally overwritten" idiom: `arg1 = -0x180; if (mod==0) {call}; if (unkFC==0x6E) arg1 = -0x2D00; slotD0(this, arg1, 0);` (single call site) | 5/49, worse | Same failure mode as #2: `arg1` must survive the intervening `slot148` call, so GCC promoted it to `$s2`, growing the frame. |
| 5, 6 | Operand-order swaps (`this->unkFC == 0x6E` vs `0x6E == this->unkFC`, `==`/`!=` branch-flip) on the flat form | 30/49, unchanged | No effect on which register ends up holding the constant or on instruction count. |

Alpha's continuation (attempts 7-10; earlier alpha attempts 7-8 restated
bravo's #2/#4 as sanity checks and are omitted):

| # | Shape | Score | Notes |
|---|-------|-------|-------|
| 9a | `goto`-based explicit CFG: `v1 = 0x6E;` written as its own statement in BOTH predecessors (inside the mod-gate arm, right before a `goto compare;`, and again on the fallthrough path), landing on a shared `compare:` label that does the comparison and the flat `if`/`else` two-call dispatch from attempt 1 | 30/49 | Same score as flat, but the SHAPE changed: the second `li v1,0x6e` now appears in the output for the first time in this function's history -- confirmed by direct disassembly diff -- but scheduled to the WRONG position (right after the `jalr`, before the `sw`/`sw` field stores, instead of after them). This is the key structural unlock: writing the constant as an explicit statement in EACH predecessor, rather than once at the merge point, is what gets GCC to emit it twice at all. Bravo's #3 tried duplicating the WHOLE `if`/`else` per predecessor (too much); this duplicates only the one assignment, landing it right before a shared label. |
| 9b | Same as 9a plus a bare `__asm__("");` between `out->unk1C = 0x1A;` and `v1 = 0x6E;` | 34/49 | Pins the reordering: without the barrier GCC hoists the independent `v1=0x6E` store up past the two `sw`s (no dependency ties them, so the scheduler floats it to right after the call's delay slot); the barrier stops it, and the store lands in its correct, retail-matching position. This is CLAUDE.md's permitted "order-only" barrier -- confirmed by re-running without it and seeing the exact same register(s) hold the same values, just reordered, per the rule-6 test. |
| 9c | Same as 9b plus `slotD0 = &this->methods->slotD0;` (alpha's `Entity__MoodCue35` lever) taken right before the final `if`, called through `(*slotD0)(this, K, 0)` in each arm (two call sites, matching attempt 1's dispatch shape) | 35/49 | This closed the SECOND residue: `this->methods` now loads into `$a2` at exactly retail's position (right after the `unkFC` reload, before the branch) instead of being refetched via `$a0` after the branch merge. Everything through that load now matches byte-for-byte. The ONE remaining word: GCC's cross-jump pass merged the two call sites into a single physical dispatch (as it did for `Entity__MoodCue25`), but had to add an extra `j` to route the `unkFC == 0x6E` (fallthrough) case around the `unkFC != 0x6E` case's literal -- retail instead places the DEFAULT value in the branch's delay slot and OVERWRITES it in the fallthrough (no extra jump needed), i.e. the "default, then conditionally overwritten" idiom bravo's #4 already tried and rejected. |
| 9d | Same as 9c but converted to single-call default-then-overwrite (`arg1 = -0x180; if (...) arg1 = -0x2D00; (*slotD0)(this, arg1, 0);`), matching retail's exact branch/delay-slot layout | 37/49, but WORSE in kind | The instruction-count residue closed completely (right word count, right branch layout) -- but a NEW residue appeared: GCC swapped which callee-saved register holds `this` (`$a0`) vs `out` (`$a1`) for the ENTIRE function, from the very first prologue `sw`. Confirmed this was not about `arg1` specifically: replacing the named local with an inline ternary in the call argument position (`(*slotD0)(this, cond ? A : B, 0)`) produced the identical swap. It was specifically MERGING the two call sites into one that triggered it (attempt 9c, with two call sites, kept `$s0`=`this`/`$s1`=`out` correctly) -- apparently GCC 2.6.3's register-allocation order is sensitive to how many times `this` is textually mentioned in the function body, and dropping from two mentions (one per call site) to one flipped a tie-break. |
| 10 | Permuter, seeded from 9d (base score 60 -- pure register differences, no insertions/deletions, per `permuter.py --debug`), found a ZERO in 23 iterations: wrap the outer mod-gate `if` in a no-op `do { ... } while (0)` block, everything else identical to 9d | **49/49, MATCH** | The `do/while(0)` is semantically inert (the block runs exactly once regardless) but changes GCC 2.6.3's live-range/scope bookkeeping enough to restore `$s0`=`this`/`$s1`=`out` while KEEPING the single-call, default-then-overwrite dispatch shape from 9d. Translated as-is (idiomatic C89, no permuter-specific artifacts) and re-verified with `build-and-verify.sh` + `funcdiff.py`: whole-image SHA1 green, 49/49. |

## Classification (superseded)

Bravo's classification ("constant-rematerialization / delay-slot-fill
residue... no source form tried reproduces this") turned out to be a
STAGING problem, not a genuine compiler-internal limit -- consistent with
the project's repeated finding (round 7, round 8) that "no source form
found" claims after a handful of attempts are frequently premature. The
residue was three SEPARATE, independently-closable issues stacked on top of
each other, and bravo's attempts (all variations on "name the constant as a
local") only ever probed one of them (the register-promotion-vs-
rematerialize axis) without first establishing the OTHER two:

1. **Which predecessor emits the constant, and how many times.** Needed an
   explicit per-predecessor statement (`goto`-based CFG), not a single
   merge-point expression -- attempt 9a.
2. **Where in that predecessor's instruction stream it lands.** Needed a
   scheduling barrier to stop it floating past unrelated independent stores
   -- attempt 9b.
3. **Where the vtable-method-table pointer load lands, and whether the
   two call sites merge into one dispatch or two.** Needed the
   `&obj->vtable->slotNN` address-of-slot lever (from `Entity__MoodCue35`) to
   position the `this->methods` load correctly, and then a further
   change (the `do/while(0)` wrapper, found by the permuter) to keep BOTH
   the merged single-call dispatch AND the correct `$s0`/`$s1` assignment
   at the same time -- attempts 9c/9d/10.

Bravo's #2/#4 (named local for the constant, causing frame growth) remain
correctly ruled out as a lever for issue 1 above -- that finding stands.
It just wasn't the only issue in play.

Clean of both open toolchain blockers (`gp_rel`, `addiu_at`) -- unchanged
from bravo's finding.

## Proposed learning

1. **A residue that "comes up exactly one instruction short" can be
   several independently-closable issues, not one.** This function needed
   THREE separate fixes stacked (per-predecessor constant duplication via
   explicit `goto`, a scheduling barrier to pin its position, and the
   address-of-slot lever to fix a second, unrelated vtable-pointer-position
   issue) before the true final residue (a global register swap) was even
   visible. Fixing them one at a time and re-measuring after each was what
   made this tractable -- diagnosing all three from the initial diff would
   have been much harder than fixing #1, re-diffing, fixing #2, re-diffing,
   etc.
2. **When retail materializes the same compile-time constant on two
   incoming paths of a merge (not a runtime value, not a struct field),
   write it as its own statement on EACH predecessor, ending in `goto` to a
   shared label, rather than as a single expression at the merge point.**
   This reproduces "recompute a cheap literal near each use" without
   forcing the value to survive a call as a live local (which promotes it
   to a callee-saved register instead -- bravo's #2/#4 already established
   this half). Combine with a `__asm__("");` barrier immediately before the
   duplicated statement if it's independent of a preceding call's result --
   otherwise GCC's scheduler is free to float it earlier than retail's
   position, past unrelated stores that have no data dependency on it.
3. **Merging two near-identical call sites into one (single dispatch, one
   `this` mention) instead of two (cross-jump-merged, two `this` mentions)
   can flip which variable gets `$s0` vs `$s1` for the WHOLE function**,
   even when the change is confined to one small tail. This is a new,
   nastier cousin of the existing "register identity" residue class: it is
   not local to the changed code, it is a GLOBAL effect from a local
   source change, and it shows up as differences starting at the function's
   very first prologue instruction. When retail's own layout genuinely
   needs the single-call form (confirmed by matching branch/delay-slot
   layout), do not assume the resulting register swap is unfixable by
   source reshaping -- a semantically-inert scoping wrapper (`do { ... }
   while (0)` around an unrelated, earlier part of the function) closed it
   here. The permuter found this in 23 iterations from a low base score
   (60, pure register differences, no insertions/deletions) after ~9 manual
   attempts had converged the SHAPE but not the register assignment --
   worth reaching for it specifically once a residue is "small number
   count, register-only" rather than "wrong instruction count," since that
   score profile is exactly what the permuter's random reordering/wrapping
   mutations are best at closing.
4. **The permuter's `--debug` base score is itself diagnostic, not just a
   sanity check.** A score dominated by "Register Differences" with zero
   "Insertions"/"Deletions" (60, all-register, attempt 10's seed) versus one
   dominated by "Insertions"/"Deletions" (665, `Entity__MoodCue35`'s original
   seed; 1085/170, this function's earlier flat-form seeds) predicts how
   promising a from-scratch search is: the all-register case converged in
   23 iterations, the mixed cases ran 15,000-27,000+ iterations without
   reaching zero from seeds that were structurally further from the answer.
   Re-seed the permuter from your CLOSEST manually-reshaped near-miss, not
   the original flat form, once you've made shape progress -- it changes
   the search from "structural" (slow, low hit rate) to "register-only"
   (fast, high hit rate).

## Preserved near-miss history (do not compile — for reference only)

Bravo's original best (30/49), the starting point for this round:

```c
#if 0
void Entity__MoodCue26(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % this->unk80 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0x1A;
    }
    if (this->unkFC == 0x6E) {
        this->methods->slotD0(this, -0x2D00, 0);
    } else {
        this->methods->slotD0(this, -0x180, 0);
    }
}
#endif
```

Alpha's attempt 9d (37/49, the global-register-swap near-miss that the
permuter's `do/while(0)` wrapper closed):

```c
#if 0
void Entity__MoodCue26(Entity *this, EntityMoodHandlerArg *out) {
    s32 v1;
    s32 arg1;
    void (**slotD0)(Entity *self, s32 arg1, s32 arg2);

    if (out->unk4 % this->unk80 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0x1A;
        __asm__("");
        v1 = 0x6E;
        goto compare;
    }
    v1 = 0x6E;
compare:
    slotD0 = &this->methods->slotD0;
    arg1 = -0x180;
    if (this->unkFC == v1) {
        arg1 = -0x2D00;
    }
    (*slotD0)(this, arg1, 0);
}
#endif
```

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 26 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the one bare `__asm__("")`,
between `out->unk1C = 0x1A;` and `v1 = 0x6E;`, is **justified** and now
commented at the site. Re-measured by deleting it alone: the image went red
(12 bytes), `funcdiff` 45/49, and asm-differ shows `li v1,0x6e` moved from
after `sw v0,0x1c(s1)` to directly after the `getProximityRatio` call's delay
slot, above `sw v0,0x10(s1)`. Same registers, instruction order only -- the
same finding as row 9b above.
