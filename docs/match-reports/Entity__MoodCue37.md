# Entity__MoodCue37

> Renamed from `func_8005FEC8` on 2026-09-24 (tools/rename.py). Address 0x8005fec8.

**Unit:** Entity_c · **Size:** 12 words · **Status:** MATCHED (12/12 words,
whole-image build verified byte-exact) — **now `void`, retyped round 68**

## What it does

`(Entity *this) -> s32`. A one-line tail-call wrapper: `return
this->methods->slotCC(this, -0x5A, 0);`.

## `slotCC` retyped to `s32`

Same "one-line wrapper" situation as `Entity__MoodCue32`/`slotC4`
(`Entity__MoodCue32.md`, same round) — `slotCC`'s only other known caller
(`Entity__MoodCue11`, matched the previous round) discards the result, and this
tail call is the first positive evidence either way. UNLIKE `slotC4`,
retyping `slotCC` to `s32` was verified NOT to perturb `Entity__MoodCue11`'s own
compiled output (isolated `cpp | cc1` recompile of `Entity_b.c`, diffed
line-for-line against the unmodified baseline — zero differences). With no
conflicting evidence, `slotCC` follows the ordinary "one-line wrapper" rule
and is retyped `s32 (*slotCC)(Entity *self, s32 arg1, s32 arg2)`.

## Final C

```c
s32 Entity__MoodCue37(Entity *this) {
    return this->methods->slotCC(this, -0x5A, 0);
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

See `Entity__MoodCue32.md` for the general rule this round established: verify
a shared-slot retype against every OTHER known caller's compiled output
before committing to it, since two superficially symmetric slots
(`slotC4`/`slotCC`, same struct, same signature shape) can have opposite
answers.


## ROUND 68 UPDATE: this function is `void`, and its bytes never said otherwise

`EntityMethods::slotCC` has been retyped back to `void`, and this function
with it:

```c
void Entity__MoodCue37(Entity *this) {
    this->methods->slotCC(this, -0x5A, 0);
}
```

**It compiles byte-identically both ways** — 12/12, whole image green, exactly
as before. So this wrapper's bytes were never positive evidence in either
direction, and the round that adopted `s32` was applying the one-line-wrapper
tie-breaker, not reading a fact off retail. That is what the rule is for and
the original call was reasonable; this note records that the tie-breaker has
since been overruled by an actual measurement.

The measurement is `Entity__MoodCue115` (Entity_d), a two-round stall that closes
198/198 the moment `slotCC` is `void`: retail tail-merges its `slotCC` call
with sibling `slotC4`/`slot134` calls that are `void`, and GCC 2.6.3 cannot
cross-jump a `(set (reg v0) (call ...))` against a bare `(call ...)`, so the
`s32` typing cost that function 6 words. Full mechanism and the general lever
are in `docs/match-reports/Entity__MoodCue115.md`.

**The gap in this report's original verification is the transferable part.**
The retype was checked against "every OTHER known caller's compiled output" —
but that check recompiled `Entity_b.c` only, because `Entity__MoodCue11` was the
only *known* caller at the time. `Entity__MoodCue115` was still `INCLUDE_ASM`, so
it was not a known caller and could not be checked, and an `INCLUDE_ASM`
function contributes retail's own bytes and therefore cannot register the
damage. **A shared-slot retype verified against the callers that happen to be
decompiled already is verified against a moving target**; the callers that
matter most are the ones still in assembly, and those are exactly the ones the
check cannot see. Nothing here was done wrong — but a retype adopted on a
tie-breaker rather than on evidence should be re-examined, not assumed settled,
whenever a later caller in the same slot stalls on a tail merge.
## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 37 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.
