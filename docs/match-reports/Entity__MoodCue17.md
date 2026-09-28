# Entity__MoodCue17

> Renamed from `func_8005EF20` on 2026-09-23 (tools/rename.py). Address 0x8005ef20.

**Unit:** Entity · **Size:** 13 words · **Status:** MATCHED (13/13 words,
whole-image build verified byte-exact)

## What it does

Another `gEntityMoodHandlerTable` mood-dispatch table entry (see `Entity__MoodCue05`'s
report), but a one-line tail-call wrapper: `return this->methods->slot48(this,
1, sScaleHalf);`. `slot48` is shared with `Entity__MoodCue08` (same offset, same
literal `1` first argument), which discards the result — but per CLAUDE.md's
"one-line wrapper" rule, a discarded return at ONE call site is never evidence
the callee is `void`, and this call site's own bytes (the vtable call is the
very last operation before the epilogue, with nothing overwriting `$v0`
afterward) are consistent with the return value flowing straight through.
`slot48` and this function are therefore both typed `s32`-returning.

## Final C

```c
s32 Entity__MoodCue17(Entity *this) {
    return this->methods->slot48(this, 1, sScaleHalf);
}
```

## Attempt log

Matched on the first attempt, once `slot48` was typed `s32` in
`include/Entity.h` (the first attempt with `slot48` declared `void` failed to
compile at all — `void value not ignored as it ought to be` — which is itself
useful confirmation: a `void`-typed slot here doesn't even type-check against
`return`, whereas the correct type compiles AND matches immediately).

## Proposed learning

**A compile error can itself be a signal, not just noise.** `return
this->methods->slot48(...)` against a `void`-declared function pointer fails
cc1's own type check before ever reaching byte comparison — worth remembering
as an extremely cheap first check when guessing whether a vtable slot is
`void` or value-returning: if the natural "one-line wrapper" spelling doesn't
even compile, that's a strong hint the slot's default typing is wrong, not
that the source must have discarded the value some other way.

## Naming

`Entity__MoodCue17` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005EF20`.

`gEntityMoodHandlerTable` row 17. Body (takes only `this`): `return updateScale(this, 1, sScaleHalf);`. Note that `updateScale`'s occupant `func_8001D008` is `void`. The `s32` slot type is carried only by this wrapper's `return`, which is the same shape the round-68 slotCC retype found non-evidential. Not retyped here, because the slot is cross-unit.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

Now `void`: updateScale is SceneNode's void slot, and `return f();` and `f();` compile to the same bytes here (whole image green).

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

Nothing to change: the body uses only the named template sScaleHalf.
