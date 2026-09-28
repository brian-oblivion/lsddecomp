# Entity__MoodCue94

> Renamed from `func_80064294` on 2026-09-25 (tools/rename.py). Address 0x80064294.

**Unit:** Entity · **Size:** 111 words · **Status:** MATCHED (111/111 words)

## What it does

On `this->unkFC == 0`: `if (this->unk94->methods->slot200(this->unk94) !=
7) { this->unk44 = 0xB; }`. Unconditionally: `out->unk10 =
this->methods->slot148(this); if (out->unk4 % 10 == 0) { out->unk1C =
0xE; }`. On `this->unkFC == this->unk80`: `slot128(this,1)`; if
`this->unk44 != 0`, on a `rand()&1` miss calls `slot48(this,1,sScaleSix)`
and `slotCC(this,0x800,0)`; independently, on `rand()%3==0`, calls
`slot44(this,0,sRotationYawPlus180)`. Finally, `if (this->unk7C != 0) {
slotC4(this,-0x80,1); }`.

## Derivation

Direct transcription. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue94` | B | `gEntityMoodHandlerTable` row 94 |

Why `MoodCue94`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 94 (base 0x80089EB0, stride 0x10; the row's
first word), read from `disk/SLPS_015.56` directly rather than inferred from
address order (rounds 76-77 measured that row order does not track code
address). Nothing else references it. `Entity__StartSoundCue` hands the row's
handler to `InitSoundCueSet`, and `ServiceSoundCueSet` calls it once per tick
as `callback(owner, set)`, so `out` is the `SoundCueSet` (`EntityMoodHandlerArg`
is entity.h's local view; field readings in `Entity__MoodCue07.md`
`## Proposed field names`: `unk4` tick, `unk10` attenuation, `unk1C`/`unk30`/`unk44`
voice 0/1/2 tone request (-2 = stop), `unk20`/`unk34`/`unk48` pitch offset).
Tier B, same as every sibling `Entity__MoodCueNN` (Entity..Entity_g): the
row mapping is a fact of the binary, which dream object or state a row is
for is not established. Row kept decimal so names sort in table order.

What it does, in the unit's current field names: At tick 0, `moodState = 0xB` unless `target->slot200()` is 7; voice-0 tone 14 every 10 set ticks; at `moodDuration`: `slot128(1)`, and when `moodState != 0` a coin-flip `updateScale(1, sScaleSix)` + `slotCC(0x800, 0)`, and a 1/3 `updateRotation(0, sRotationYawPlus180)`; `slotC4(-0x80, 1)` while `unk7C != 0`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 95, bravo)

### Constants

- `getDreamColor() != 7` is `!= DREAM_COLOR_WHITE`: the slot returns `DreamColors` (include/dream_sys.h), whose eighth member is WHITE.
- `state = 0xB` is this handler's own phase, now 11.
- Every other literal went to decimal (tick counts, TOD frames, distances, VAB programs; no masks): they are this handler's tuning, named by nothing else.
