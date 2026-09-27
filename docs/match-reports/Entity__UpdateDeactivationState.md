# Entity__UpdateDeactivationState

> Renamed from `Entity__UpdateLinkState` on 2026-09-19 (tools/rename.py). Address 0x8005dd18.

> Renamed from `func_8005DD18` on 2026-09-19 (tools/rename.py). Address 0x8005dd18.

**Unit:** Entity · **Size:** 64 words · **Status:** MATCHED (64/64 words, whole-image build verified byte-exact)

## What it does

Gated by `this->unkF0` (set by `Entity__Activate`, cleared by `Entity__Deactivate`):
looks up this entity's mood row (`gEntityMoodTable[this->moodIndex]`), calls
`Entity__NotifyIfTargetInRange(this, 0)` (still-uncarved, in `Entity_b` — see "Proposed
learning" below for the second argument), then decides whether to detach
based on `row->linkKind`:

- `linkKind == 0` or `== 3`: no detach.
- `linkKind >= 10`: detach iff `(row->linkKind * 15) ^ this->unk24 == 0`.
- otherwise (1, 2, 4..9), only if `row->unk5 != 0`: calls
  `Entity__IsNearTarget(this, &this->unk14->x, row->unk5, row->unk9)` (still
  `addiu_at`-blocked, but its own call-site shape is fully known from its
  `.s` file) and detaches iff `linkKind == 1` when that call returned
  non-zero, or `linkKind == 2` when it returned zero.

Detaching calls `this->methods->slot160(this)` — the same slot
`Entity__DetachFromParent`/`Entity__OnGridCellLinkCommand` dispatch through.

## Final C

```c
s32 Entity__UpdateDeactivationState(Entity *this) {
    EntityMoodRow *row;
    s32 doDetach;
    s32 dist;
    s32 scaled;

    if (this->unkF0 != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        doDetach = 0;
        Entity__NotifyIfTargetInRange(this, 0);
        if (row->linkKind != 0 && row->linkKind != 3) {
            if (row->linkKind >= 10) {
                scaled = this->unk24;
                if ((scaled ^ (row->linkKind * 15)) == 0) {
                    doDetach = 1;
                }
            } else if (row->unk5 != 0) {
                dist = Entity__IsNearTarget(this, &this->unk14->x, row->unk5, row->unk9);
                if (dist != 0) {
                    if (row->linkKind == 1) {
                        doDetach = 1;
                    }
                } else if (row->linkKind == 2) {
                    doDetach = 1;
                }
            }
        }
        if (doDetach) {
            this->methods->slot160(this);
        }
    }
    return this->unkF0;
}
```

## Attempt log

Started at 37 words short (this function's queue-order predecessor,
`Entity__UpdateActivationState`, and this one share almost identical logic shapes — see that
report). Three separate residues, closed one at a time:

1. **Boolean-expression assignments compile to arithmetic, not branches.**
   `doDetach = ((row->linkKind*15) ^ this->unk24) == 0;` and
   `doDetach = row->linkKind == (dist != 0 ? 1 : 2);` each got turned into
   `xori`/`sltiu` compare-and-set sequences by GCC — shorter and
   register-cheaper than what retail actually has, which is genuine branches
   (`bne`/`beqz` to a shared `li $s2,1`). Retail's C almost certainly wrote
   these as plain `if (...) { doDetach = 1; }`, not as a direct boolean
   assignment; rewriting both this way closed a 20-word gap immediately
   and, as a side effect, moved `doDetach` from a caller-saved temp (`$v1`,
   dead across the `Entity__IsNearTarget` call — a LATENT BUG, since `doDetach`
   genuinely needs to survive that call on one path) into the correct
   callee-saved `$s2` your prologue already reserves for it.
2. Retail computes the `dist != 0` "expected value" as two SEPARATE
   `lbu $v1,4($s1)` reloads (one per branch of the `dist` check), each
   followed by its own `li`/compare, joined by an explicit `j`, rather than
   computing "expected" once and comparing after a single shared reload —
   even though the shared-reload form is objectively fewer instructions.
   Writing the `if (dist != 0) { if (linkKind==1) ... } else if (linkKind==2)
   ...` form directly (no `expected` intermediate) reproduced retail's
   redundant-reload shape exactly.
3. **The argument-register lever** (flagged by the head mid-round): one
   instruction never moved no matter how `doDetach`/`row` were reshaped —
   retail's very first branch (`beqz $v0,SKIP` on `this->unkF0`) fills its
   delay slot with `move $a1,zero`, and `$a1` is never overwritten before
   the next `jal` (`Entity__NotifyIfTargetInRange`). Tracing forward per the lever's test:
   `$a1` is live into that call, so it's an ARGUMENT, not scheduler filler —
   even though `Entity__NotifyIfTargetInRange`'s own body immediately overwrites its
   incoming `$a1` with `sll $a1,$v0,4` and so provably never reads the
   caller's value. Changing the call to `Entity__NotifyIfTargetInRange(this, 0)` (and its
   extern prototype to take the unused second `s32` parameter) closed the
   very last word.

## Proposed learning

**Confirms the head's mid-round lever, with a wrinkle worth recording:** the
argument-register test (trace forward from a suspicious load/const-set to
the next `jal`; if the register is `$a0`-`$a3` and nothing overwrites it
first, it's an argument) caught a real case here — but the callee
(`Entity__NotifyIfTargetInRange`) *does not use* the parameter internally at all (it's
clobbered as scratch before any read). A function can have a dead parameter
that's still part of its real signature and must still be passed by every
caller; "the callee doesn't seem to read it" is not evidence the caller
doesn't pass it. Anyone carving `Entity__NotifyIfTargetInRange` out of `Entity_b` should
give it a real 2-parameter signature (`Entity *this, s32 arg1`) even though
`arg1` looks unused in its body — other call sites may rely on side effects
this one doesn't need, or it may simply be dead in the source too.

## Naming

**Tier B, CORRECTED this round.** Originally `Entity__UpdateLinkState`.
Same `tools/classtable.py` finding as `Entity__UpdateActivationState.md`:
this function is gated on `this->unkF0 != 0` (active) and, when its
`row->linkKind`-derived condition fires, calls `this->methods->slot160(this)`
-- confirmed to resolve to `Entity__Deactivate` for a base Entity. So this
one WAS already correctly directioned by its old name ("Link" conditions
trigger detach/deactivate) -- it is its sibling, `func_8005DBF0`, that had
the mismatch. Renamed anyway for a matching, symmetric pair with the
corrected `Entity__UpdateActivationState`. See that report for the full
reasoning on why `detachKind`/`linkKind` are left alone.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 2: EntityMoodRow::unk5 (+0x05) -> activeRange, tier B: its two readers (this body and its sibling) pass it as Entity__IsNearTarget's distance and skip the range test when it is 0; no other accessor (compiler error list).

- Step 3: locals doDetach, dist, scaled -> doDeactivate, near, tick.

- Step 2: EntityMoodRow::linkKind -> deactivateKind (tier A): it selects the condition under which this body calls deactivate; nothing links. Only Entity.c reads it.
