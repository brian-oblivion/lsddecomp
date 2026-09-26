# Entity__MoodCue56 -- MATCHED (16/16 words)

> Renamed from `func_80061158` on 2026-09-24 (tools/rename.py). Address 0x80061158.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Smallest function in the
unit's queue -- a one-line gated vtable dispatch, no `out` parameter.
`void Entity__MoodCue56(Entity *this)`.

## Final source

```c
void Entity__MoodCue56(Entity *this) {
    if (this->unkFC == 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed. Already-typed
`EntityMethods::slotCC` (`s32 (*)(Entity*, s32, s32)`) used as a bare
statement call, return value discarded (consistent with its existing header
comment -- both known callers discard or tail-return it, never assume
`void` from that alone, but here the assignment isn't attempted since the
call is a plain statement, not a `return`).

### Proposed learning

None -- trivial match, confirms `slotCC`'s existing typing needed no
changes.

## Naming

`Entity__MoodCue56` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 56, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
