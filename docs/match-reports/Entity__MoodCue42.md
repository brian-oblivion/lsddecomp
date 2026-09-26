# Entity__MoodCue42 -- MATCHED (70/70 words)

> Renamed from `func_800603C4` on 2026-09-24 (tools/rename.py). Address 0x800603c4.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue42(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue42(Entity *this, EntityMoodHandlerArg *out) {
    s32 divisor;

    if (this->unkFC < 0x14) {
        this->methods->slot130(this);
        this->methods->slotC4(this, -0x1E, 0);
    } else if (this->unkFC == 0x14) {
        this->methods->slot12C(this);
        out->unk10 = 0;
        out->unk1C = 5;
    } else {
        divisor = this->unk80 * 3 + 0x14;
        if (this->unkFC % divisor == 0) {
            this->methods->slot130(this);
            out->unk1C = -2;
        }
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed -- a plain three-way `if`/`else
if`/`else` on `this->unkFC` reproduced retail directly, including the
non-constant `%` (`this->unkFC % divisor`), which lowers to the standard
`div`/two-`break`/`mfhi` sequence maspsx expands for a runtime divisor (the
divisor here, `this->unk80 * 3 + 0x14`, is not known at compile time, so this
is NOT the magic-multiply constant-divisor case).

New vtable slot discovered: `EntityMethods::slot12C` (`void
(*)(Entity *self)`), called only by this function. Added to
`include/Entity.h`, replacing part of the `pad118` gap between `slot114` and
`slot130`. `slot130`'s own "called by" comment gained this function as a
second caller (it was previously only reached from `Entity__StopSoundCue`).

### Proposed learning

None beyond what's already documented -- this one was a clean first-attempt
match once the mood-dispatch family's established shape (three-way dispatch
on a small integer field, `this->methods->slotNN(this)` calls, `out->unkNN`
writes) was recognized from the sibling functions already matched in
`Entity_c.c`.

## Naming

`Entity__MoodCue42` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 42, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
