# Entity__UpdateSoundCueStop

> Renamed from `func_8005E0B0` on 2026-09-23 (tools/rename.py). Address 0x8005e0b0.

**Unit:** Entity_b · **Size:** 40 words · **Status:** MATCHED (40/40 words,
whole-image build verified byte-exact)

## What it does

A close sibling of `Entity__UpdateTargetProximity`/`Entity__UpdateSoundCueStart` (both in `src/Entity_b.c`,
top of the file): another mood-row-driven "detach if too far" check, keyed off
`row->unkB` instead of `row->unk6`/`row->unkB` in the other two. Guarded by
`this->unkF0 != 0 && this->unkF8 != 0` (both must be true to enter the body,
unlike `Entity__UpdateSoundCueStart`'s three-way `&&` chain), and — unlike its two siblings,
which call `Entity__IsNearTarget` unconditionally once inside their guard — this one
only calls it when `row->unkB` is negative (checked via the sign bit before
computing the absolute value, not via a `!= 0` guard). Always returns
`this->unkF8`.

## Final C

```c
s32 Entity__UpdateSoundCueStop(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    if (this->unkF0 != 0 && this->unkF8 != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        dist = row->unkB;
        if (dist < 0) {
            dist = ~dist + 1;
            xptr = &this->unk14->x;
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) == 0) {
                this->methods->slot16C(this);
            }
        }
    }
    return this->unkF8;
}
```

`EntityMethods::slot16C` already existed in `include/Entity.h` (called by
`Entity__Deactivate` in `src/Entity.c`); only its caller-list comment was updated
to add this function.

## Attempt log

Two attempts. The first attempt used `Entity__IsNearTarget(...) != 0` (mirroring
`Entity__UpdateTargetProximity`/`Entity__UpdateSoundCueStart`'s sibling shape), which built at the right
LENGTH (40 words) but left one register-identical, condition-inverted branch
(`beqz` where retail has `bnez`, same target). Reading the raw retail bytes
directly (`asm/nonmatchings/Entity_b/Entity__UpdateSoundCueStop.s`) showed the call to
`slot16C` gated on the *opposite* polarity from its two siblings — this
function's own body genuinely differs from `Entity__UpdateTargetProximity`/`Entity__UpdateSoundCueStart`,
not just superficially. Flipping to `== 0` matched immediately.

## Proposed learning

**Sibling functions that look structurally identical can still invert one
comparison's polarity.** `Entity__UpdateTargetProximity`/`Entity__UpdateSoundCueStart` call their
"detach"-style vtable slot when `Entity__IsNearTarget(...) != 0`; this function
calls its own (`slot16C`) when the SAME callee returns `== 0`. Do not
transcribe a sibling's condition polarity without checking the actual
`bnez`/`beqz` in the function under test.

## Naming

`Entity__UpdateSoundCueStop` -- tier A (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E0B0`.

gEntityMethods +0x180; `Entity__Update` calls it only when +0x17C (`Entity__UpdateSoundCueStart`) returned non-zero. While `active` with a cue running, and only when the row's `cueRange` is NEGATIVE, it calls `stopSoundCue` (+0x16C, `Entity__StopSoundCue`) once the target is no longer within `|cueRange|`. It returns `soundCueActive`. The stop half of the pair named above.

## Proposed field names

| member | proposed | tier | evidence |
| --- | --- | --- | --- |
| `EntityMethods::slot180` (+0x180) | `updateSoundCueStop` | A | occupant is this function; only accessor is Entity__Update (Entity.c), so it is cross-unit |

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

Local `xptr` renamed `pos` (tier A). `~dist + 1` is MATCHING (measured; see Entity__UpdateTargetProximity.md).
