# Entity__MoodCue08

> Renamed from `func_8005E694` on 2026-09-23 (tools/rename.py). Address 0x8005e694.

**Unit:** Entity_b · **Size:** 23 words · **Status:** MATCHED (23/23 words,
whole-image build verified byte-exact)

## What it does

A short two-call dispatcher, not itself a `gEntityMoodHandlerTable` table entry (no
`EntityMoodHandlerArg` argument): `this->methods->slot48(this, 1,
SCALE_DOUBLE)` followed by `this->methods->slotBC(this, TRANSLATE_Y_MINUS64)`. Both
`SCALE_HALF`/`SCALE_DOUBLE`/`TRANSLATE_Y_MINUS64` are opaque data blobs only ever
address-taken (never dereferenced) by this unit's functions, so they are
declared as plain `u8[]` in `Entity_b.c`.

`slot48` is shared with `Entity__MoodCue17` (same table offset, same literal `1`
first argument) — see that report for why it is typed `s32`-returning rather
than `void`.

## Final C

```c
void Entity__MoodCue08(Entity *this) {
    this->methods->slot48(this, 1, SCALE_DOUBLE);
    this->methods->slotBC(this, TRANSLATE_Y_MINUS64);
}
```

## Attempt log

Matched on the first attempt once written after `Entity__MoodCue17` had already
established `slot48`'s signature and after the whole-unit address drift from
earlier residues was resolved.

## Proposed learning

None new — straightforward two-call body, no residue.

## Naming

`Entity__MoodCue08` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E694`.

`gEntityMoodHandlerTable` row 8. Body (takes only `this`): `updateScale(1, SCALE_DOUBLE)` then `addVec14(TRANSLATE_Y_MINUS64)`, every tick.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
