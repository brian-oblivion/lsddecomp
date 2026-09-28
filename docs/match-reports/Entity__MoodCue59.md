# Entity__MoodCue59 -- MATCHED (93/93 words)

> Renamed from `func_80061A90` on 2026-09-24 (tools/rename.py). Address 0x80061a90.

Unit: `Entity` (round 13, first function in this file). Two independent
"divisible by 10" checks (one gated on `unkFC==0` against `rand()`, one on
`out->unk4` unconditionally), a `unkFC==0` coin-flip `slotCC` call, an
unconditional `slotC4`, and a final `unk44`/`unkFC` combo that reaches
through `unk4C`'s own vtable. `void Entity__MoodCue59(Entity *this,
EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue59(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0 && rand() % 10 == 0) {
        this->unk44 = 0xC;
    }
    if (out->unk4 % 10 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->unkFC == 0) {
        if (rand() & 1) {
            this->methods->slotCC(this, 0x800, 0);
        }
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->unk44 == 0xC && this->unkFC == 0x12C) {
        this->unk4C->methods->slot138(this->unk4C, 1, 1);
    }
}
```

## Derivation notes

- Both "divisible by 10" tests use the same `0x66666667`/shift-2 magic-
  constant idiom identified in `Entity__MoodCue79`'s report; matched clean on
  the first pass without needing to hoist either remainder into a named
  local (unlike `Entity__MoodCue71`), because neither result is subsequently
  multiplied by anything -- both are compared directly against the
  dividend.
- **The shared `lui $v0, 0x6666` between the two divisibility checks is a
  codegen artifact, not evidence of merged control flow.** Disassembly
  shows `v0`'s upper half loaded once at a point reachable from BOTH (a)
  skipping the whole `unkFC==0 && rand()%10==0` block via the initial
  `bnez`, and (b) falling through after `this->unk44 = 0xC`. GCC hoists the
  constant load to the confluence point because it is needed next
  regardless of which path was taken -- writing the two checks as separate,
  ordinary `if` statements (no shared temp, no merged condition) reproduces
  this exactly; no source-level restructuring was needed to get the shared
  load.
- `this->unk4C->methods->slot138(this->unk4C, 1, 1)` is a direct instance
  of the class-framework pattern already documented on `Unk4CMethods` in
  `include/Entity.h` (`this->unk4C` dereferenced through its own vtable at
  `+0x138`) -- no new struct knowledge, just its second confirmed call
  site (the first, in `Entity__MoodCue12`, is what typed the slot originally).

No new struct or vtable-slot knowledge; every field/slot here was already
known from earlier work in `Entity`/`Entity_e`.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 59 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`, `voice0Pitch`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). Byte-identical (whole image green).

## Unit notes (moved from src/Entity.c's banner, round 93)

The pre-track-7 banner of `src/Entity.c` carried this history, now here:
the unit was carved as the third 20-function slice of the Entity class's
97-function remainder, after Entity and Entity_d. Its functions are the
`gEntityMoodHandlerTable` handlers of rows 59, 61-62, 64-71 and 73-81, each
named `Entity__MoodCueNN` for its row; row order does not track code
address (each report derives its row). Rows 60, 63 and 72 have a NULL
handler word: those mood indices dispatch no per-tick callback, not a gap
in the unit. `Entity__MoodCue81` also occupies row 120 (its report), and
`Entity__MoodCue71` is called from Entity's `Entity__MoodCue108`.
`sMoodCue78TransitionDone` is a one-shot s32 flag used only by
`Entity__MoodCue78`.
