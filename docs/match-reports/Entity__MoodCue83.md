# Entity__MoodCue83

> Renamed from `func_800636E4` on 2026-09-25 (tools/rename.py). Address 0x800636e4.

**Unit:** Entity · **Size:** 40 words · **Status:** MATCHED (40/40 words)

## What it does

Small mood-dispatch handler. `if (this->unk84 % 15 == 0) { out->unk10=0;
out->unk1C=0xC; out->unk20=2; }` then, independently, `if (this->unkFC ==
this->unk80) { out->unk1C=-2; this->methods->slot16C(this); this->unk44=1;
}`.

## Derivation

The `%15` divisor was NOT obvious from the magic constant alone
(`0x88888889`, GCC's magic multiplier, is shared across several nearby
divisors depending on the accompanying shift/reconstruction). Read the
RECONSTRUCTION instructions instead of the magic constant: retail rebuilds
the multiple as `(quotient << 4) - quotient` (`sll` by 4 then `subu`
quotient), i.e. `quotient * 15` — that arithmetic identity is what pins
the divisor, not the magic number by itself. Verified by matching, not
guessed.

## Proposed learning

**When reverse-engineering a `%N` from a magic-multiply sequence, read the
RECONSTRUCTION shift/add chain (the `sll`/`addu`/`subu` after `mfhi`), not
just the magic constant** — the same magic constant is reused by GCC 2.6.3
across a small family of related divisors, and only the final
multiply-back tells you which one. An early guess of `%8` here (plausible
from a passing glance at the same magic constant used elsewhere) would
have been silently wrong; the reconstruction math (`quotient*16 -
quotient*1 = quotient*15`) is unambiguous.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue83` | B | `gEntityMoodHandlerTable` row 83 |

Why `MoodCue83`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 83 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: Requests voice-0 tone 12, pitch +2, full volume whenever `unk84 % 15 == 0` (unk84 = TOD frame, proposed); at `moodTimer == moodDuration` stops voice 0, stops the cue, `moodState = 1`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 95, bravo)

### Constants

- `program = -2` is `SOUND_CUE_STOP` (include/sound_cue_set.h: ServiceSoundCueSet stops the slot's voice)
- `state = 1` after `stopSoundCue` is `ENTITY_STATE_DONE`: nothing in this handler reads `state == 1`, and it is the value Entity__UpdateActivationState and Entity__UpdateSoundCueStart read as "do not reactivate / restart the cue"
- Every other literal went to decimal (tick counts, TOD frames, distances, VAB programs; no masks): they are this handler's tuning, named by nothing else.
