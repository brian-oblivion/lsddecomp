# Entity__MoodCue84

> Renamed from `func_80063784` on 2026-09-25 (tools/rename.py). Address 0x80063784.

**Unit:** Entity · **Size:** 60 words · **Status:** MATCHED (60/60 words)

## What it does

A five-way `this->unk84` range/value dispatch (`< 0x28`, `== 0x28`,
`== 0x2D`, `== 0x40`, `== 0x59`), each writing a different subset of
`out`'s fields (`unk1C`/`unk20`/`unk44`/`unk48`/`unk30`/`unk34`), all but
the last returning immediately; the `== 0x59` arm calls `slot16C` and sets
`this->unk44 = 1`.

## Derivation

**First transcription got THREE of the five comparison values and one of
the stored constants wrong**, all from the same mechanism: a chained
`bne`/delay-slot sequence where each link's comparison constant is set in
the PREVIOUS link's delay slot (executes unconditionally, "belongs to the
target path" per CLAUDE.md's residue-reading guide) rather than freshly
loaded at each link. Reading the mnemonics in isolation (`ori $v0, $zero,
0x40` appearing textually near the "third" check) suggested `0x40` was
compared there; it was actually the compare value carried INTO the
following link, with the actual third-link comparison constant sitting in
this link's own delay slot one instruction earlier. Corrected by
re-tracing every link with the actual register value at branch time (not
the nearest visible `ori`):

- `unk84 == 0x28` arm: `out->unk44` is **`-2`** (same value as `out->unk1C`
  in that arm, both from a single `-2` computed once), not `0x2D`
  — `0x2D` was the NEXT link's carried comparison constant, sitting in
  this link's delay slot, dead on this path.
- Third link's actual comparison is `unk84 == 0x2D`, not `0x40`.
- Fourth link's actual comparison is `unk84 == 0x40` (with `out->unk1C =
  7`), not `0x59`.
- Fifth link's actual comparison is `unk84 == 0x59`, not `0x1E4`
  (`0x1E4` doesn't appear anywhere in the real function — it was invented
  by misreading the chain, a category of error worth naming: a
  transcription mistake in this family doesn't just get a stored value
  wrong, it can fabricate a comparison constant that isn't in the binary
  at all).

## Proposed learning

**A chained `bne v1,v0,.LNEXT` / delay-slot `ori v0,zero,K` sequence must
be traced value-by-value, not read as "the constant near this branch is
what this branch tests."** The constant visible in a link's own
instruction stream belongs to the NEXT link (it's the delay slot,
unconditional, sets up the comparison value for wherever control goes
next); the constant this link actually tests against was set in the
PREVIOUS link's delay slot. Getting this backwards doesn't just mis-time a
value, it silently relabels which literal belongs to which branch —
confirmed on a real function this round (`Entity__MoodCue84`) and again, more
severely, in `Entity__MoodCue85` in this same unit (see that report, where it
swapped which of two ENTIRE CODE BLOCKS belonged to which `this->unk44`
value). When a function has 3+ chained equality checks on one field, write
out the delay-slot-carried value at each link explicitly before writing
any C, and verify the reconstructed chain against `asm-differ` rather than
trusting a single read-through.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue84` | B | `gEntityMoodHandlerTable` row 84 |

Why `MoodCue84`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 84 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: Attenuation from `getProximityRatio`; a fixed timeline on `unk84`: voices 0 and 2 before frame 40, stop both at 40, voice 1 tone 18 at 45, voice 0 tone 7 at 64, stop the cue and `moodState = 1` at 89.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 95, bravo)

### Constants

- `program = -2` is `SOUND_CUE_STOP` (include/sound_cue_set.h: ServiceSoundCueSet stops the slot's voice) (twice, at TOD frame 40)
- `state = 1` after `stopSoundCue` is `ENTITY_STATE_DONE`: nothing in this handler reads `state == 1`, and it is the value Entity__UpdateActivationState and Entity__UpdateSoundCueStart read as "do not reactivate / restart the cue"
- Every other literal went to decimal (tick counts, TOD frames, distances, VAB programs; no masks): they are this handler's tuning, named by nothing else.
