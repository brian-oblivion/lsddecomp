# Entity__UpdateActivationState -- MATCHED (byte-exact, 74/74 words). Round 25, head.

> Renamed from `Entity__UpdateDetachState` on 2026-09-19 (tools/rename.py). Address 0x8005dbf0.

> Renamed from `func_8005DBF0` on 2026-09-19 (tools/rename.py). Address 0x8005dbf0.

> **ROUND 25 (2026-09-08), head. CLOSED, and the fix is PURE BLOCK PLACEMENT
> -- not one character of the logic below changed.** The eleven variants
> recorded in this report were all searching the expression/CFG-shape axis.
> The residue was on a third axis: WHERE the two `doDetach = 1;` writes sit
> RELATIVE TO EACH OTHER in the source text.
>
> This report already had the mechanism exactly right -- "retail spends 2
> EXTRA instructions to keep them separate ... this build's compiler lets the
> `detachKind==2` case simply fall through into the SAME `li $s2,1`". What it
> was missing is that **which write falls through is a source decision, and
> the rule is that GCC 2.6.3 gives the fallthrough to whichever one is
> LAST.**
>
> In the preserved body, `randCheck:` (with its `doDetach = 1;`) is nested
> INSIDE the `Entity__IsNearTarget != 0` arm, textually BEFORE the
> `else if (row->detachKind == 2)` arm. So the kind==2 write is last, it gets
> the fallthrough, and the two writes merge into one. Retail has the opposite:
> the kind==2 arm jumps (`j MERGE` with `li $s2, 0x1` in its own delay slot)
> and the rand arm's write is the one sitting immediately before the merge.
>
> **The fix is to lift `randCheck:` out of the nested `if` to the END of the
> gated block, after an explicit `goto merge`, so the rand write becomes the
> last one and the kind==2 write is forced to jump:**
>
> ```c
>         if (row->detachKind != 0) {
>             if (row->detachKind == 4) {
>                 goto randCheck;
>             }
>             if (row->unk5 != 0) {
>                 if (Entity__IsNearTarget(this, &this->unk14->x, row->unk5, row->unk9) != 0) {
>                     if (row->detachKind == 1) {
>                         doDetach = 1;
>                     } else if (row->detachKind == 3) {
>                         goto randCheck;
>                     }
>                 } else if (row->detachKind == 2) {
>                     doDetach = 1;          /* now NOT last -> retail's `j` */
>                 }
>             }
>         }
>         goto merge;
>
>     randCheck:
>         if ((rand() & 0x7F) == 0) {
>             doDetach = 1;                  /* now last -> gets the fallthrough */
>         }
>
>     merge:
>         if (doDetach) {
>             this->methods->slot15C(this);
>         }
> ```
>
> Every other line is byte-for-byte what this report already had, including
> the `detachKind == 4` / `detachKind == 3` shared `goto randCheck` that
> reaches retail's single `jal rand` from both arms. **74/74, whole-image
> SHA1 green.**
>
> ### Why the eleven prior variants could not find it
>
> Read the list below and every one of them permutes something INSIDE the
> nesting: switch versus if, which gate is tested first, which of
> `detachKind==1`/`==3` comes first, and a `__asm__("")` barrier. None of
> them moves a write OUT of the nesting, because the nesting is what encodes
> the logic and moving a statement out of it looks like it would change the
> meaning. It does not, once the `goto merge` is explicit -- and that is the
> whole trick.
>
> The report's own barrier result was the clue and was correctly interpreted:
> "no effect, confirming this is a cross-basic-block CFG/tail-merge decision,
> not an intra-block scheduling one, and therefore not something the
> permitted barrier can influence." That is right, and the missing next step
> is that a cross-basic-block decision has a source lever too -- it is just
> textual placement rather than a barrier.
>
> ### Proposed learning
>
> **This is the SECOND instance of round 25's block-order lever, in a
> different unit and a different shape, which is what makes it a rule rather
> than an anecdote.** `CheckDreamAuxTriggerCondition` (`dream_aux`) was an if/else arm that
> had to jump over a join; this is a DUPLICATED ASSIGNMENT where retail keeps
> both copies and GCC wants to merge them. Same underlying fact in both:
> **GCC 2.6.3 gives the fallthrough to the LAST candidate in source order, so
> if retail's jumping block is the one your source puts last, no amount of
> expression reshaping will fix it and no barrier will either.**
>
> The generalised tell, now confirmed twice: a bare unconditional `j` (not a
> conditional branch) to a nearby join, with REAL WORK in its delay slot.
> Retail's compiler had a block after that jump; your source has to put one
> there too. The fix is always textual: make the block you want to jump
> not-last, using an explicit `goto` over the block you want to fall through.
>
> **A twin worth re-reading:** this report notes `Entity__UpdateDeactivationState` is
> `Entity__UpdateActivationState`'s "near-identical twin", matched in the same earlier pass.
> That one is already matched, so nothing to do -- but the pattern
> generalises to any pair of sibling handlers where one matched and the other
> stalled 2 words short.
>
> **One dead cause to correct while here:** the "What it does" section below
> describes `Entity__IsNearTarget` as "`addiu_at`-blocked". `addiu_at` was RESOLVED
> in round 21 (`docs/research/addiu-at-blocker.md`); that parenthetical is a
> dead cause. `Entity__IsNearTarget` is matched C in this unit now.
>
> Everything below is the round-2026-09-01 derivation, kept because it is
> correct and because it is what made this hour's fix a five-minute change.


**Unit:** Entity · **Size:** 74 words · **Status:** STALL — 8 bytes / 2 words
short of byte-exact, everything else matches. Restored to `INCLUDE_ASM`.
Attempted round 2026-09-01 (runner bravo, Entity 11-function pass).

## What it does

Gated by `this->unkF0 == 0 && this->unk44 != 1`: looks up this entity's mood
row (`sEntityMoodTable[this->moodIndex]`) and decides whether to detach based on
`row->detachKind`:

- `detachKind == 4`: detach iff `(rand() & 0x7F) == 0`.
- otherwise, only if `row->unk5 != 0`: calls
  `Entity__IsNearTarget(this, &this->unk14->x, row->unk5, row->unk9)`
  (`addiu_at`-blocked, but its call-site shape is fully known). If that call
  returned non-zero: detach iff `detachKind == 1`, or (iff `detachKind ==
  3`) with the SAME `rand()&0x7F==0` check the `detachKind==4` arm uses — a
  genuinely shared code path (retail reaches the one `jal <rand>` from both
  the `detachKind==4` case and this `==3` case). If it returned zero: detach
  iff `detachKind == 2`.

Detaching calls `this->methods->slot15C(this)`.

This is Entity__UpdateDeactivationState's near-identical twin (see that report, matched in
the same pass) — same mood-row shape, same `Entity__IsNearTarget` call, same
"detach based on a small kind enum, with one shared branch" structure. The
techniques that closed DD18 (branchy `if`, not boolean-expression
assignment; per-branch reloads, not a shared "expected" temp) got this one
from 37 words short down to 8, but not the rest of the way.

## Final C (compiles, size-correct, NOT byte-exact — preserved for the next attempt)

```c
#if 0
s32 Entity__UpdateActivationState(Entity *this) {
    EntityMoodRow *row;
    s32 doDetach;

    if (this->unkF0 == 0 && this->unk44 != 1) {
        row = &sEntityMoodTable[this->moodIndex];
        doDetach = 0;
        if (row->detachKind != 0) {
            if (row->detachKind == 4) {
                goto randCheck;
            }
            if (row->unk5 != 0) {
                if (Entity__IsNearTarget(this, &this->unk14->x, row->unk5, row->unk9) != 0) {
                    if (row->detachKind == 1) {
                        doDetach = 1;
                    } else if (row->detachKind == 3) {
                    randCheck:
                        if ((rand() & 0x7F) == 0) {
                            doDetach = 1;
                        }
                    }
                } else if (row->detachKind == 2) {
                    doDetach = 1;
                }
            }
        }
        if (doDetach) {
            this->methods->slot15C(this);
        }
    }
    return this->unkF0;
}
#endif
```

## The residue

Retail's `detachKind == 2` case (the `Entity__IsNearTarget`-returned-zero arm) does
**not** share its `doDetach = 1;` with the `rand()==0` case's `doDetach =
1;`, even though both are the literal same statement reaching the literal
same merge point. Retail spends 2 EXTRA instructions to keep them separate:

```
; kind==2 case (reached when Entity__IsNearTarget returned 0):
lb   $v1, 0x3($s0)
li   $v0, 0x2
bne  $v1, $v0, MERGE
 nop
j    MERGE
 li  $s2, 0x1          ; <-- retail's OWN dedicated "doDetach=1", in the delay slot of an otherwise-unconditional jump

; ... elsewhere, the rand-check-passed case ...
li   $s2, 0x1          ; <-- a SECOND, textually distinct "doDetach=1"
MERGE:
beqz $s2, SKIP
```

This build's compiler (the SAME pinned toolchain) instead recognizes the two
`doDetach = 1;` writes as reaching an identical control-flow join and lets
the `detachKind==2` case simply fall through into the SAME `li $s2,1` the
rand-check-passed case uses — objectively fewer instructions, and correct,
but not what retail has.

## What was tried (11+ structural variants, all logically equivalent)

- Both `switch (row->detachKind) { case 3: ...; case 1: ...; }` (Duff's-device-style
  fallthrough matching m2c's own "irregular switch" reading) and the
  equivalent `if`/`else if`/goto form — byte-identical output either way, so
  switch-vs-if is not the axis that matters here.
- Both orderings of the primary gate (`if (Entity__IsNearTarget(...) != 0) {kind1/3}
  else if (kind==2) {...}` vs. the inverted `if (...== 0) {kind==2} else
  {kind1/3}`) — the inverted form does NOT reproduce retail's layout order
  either; instead it triggers a DIFFERENT unwanted optimization (GCC
  recognizes `if (kind==2) { doDetach=1; } /* nothing else in this arm */`
  as a boolean-settable pattern and compiles it via `xori`/`sltiu` instead
  of a branch — worse, and a new failure mode, not a step closer).
- Reordering which of `detachKind==1` / `detachKind==3` is checked first
  inside the `dist != 0` arm — changes the compiled layout, does not fix the
  duplication, and stops matching retail's proven-correct instruction ORDER
  for the parts that already matched.
- A scheduling barrier (`__asm__("")`, the one legitimate lever for
  order-only residues) placed both before and after the `detachKind==2`
  arm's `doDetach = 1;` — no effect, confirming this is a cross-basic-block
  CFG/tail-merge decision, not an intra-block scheduling one, and therefore
  not something the permitted barrier can influence.
- The argument-register lever (per the head's mid-round broadcast): checked
  every call in this function (`Entity__IsNearTarget` — already fully matched,
  4-argument call site confirmed correct; `rand()` — genuinely no
  arguments, confirmed against `include/psyq/rand.h`'s
  `extern int rand(void);`; `this->methods->slot15C(this)` — single-argument
  vtable dispatch, matches). **No hidden-argument instance found in this
  function** — every call's argument registers are fully accounted for.
  (This lever DID close the analogous residue in `Entity__UpdateDeactivationState` — see that
  report — so it was worth checking carefully here too; it just isn't the
  answer for this specific function.)

## Proposed learning

Add a fourth entry to `docs/MATCHING-GUIDE.md`'s residue list, tentatively:
**"Identical assignment, different merge points."** When two textually
identical simple statements (here, `doDetach = 1;`) are reached from
different predecessors and retail keeps them as two separate instructions
where this compiler tail-merges them into one, that is NOT a
declaration-order or scheduling residue — no reshaping of the surrounding
`if`/`else`/`switch`, no barrier, and no argument fix moved it in this case.
It may be specific to how many total predecessors converge on the shared
target (2 in `Entity__UpdateDeactivationState`'s analogous spot, which matched; 3 here, which
didn't) — worth testing on the next instance of this shape before spending
another 10+ attempts re-deriving the same negative result.

---

## Head follow-up, round 8 (2026-09-02) — permuter run, NEGATIVE

Still a stall. Recorded so the next reader knows this has now been attacked
with the permuter and not just by hand.

`tools/setup-permuter.sh` was set up this round and run against the preserved
body above. Scaffold verified faithful first (`--debug` base score 700 = 1
insertion + 3 deletions + 5 reorderings, which is the 2-net-word residue this
report describes). The search improved to **660 and no further** across a full
15-minute run at `-j 6`. No zero, and nothing that translated to a lead.

One hand test the report had not tried was also run: **inverting the
`Entity__IsNearTarget` guard so the returned-zero arm comes textually FIRST**, on the
theory that cross-jumping merges in one direction and reversing the arms would
change which block is the merge tail. It moved nothing (33 vs 34 diff lines in
an instruction-text comparison — noise, not a lever).

**What this narrows.** The residue is GCC's cross-jumping/tail-merging pass
collapsing two textually identical `doDetach = 1;` writes that retail keeps
separate. That is not a source SHAPE the permuter can reach by rewriting
expressions and control flow, which is consistent with it finding nothing: the
permuter searches shapes, and this is a decision the optimiser makes about
identical blocks regardless of how they are spelled.

There IS one known precedent for suppressing this on this project, and it is
not a shape change: round 7 found that `EntityMethods::slotC4` typed `void`
made GCC tail-merge two identical discarded calls in `Entity__MoodCue00`, and
retyping the slot to `s32` stopped the merge. **So the lever for tail-merge
suppression here, if one exists, is more likely to be a TYPE change than a
control-flow change** — something that makes the two blocks non-identical at
the RTL level without changing what the C says. Worth trying before another
permuter run: check whether `slot15C`'s signature, or `doDetach`'s type, or
`EntityMoodRow`'s field types make the two assignments differ.

Do NOT spend another permuter run on this without trying that first. And note
the per-slot warning from round 7: a slot retype is not local, so check every
other caller and the whole-image SHA1 before believing it.

## ROUND 20 (runner echo): tested the type-retype lever flagged above -- negative

Tested the specific untried lever this report's round-8 section flagged
(the `Entity__MoodCue00`/`EntityMethods::slotC4` precedent: retyping a slot's
return type suppressed an unwanted tail-merge of two identical discarded
calls). The only retypeable value in THIS function's residue is
`doDetach` itself (there is no discarded call return in play here, unlike
the precedent) -- tried narrowing it from `s32` to `s8`.

**Result: regressed, 33/74 with a genuine 125566-byte outside-range
drift** -- a real size change (narrower `doDetach` forces different store/
test instructions elsewhere, not just a suppressed merge), not a
neutral rephrasing. Reverted immediately. The type lever does not apply
here the way it did for the `slotC4` precedent: that case worked by
changing what the COMPILER SEES ABOUT A DISCARDED CALL RESULT (an
implicit dead-value analysis difference between `void` and `s32`
returns); here there is no call at either merge site, only a plain
`s32 = 1;` assignment, so there is no analogous "discarded value" axis
for a type change to perturb. Confirmed empirically rather than assumed.

**This function's residue, precisely characterized against
`Entity__MoodCue115`'s (this same round's OTHER tail-merge assignment) for the
coordinator's discriminator question:** `Entity__UpdateActivationState`'s residue is a
**whole-statement, single-level merge-count question** -- exactly THREE
predecessors reach an identical trivial statement (`doDetach = 1;`, one
instruction, `ori $s2,$zero,0x1`), and retail's cross-jump pass unifies
only TWO of them (the `detachKind==1` direct branch and the
rand-check-passed fallthrough), leaving the THIRD (`detachKind==2`) with
its own separate, textually-identical copy of the same one-word
statement plus its own now-unnecessary `j`. There is no partial-suffix
question here -- the merged/unmerged unit IS the entire content of each
predecessor's block (one instruction). This is the simplest possible
version of the phenomenon, and it is the one where NEITHER of this
round's two tested levers (type retype here; operand/statement reshaping
in the original round-8 pass) reaches it.

**Disposition unchanged: STALL at 72/74 words (8 bytes / 2 words short),
`INCLUDE_ASM` restored.** PERMUTER-EXHAUSTED (round 8, 15-minute run,
floor 660 vs base 700, no zero). No new lever found this round; the
type-retype avenue this report flagged as the one remaining untried idea
is now a confirmed negative, not an open thread.

## Naming

**Tier B, CORRECTED this round.** Originally renamed `func_8005DBF0` to
`Entity__UpdateDetachState` earlier in round 56; that was backwards.
`tools/classtable.py` on `gEntityMethods` shows this table's own
self-referential occupants: +0x15C is `Entity__Activate`, +0x160 is
`Entity__Deactivate`. This function is gated on `this->unkF0 == 0` (NOT
active) and, when its `row->detachKind`-derived condition fires, calls
`this->methods->activate(this)` (the +0x15C slot, renamed this round) --
i.e. it ACTIVATES, not detaches. Corrected to `Entity__UpdateActivationState`
via `tools/rename.py`.

`EntityMoodRow::detachKind`/`linkKind` and the local `doDetach` variable are
deliberately left untouched. A plausible reading survives the correction
intact: "detach" as leaving an idle/linked state (i.e. BECOMING active) and
"link" as the reverse would make the ORIGINAL field names right all along,
and only this function's OWN name (built around the wrong axis -- "what
does detachKind gate" rather than "what does the gated call do") was wrong.
Recorded here rather than adjudicated, since both readings are consistent
with the bytes and re-litigating the row-field names is Track 4 territory
(they are entity.c-exclusive, confirmed by grep, so nothing stops a future
round from revisiting it with more cross-unit context).

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 2: EntityMoodRow::unk5 (+0x05) -> activeRange, tier B: its two readers (this body and its sibling) pass it as Entity__IsNearTarget's distance and skip the range test when it is 0; no other accessor (compiler error list).

- Step 3: local doDetach -> doActivate (the flag gates activate).

- Step 2: EntityMoodRow::detachKind -> activateKind (tier A): it selects the condition under which this body calls activate, and Entity__AttachToParent activates at once when it is 0. The Naming section above left it open between two readings; the table slots settle it. Only entity.c reads it.

- Step 4: activateKind 0..4 -> enum EntityActivateKind (ENTITY_ACTIVATE_AT_ATTACH/NEAR/FAR/NEAR_RANDOM/RANDOM, entity.h); state 1 -> ENTITY_STATE_DONE (entity.h already says this function tests it).

- Step 5: MATCHING line for the goto layout (randCheck after the block; see the fix above).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
