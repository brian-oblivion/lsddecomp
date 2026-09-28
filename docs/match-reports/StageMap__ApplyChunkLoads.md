# StageMap__ApplyChunkLoads -- MATCHED round 73 (105/105, exact length, whole-image SHA1 green)

> Renamed from `StageMap__ApplyRateEntries` on 2026-09-26 (tools/rename.py). Address 0x8004bb3c.

> Renamed from `Class866E8__ApplyRateEntries` on 2026-09-26 (tools/rename.py). Address 0x8004bb3c.

> Renamed from `func_8004BB3C` on 2026-09-24 (tools/rename.py). Address 0x8004bb3c.

REVISITED, round 73: MATCHED 105/105 (walker is a strength-reduced giv of the walked parameter); names/types not relevant (existing ChunkLoadEntry/ChunkLoadEntryTail views reused unchanged)

## Round 73 (bravo) -- revisit, MATCHED

**Preserved body rebuilt first, unchanged from the `#if 0` block:** 90/105,
`insertions 4 / deletions 4`, positional skeleton diffs 15, exact length.
Residue as filed: `ep`/`sp` in `$s3`/`$s4` where retail has `$s4`/`$s3`,
and the `sp` initialiser scheduled in the prologue BEFORE the `blez`
guard, where retail has it after the guard.

**The word that decided it:** retail's `addiu $s3, $a1, 4` sits in the
loop PREHEADER (after `blez $s5`, next to the hoisted `li $s6, 1`) and it
reads `$a1`, not the callee-saved register that holds `arr1`. A user
variable initialised in C lands before the guard; only loop.c puts code in
the preheader. So the `+4` register is a strength-reduced giv, and a giv
whose init reads the INCOMING argument register means the biv's
initial value was that register: the biv is the parameter `arr1` itself,
not a copy.

Builds, one per line (funcdiff score):
- preserved two-walker body (`ep = arr1`, `sp = arr1 + 4`, both `++`): 90/105, ins/del 4/4
- `ep` walker + indexed `arr1[i].id` / `arr1[i].rate`: 93/105, ins/del 2/2 (giv anchored at +0, `id` at +8)
- `sp = (Sub *)&arr1[i].rate` inside the loop: 93/105, byte-identical to the previous (CSE folds to one base)
- `sp = &((Sub *)((u8 *)arr1 + 4))[i]`: longer (drift); the +4 base stays a separate invariant register, `sp = s6 + s5` recomputed each pass, but the s3/s4 roles came out like retail's
- same with a hoisted `sub = arr1 + 4` local: one word longer (a `move` from the pre-guard copy)
- same inlined with no `sp` local: 93/105, folds back to the +0 anchor
- `ep` walker + `sp = (Sub *)&ep->rate` inside the loop: 90/105 at retail's SHAPE (init now in the preheader) but reading `$s3` (= `ep`) and roles still swapped
- **walk `arr1` itself (no `ep`), `sp = (Sub *)&arr1->rate` inside the loop: 105/105, `build exit=0`, OK: build matches retail**

Checked hypothesis for why the `ep` copy swapped the registers: with
`ep = arr1`, the giv init reads `ep`'s pseudo, which ties `arr1`/`ep`
together into the first-allocated callee-saved register; with the
parameter as the biv, the giv is seeded from `$a1` directly and the
parameter pseudo's live range and priority change, so the giv outranks
it for `$s3`.

Header: one additive comment paragraph in `include/class_3bb8c.h` after
the existing ChunkLoadEntry note; no declaration changed.

### Proposed learning

**A walker initialised in the loop PREHEADER (after the count guard, next
to hoisted constants) is a strength-reduced giv, not a user pointer; if
its init reads an ARGUMENT register, the biv is the parameter itself.**
Write `arr1++` on the parameter and assign `sub = (T *)&arr1->field`
inside the body. A second walker declared and initialised in C always
lands BEFORE the guard; a `ep = arr1` copy makes the init read the copy's
s-register instead of `$aN`. Distinct from the "two differently-based
walkers" shape round 13 found, which gets the increments right and the
allocation wrong.

## Earlier history (superseded by the match above)

#### Old title: StageMap__ApplyChunkLoads -- STALL (register-identity, 90/105 words at correct length)

> **ROUND 47 (charlie): Gate 1b re-verified 90/105, no drift** (rebuild via
> `make clean && make extract` then the standard `#if 0`->`#if 1` swap,
> after fixing a regex bug in the batch script that had briefly corrupted
> an unrelated block -- caught before anything was committed). Identical
> whole-function `$s3`<->`$s4` register-identity residue, unchanged from
> round 46.
>
> **This function is one of the FIVE `class_3bb8c`/`Obj866E8`-family
> members round 46 confirmed have a scaffold-vs-real-build MISMATCH**
> (round 17's own search: 2 insertions/2 deletions isolated vs 0/0 in
> context; round 40 rebuilt an independent scaffold and got the SAME 2/2
> mismatch). No search has ever been trusted for this function -- both
> rounds that touched a scaffold here correctly DECLINED to search rather
> than running one against a program not being built. There is therefore
> no inherited "permuter tried, negative" to revert to UNKNOWN: the
> correct prior disposition was already "not searched, scaffold
> untrustworthy", and it still is. Not re-attempted; the family-wide
> pattern is now 5 confirmed instances (`StageMap__ApplyChunkLoads`, `StageMap__ComputeFootprintDescriptor`,
> `StageMap__SplitFootprintRect`, `StageMap__BuildFootprintRects`, `IsPointOutOfBounds`), reinforcing round
> 46's own read that this is structural to the family's
> `self->methods->slotNN` call-chain shape, not a per-function fluke --
> still a tooling question for the operator, not something to fix
> mid-round.
>
> Checked the 12th lever (hoist a field pair used on every path into
> locals PER `if`) against this residue: does not apply -- confirmed
> again this round (as in round 41) that the body has no
> already-cached-then-reread field anywhere, and the residue itself is a
> pure register-coloring choice with no branch or field-pair shape nearby.
> **Disposition unchanged: 90/105.** No new attempt made; `INCLUDE_ASM`
> untouched throughout.

> **ROUND 46 (charlie): Gate 1b re-verified 90/105, no drift; this round's
> split-combined-declaration lever tried on `ep`/`sp`, INERT; the
> beq/bne-polarity lever checked and does not apply -- SKIPPING.** Rebuilt
> the committed 90/105 body from a clean `INCLUDE_ASM` baseline: confirmed
> 90/105, exact length, identical whole-function `$s3`<->`$s4` register-
> identity swap.
>
> **Beq/bne delay-slot-polarity lever: does not apply.** Every branch
> target in the function already agrees with retail (established since
> round 13); the residue is a register-COLORING choice, not a branch
> polarity or delay-slot-assignment issue.
>
> **Split-combined-declaration lever, tried directly:** `ep`/`sp` (the two
> differently-based walking pointers whose introduction in round 13 is
> what got this function to 90/105 in the first place) are declared with
> combined initializers (`ChunkLoadEntry *ep = arr1;` / `ChunkLoadEntryTail
> *sp = (ChunkLoadEntryTail *)((u8 *)arr1 + 4);`) -- exactly the shape the
> lever targets. Split into separate declaration and assignment
> statements, rebuilt: **90/105, IDENTICAL diff, no drift -- fully
> inert.** Reverted (`git checkout -- src/class_3bb8c.c`; clean
> `OK: build matches retail` confirmed after).
>
> This is a THIRD confirmed instance of the scope limitation this round's
> own broadcast already established on two other functions
> (`Snd_setVabAttr`, `NoteOn`, both in `code_179d8_k`): the
> split-declare lever closes a register-CLASS residue for a value crossing
> a CALL boundary with independently-confirmed-correct timing
> (`GetCdFileEntry`'s case), and does nothing for a whole-function
> parameter/induction-variable register-COLORING residue. `ep`/`sp` here
> are exactly the latter shape -- two loop-local walking pointers seeded
> once before the loop and never crossing a call boundary in a way their
> registers depend on -- so the negative result was expected and is now
> measured, not assumed.
>
> **Disposition unchanged: 90/105, `INCLUDE_ASM` restored, `git diff`
> against the round-13 commit for this function's C is empty.** This
> residue remains blocked on the same axis it has been blocked on since
> round 13 (six prior rounds' worth of confirmation: 17, 19, 20, 27, 32,
> 39, 40, 41), the isolated permuter scaffold provably scores a different
> residue than the real build (round 17/40, and this class recurs
> elsewhere in this unit this round -- see `StageMap__SplitFootprintRect`,
> `StageMap__ComputeFootprintDescriptor`, `StageMap__BuildFootprintRects`, `IsPointOutOfBounds`), and neither of this
> round's two new levers applies. **SKIPPING further attempts this
> round** per this round's own guidance on functions whose cheap levers
> are spent.

> **ROUND 41 (alpha): Gate 1b re-verified 90/105, no drift; dead-reload
> lever read BY HAND per this round's staffing instruction, one new
> structural variant tried, REGRESSED hard.** Rebuilt the exact preserved
> body from a clean `INCLUDE_ASM` baseline first: confirmed 90/105, exact
> length, identical `$s3`<->`$s4` residue -- unchanged from round 39.
>
> **Read the body for the dead-reload shape before touching anything, per
> this round's brief.** The lever closed on `StageMap__LoadChunksAround`/`StageMap__PopulateSlotCells`
> this same unit last round is "a value already held in a local gets
> reloaded via the same expression a second time, into the same local, for
> a second single use" -- a same-block RELOAD of an already-computed value.
> This function's `e->unk4` is dereferenced up to five times per loop
> iteration (`unk2C`, `unk30`, `methods->slot78`'s receiver, `unk2A`,
> `methods->slot74`'s receiver) but **never through a local that already
> holds it** -- there is no `p = x->y; ...; q = x->y;` pair anywhere in
> this body, only repeated `e->unk4->field` dereferences with no prior
> caching at all. That is a different shape: introducing a cache is adding
> a computation, not removing a redundant one, so this is NOT the
> documented lever, it is its mirror image.
>
> Tried it anyway as the closest analogue, in case caching `e->unk4`
> resembled the win seen elsewhere: `ElemTarget *et = e->unk4;` assigned
> once right after `slot88`, with every subsequent `e->unk4->X` rewritten
> to `et->X`. **Regressed hard: 12/105, with the compiled length itself
> shrinking (142607 bytes of drift outside the function's own range,
> confirming an address shift, not just a worse register choice).**
> Reverted immediately. This confirms the shape difference is real: unlike
> the reload-removal lever (which deletes an instruction retail's own
> disassembly never had), caching `e->unk4` here removes instructions
> retail actually DOES perform as separate dereferences, so it can only
> ever produce a shorter, wrong-length function.
>
> **Verdict unchanged: this residue is the two-independently-based-walker
> register swap (`ep`/`sp`, i.e. `$s3`/`$s4`) documented across 6 prior
> rounds (13, 17, 19, 20, 27, 32, 39), not a dead-reload instance.** The
> dead-reload lever's precondition (a local already holding a value, reread
> via the same expression into the same local) simply does not occur in
> this body anywhere -- confirmed by direct inspection, not inferred. Per
> project rule 6 this remains a register-identity stall, not to be forced
> with a register pin. No permuter run this round (time went to
> `StageMap__FindSlotForPosition` instead, per this round's own closer-target priority);
> the existing round-17/40 finding that this function's isolated-compile
> scaffold provably scores a DIFFERENT residue than the real build (0
> insertions/deletions in context vs 2/2 isolated) still stands and still
> blocks a trustworthy search without more surrounding-file context than
> `setup-permuter.sh` currently provides.

> **ROUND 40 (bravo): Gate 1b re-verified 90/105, no drift; fresh permuter
> scaffold built and independently RE-CONFIRMS round 17's scaffold-mismatch
> finding, no search run.** Rebuilt the exact preserved body per Gate 1b:
> confirmed 90/105, no drift, identical `$s3`<->`$s4` residue.
>
> This round's assignment table listed this function as "searched", but
> round 17's own search ran against a scaffold it explicitly documented as
> scoring a DIFFERENT residue than the real build (2 insertions/2 deletions
> in isolation vs 0/0 in context) -- a permuter-inconclusive result, not an
> exhausted one. Built a brand-new scaffold from scratch this round
> (`tools/setup-permuter.sh StageMap__ApplyChunkLoads <seed>`, seed = this report's own
> 90/105 body verbatim) and ran `--debug --stack-diffs` BEFORE searching,
> per the script's own advice: **base score 480, with 20 stack differences
> and 2 insertions / 2 deletions** -- the real build has ZERO stack
> difference and ZERO insertions/deletions (90/105 is pure register
> identity, correct frame, correct length). This is round 17's exact
> mismatch, independently reproduced with a fresh scaffold four rounds
> later -- not a fluke of that one attempt, but a STRUCTURAL property of
> isolating this function: the `sp = (ChunkLoadEntryTail*)((u8*)arr1+4)`
> computation's scheduling (and evidently now the frame size too) depends
> on surrounding-file register pressure that a single-function compile unit
> cannot reproduce.
>
> **No search was run against this scaffold** -- a result against a
> provably-mismatched scaffold does not transfer, per the project's own
> documented rule, and running one anyway would only waste the round's
> budget on a number nobody could trust. This function's residue remains
> genuinely permuter-untested in the sense that matters (a scaffold that
> scores the SAME thing the real build does), six rounds of manual
> confirmation deep, with the permuter route requiring more than
> `setup-permuter.sh` can currently provide -- likely more surrounding
> file context, not a different seed shape. Not attempted further this
> round; time went to the three never-searched functions elsewhere in this
> unit instead, per this round's staffing priority.

> **ROUND 39 (charlie): re-verified, no new attempt.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline: confirmed 90/105, no
> drift, identical `$s3`<->`$s4` whole-function register-identity swap
> (diff sites unchanged: prologue `sw` scheduling at `0x8004BB48`-`0x8004BB80`
> plus two later reload sites at `0x8004BBB8`/`0x8004BBFC`/`0x8004BC14`).
> Checked whether this round's headline lever ("hoist both values before
> either is consumed") applies: it does not, and not for lack of an
> instruction-adjacency hit -- the pre-screen's 11 adjacent load/mult pairs
> here are the walker-pointer dereferences (`sp->id`, `sp->rate`, `ep->ptr0`)
> that the identity swap already touches, not two independently-computed
> VALUES feeding one later shared consumer the way `StageMap__FindSlotForPosition`'s two
> field reads did. The residue here is a swap between the two INDUCTION
> VARIABLES themselves (`ep`/`sp`), both already live across the whole loop
> body by construction -- there is no "consume A late, consume B later
> still" shape to restructure. Six rounds deep (13, 17, 19, 20, 27, 32) of
> confirmation via independent angles (callee-saved order, saturated-register
> check, aggregate-assignment check, a permuter run whose scaffold was
> proven to score a different residue) with nothing left un-checked; not
> re-attempted further this round. Time went to the higher-yield queue
> entries instead (`StageMap__FindSlotForPosition`, `ComputeCellWorldOffsets`, and the two large
> multi-load functions with higher pre-screen hit counts).

> **ROUND 32 (bravo2): re-verified, no new attempt.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline: confirmed 90/105, no
> drift, identical `$s3`<->`$s4` whole-function register-identity swap.
> Given the depth of prior confirmation (rounds 13, 17, 19, 20, 27 --
> including a callee-saved-register-order check and a permuter run whose
> scaffold was found to score a different residue than the real build), no
> new structural variant was attempted this round; time went to
> `StageMap__FindSlotForPosition` (matched, 63/70 -> 68/70) instead.

> **ROUND 27 (delta): callee-saved-register check per the head's broadcast.**
> Rebuilt the exact preserved body from a clean `INCLUDE_ASM` baseline
> (90/105 confirmed, correct length) and disassembled the compiled `.o`'s
> prologue directly: saves `s1,s3,s4,s2,s5,ra,s6,s0` at offsets
> `0x1C,0x24,0x28,0x20,0x2C,0x34,0x30,0x18` off a `-0x38` frame. Retail's
> own prologue (`asm/nonmatchings/class_3bb8c/StageMap__ApplyChunkLoads.s`) saves
> `s1,s4,s5,s2,ra,s6,s3,s0` at offsets `0x1C,0x28,0x2C,0x20,0x34,0x30,0x24,
> 0x18` -- **the SAME seven registers (`s0`-`s6`) at the SAME per-register
> stack slots, no `fp`, same `0x38` frame size, only the ORDER of the `sw`
> instructions (i.e. which C variable got assigned to which physical
> register) differs.** This is exactly the discriminator the broadcast
> named for "genuine register identity, not this lever": same registers
> saved, different roles. **The lever does NOT apply here either; the
> register-identity verdict is CONFIRMED, not merely plausible.** Not
> re-attempted further this round.

> **ROUND 20 (charlie): re-verified, no new attempt.** Confirmed against
> this round's build (header additions from `StageMap__PopulateSlotCells` this same
> round touch DIFFERENT structs -- `Elem::unk8`, `ElemTarget::field10`,
> `EntryChildObj::unk14`/`unk36` -- none of which this function reads, so
> no re-check needed beyond the standard `cmp -l`-clean whole-image build
> already confirmed for the round). Still 90/105, correct length, same
> `$s3`/`$s4` whole-function identity swap. Not re-attempted: the permuter
> already ran against this exact function (round 17, logged below,
> inconclusive due to a scaffold mismatch, not exhausted) and a second
> manual pass would only re-tread the same well-documented wall. Time this
> round went to `StageMap__PopulateSlotCells` (fresh ground) instead, per the staffing
> guidance to move off a register-identity wall once it's this well
> established.

> **ROUND 19 (bravo): drift claim RE-VERIFIED per the head's mid-round
> broadcast** (two other reports' "clean/drift-free" claims turned out
> false this round). Rebuilt this exact preserved body from a clean
> `INCLUDE_ASM` baseline: `funcdiff.py` reports 90/105 with NO "differs
> outside range" warning, and `objdump -t` confirms the compiled length is
> `0x1A4` (105 words) -- IDENTICAL to retail's own `.s` header
> (`nonmatching StageMap__ApplyChunkLoads, 0x1A4`). `asm-differ` shows only the
> already-documented `$s3`<->`$s4` prologue-scheduling difference; no
> genuine extra or missing instruction. **This report's claim is
> accurate.** Restored to `INCLUDE_ASM` unchanged; no new attempt made
> this round beyond the verification (this residue matches the round's
> independently-confirmed "declaration order is inert" finding for this
> exact class, already tested twice on this function specifically -- see
> the round-13 entries below).
>
> **Checked against the head's second mid-round broadcast, which named
> this function as "a candidate shape" for a saturated-register-file
> misdiagnosis (a cross-call cached local inflating the count by one).**
> `grep -oE 'sw +\$s[0-9]' asm/nonmatchings/class_3bb8c/StageMap__ApplyChunkLoads.s | sort -u`
> gives exactly **7** distinct registers (`$s0`-`$s6`), both in retail
> and in this build -- not 8 or 9, so this is NOT a saturated-plus-one
> case. The register COUNT already matches exactly; the residue is a
> clean 2-way identity swap between two already-correctly-counted
> registers (`$s3`<->`$s4`, ep/sp), not an extra live value to hunt
> down and remove. The saturation mechanism does not apply here.
>
> Also checked against the aggregate-assignment lever (5 closures
> elsewhere this round, whole-struct vs field-by-field copy): **does not
> apply here.** Every struct field write is either freshly computed
> (`e->unk4->unk30 = sp->rate;`) or a single isolated field; never
> several adjacent fields of one struct copied verbatim from another
> instance.

> **UPDATE, round 17, targeted permuter pass.** Still a STALL. Re-verified
> the preserved 90/105 body against the current build first (per
> CLAUDE.md's struct-edit/header-merge discipline after a `git merge main`)
> -- confirmed unchanged: `funcdiff.py StageMap__ApplyChunkLoads` reports 90/105 words,
> `build exit=2` (not byte-exact, correctly left as `INCLUDE_ASM`).
>
> **A permuter run was attempted and turned out to be measuring a DIFFERENT
> problem than the real build, which is worth recording as its own
> finding.** `tools/setup-permuter.sh`'s own scaffold, `--debug`'d before
> searching (per its own printed instructions), reported **base score 460**
> with 2 insertions and 2 deletions -- but the REAL build's residue, per
> `funcdiff.py` and `asm-differ`, has NO insertions or deletions at all
> (105/105 words present, only 15 differ, all register-identity/scheduling).
> Comparing the scaffold's own rendered diff against the real build's
> `asm-differ` output showed WHY: in the isolated single-function
> compilation, the `sp = (ChunkLoadEntryTail *)((u8 *)arr1 + 4)` computation
> gets scheduled EARLY (an `addiu $s4,$s3,4` right in the parameter-save
> preamble), where the REAL in-context build defers it PAST the
> `blez`-guarded early-exit (`addiu $s3,a1,4`, computed from the raw
> parameter register, right before the loop body) -- exactly matching
> retail's OWN placement for that instruction. **The scaffold is not
> reproducing the surrounding register-pressure/scheduling context that
> makes retail's placement reachable at all**, so search results against it
> do not transfer. This is precisely the trap `tools/setup-permuter.sh`'s
> own header warns about ("A scaffold that scores something other than the
> reported residue is scoring a different function than you think").
>
> Ran anyway, bounded (`timeout 300`, `-j 6 --stop-on-zero --best-only`),
> as a low-cost check in case it stumbled onto a transferable lever despite
> the mismatch. It did not: best score reached was **310** (never
> approached 0), and the run eventually hit an internal scoring crash
> (`KeyError: 'StageMap__CountPendingLoads'`, from the same missing-prototype situation
> that produces an ordinary, harmless `implicit declaration` warning in the
> real build) partway through, around iteration 40900. **No exit code was
> captured** -- the trailing `echo "permuter exit=$?"` after the `timeout`
> invocation never ran, meaning the wrapping shell itself was torn down
> before reaching it (not a `137` vs `124` distinction either, just no
> signal at all reached the log) -- worth knowing for whoever next tries to
> rely on that pattern for THIS kind of background permuter invocation.
>
> **Verdict: permuter-inconclusive, not permuter-exhausted.** The search
> ran against a scaffold that was provably scoring a different residue than
> the real build's, so a negative result here says nothing about whether
> the actual register-identity swap is permuter-reachable. If this function
> is revisited, the scaffold itself needs fixing first -- likely by
> including enough of the REAL surrounding context (more of
> `src/class_3bb8c.c`, or matching whatever produces the early-vs-late
> `addiu` scheduling difference) before trusting a base-score sanity check,
> per the setup script's own advice, rather than re-running the search
> as-is.
>
> **HEAD UPDATE, round 13 (2026-09-03).** Attempt 6 below -- left explicitly
> unfinished ("the most promising untried direction") -- was finished, and it
> WORKED. The missing `addiu $s4,$s4,0xc` is recovered, the function's total
> length is now correct, and the score went **14/105 -> 90/105**.
>
> **The lever: two walkers of DIFFERENTLY BASED types, not two pointers of
> one type.** Retail seeds `$s4` at `arr1` and `$s3` at `arr1 + 4`, so the
> two induction variables have different BASES, which is what stops GCC
> 2.6.3's strength reduction proving them one family. The blocker the
> original attempt hit -- "padding `ChunkLoadEntryTail` to 0xC would corrupt
> `sizeof(ChunkLoadEntry)`" -- dissolves once the sub type is never
> embedded in `ChunkLoadEntry` at all; it exists only as a local walking
> pointer's target type, and `ChunkLoadEntry` is already 0xC so its own
> `++` needs no help:
>
> ```c
> ChunkLoadEntry *ep = arr1;                                    /* ptr0  */
> ChunkLoadEntryTail   *sp = (ChunkLoadEntryTail *)((u8 *)arr1 + 4);       /* rate/id */
> ```
>
> The single `(u8 *)` cast is OUTSIDE the loop, so it costs the one
> `addiu` retail also has and none of the per-iteration overhead that sank
> attempt 5's cast-based walking.
>
> **What remains is a whole-function `$s3` <-> `$s4` identity swap** (retail
> `$s4` = the `ptr0` walker, built `$s3` = it) plus the prologue `sw`/`move`
> scheduling that follows from it. Every one of the 15 differing words is a
> same-instruction, different-register diff; asm-differ shows two
> order-only markers and no inserted or deleted instruction in the loop or
> the epilogue. That is a **register-identity stall**, which CLAUDE.md rule
> 6 forbids fixing with `register T v asm("$N")`, so the function stops
> here rather than being reshaped further.
>
> Two further round-13 attempts, both no better, both recorded so nobody
> repeats them:
> - **Declaring `sp` before `ep`** -- WORSE (the `+4` moves to `a1`-relative
>   but register numbering shifts further out of line).
> - **Declaring `sp` after `i`/`e` instead of first** -- byte-identical to
>   the best shape. GCC 2.6.3's `$s`-register assignment here does not
>   follow declaration position.

Unit: `class_3bb8c`. Slot `Obj866E8Methods::slotFC` (verified against
`tools/classtable.py 0x800866E8`). Not toolchain-blocked: no `gp_rel` hit,
no `addiu $at,$at,%lo` hit in `asm/nonmatchings/class_3bb8c/StageMap__ApplyChunkLoads.s`,
and no dense-`switch`/`jr $v0` table dispatch either.

## What it does

`void StageMap__ApplyChunkLoads(Obj866E8 *self, ChunkLoadEntry *arr1, s32 count)`:
iterates `arr1[0..count)` (a 0xC-byte-strided array). Per entry:

1. `e = self->methods->slot118(self, arr1[i].id)` (resolve an `Elem` by
   index/key).
2. `self->methods->slot88(self, 6, e, i)` -- the SAME already-documented
   slot88 (dispatches to `StageMap__OnSlotEvent`, outside this unit) that
   `StageMap__OnDrawSystemEvent` also calls, there with a literal `7` instead of `6`. No
   header change needed for this slot, it already existed.
3. If `arr1[i].ptr0 != 0`: conditionally call `slot108(self, e)` (new slot,
   guarded by `e->unk4->unk2C != 0`), copy `arr1[i].rate` into
   `e->unk4->unk30`, call `e->unk4->methods->slot78(e->unk4, arr1[i].ptr0)`
   (new `ElemTargetMethods` slot -- `ptr0` forwarded verbatim, never itself
   dereferenced in this unit), set `e->flag = 1` and `self->unk1B0 = 1`.
4. Else: the SAME `slot108` guard, then if `e->unk4->unk2A != 0`, call
   `e->unk4->methods->slot74(e->unk4)` (new `ElemTargetMethods` slot,
   self-only) and clear `e->flag`.

After the loop: `self->unk1B4 = StageMap__CountPendingLoads(self)` (already matched, a
plain count of `self->arr[i].flag != 0`).

New header additions (all committed, additive): `Obj866E8Methods::slotFC`
(StageMap__ApplyChunkLoads's OWN identity slot, verified via classtable -- signature
`(self, ChunkLoadEntry *arr1, s32 count)`; later corrected once
StageMap__LoadChunksAround needed to CALL this slot and the earlier draft signature here
turned out to have been copy-pasted from slot88's shape by mistake -- see
that function's own report) and `::slot108` (split out of the
`pad0FC`/`pad108` padding gaps -- `pad0FC` was actually TWO slots, `0xFC`
and `0x100`; `slot104` sits between them at `0x104`);
`ElemTargetMethods::slot74`/`slot78` (split out of what was
`pad000[0x7C]`, now `pad000[0x74]` + the two new slots + existing
`slot7C`); `ChunkLoadEntry` (new type, `void *ptr0` @0x0, `s16 rate`
@0x4, `s32 id` @0x8, size 0xC).

## Best body reached

```c
void StageMap__ApplyChunkLoads(Obj866E8 *self, ChunkLoadEntry *arr1, s32 count) {
    s32 i;
    Elem *e;

    for (i = 0; i < count; i++) {
        e = self->methods->slot118(self, arr1[i].id);
        self->methods->slot88(self, 6, e, i);
        if (arr1[i].ptr0 != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            e->unk4->unk30 = arr1[i].rate;
            e->unk4->methods->slot78(e->unk4, arr1[i].ptr0);
            e->flag = 1;
            self->unk1B0 = 1;
        } else {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            if (e->unk4->unk2A != 0) {
                e->unk4->methods->slot74(e->unk4);
                e->flag = 0;
            }
        }
    }
    self->unk1B4 = StageMap__CountPendingLoads(self);
}
```
(Correction from an earlier draft of this report: step 2 dispatches
`slot88`, an ALREADY-DOCUMENTED slot shared with `StageMap__OnDrawSystemEvent`
-- StageMap__ApplyChunkLoads's own identity is `slotFC` at `+0xFC`, confirmed via
`tools/classtable.py`, and is dispatched INTO from `StageMap__LoadChunksAround`
elsewhere in this unit, not from within this function's own body. An
earlier draft of this report conflated the two and, worse, propagated
`slot88`'s parameter shape onto the `slotFC` header entry itself -- that
header mistake was caught and fixed while deriving StageMap__LoadChunksAround's call
site, which needed `slotFC`'s REAL signature, `(self, ChunkLoadEntry*,
s32 count)`, to compile. If re-deriving this function, `self->methods`
offset `0xFC` is `slotFC` = StageMap__ApplyChunkLoads itself; do not use that name for
the offset-`0x88` call above.)

**This body is 104/105 instructions structurally IDENTICAL to retail** --
every single instruction from the prologue through the epilogue matches
retail's OPCODE and OPERAND SHAPE one-for-one, differing only in WHICH
callee-saved register (`$s0`-`$s6`) holds which value (a consistent,
total renaming, not a partial one), **except for exactly one missing
instruction**: retail has an extra `addiu $s4,$s4,0xc` right before the
loop-continue branch (at `3CA4`, in the delay slot of the `bnez` back to the
loop head) that this build's compiled output does not emit anywhere. That
single missing 4-byte instruction is what shifts everything after it and
produces the LARGE-looking "14/105" and the drift warning -- the underlying
match is much closer than that score suggests.

## Residue: retail walks the array with TWO independently-incrementing
## pointers; every source shape tried here produces only ONE

Retail keeps TWO separate live pointers through the loop, `$s3` (= `arr1+4`,
walking the `rate`/`id` pair) and `$s4` (= `arr1`, walking `ptr0`), each
incremented by `0xC` **independently** every iteration (`$s3`'s increment is
folded into an earlier `jalr`'s delay slot; `$s4`'s is the extra
instruction). This build's compiled output, however it's written, only
ever derives ONE `$s`-register induction variable and reads `ptr0`/`rate`/
`id` all relative to it -- functionally identical, one instruction shorter,
does not match.

Tried and rejected (all against this exact function, in this order). Note
that (6), listed here as unfinished, was completed in round 13 and is the
shape that worked -- see the head update at the top of this file:

1. Plain `arr1[i].field` for everything (id/ptr0/rate) -- 14/105, one
   induction variable, missing the extra `addiu`.
2. Explicit intermediate element pointer, `ChunkLoadEntry *entry =
   &arr1[i];`, then `entry->field` throughout (the documented
   "intermediate element pointer" idiom from DECOMPILATION_LEARNINGS,
   normally a strong lever for array loops in this codebase) -- IDENTICAL
   14/105, no change at all.
3. A nested sub-struct (`ChunkLoadEntry { void *ptr0; struct { s16 rate;
   s32 id; } sub; }`), accessed as `arr1[i].sub.field` -- IDENTICAL 14/105.
4. The same nested sub-struct, but with an explicit `ChunkLoadEntryTail *sub =
   &arr1[i].sub;` computed once per iteration (mixing indexed `ptr0` access
   with a sub-struct pointer for the rest) -- IDENTICAL 14/105.
5. Fully manual pointer walking with NO array indexing at all: two
   independent locals (`void **ptrWalk`, `ChunkLoadEntryTail *subWalk`) seeded
   once before the loop and advanced via explicit `(u8 *)p + 0xC` casts at
   the bottom of the loop body -- REGRESSED to 4/105 (extra pointer-
   arithmetic instructions the cast-based increment needs, that retail
   doesn't have).
6. The same two-pointer idea using NATURAL (non-cast) pointer arithmetic
   (`sub++`/`entry++` on properly-typed, correctly-strided pointers) --
   not fully evaluated; discovered mid-attempt that giving `ChunkLoadEntryTail`
   a padded size of `0xC` (so `sub++` advances by the right amount) would
   corrupt `sizeof(ChunkLoadEntry)` if the padded type were embedded in
   it, and unpicking that cleanly was not finished this round. This is the
   most promising untried direction -- a `ChunkLoadEntryTail` sized 0xC used
   ONLY as a local walking-pointer's target type, kept entirely separate
   from the real (unpadded, 0xC via 3 flat fields) `ChunkLoadEntry`.

GCC 2.6.3's strength reduction appears to reliably PROVE that `arr1[i].ptr0`
and `arr1[i].rate`/`arr1[i].id` (or any sub-expression derived from indexing
the SAME array with the SAME index) share one linear family and collapses
them to one induction variable, REGARDLESS of how the C groups or aliases
the fields (attempts 1-4 all landed on the identical instruction count and
register set). Only fully severing the provable relationship (attempt 5)
produced two variables, but at the cost of extra addressing instructions
that made the total WORSE, not better. Attempt 6 (natural pointer
arithmetic instead of cast-based) is the untried lever most likely to
resolve this without the cast overhead, but needs the padding kept local
rather than baked into the shared struct.

## Attempts

6 manual structural variants (`arr1[i].field` plain / `entry->field]` /
nested-sub-indexed / nested-sub-with-per-iteration-pointer / manual
cast-based dual pointer walk / started-but-unfinished natural dual pointer
walk). Best and current-best: attempt 1 (also 2, 3, 4 -- all tied), 14/105
apparent, structurally 104/105 real instructions matching, one missing
`addiu`.

### Proposed learnings

**GCC 2.6.3's strength reduction IS controllable from source, but only by
changing the BASE, never by regrouping the fields.** Confirmed both ways on
this function: every shape that keeps one base (`arr1[i].fieldA` /
`arr1[i].fieldB`, an `&arr1[i]` element pointer, a nested
`arr1[i].sub.field`, a per-iteration sub-pointer) collapses to ONE
induction variable; two walking pointers whose TARGET TYPES are based at
different offsets into the same stride (`T *` at `arr1`, `U *` at
`arr1 + 4`, where `sizeof(U) == sizeof(T) ==` the stride) produce TWO. The
recipe, when retail shows N independently-incrementing walkers over one
array: define N types, each a view of the same stride starting at a
different field offset, keep them OUT of the real element struct, seed each
walker with one cast outside the loop, and advance with a natural `++`.
A byte-cast `+= stride` inside the loop reaches the same CFG but costs
extra addressing instructions.

**And a preserved body's own "untried direction" note is worth more than a
fresh derivation.** This function went 14/105 -> 90/105 in three builds
because the previous author wrote down exactly which lever they had not
pulled and why they could not pull it -- including the wrong reason
(struct-size corruption) that made it look blocked. Reading that reason
carefully is what showed it was avoidable.

**Superseded (kept for the record):** this report originally concluded
"GCC 2.6.3's strength reduction does not appear controllable from source
once it can PROVE two array accesses share a base+index" -- every
C-level regrouping of `arr1[i].fieldA` vs `arr1[i].fieldB` (plain indexing,
an intermediate element pointer, a nested sub-struct, a per-iteration
sub-pointer) produced the IDENTICAL single-induction-variable output in
this case, even though the documented "explicit intermediate element
pointer" idiom (`DECOMPILATION_LEARNINGS`, confirmed twice elsewhere in this
project) normally changes register allocation for array loops. **The
lever that finally splits an array walk into TWO independent pointers, if
there is one, is not a grouping change but a genuine SEVERING of the
provable relationship** -- e.g. two loop-local pointers whose relationship
to a shared base is established once, OUTSIDE the loop, and never
recomputed from `arr1[i]` inside it (closer to attempt 5/6 above) --
and even that needs the INCREMENT to be free (natural typed `ptr++`, sized
so the type's own stride equals the real array stride) rather than an
explicit byte-cast `+0xC`, which visibly costs extra instructions. Screen
for "two independently-incrementing registers walking what is provably one
array" early (compare the FIRST diverging address's instruction to see if
it's a lone extra `addiu $sN,$sN,<stride>` right at the loop-continue
branch) -- it is cheap to misdiagnose as a big structural bug when 104 of
105 instructions already agree.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004BB3C` | `StageMap__ApplyChunkLoads` | B | Occupant of `gStageMapMethods` +0x0FC (`slotFC`), verified via classtable as its own identity slot (already documented in `include/class_3bb8c.h`). Iterates the `ChunkLoadEntry[count]` array `StageMap__LoadChunksAround` just filled, resolving an `Elem` per entry (`slot118`) and either attaching (`ptr0 != 0`: `slot78`, sets `rate`, `flag = 1`) or detaching (`slot74`, `flag = 0`) it, then recomputes `self->unk1B4` via `StageMap__CountPendingLoads`. "Apply...Entries" mirrors the "Build...Entries" name of its own caller-side producer. |

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/stage_map.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/stage_grid.h), `Unk54Struct` is `LongVec3` (include/scene_node.h),
`EntryDesc866E8` is `Ratio16[3]` (include/scene_node.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

This function: `StageMap__ApplyRateEntries` -> `StageMap__ApplyChunkLoads` (`python3 tools/rename.py StageMap__ApplyRateEntries StageMap__ApplyChunkLoads`, tier B): per entry: event 6 to the slot, clear its cells, then start the entry's LbdFile load (chunkIndex, loadPending) or cancel the slot's load; then count the pending loads.

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals, tier A: `arr1` -> `entry` (the walked parameter), `sp` -> `tail` (the ChunkLoadEntryTail view), `e` -> `slot`. Constant: 6 -> `STAGEMAP_EVENT_SLOT_RELEASE`. The walker shape keeps a two-line `MATCHING` note in the function comment.

The comment that stood above the function in `src/world/dream_day.c`, moved here verbatim (its local names are the pre-track-7 ones):

```c
/* MATCH, round 73 (bravo): 105/105. Retail's `+4` walker is a
 * strength-reduced giv of the walked PARAMETER, not a second user
 * pointer: its init (`addiu s3,a1,4`) sits in the loop preheader after
 * the count guard and reads $a1, which is what loop.c emits when the biv
 * is `arr1` itself (initial value = the incoming argument register).
 * `sp` is therefore assigned from `arr1` inside the body and `arr1` is
 * advanced directly; the old `ep = arr1` copy is what swapped s3/s4.
 * See docs/match-reports/StageMap__ApplyChunkLoads.md. */
```

## History: track 12 (round 106, delta), comments moved out of the source

What the source said before track 12 moved it here (the one-line `MATCHING:` note stays in the .c):

```c
/* Starts each entry's load in the slot holding its neighbour key (after
 * clearing the cells of a chunk already linked there), or cancels the slot's
 * load for a NULL file; then counts the slots left pending.
 * MATCHING: `tail` is taken from `entry` inside the loop and `entry` itself
 * advances; a copy of the parameter swaps two saved registers. */
```
