# SceneNode__TryAttachNearby — MATCHED (round 76, bravo: 143/143, whole image green; the round-55 "stack-slot swap" was one struct local, a count header plus eight corners, with an unused 0x20-byte array ahead of it)

> Renamed from `Class6B5CC__TryAttachNearby` on 2026-09-26 (tools/rename.py). Address 0x8001d714.

> Renamed from `func_8001D714` on 2026-09-18 (tools/rename.py). Address 0x8001d714.

REVISITED, round 76: MATCHED 143/143 (build exit=0, whole-image SHA1 green); names/types used -- the fix is a TYPE: `count` and `buf54` are one `{s32 count; Vec3S16_d294 v[8];}` local, the shape `SceneNode__CheckBoundsOverlap`'s own `CornerList_d294` had already measured.

## Round 76 (bravo): MATCHED -- the swap was one struct, not two locals

**Rebuild first.** The round-55 preserved body, with round-54 field names
applied (`unkC`->`parent`, `unk14`->`coord2`, `slot38`->`onNotify`, no
offset change), rebuilt live: **140/143, insertions 0 / deletions 0,
positional skeleton diffs 0**. So this was a real equal-length residue, not a
false alignment. The three diffs were all stack offsets: `addiu a2,sp,0x54`
(built `0x30`), `sw v0,0x50(sp)` (built `0x80`), `addiu s2,sp,0x50` (built
`0x80`).

**Reading the retail frame, not the swap.** Retail: `diffRaw` at `sp+0x18`
(12 bytes), `diff` at `sp+0x28` (6 bytes), **nothing touches `sp+0x30..0x4F`**,
`count` at `sp+0x50`, `buf54` at `sp+0x54`, callee-saves at `sp+0x88`. So
`buf54` can only be 0x34 bytes, not the 0x4C bytes declared. And `count` sits
EXACTLY 4 bytes before `buf54`. `&count` goes to `checkBoundsOverlap` and
`classifyAgainstPlanes`, and `CheckBoundsOverlap` (matched round 73) reads
its `arg1` as `CornerList_d294`: `s32 count` at +0, corners from +4, and
`count*8` corners. `buf54` is where `composeAndApplyRotation` writes
`count*8` corners. So `&count` and `buf54` are `&list` and `list.v`, one
local: `{ s32 count; Vec3S16_d294 v[8]; }` = 0x34 bytes, 0x50..0x84, rounded
up to 0x88.

**Why no declaration order could fix it (the mechanism).** GCC 2.6.3 gives an
array or struct local its stack slot when it expands the DECLARATION. It gives
an address-taken scalar a slot only when it first expands `&x`
(`put_var_into_stack`), which is after every array declared in the block. So a
standalone scalar `count` always lands above `buf54`, whatever order the
declarations are in. That is why round 55's seven declaration-order variants
were all byte-identical. Putting the scalar inside the struct is the only way
to place it below the array.

**The 0x20-byte gap** at `sp+0x30` is an unused `u8 unused[0x20]` declared
before the list (the frame-padding idiom, §3e). Nothing in the body reads it.
It may be a MATRIX left in the source unused. Without it the list would sit
at 0x30.

Builds this round: 1 rebuild (140/143), 1 with a compile error (a
replace-all typo, `listList`; the grep caught it and the score was stale),
**1 build to the match**, then 1 simplification check: dropping round 55's
cached `countList` (`&other->unk30->unk4` inline) -> 141/143, ins/del 1/1
(the `lw a3`/`lw v1` pair reorders). So the cache is load-bearing and stays.
The goto-CFG form was not re-tested; round 55 measured it as the length fix.
No permuter. Gate 3 not run (no search spent).

### Proposed learning (round 76)

**When an address-taken scalar's slot is the wrong side of an array, and the
scalar's slot sits exactly `sizeof` before the array in retail, the two are
ONE struct.** 2.6.3 gives arrays and structs a slot at their declaration. It
gives an address-taken scalar a slot at its first `&`, so the scalar always
lands after every array in scope, and declaration order is inert (round 55
measured this seven times). The tell: the scalar's address and the array go
to related callees, often the SAME callee reading `p->count` and then
`p->items`. Read the callee's own struct view before touching declaration
order. A retail frame region nothing references is an unused array declared
earlier (§3e), and it is part of the same fix.

## History (rounds 13-55)

Unit: `code_d294_b`. Round 13, runner delta. Best score: 47/143 words
in-range, but a genuine size deficit remains (compiled body ~52 bytes/13
words shorter than retail). ~15 real attempts. Restored to `INCLUDE_ASM`.

## Signature (as attempted)

```c
void SceneNode__TryAttachNearby(SceneNodeObj *self, GenericObj_d294 *other);
```

Confirmed: this is `SceneNodeMethods`'s own `+0x0A0` slot occupant
(`self` dispatched as `$a0`); `other` (`$a1`) is a `GenericObj_d294 *`
(shares `unkC`/`methods` shape already established elsewhere in this unit,
plus three newly-discovered fields — see below).

## What it does

```c
void SceneNode__TryAttachNearby(SceneNodeObj *self, GenericObj_d294 *other) {
    LongVec3 *posA;
    LongVec3 *posB;
    LongVec3 diffRaw;
    Vec3S16_d294 diff;
    s32 count;
    u8 buf54[0x4C];

    if (self->unk20 == NULL) {
        return;
    }
    if (!TmdModel__GetBoundsCount(self->unk20)) {
        return;
    }

    posA = (other->unkC != NULL) ? (LongVec3 *)other->unk14->unk38 : NULL;
    diffRaw = *posA;

    posB = (self->unkC != NULL) ? (LongVec3 *)self->unk14->unk38 : NULL;
    diffRaw.x = diffRaw.x - posB->x;
    diffRaw.y = diffRaw.y - posB->y;
    diffRaw.z = diffRaw.z - posB->z;

    /* per-axis: if (|diffRaw.axis| >= 0x4001) return; -- see residue below
       for why this can't be written as a single unified check */

    diff.x = diffRaw.x;
    diff.y = diffRaw.y;
    diff.z = diffRaw.z;

    count = other->unk30->unk0;
    self->methods->slotA4(self, &diff, buf54, &other->unk30->unk4, count * 8);

    if (!self->methods->slotA8(self, &count, &diff)) {
        return;
    }
    if (!self->methods->slotAC(self, other->unk2C, &diff, &count)) {
        return;
    }

    self->unk28 = other;
    other->methods->slot38(other, self, 4);
}
```

Range-checks `other` against `self` (each axis of position difference must
fit in `+/-0x4000`), then hands off to three vtable slots — `+0xA4`
(`SceneNode__ComposeAndApplyRotation`, already matched this round), `+0xA8` (`SceneNode__CheckBoundsOverlap`,
still queued), `+0xAC` (`SceneNode__ClassifyAgainstPlanes`, the documented `gp_rel` blocker) —
with the resulting `Vec3S16` difference, before registering `other` into
`self->unk28` and notifying it via its own `+0x038` slot.

## New class knowledge (kept — MEASURED, not guessed)

- `SceneNodeSub14` gained `LongVec3 pos` at `+0x038` (previously opaque,
  part of `unk24[0x044-0x024]`). Both `self->unk14->unk38` and
  `other->unk14->unk38` are read through the identical offset in this one
  function — the evidence that established it.
- `GenericObj_d294` gained three fields: `unk14` (`SceneNodeSub14 *`, same
  shape as `SceneNodeObj`'s own `unk14` — MEASURED via the identical
  `+0x038` access, not a name-unification guess), `unk2C` (opaque,
  address-only), `unk30` (`GenericCountList_d294 *` — same shape/usage
  pattern as `SceneNode__TransformAndNotifyParents`'s own 2nd argument: `unk0` read once and
  multiplied by 8, `&unk4` forwarded as an opaque data pointer).
- `GenericMethods_d294` gained `+0x038` `slot38(GenericObj_d294 *self,
  SceneNodeObj *arg1, s32 arg2)`.
- `SceneNodeMethods` gained three slots: `+0xA4` (`slotA4`, same signature
  as the already-matched `SceneNode__ComposeAndApplyRotation` — this class's own vtable happens
  to point there, dispatched indirectly per retail's own `jalr`, not a
  direct `jal`), `+0xA8` (`slotA8`, `s32 (*)(SceneNodeObj*, void*,
  Vec3S16_d294*)`, occupant `SceneNode__CheckBoundsOverlap`), `+0xAC` (`slotAC`, `s32
  (*)(SceneNodeObj*, void*, Vec3S16_d294*, void*)`, occupant
  `SceneNode__ClassifyAgainstPlanes` — the documented blocker, NOT decompiled here, only its
  call-site shape is typed).
- **`self->unk28` retyped** from `s32` to `GenericObj_d294 *` (existing
  declaration change — see round summary). Checked the other write site
  (`SceneNode__DispatchLinkCommand`'s `self->unk28 = a1;`, `a1` still `s32`) compiles under
  the new type via ordinary int-to-pointer conversion (a warning, not an
  error) and rebuilt the WHOLE image to confirm `SceneNode__DispatchLinkCommand` (already
  matched) is still byte-exact before proceeding — this is the exact
  scenario CLAUDE.md's vtable-retype warning describes, and it came up
  again this round (see `SceneNode__NotifyTaggedParents`'s report for the other instance,
  a genuine struct-layout bug rather than a retype).

## What got the structure this close (in order — each one was real
## progress, verified by objdump against retail's own disassembly)

1. **Frame size** (0xA0/160 bytes): the "unused" 5th argument to the
   `slotA4` dispatch (`SceneNode__ComposeAndApplyRotation`'s own `count` parameter, computed as
   `other->unk30->unk0 * 8` and never read back in THIS function) has to
   live at `sp+0x10` per MIPS o32's stack-arg convention, and a second,
   otherwise-untouched output buffer (`buf54`) has to be sized 0x4C bytes —
   found empirically by matching the compiled frame size to retail's after
   two wrong guesses (0x34, sized from "whatever's left before the saved
   registers" — wrong; 0x4C, sized to also cover a genuine ~34-byte gap
   between the `Vec3S16` diff buffer and the `count` local that this
   project's compiler apparently leaves unpacked). **Do not re-derive this
   from first principles** — reproducing retail's exact frame size is a
   floor, not a proof the buffer's real size/type is known.
2. **Whole-struct copy before subtraction, not per-field subtraction.**
   Retail copies `*posA` into a stack-resident `LongVec3` WHOLESALE first
   (3 plain `lw`/`sw` pairs — `LongVec3`'s fields are `s32`, so no
   `lwl`/`lwr` needed, unlike the all-`s16` structs elsewhere this round),
   THEN computes `posB` and subtracts field-by-field IN PLACE (`diffRaw.x =
   diffRaw.x - posB->x;`, reading the just-stored value back off the
   stack). Writing it as a direct `diffRaw.x = posA->x - posB->x;` (no
   intermediate whole-struct copy) does NOT reproduce this — it keeps
   everything in registers and never touches the stack until the final
   `sh` stores, a completely different (shorter) instruction sequence.
3. **Per-axis `if (x >= 0) { check-positive } else { check-negated }`**,
   not a unified `abs = ...; if (abs >= K) return;`. Lever 1 from this
   round's head broadcast (`~x + 1` is not `-x`, confirmed independently
   here) explains the NEGATED side's shape; the reason a unified `abs`
   variable scores much worse is that it collapses BOTH the positive and
   negative in-range tests into one shared `slti`+`beqz` pair, which is
   NOT what retail does (see residue below) — retail duplicates the
   `slti`+`beqz` per branch.

## The residue: GCC merges a pair retail keeps separate, and no source
## reshaping tried so far stops it

For each axis, retail's positive-value branch and negative-value branch
each end in their OWN `slti $v0,...,0x4001` / `beqz $v0,.L8001D930`
instruction pair — SIX total across X/Y/Z, all jumping to the same single
epilogue label, never merged. This attempt's compiled output collapses
each axis's pair into ONE shared `slti`/`beqz` (the positive path jumps
INTO the middle of the negative path's own instructions to reach a shared
test) — THREE total instead of six, ~2 words shorter per axis, which is
most of the ~13-word overall size deficit.

**This did not move under any reshaping tried:**
- `if/else` with a per-branch `if (... >= 0x4001) return;` in each arm
  (the version quoted above) — merges.
- An explicit `goto`-based rewrite of the X-axis check reproducing
  retail's own literal jump graph (`bltz`→negative branch;
  positive-falls-through→check→`goto y_check`; negative branch→check→falls
  into `y_check`) — **produced BIT-IDENTICAL compiled output to the
  if/else version.** This is the most informative negative result in this
  report: for this specific pattern, `goto` and structured `if/else`
  reach GCC 2.6.3's RTL merge pass in a form it treats identically, unlike
  the already-documented cross-jump case in DECOMPILATION_LEARNINGS where
  `goto` DID prevent an unwanted merge. **The existing learnings entry
  ("write the literal jump graph with goto") is not a universal fix for
  this optimization — it worked in that instance and did not work here.**
- Reusing one `abs` variable across all three axes vs. three independently
  named `absX`/`absY`/`absZ` locals (block-scoped inside each `else`) — no
  difference; ruled out variable-reuse/liveness as the trigger.

**Not yet tried, flagged for the next attempt:** making the two branches'
trailing content genuinely non-identical at the RTL level (both currently
lower to a bare `return;` with no operand and no side effect — GCC 2.6.3's
cross-jump pass operates on already-generated RTL, so source-level
rephrasing that preserves an IDENTICAL final RTL shape for both tails
appears unable to stop it, based on the goto experiment above). A
permuter run seeding from this near-body may be more productive than
further hand reshaping — the CFG, register mapping, and frame are already
correct; the residue is a pure peephole-adjacent merge decision.

## Header changes kept

`include/code_d294.h`:
- `SceneNodeSub14`: added `LongVec3 pos` at `+0x038` (see above). Also
  **moved `LongVec3`'s typedef earlier in the file** (it was previously
  defined AFTER `SceneNodeSub14`, which only worked because nothing inside
  that struct referenced it before this round) — purely a reordering, no
  field/offset change.
- `GenericObj_d294`/`GenericMethods_d294`: added the fields/slot described
  above; also added a forward `typedef struct GenericCountList_d294
  GenericCountList_d294;` ahead of `GenericObj_d294` (full struct
  definition unmoved) so `unk30` could be typed to it.
- `SceneNodeMethods`: added `slotA4`/`slotA8`/`slotAC`.
- **`SceneNodeObj::unk28` retyped** `s32` → `GenericObj_d294 *` (existing
  declaration change, checked against both write sites — see above).
- Prototype for `SceneNode__TryAttachNearby` itself left in place.

## Proposed learning

**DECOMPILATION_LEARNINGS' "write the literal jump graph with goto" entry
needs a caveat, not a retraction:** it fixed a cross-jump merge in one
documented case, and this round found a second cross-jump-shaped residue
where an equivalent `goto` rewrite made no difference at all (bit-identical
compiled output to the `if/else` it replaced). The discriminator isn't
obvious yet from these two data points alone. Worth recording so the next
person hitting a stubborn tail-merge doesn't assume `goto` is a guaranteed
lever and burn attempts on rephrasing alone — verify with `objdump` after
the FIRST goto attempt before trying further variations of the same idea.

**A local variable that's read exactly once, right after being written by
an already-matched function's own call arguments, and never read again by
THIS function can still need real stack space matching retail's frame —
its "unused-looking" nature does not mean its size is free to guess low.**
`buf54` (this report) and `SceneNode__NotifyWithHull`'s own similarly-unread output
buffer (round 13, same unit) are now two independent instances of "size
the local to close a frame-size gap, not to what this function's own code
appears to need" — worth a shared idiom entry if a third instance turns up.

## Head note, round 13: `pos` was renamed to `unk38`

`SceneNodeSub14`'s +0x038 field is no longer called `pos`. Runner bravo,
matching `SceneNode__LocalOffsetToWorldPos` and `SceneNode__FaceTarget` in `code_d294_c` in the same
round, measured the SAME 12 bytes and needed them as an INDEXABLE
`s32 unk38[3]` — it takes the field's address and walks `[i]` for `i` in
0..2, adding each word into a caller-supplied vector as a per-axis delta.
A `LongVec3` cannot be indexed, and matched code decides: nothing matched
used the named-vector form, while two matched functions require the array.

The two readings are compatible — a position and a per-axis delta differ in
interpretation, not in shape — so only the name and the indexability
changed. The body above has been retargeted with a `(LongVec3 *)` cast on
the array so it still compiles as preserved, which is the point of
preserving it. If you resume this function, consider whether the cast is
hiding something: retail reads these 12 bytes both ways, and the honest
possibility is that the field is a union of the two views.

## Round 19 (echo): re-verified, literal per-axis body constructed and
## inlined, permuter deferred

Re-derived and built the literal per-axis body this report's "What it
does" section only sketched in prose (the `/* per-axis: if (|diffRaw.axis|
>= 0x4001) return; */` placeholder), using Lever 1's `~x + 1` negation
idiom explicitly for each axis's negative branch. Confirmed **47/143,
with the same ~13-word drift** this report already describes -- matches
exactly, no contamination, and the diff's shape (missing words clustered
around the per-axis check tail) is consistent with the documented
cross-jump merge.

Set up a permuter scaffold (`permuter-work/SceneNode__TryAttachNearby`, base score
2582 under `--stack-diffs`, `Stack Differences: 0` contribution --
consistent with this report's claim that the frame size itself is
already correct and the deficit is purely the cross-jump merge) and
launched a bounded background search -- but two searches were briefly
running at once (this one alongside an already-in-flight search for
`Class866E8__ResetAllElements`), which violates this round's "one bounded search at a
time" rule. Killed this function's search immediately upon noticing;
`Class866E8__ResetAllElements`'s continued undisturbed. This function's own permuter
pass is DEFERRED, not run, this round -- the scaffold is left in place
(`permuter-work/SceneNode__TryAttachNearby`, gitignored) for whoever picks this up
next, or for a future round of this same runner once `Class866E8__ResetAllElements`'s
search completes.

Inlined the literal per-axis body into `src/code_d294_b.c` as `#if 0`
(the report's own prose placeholder is preserved here as compilable
code for the first time). No header changes needed -- all supporting
types/slots were already present from the original round-13 pass.
Filing unchanged as STALL at 47/143, `INCLUDE_ASM` restored.

## Round 20 (alpha): the cross-jump lever this report needed -- found via
## permuter, closed 11 of the 13-word deficit (130 -> 141 words)

Re-verified first: dropped the preserved 47/143 body in live, confirmed
via `nm -S` on the built object that it compiles to exactly **130 words**
(`0x208` bytes) against retail's 143 (`0x23C`) -- the same 13-word deficit
this report has always described, no new drift.

**Set up a fresh permuter scaffold** (`permuter-work/SceneNode__TryAttachNearby`,
`tools/setup-permuter.sh`) -- the round-19 scaffold no longer exists in
this worktree (gitignored, lived in a different session's checkout).
Sanity-checked: `--debug --stack-diffs` gives base score 2582, matching
round 19's own recorded sanity check exactly. Ran a single bounded
background search (`timeout 600`, `-j 6`, one search at a time, per the
standing rule): **`permuter exit=0`** (the search's own zero-stop
triggered before the timeout, not the timeout firing), best score found:
**768** (down from base 2582), via a mutation that (a) reassigns the Y
positive-branch's magnitude check inside a spurious-looking
`self->methods || self->methods->slotAC` guard (junk, discarded), and
(b) rewrites the Z negative-branch check as `count = abs >= 0x4001; if
(count) { return; }` instead of `if (abs >= 0x4001) { return; }` --
i.e. routing the comparison through an existing local instead of testing
the expression directly inline.

**Per CLAUDE.md's "a permuter score drop is a LEAD, not a RESULT":
translated component (b) into the real source and verified against
`nm -S` + `funcdiff.py`, not the permuter's own score.** Applying it to
the Z-axis's negative branch ALONE: **130 -> 137 words** (+7, closing
more than half the deficit from a single one-line change). This is a
GENUINE lever: routing the "return if out of range" test through an
intermediate assignment, rather than testing the expression inline,
changes whether GCC's cross-jump pass finds this branch's tail
"identical enough" to the sibling branch's tail to merge them --
confirmed by reading the disassembly directly: the Z-axis positive and
negative branches now each end in their OWN separate `beqz`/`bnez`,
matching retail's un-merged structure, where before both funneled
through one shared `beqz`.

**Systematically explored applying the SAME lever to each of the six
branches (X/Y/Z x positive/negative), one combination at a time, keeping
only real oracle results (`nm -S` word count), not the permuter's
score:**

| combination applied | words (target 143) |
| --- | --- |
| none (baseline) | 130 |
| Z-negative only | 137 |
| Z-negative + Z-positive | 136 (worse than Z-neg alone) |
| Z-negative, using a fresh dedicated local instead of reusing `count` | 136 (worse) |
| X-negative + Z-negative | **141 (best)** |
| X-negative + Y-negative + Z-negative (all three negatives) | 147 (overshoot) |
| X-negative + Z-negative + Y-positive | 147 (overshoot) |
| X-negative + Z-negative + Z-positive | 136 (worse) |
| X-negative + Z-negative + X-positive | 138 (worse) |

**Best verified result: X-negative + Z-negative, 141/143 words (2-word
deficit, down from 13).** Every other combination tried over-corrects or
under-corrects; the lever is not simply "apply to more branches for more
effect" -- it interacts non-monotonically across branches, exactly the
character this project's other cross-jump/tail-merge residues have shown
(see `ClipSegmentToBox`'s own round-20 section, a different function, same
project). The remaining 2-word gap was not closed by any tried
combination.

**Read the actual instruction cost of the lever, since it is not free:**
applying `count = expr; if (count)` to a branch produces retail's
separated-branch structure but ALSO adds a spurious `xori $v0,$v0,0x1`
(inverting the test's polarity) and a `sw $v0,offset(sp)` (spilling
`count`'s value, even though it is unconditionally overwritten before any
read) -- 2 extra words per branch where applied, beyond what un-merging
alone would cost. This is why the lever does not simply "add per-branch"
cleanly to 143: each application both removes merge-related savings AND
adds this fixed overhead, and the net depends on which branches already
interact via cross-jumping with which others.

Preserved body updated below to the 141/143 best. Filing as STALL,
`INCLUDE_ASM` restored; confirmed clean rebuild (`build exit=0`,
whole-image OK, `git diff --stat src/code_d294_b.c` shows only the
preserved-body text differs from the committed state, no functional
change).

## Preserved body (round 20 best, 141/143, X-negative + Z-negative fixed)

```c
#if 0
void SceneNode__TryAttachNearby(SceneNodeObj *self, GenericObj_d294 *other) {
    LongVec3 *posA;
    LongVec3 *posB;
    LongVec3 diffRaw;
    Vec3S16_d294 diff;
    s32 count;
    u8 buf54[0x4C];
    s32 abs;

    if (self->unk20 == NULL) {
        return;
    }
    if (!TmdModel__GetBoundsCount(self->unk20)) {
        return;
    }

    posA = (other->unkC != NULL) ? (LongVec3 *)other->unk14->unk38 : NULL;
    diffRaw = *posA;

    posB = (self->unkC != NULL) ? (LongVec3 *)self->unk14->unk38 : NULL;
    diffRaw.x = diffRaw.x - posB->x;
    diffRaw.y = diffRaw.y - posB->y;
    diffRaw.z = diffRaw.z - posB->z;

    if (diffRaw.x >= 0) {
        if (diffRaw.x >= 0x4001) {
            return;
        }
    } else {
        abs = ~diffRaw.x + 1;
        count = abs >= 0x4001;
        if (count) {
            return;
        }
    }
    if (diffRaw.y >= 0) {
        if (diffRaw.y >= 0x4001) {
            return;
        }
    } else {
        abs = ~diffRaw.y + 1;
        if (abs >= 0x4001) {
            return;
        }
    }
    if (diffRaw.z >= 0) {
        if (diffRaw.z >= 0x4001) {
            return;
        }
    } else {
        abs = ~diffRaw.z + 1;
        count = abs >= 0x4001;
        if (count) {
            return;
        }
    }

    diff.x = diffRaw.x;
    diff.y = diffRaw.y;
    diff.z = diffRaw.z;

    count = other->unk30->unk0;
    self->methods->slotA4(self, &diff, buf54, &other->unk30->unk4, count * 8);

    if (!self->methods->slotA8(self, &count, &diff)) {
        return;
    }
    if (!self->methods->slotAC(self, other->unk2C, &diff, &count)) {
        return;
    }

    self->unk28 = other;
    other->methods->slot38(other, self, 4);
}
#endif
```

### Proposed learning

**"Routing a return-guard's comparison through an intermediate
assignment (`x = expr; if (x)`) instead of testing the expression inline
(`if (expr)`)" is a genuine, verified lever against GCC 2.6.3's cross-jump
tail-merge -- but it is NOT free (adds a polarity-inversion + spill per
application) and does NOT compose linearly across branches.** This
project's existing "write the literal jump graph with goto" entry
(closed a DIFFERENT cross-jump case) explicitly did NOT work here (round
13 confirmed bit-identical output); this is a DIFFERENT, independently
effective lever for the same optimization class. Try it per-branch and
verify with `nm -S`/`funcdiff.py` after EACH addition -- a combination
that looks like "the same fix, more places" can regress (this function:
applying to all three negative branches overshot by 4 words, when two of
the three closed 11 of 13). Escalate-do-not-experiment is premature for
this residue class; this lever alone took it from "no known lever" to
"2 words from exact" in one round.

Also: **the permuter's own candidate (mutation (a), the spurious
`self->methods` guard) was NOT the useful part of its 768 score** --
mutation (b) alone accounts for essentially all of the real
oracle-verified improvement. A permuter zero/improvement can bundle a
genuinely load-bearing change together with unrelated noise; isolate
each textual diff hunk and test it independently against the real oracle
before crediting the whole diff.

## Round 41 (bravo): did not reach as primary work; one confirmation check only

This function was item 4 of this round's ordered work list ("hardest,
reach only if the others land"). Items 1-3 (`SceneNode__NotifyTaggedParents`,
`ClipSegmentToBox`, `SceneNode__CheckBoundsOverlap`) consumed the round's attempt budget
first, per the assignment's own ordering, so this function got only a
single confirmation check, not a fresh attempt cycle.

**Re-verified the 141/143 claim live first**, per this round's "build
any inherited body before trusting its score" discipline: dropped the
preserved round-20 body into `src/code_d294_b.c`, confirmed via `nm -S`
on the built object that it compiles to exactly **`0x234` bytes = 141
words**, matching the report's own figure exactly (retail is `0x23C` =
143 words, the same 2-word deficit). No drift beyond what the 2-word
size gap itself causes
(confirmed via `build/lsdde.map`: `SceneNode__ComposeAndApplyRotation`, the very next
function, lands 8 bytes early, `0x8001d948` vs retail's `0x8001d950` --
exactly the 2-word shortfall, nothing more).

Read `tools/asm-differ/diff.py SceneNode__TryAttachNearby`'s realigned output in
full (not just the raw funcdiff word count, which is meaningless here
without realignment once the function's own length differs from
retail's) and found the residue is entirely contained in ONE five-
instruction block: retail's Y-axis POSITIVE branch (`if (diffRaw.y >=
0x4001) return;`) still ends in its own separate `slti`/`beqz`/`nop`/
`j`/`nop` (vram `0x8001DA18`-`0x8001DA28` retail-side), un-merged from
the negative branch's tail, while this build's Y-positive branch is
cross-jumped directly into the shared merge point. Every other part of
the function realigns cleanly with no other diff of any kind (only 5
lines in the entire ~1000-line realigned diff are flagged as a genuine
insertion/deletion, all in this one block).

**Tried applying the report's own already-verified lever
(`count = expr; if (count) return;`) to this exact branch, on top of the
committed 141/143 body** -- this is the one combination round 20's own
table calls "X-negative + Z-negative + Y-positive" and already records
as "147 (overshoot)". Reproduced EXACTLY: `nm -S` gives `0x24c` = 147
words, i.e. +6 words against the 141 baseline where the diff read above
predicted the missing block is only 5 words -- confirming round 20's own
finding rather than adding anything new. Reverted immediately (`git
diff --stat` empty afterward, whole-image `build exit=0`/`OK` confirmed).

**No new attempt was made beyond this one confirmation** -- per this
round's own explicit prioritization ("reach only if the others land")
and because round 20 already systematically explored the neighboring
combinations (see its table) without finding anything that lands
exactly on 143. Filing unchanged as STALL at 141/143 words (2 words
short), raw funcdiff word-match figure not meaningful without
realignment (see above), first realigned/real divergence at retail vram
`0x8001DA18` (the Y-axis positive-branch un-merge). `INCLUDE_ASM`
restored, `src/code_d294_b.c` confirmed byte-identical to the committed
state after the check.

### Proposed learning (round 41)

**This function's raw `funcdiff.py` word-match figure is not usable at
all once the function's own compiled length differs from retail's** --
the tool's per-function window is a fixed byte range keyed to retail's
own addresses, so a 2-word-short function reports a low raw match purely
from the tail of its own window aliasing onto the START of the NEXT
function's bytes, with no relation to how close the function actually
is. `tools/asm-differ/diff.py`'s realignment is the only trustworthy
read once ANY length mismatch is known to exist, not just a convenience
for readability -- this function's realigned diff shows all-but-5-of-143
words already correct, which the raw funcdiff score cannot show at all
(compare: this report's own history uses `nm -S` object size, not
`funcdiff.py`'s printed word-match count, to state this function's score
in every round including this one).

Confirms (does not extend) round 20's "the lever does not compose
linearly across branches" finding: the missing block is precisely 5
words, applying the lever there costs 6, and no partial or alternate
phrasing of that single branch was tried this round given the explicit
prioritization against it.

## Round 46 (echo): two Y-positive-branch variants tried, both overshoot, unchanged

Re-verified the 141/143 claim live first (per this round's own "build any
inherited body before trusting its score" discipline): `nm -S` on the
rebuilt object gives `0x234` = 141 words exactly, matching this report's
own figure with no drift. Then targeted the specific 5-instruction
residue round 41 localized (the Y-axis POSITIVE branch, still cross-jumped
into the shared merge point instead of ending in its own `slti`/`beqz`).

1. **Applying the report's own `count = expr; if (count)` lever to the
   Y-positive branch alone** (on top of the committed X-negative +
   Z-negative fixes): `nm -S` gives `0x24c` = **147 words** (+6 from the
   141 baseline). This reproduces round 20's own table entry for "X-negative
   + Z-negative + Y-positive" exactly (that combination is already recorded
   there as 147) -- not a new data point, just a confirmation via a
   differently-phrased edit reaching the identical byte count.
2. **A polarity-flipped variant avoiding the extra spill**
   (`if (diffRaw.y < 0x4001) { } else { return; }`, no intermediate
   `count` assignment): `nm -S` gives `0x248` = **146 words** (+5 from
   baseline) -- one word better than variant 1, but still an overshoot,
   not the exact +5 (from 141 to 143 needs only +2) the realigned diff
   says is missing. This confirms round 20's own "the lever does not
   compose linearly" finding from a different angle: even the CHEAPEST
   phrasing of "separate the Y-positive branch's tail" tried this round
   still costs more instructions than the 2-word gap it needs to fill,
   because separating the branch's tail from the shared merge point
   necessarily emits its own dedicated `slti`/`beqz`/`nop`/`j`/`nop`
   (5 words) MINUS whatever shared-block instructions become
   unreachable/prunable -- and in both variants tried, GCC did not prune
   as much of the old shared path as retail's own un-merged structure
   implies it should.

Both variants reverted immediately; `git diff --stat src/code_d294_b.c`
confirmed empty and whole-image build restored to `OK` after each check.
Filing unchanged as STALL at 141/143 words (2 short), first real diff
unchanged at retail vram `0x8001DA18`.

### Proposed learning (round 46)

**"Cheapest phrasing found so far still overshoots by more than the target
gap" is itself worth recording as a distinct outcome from "no lever found
at all" or "lever found, gap closed exactly".** This function's remaining
2-word deficit is bracketed between 141 (branch merged, too short) and 146
(branch fully separated with the cheapest tried phrasing, too long) with
no tried phrasing landing in between -- suggesting the exact retail
instruction sequence for this one branch uses a phrasing this round didn't
try (perhaps one that prunes a specific piece of the old shared block
without also duplicating unrelated overhead), not that the lever is
inapplicable outright.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `SceneNode__TryAttachNearby`
(tier B). STALL, still `INCLUDE_ASM`; not attempted for a match this
round (naming pass only). Slot `+0x0A0` occupant: range-checks `other`
against `self` (each axis of position difference must fit `+/-0x4000`,
via the two now-renamed geometry helpers' sibling checks), then hands
off to `SceneNode__ComposeAndApplyRotation` (slot `+0x0A4`),
`SceneNode__CheckBoundsOverlap` (slot `+0x0A8`), and
`SceneNode__ClassifyAgainstPlanes`/proposed `SceneNode__ClassifyAgainstPlanes` (slot
`+0x0AC`) in turn, only registering `other` into `self->unk28` and
notifying it (`other->methods->slot38`) if ALL three pass. "TryAttachNearby"
describes the measured gate-then-link mechanics; the game-level meaning
of "nearby" (why `+/-0x4000` specifically, what kind of object `other`
is) is not established. Held back from an actual rename because this
symbol is referenced (in comments) from `include/DreamSys.h:895` and
`include/code_2cc8c.h:752` -- two different units, both discussing this
function as cross-unit precedent (DreamSys.h for the vtable slot
resolution, code_2cc8c.h for the `switch`-vs-`if` residue). Posted to
the broadcast.

### Proposed field names (SceneNodeMethods, shared header)

This function's own dispatch chain touches four vtable slots that are
still named generically by offset (`slotA0`/`slotA4`/`slotA8`/`slotAC`)
in `include/code_d294.h`'s `SceneNodeMethods` struct. That struct is
shared: `slotA4` is also dispatched from `code_d294_c.c` (a different
unit, `self->methods->slotA4(self, 0, buf18, delta, 1)`), so per
FINISHING-PLAN track 3's field-ownership rule this is PROPOSED, not
renamed directly. Proposed slot names, backed by this round's function
renames/proposals:

| slot | current | proposed | occupant (this unit) |
| --- | --- | --- | --- |
| `+0x084` | `slot84` | `getRotMatrix` | `SceneNode__GetRotMatrix` (proposed `SceneNode__GetRotMatrix`) |
| `+0x08C` | `slot8C` | `readUnk20Data` | `SceneNode__GetModelHull` |
| `+0x090` | `slot90` | `transformAndNotifyParents` | `SceneNode__TransformAndNotifyParents` |
| `+0x0A0` | `slotA0` | `tryAttachNearby` | `SceneNode__TryAttachNearby` (proposed `SceneNode__TryAttachNearby`, this function) |
| `+0x0A4` | `slotA4` | `composeAndApplyRotation` | `SceneNode__ComposeAndApplyRotation` |
| `+0x0A8` | `slotA8` | `checkBoundsOverlap` | `SceneNode__CheckBoundsOverlap` |
| `+0x0AC` | `slotAC` | `classifyAgainstPlanes` | `SceneNode__ClassifyAgainstPlanes` (proposed `SceneNode__ClassifyAgainstPlanes`) |

Apply by type scope (edit `SceneNodeMethods`'s own definition in
`include/code_d294.h`, rebuild, fix exactly the accessors the compiler
lists), per CLAUDE.md's shared-struct-hazard rule -- a whole-tree text
replace of e.g. `slotA4` would corrupt every OTHER class's own
identically-named, unrelated `slotA4` field (measured: `class_3bb8c_f.c`,
`code_2cc8c_d.c`, `class_3bb8c_i.c`/`_j.c`/`_l.c` all have their own,
different `slotA4`).

## Round 55 (charlie): REVISITED (round 55) -- LENGTH closed exactly
## (141 -> 143/143), 2-word stack-layout residue remains, from 46/143 down
## to 3/143 raw mismatch

Assigned as a track-1 REVISIT job (FINISHING-PLAN.md's revisit rule): this
unit passed track 3 naming last round. Rebuilt the round-20 preserved body
(141/143, `nm -S` confirmed `0x234`) live first, per this round's "build any
inherited body before trusting its score" discipline -- reproduces exactly,
no drift.

**The round-54 renaming DID reach this function** (unlike its two siblings
this round) -- the preserved body already called through
`composeAndApplyRotation`/`checkBoundsOverlap`/`classifyAgainstPlanes`
rather than `slotA4`/`slotA8`/`slotAC`, since those are the exact renames
round 54 applied. But the rename itself is a pure accessor-name change with
no type or offset difference, so it gave no NEW shape on its own -- the real
progress this round came from re-reading `tools/asm-differ/diff.py`'s
realigned output fresh rather than from anything the naming pass supplied.

### Lever 1: the plain goto-CFG form, applied to ALL THREE axes, closes the
### length exactly

Round 41/46 had already localized the residue to ONE 5-instruction block:
retail's Y-axis POSITIVE branch ends in its own separate
`slti`/`beqz`/`nop`/`j`/`nop`, un-merged from the negative branch's tail,
while every attempt through round 46 only tried the `count = expr; if
(count) return;` intermediate-assignment lever on THAT one branch (always
overshooting: 146 or 147 words against a target of 143).

Re-read the disassembly for the X-axis block (which the round-20 lever
`count=expr;if(count)` DOES already close, at the cost of 2 extra words for
an `xori`+`sw` pair the lever introduces) and noticed retail's X-axis
positive branch has the IDENTICAL shape to the still-broken Y-axis: `bltz ->
negative branch; positive path falls through to its own slti+beqz(fail
return)+j(skip negative, success)`. The round-20/41/46 lever fixes X and Z's
NEGATIVE branch via an intermediate assignment (which is a different,
more expensive mechanism than what retail's own X/Z NEGATIVE branches
actually need), but had never been tried as a **plain goto rewrite mirroring
retail's actual jump graph on all three axes uniformly**:

```c
if (diffRaw.x < 0) {
    goto x_neg;
}
if (diffRaw.x < 0x4001) {
    goto x_done;
}
return;
x_neg:
    abs = ~diffRaw.x + 1;
    if (abs >= 0x4001) {
        return;
    }
x_done:
    /* ...same shape repeated for y_neg/y_done, z_neg/z_done... */
```

Applied to Y ALONE first (isolated test, keeping X/Z on the old
`count=expr;if(count)` lever): **bit-identical to retail for the ENTIRE
Y-axis block**, confirmed via `tools/asm-differ/diff.py` -- no `xori`, no
spill, exactly retail's `bltz`/`slti`/`beqz`/`j` sequence. This is the first
time in this function's history that ANY block closed with zero extra
overhead.

Applying the SAME plain-goto form to X and Z as well (replacing the
round-20 `count=expr;if(count)` lever entirely, not just adding to it):
**141 -> 143/143 words, `nm -S` confirms `0x23c` exactly matching retail's
`0x23c`, zero drift.** The round-13 note that "an explicit `goto`-based
rewrite of the X-axis check... produced BIT-IDENTICAL compiled output to the
if/else version" turns out to have been testing the WRONG goto shape (one
that still routed through an `if/else` with the negative branch computing
`abs` unconditionally); the plain "if negative goto neg; if in-range goto
done; return; neg: ...; done:" shape is a different, and here decisive,
source structure. DECOMPILATION_LEARNINGS' "write the literal jump graph
with goto" entry is vindicated on THIS residue, three rounds after this
report's own round-13 section reported it as bit-identical to if/else --
the discriminator was which of two nearly-identical goto shapes was tried,
not whether goto helps at all.

### Lever 2: caching the pointer chain closes the last-but-two words
### (138 -> 140/143), found by permuter

With length exact, the remaining residue (5/143 raw mismatch) was entirely
stack-offset noise from comparing against retail's addresses at the OLD
141-word layout; re-scoring after the length fix gave **138/143** with 3
clean `DIFF` lines, all stack-slot-address class (`addiu a2,sp,0x54` vs
`sp,0x30`; `sw v0,0x50(sp)` vs `0x80(sp)`; a swapped `lw a3`/`lw v1` order).

Ran Gate 3's three checks (PARALLEL-RUNS 3.5) before spending a search:

1. Scaffold compiles and scores: yes (`tools/setup-permuter.sh`, seed = the
   143-word goto-CFG body).
2. `--debug --stack-diffs`: base score **88** (48 stack-difference points, 8
   register-difference points, **0 insertions, 0 deletions**) -- a pure
   stack/register penalty, no structural difference, matching the 3-line
   hand diff exactly.
3. Scaffold/real-build signature AGREEMENT: the scaffold's penalty
   breakdown (stack + register only, zero insert/delete) matches the real
   build's own 3-line diff shape exactly -- AGREE, search is meaningful
   (not a whole-file artifact, not a scaffold-only signature).

Bounded search (`timeout 900`, `-j 6`, `--stop-on-zero --best-only
--stack-diffs`): rc=124 (bound fired), 79117 iterations, but **one
`output-58-1/` candidate WAS produced** (score 58, down from base 88) --
caching `other->unk30` into a local before the `composeAndApplyRotation`
call:

```c
count = other->unk30->unk0;
{
    GenericCountList_d294 *countList = other->unk30;
    self->methods->composeAndApplyRotation(self, &diff, buf54, &countList->unk4, count * 8);
}
```

instead of `&other->unk30->unk4` computed inline. Per CLAUDE.md's "a
permuter score drop is a LEAD, not a RESULT": translated to real source and
verified against `nm -S` + `funcdiff.py`, not the permuter's own score --
**138 -> 140/143, length still exactly 143 (`0x23c`), zero drift.** This
also incidentally fixed the `lw a3`/`lw v1` reordering diff for free (the
cached pointer changed instruction scheduling enough that the load order
now matches retail too), leaving only the two stack-slot-address lines.

**Follow-up hand attempts, all negative, all confirmed via real
`nm -S`/`funcdiff.py`, not permuter score:**

- Seven declaration-order variants (`count`/`diff`/`buf54`/`abs` in every
  relative order tried across rounds 13/19/20/41/46 plus new ones with
  `countList` inserted at different points; `countList`'s own scope
  narrowed to a nested block scoped tightly around its one use) -- **zero
  effect on the score in every case, still 140/143 byte-identical.**
  Consistent with this project's standing "C89 declaration order does not
  affect RTL emission order" finding (already established for this same
  function in round 13's own table).
- Caching `buf54`'s own address into a pointer local
  (`u8 *buf54Ptr = buf54;`, used at the call site) alongside the `countList`
  cache -- **zero effect, still 140/143.**
- A second, fresh permuter search (Gate 3 re-run: base score now **58** on
  the 140-word seed, still 0 insertions/0 deletions, confirmed AGREEMENT
  with the real build's now-2-line diff), bounded at 400s: **rc=124 (bound
  fired), 52653 iterations, no candidate ever beat the base score of 58** --
  confirmed via the raw score log's minimum (58, achieved only by the
  starting seed re-scoring itself) and the absence of any new `output-*`
  directory. Clean negative: this specific 2-word residue did not yield to
  either hand-driven declaration/caching variants or ~132000 combined
  permuter iterations across both searches this round.

**Remaining residue at 140/143 (or whatever the second search's outcome
leaves it at): two absolute stack-slot offsets trade places.** Retail puts
the scalar `count`'s own address-taken slot at the LOWER offset (`sp+0x50`)
and the `buf54` array at the HIGHER offset (`sp+0x54`, immediately
following); this build's compiled output puts them in the opposite relative
order (`buf54` low at `sp+0x30`, `count`'s slot high at `sp+0x80`). Frame
SIZE is already exactly correct (`0xa0` both sides) and every other
instruction in the function is now byte-identical -- this is purely a stack
SLOT ASSIGNMENT decision, unresponsive to every declaration-order and
caching variant tried. Flagging for whoever picks this up next: the
discriminator has not been found, but the search space is now extremely
narrow (2 words, 1 conceptual swap, 0 other differences anywhere in a
143-word function).

REVISITED (round 55): closed the LENGTH exactly (141 -> 143/143) via a
plain goto-CFG rewrite applied uniformly to all three axes (a genuinely new
source shape, not tried in this exact form in rounds 13/19/20/41/46), then
closed 2 more words via a permuter-found pointer-caching lever (138 ->
140/143), leaving a 2-word pure stack-slot-swap residue that survived seven
hand declaration/caching variants plus two bounded permuter searches
(79117 + 52653 = 131770 combined iterations, neither beating its own base
score). `INCLUDE_ASM` restored, preserved body updated below to the new
best, `git diff --stat src/code_d294_b.c` confirmed clean after the check.

### Proposed learning (round 55)

**Two goto rewrites that look like the same lever ("write the literal jump
graph") can differ in exactly the detail that decides whether the merge is
prevented.** Round 13 tried a goto form for this same X-axis block and
reported "bit-identical to if/else" -- correctly, for THAT shape (an
if/else-equivalent goto that still let the negative branch's `abs`
computation sit on the positive path's fall-through). The shape that
actually worked keeps the positive path's `return`-on-failure and
success-`goto` structurally separate from the negative label, with no
shared fall-through at all. Before concluding "goto doesn't help here" from
one attempt, check whether the specific CFG shape written actually differs
from the if/else the compiler would generate anyway -- read the disassembly
of the FIRST goto attempt, per this report's own round-19 learning, rather
than generalizing from a single negative.

**A permuter search that finds a real lever on a near-zero-insertion/
deletion scaffold is not "just register shuffling"** -- the `countList`
cache is a genuine, meaningful source change (an aliasing/CSE decision, not
a register pin), and it closed 2 words that seven hand-driven
declaration-order variants could not touch. Once a residue is down to a
handful of words with 0/0 insertions/deletions, a short bounded search is
cheap (79117 iterations in 900s here) and can still out-perform an
experienced hand sweep on the specific expression-level change needed.

## Preserved body (round 55 best, 140/143, length exact)

```c
#if 0
void SceneNode__TryAttachNearby(SceneNodeObj *self, GenericObj_d294 *other) {
    LongVec3 *posA;
    LongVec3 *posB;
    LongVec3 diffRaw;
    Vec3S16_d294 diff;
    s32 count;
    s32 abs;
    u8 buf54[0x4C];

    if (self->unk20 == NULL) {
        return;
    }
    if (!TmdModel__GetBoundsCount(self->unk20)) {
        return;
    }

    posA = (other->unkC != NULL) ? (LongVec3 *)other->unk14->unk38 : NULL;
    diffRaw = *posA;

    posB = (self->unkC != NULL) ? (LongVec3 *)self->unk14->unk38 : NULL;
    diffRaw.x = diffRaw.x - posB->x;
    diffRaw.y = diffRaw.y - posB->y;
    diffRaw.z = diffRaw.z - posB->z;

    if (diffRaw.x < 0) {
        goto x_neg;
    }
    if (diffRaw.x < 0x4001) {
        goto x_done;
    }
    return;
x_neg:
    abs = ~diffRaw.x + 1;
    if (abs >= 0x4001) {
        return;
    }
x_done:
    if (diffRaw.y < 0) {
        goto y_neg;
    }
    if (diffRaw.y < 0x4001) {
        goto y_done;
    }
    return;
y_neg:
    abs = ~diffRaw.y + 1;
    if (abs >= 0x4001) {
        return;
    }
y_done:
    if (diffRaw.z < 0) {
        goto z_neg;
    }
    if (diffRaw.z < 0x4001) {
        goto z_done;
    }
    return;
z_neg:
    abs = ~diffRaw.z + 1;
    if (abs >= 0x4001) {
        return;
    }
z_done:

    diff.x = diffRaw.x;
    diff.y = diffRaw.y;
    diff.z = diffRaw.z;

    count = other->unk30->unk0;
    {
        GenericCountList_d294 *countList = other->unk30;
        self->methods->composeAndApplyRotation(self, &diff, buf54, &countList->unk4, count * 8);
    }

    if (!self->methods->checkBoundsOverlap(self, &count, &diff)) {
        return;
    }
    if (!self->methods->classifyAgainstPlanes(self, other->unk2C, &diff, &count)) {
        return;
    }

    self->unk28 = other;
    other->methods->slot38(other, self, 4);
}
#endif
```

NON_MATCHING body promoted, round 69
