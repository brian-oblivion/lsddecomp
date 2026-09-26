# Entity__MoodCue75 -- MATCHED (39/39 words)

> Renamed from `func_800628D4` on 2026-09-24 (tools/rename.py). Address 0x800628d4.

Unit: `Entity_e` (round 12). Calls `Class6B5CC__FaceTarget` with the same
`(this, this->unk94, 1, 0, 0)` argument shape already established in
`Entity_d.c`'s handlers, plus a same-value dispatch through
`EntityMethods::slot130`/`slot30`.
`void Entity__MoodCue75(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue75(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot130(this);
        this->methods->slot30(this, 0xA);
    }
}
```

## Derivation notes

Matched first attempt (once written after `Entity__MoodCue73`'s lesson about
delay-slot placement was fresh -- double-checked here that `sw zero,
0x10(a1)` really is straight-line code following the `bnez`, not the
branch's delay slot, before nesting it inside the `if`; in this function it
genuinely is guarded, unlike `Entity__MoodCue73`). No new struct or vtable
knowledge -- `slot130`, `slot30`, and `Class6B5CC__FaceTarget`'s signature were all
already established.

### Proposed learning

None new; this one is a clean confirmation that the delay-slot-placement
check from `Entity__MoodCue73`'s report generalizes correctly rather than
over-correcting into "always hoist stores out of `if` bodies".

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 75 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
