# Entity__MoodCue31

> Renamed from `func_8005F970` on 2026-09-24 (tools/rename.py). Address 0x8005f970.

**Unit:** Entity_c · **Size:** 61 words · **Status:** MATCHED (61/61 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. Same mood-dispatch
handler family as the rest of this unit, but opens with a positional-object
update instead of the usual `slot148` snapshot, and closes with a distance
check against `this->unk80` instead of the usual `slotC4` nudge:

```c
void Entity__MoodCue31(Entity *this, EntityMoodHandlerArg *out) {
    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
    this->unk94->methods->slot130(this->unk94, 1);
    if ((out->unk4 % 10) < 3) {
        out->unk10 = 0;
        out->unk1C = 0xD;
        out->unk30 = 0xD;
        out->unk44 = 0xD;
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot130(this);
        this->methods->slot30(this, 0xA);
    }
}
```

Two new call shapes, both filling in previously-unnamed slots with a second
caller:

- `this->unk94->methods->slot130(this->unk94, 1)` -- `Unk94Methods::slot130`
  was known only from `Entity__MoodCue01` (also called with literal `1`); this is
  its second confirmed caller, same signature.
- `this->methods->slot130(this)` and `this->methods->slot30(this, 0xA)` --
  `EntityMethods::slot130`/`slot30` were known from `Entity__StopSoundCue` /
  `Entity__SetTargetReached`+`Entity__NotifyIfTargetInRange` respectively; this adds a third/second
  caller with no new signature information.

The `mult`/`mfhi`/`sra`-`subu` sign-fix chain (magic constant `0x66666667`,
extra `sra $v1, $v1, 2` after `mfhi`) reconstructs `10 * quotient` via
`(q<<2 + q)<<1`, i.e. division by 10 -- `out->unk4 % 10` reproduces it
directly, same idiom as `Entity__MoodCue38` (divisor 120) and `Entity`'s
`Entity__MoodCue00`/`Entity__MoodCue13` (divisor 10/3). The guard compares the
remainder against `3` with `slti` (`< 3`), not `== 0` like the other
handlers in this unit -- a wider mood-trigger window, not a different
mechanism.

## Attempt log

Matched on the first attempt.

## Proposed learning

None new -- confirms `EntityMethods::slot130`/`slot30` and
`Unk94Methods::slot130` each have a second/third caller with no new
signature information, and that this unit's mood-dispatch handlers can gate
on `remainder < K` as well as `remainder == 0`.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 31 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
