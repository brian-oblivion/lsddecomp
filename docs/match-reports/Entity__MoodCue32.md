# Entity__MoodCue32

> Renamed from `func_8005FA64` on 2026-09-24 (tools/rename.py). Address 0x8005fa64.

**Unit:** Entity · **Size:** 12 words · **Status:** MATCHED (12/12 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. A one-line wrapper: `this->methods->slotC4(this,
-0x1E, 0);` -- reuses `EntityMethods::slotC4`, already declared `void` in
`Entity`'s round.

## `slotC4`'s return type: a real conflict, resolved in favor of a
DIFFERENT already-matched function's own bytes

This function's own shape is the textbook "one-line wrapper" case CLAUDE.md
flags: `slotC4`'s return value is discarded at every OTHER known call site,
which per the "a discarded return is never evidence of void" rule is not
positive evidence either way, and the naive next step would be to retype
`slotC4` to `s32` and write `return this->methods->slotC4(...);` here (the
same move that resolved `slot48`/`Entity__MoodCue17` in `Entity`).

**That move was tried and reverted.** Retyping `EntityMethods::slotC4` to
`s32` compiles byte-identically for `Entity__MoodCue32` itself, but it also
changes `Entity__MoodCue00`'s OWN codegen — a function matched in the PREVIOUS
round, unrelated to this one except for also calling `slotC4` (discarding
the result, from two different branches, with the same literal arguments).
With `slotC4` `void`, GCC tail-merges those two identical discarded calls
into one shared call site (retail's actual shape). With `slotC4` retyped to
`s32`, GCC stops performing that merge, and `Entity__MoodCue00` grows 4 words
-- which, because this project keeps every function in strict ROM-address
order, shifts every later function in `entity.c` and would have silently
un-matched a function nobody was even touching this round. Verified with an
isolated `cpp | cc1` recompile of `entity.c` alone, diffing the `.s`
output line-for-line against the unmodified baseline.

`Entity__MoodCue00`'s own bytes are the stronger, more direct evidence (an
ACTUAL verified byte-exact match, not an inference from a single tail-call
shape), so `slotC4` stays `void`, and `Entity__MoodCue32` is written as a plain
`void` wrapper with a bare statement call, not `return`.

## Final C

```c
void Entity__MoodCue32(Entity *this) {
    this->methods->slotC4(this, -0x1E, 0);
}
```

## Attempt log

Matched on the first attempt (after settling the `slotC4` typing question
described above before writing this function at all).

## Proposed learning

**The "one-line wrapper" rule and "a discarded return is never evidence of
void" rule can point toward retyping a shared vtable slot to non-void, but
that retype has a blast radius beyond the function under test — check it
against EVERY caller's compiled output, not just the one motivating the
change.** Here, retyping would have been byte-safe for the function being
matched and would have silently broken a DIFFERENT, already-matched
function elsewhere in the same header's blast radius, by changing whether
GCC tail-merges two of that function's own discarded calls. When a vtable
slot has multiple discarding callers already matched, recompile ALL of them
(not just spot-check the one being worked) before committing to a retype
motivated by a single tail-call site. `slotCC` (`Entity__MoodCue37.md`, same
round) faced the identical question and was independently verified NOT to
have this problem — the two slots needed opposite answers despite looking
symmetric.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 32 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
