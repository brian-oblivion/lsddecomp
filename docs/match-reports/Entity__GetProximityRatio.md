# Entity__GetProximityRatio -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_8005D864` on 2026-09-19 (tools/rename.py). Address 0x8005d864.

Unit `Entity`. **56/56 words, byte-exact.** Never attempted before this round
(the head's round-44 correction withdrew the last stale "blocked" verdict --
see the superseded history below, kept for the record).

## What it does

Occupant of `EntityMethods::slot148` (see `include/Entity.h`'s own comment
at that slot, now updated). Gated on `this->unk94` being non-NULL; if so,
calls `this->methods->slot144(this, this->unk94)` (already documented in
`Entity.h` as returning a value, not void, and noting that retail keeps
`this->unk94` live in `$a1` from its first load all the way to this call --
which is exactly what happens here). Looks up a per-mood byte,
`gEntityProximityThresholdTable[this->moodIndex * 0x10]` (the same 16-byte-row family as
`gEntityUnlockKindTable`/`gEntityLinkStageTable`/`gEntityEventVideoTable`/`D_80089EAF`, already documented in
`Entity.h`; `gEntityProximityThresholdTable` itself was new -- added to that list, sitting
between `gEntityEventVideoTable` and `D_80089EAF`), shifts it left 11, and if that value
is less than the slot144 result, returns -1. Otherwise computes
`slot144_result / (threshold / this->unkB0)` (two chained integer
divisions, each expanding to the standard div-by-zero/`INT_MIN/-1`
`break 7`/`break 6` guarded sequence -- this is the `nop_mflo_mfhi`
construct the function's superseded stall history refers to, resolved
project-wide in round 42).

```c
s32 Entity__GetProximityRatio(Entity *this) {
    s32 result;
    Entity *self;
    s32 threshold;

    do {
        if (this->unk94 == NULL) {
            return -1;
        }
    } while (0);
    self = this;
    result = this->methods->slot144(this, self->unk94);
    threshold = gEntityProximityThresholdTable[self->moodIndex * 0x10] << 11;
    if (threshold < result) {
        return -1;
    }
    return result / (threshold / self->unkB0);
}
```

## New struct field

`Entity::unkB0` didn't exist yet -- it sat inside `padA0[0xF0 - 0xA0]`. Split
additively: `padA0[0xB0 - 0xA0]` + `s32 unkB0` + `padB4[0xF0 - 0xB4]`, same
total span (0x50 bytes), so no other already-matched `Entity` function's
offsets moved.

## The lever: a do/while(0) wrapper around the guard clause changes codegen, and this is REPRODUCIBLE, not folklore

First attempt (ordinary early-return guard, matching every other guard clause
already written in this file) built 55/56 words, one word SHORT, always the
same single spot: an extra `move a0,s0` sitting in the `jalr`'s delay slot
where retail has a plain `nop`. Every rewrite of the CALL itself --
caching `this->unk94` in a local, not caching it, an explicit function-
pointer intermediate for `slot144`, aliasing `this` into a second variable
for the receiver vs. the argument, declaration-order changes -- reproduced
the identical single-word diff. That is the tell that the cause isn't in the
call at all: it's something about the STATEMENT BEFORE it.

Ran the permuter (`tools/setup-permuter.sh Entity__GetProximityRatio <seed>`, seed = the
55/56 body) for a bounded search (`-j 6 --stop-on-zero --best-only`,
~1250 iterations, load acknowledged, not exhausted) and it found a byte-exact
candidate. Isolated the ONE change that mattered by bisecting the permuter's
output against the 55/56 base, verified directly with `compile.sh` (not
`build-and-verify.sh`, to iterate fast) and confirmed on the full project
build afterward: the permuter had wrapped the guard clause --

```c
if (this->unk94 == NULL) {
    return -1;
}
```

-- in a `do { ... } while (0);` loop. Confirmed by direct A/B compilation
(same source, only this one span changed) that:
- the plain `if` (with or without braces, with or without a bare `{ }`
  compound-statement wrapper) always produces the extra `move a0,s0`;
- ONLY the `do { if (...) { return -1; } } while (0);` form produces
  retail's plain `nop`.

Renaming the introduced second variable, reordering declarations, and NULL
vs `0` all made no difference in either direction -- the do/while(0) wrapper
around the FIRST guard clause is the entire and only cause. Plausible
mechanism: GCC 2.6.3's control-flow-graph construction treats a
`do`-loop as its own basic block distinct from a plain `if`, and that
changes which instructions the local scheduler considers available to fill
the FOLLOWING call's delay slot -- but this is a hypothesis, not something
verified against cc1's internals; the reproducible fact is the A/B pairing
above, not the explanation for it.

### Proposed learning

A near-miss that is exactly one word SHORT, where the only difference is a
dead-looking extra `move $an,$sM` sitting in a delay slot with everything
else -- including all the surrounding register choices -- identical, can be
a GCC 2.6.3 quirk tied to how an EARLIER, unrelated guard clause is
syntactically wrapped, not anything about the call itself. Rewriting the
call in every way that changes VALUES (caching, aliasing, function-pointer
indirection) will not touch it if the true cause is a CFG-shape difference
several statements upstream. When several call-focused rewrites all reproduce
the identical single-word diff, that itself is the signal to stop mutating
the call and permuter-search the WHOLE function body instead -- a bounded
permuter run (here, ~1250 iterations, well under half of one -j6 minute) is
cheaper than continuing to guess by hand once that signal shows up, and its
output can be bisected by hand-recompiling with `compile.sh` in the
permuter's own scaffold directory without needing another full project
rebuild per trial.

## Superseded history (kept for the record only -- see the REOPENED box the head added, now itself superseded by this match)

The original stub attributed the block to `addiu_at` (resolved round 21);
round 23 corrected the cause to `nop_mflo_mfhi` (resolved round 42); round 44
reopened it as assignable and it is now closed. Nothing in the corrected
cause history needed re-deriving -- both divisions in the final C are
ordinary `/` operators, and the pinned `--no-nop-mflo-mfhi` flag reproduced
retail's exact `mflo`/`div`/`break` sequence with no special handling.

## Naming

**Tier B.** Renamed from `func_8005D864` this round (tools/rename.py).
Occupies `EntityMethods` +0x148 (`tools/classtable.py`, confirmed CROSS-UNIT
-- Entity_b/c/d/e/f/g all dispatch through `slot148`). Computes
`slot144_result / (gEntityProximityThresholdTable_value / this->
proximityDivisor)`, or -1 when out of range -- a ratio (or sentinel), fully
described by the body; what the many cross-unit callers DO with that ratio
is not established from this unit alone, so "Ratio" rather than a stronger
claim.

## Proposed field names

- `EntityMethods::slot148` -> `getProximityRatio` -- **tier A.** Its
  occupant IS this very function (self-referential dispatch, the same idiom
  every other named slot in this table uses). CROSS-UNIT caller (already
  noted in `Entity.h`'s existing comment: "called by Entity__MoodCue05"),
  proposed rather than applied.

## Track 4 (2026-09-26, round 88, echo)

`proximityDivisor` (+0x0B0) is the embedded SoundCueSet's +0x14 (`soundCueSet.unk14`), the divisor InitSoundCueSet sets (10): the ratio is distance / (threshold / 10).

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
