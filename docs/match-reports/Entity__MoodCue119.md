# Entity__MoodCue119

> Renamed from `func_8005F6D4` on 2026-09-24 (tools/rename.py). Address 0x8005f6d4.

**Unit:** Entity_c · **Size:** 13 words · **Status:** MATCHED (13/13 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> s32`. A one-line tail-call wrapper: `return
this->methods->slot48(this, 1, SCALE_SIX);` -- the same `slot48`
(already `s32`-returning, established in `Entity_b`'s `Entity__MoodCue17`) and
the same `arg1==1` convention as `Entity__MoodCue08`/`Entity__MoodCue17`, just with
a new data row.

## New symbol

`SCALE_SIX` -- a fifth `D_8008xxxx` opaque data-row extern, same
convention as the ones already declared in `Entity_b.c`. Declared fresh in
`Entity_c.c` (a separate translation unit, so it needs its own `extern`).

## Final C

```c
s32 Entity__MoodCue119(Entity *this) {
    return this->methods->slot48(this, 1, SCALE_SIX);
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 119 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

Now `void`: updateScale is SceneNode's void slot, and `return f();` and `f();` compile to the same bytes here (whole image green).

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
