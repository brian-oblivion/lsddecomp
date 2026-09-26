# Entity__MoodCue38

> Renamed from `func_8005FEF8` on 2026-09-24 (tools/rename.py). Address 0x8005fef8.

**Unit:** Entity_c · **Size:** 33 words · **Status:** MATCHED (33/33 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. Another mood-dispatch
handler, same family as `Entity_b`'s `Entity__MoodCue14`/`Entity__MoodCue09`/etc,
but with the `slot148` opener made CONDITIONAL rather than unconditional:

```c
void Entity__MoodCue38(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 120 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 1;
    }
}
```

The magic-multiply constant `0x88888889` at shift 6, reconstructed by
retail's own multiply-back (`*16` via `sll 4` minus itself = `*15`, then
`*8` via `sll 3` = `*120`), is division by 120 -- `out->unk4 % 120`
reproduces it directly, same idiom as `Entity_b`'s `Entity__MoodCue07` (divisor
90) and `Entity__MoodCue00`/`Entity__MoodCue13` (divisor 10/3).

## Attempt log

Matched on the first attempt.

## Proposed learning

None new -- confirms the mood-dispatch handler family extends into
`Entity_c` with the same idioms `Entity_b` established, including a new
variant (conditional rather than unconditional `slot148` opener).

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 38 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
