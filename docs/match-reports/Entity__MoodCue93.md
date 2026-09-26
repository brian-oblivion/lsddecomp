# Entity__MoodCue93

> Renamed from `func_800641C0` on 2026-09-25 (tools/rename.py). Address 0x800641c0.

**Unit:** Entity_f · **Size:** 53 words · **Status:** MATCHED (53/53 words)

## What it does

`out->unk10 = this->methods->slot148(this); if (out->unk4 % 10 == 0) {
out->unk1C = 3; } if (this->unkFC == this->unk80) { slot128(this,1); }
if (this->unk7C == 1) { slotC4(this,-0x80,1); }`.

## Derivation

Direct transcription; the `%10` magic-multiply expansion (`0x66666667`)
matched the plain `%` operator immediately, consistent with this
project's established idiom for constant-divisor modulus. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue93` | B | `gEntityMoodHandlerTable` row 93 and 107 |

Why `MoodCue93`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 93 (and row 107, same handler word, different data words -- named for its lower row, as `Entity__MoodCue30`/`81`/`123`) (base 0x80089EB0, stride 0x10; the row's
first word), read from `disk/SLPS_015.56` directly rather than inferred from
address order (rounds 76-77 measured that row order does not track code
address). Nothing else references it. `Entity__StartSoundCue` hands the row's
handler to `InitSoundCueSet`, and `ServiceSoundCueSet` calls it once per tick
as `callback(owner, set)`, so `out` is the `SoundCueSet` (`EntityMoodHandlerArg`
is Entity.h's local view; field readings in `Entity__MoodCue07.md`
`## Proposed field names`: `unk4` tick, `unk10` attenuation, `unk1C`/`unk30`/`unk44`
voice 0/1/2 tone request (-2 = stop), `unk20`/`unk34`/`unk48` pitch offset).
Tier B, same as every sibling `Entity__MoodCueNN` (Entity_b..Entity_g): the
row mapping is a fact of the binary, which dream object or state a row is
for is not established. Row kept decimal so names sort in table order.

What it does, in the unit's current field names: Attenuation from proximity; voice-0 tone 3 every 10 set ticks; `slot128(1)` (SetTod) at `moodTimer == moodDuration`; `slotC4(-0x80, 1)` while `unk7C == 1`.

## Proposed field names

Entity is a Class65650 subclass: `Entity__Entity` runs
`Get_vtable_Class65650()->ctor`, and `tools/classtable.py gEntityMethods`
keeps Class65650's TOD slots at +0x128..+0x138. `code_55dd4.h` names both the
slots and the fields they write; the Entity offsets are the same.

| field / slot | proposed | tier | evidence | accessor outside Entity_f |
| --- | --- | --- | --- | --- |
| `EntityMethods::slot128` | `setTod` | A | gEntityMethods +0x128 = `Class65650__SetTod` (writes todIndex +0x7C, todFrameCount +0x80, todFramePtr +0x88, todFrame +0x84 = 0) | Entity_g (first compiler failure, `Entity_g.c:184`) |
| `EntityMethods::slot12C` | `playTod` | A | +0x12C = `Class65650__PlayTod` (todPlaying = 1) | Entity_g (first compiler failure, `Entity_g.c:239`) |
| `EntityMethods::slot130` | `stopTod` | A | +0x130 = `Class65650__StopTod` (todPlaying = 0) | Entity_g (first compiler failure, `Entity_g.c:177`) |
| `Entity::unk7C` (+0x7C) | `todIndex` | A | Class65650 +0x7C; here `setTod(this, 1)` is followed by `if (unk7C == 1)`, which is SetTod's own write read back | Entity_e (first compiler failure, `Entity_e.c:338`) |
| `Entity::unk84` (+0x84) | `todFrame` | A | Class65650 +0x84; `Entity__MoodCue91/92` fast-forward only while it is 0 and increment it with each `applyTodFrame`; `Entity__MoodCue92` compares it to `moodDuration - 1` | Entity_g (first compiler failure, `Entity_g.c:93`) |

**Correction proposed for a head-applied name:** `Entity::moodDuration`
(+0x80, renamed from `unk80` in round 78) is Class65650's `todFrameCount`,
written by `Class65650__SetTod` from the TOD header's frame count. Every use
in this unit reads that way: `todFrame == moodDuration - 1` (the last frame,
`Entity__MoodCue92`), `moodTimer == moodDuration` right before `setTod(1)`
(here, `Entity__MoodCue94/95`: wait out the current animation, then switch).
Proposed `todFrameCount`, tier A by the same offset/ctor evidence. It is
accessed in every Entity_x unit, so it is the head's call.

Unk94Methods `slot44`/`slot100`/`slot130`/`slot200` (on `this->target`) and
Unk100Methods `slotD4`/`slotD8` are not named: the target's and `unk100`'s
classes are not established.

The accessor column is the first unit outside Entity_f that failed to compile
with the definition renamed and Entity_f's own accessors fixed (make stops
at the first failing unit, so it is a witness, not the full list); the head's
type-scope apply lists the rest.

**Applied by the head at merge, round 79**, by type scope, each field separately with both oracles green. `moodDuration` (round 78) is now `todFrameCount`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
