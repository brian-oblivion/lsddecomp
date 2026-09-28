# Entity__MoodCue51 -- MATCHED (110/110 words)

> Renamed from `func_80060D80` on 2026-09-24 (tools/rename.py). Address 0x80060d80.

Unit: `Entity` (second pass, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue51(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue51(Entity *this, EntityMoodHandlerArg *out) {
    void *table;

    if (this->unkFC == 0 && rand() % 5 == 0 && this->unk44 == 0) {
        this->methods->slot48(this, 1, SCALE_SIX);
        this->methods->slotCC(this, 0x320, 0);
        this->unk44 = 0xB;
    }
    table = NULL;
    if (out->unk4 % 5 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 8;
    }
    if (this->unkFC == 0x5A) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->unkFC == 0xA0) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->unkFC == 0xDC) {
        if (rand() & 1) {
            table = ROTATION_YAW_PLUS180;
        }
    }
    if (table != NULL) {
        this->methods->slot44(this, 0, table);
    }
    this->methods->slotC4(this, -0x50, 1);
}
```

## Derivation notes

Matched first attempt, no iteration needed -- the largest function matched
in this unit so far (110 words) to go clean on the first try. Two things
worth recording:

- **`table`'s `= NULL` assignment sits AFTER the first `if
  (unkFC==0 && rand()%5==0 && unk44==0) {...}` block, not before it, and
  this is the position that matched.** Contrast `Entity__MoodCue40`, matched
  earlier in this same unit, where the equivalent `void *table = NULL;`
  had to be the function's very FIRST statement (ahead of an early call) to
  avoid a size-drift residue. Here, retail's own bytes initialize the
  table pointer's register (`s1`) in the delay slot of the SECOND check
  (`out->unk4 % 5`), i.e. still after the first block's two vtable calls --
  so `table` genuinely does not need to survive those first two calls, and
  writing the C in the same order as retail's actual initialization point
  (not the earliest possible point) reproduced it directly. **The general
  rule from `Entity__MoodCue40`/`Entity__MoodCue44` stands (a value that must
  survive a call needs its assignment positioned before that call), but
  this function is the confirming converse: don't reflexively hoist an
  initializer to the top of the function -- match retail's ACTUAL
  initialization point, which is the first point after which the value is
  live all the way to its uses.**
- A nested `if (A && B && C) { ... }` reproduced retail's three-stage
  early-exit chain (`unkFC==0`, then `rand()%5==0`, then `unk44==0`)
  directly, with all three skip paths converging on the same reload point
  in retail's bytes -- no `goto` needed here, unlike `Entity__MoodCue46`'s
  superficially similar-looking multi-predecessor shape. The difference:
  here it's a genuine short-circuit chain (each condition gates whether the
  NEXT is even evaluated), not four independent alternative branches
  feeding one shared call with different arguments.
- Reuses already-typed `slot48`, `slotCC`, `slot148`, `slot44`, `slotC4` --
  no header changes needed at all for this function.

### Proposed learning

- **When a local's initializer's correct POSITION isn't obvious, check
  where retail's own delay slot sets the corresponding register** -- it is
  not always "as early as possible" (see `Entity__MoodCue40`) and not always
  "right before first real use" either; it is specifically the position
  retail's compiler chose, which is often tied to a nearby branch's delay
  slot. Reading the disassembly's delay-slot placement directly, rather
  than guessing from C-level intuition, resolved this in one attempt.

## Naming

`Entity__MoodCue51` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 51, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/world/Entity.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Byte-identical (whole image green).
