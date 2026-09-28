# Entity__MoodCue47 -- MATCHED (80/80 words)

> Renamed from `func_8006090C` on 2026-09-24 (tools/rename.py). Address 0x8006090c.

Unit: `Entity` (second pass, round 2026-09-03). State-machine-style
dispatch on `this->unk44`, no "out" parameter (confirmed from its own body,
same as `Entity__RollScaleOrDelayedDrift`/`Entity__MoodCue56` earlier in this unit).
`void Entity__MoodCue47(Entity *this)`.

## Final source

```c
void Entity__MoodCue47(Entity *this) {
    if (this->unkF4 == 0) {
        return;
    }
    if (this->unk44 == 0) {
        this->unk44 = 0xC;
        this->unkFC = 0;
        return;
    }
    if (this->unk44 == 0xC) {
        if (this->unkFC < 0x1E) {
            if (this->unk94->methods->slot100(this->unk94) != 0) {
                this->unk94->methods->slot130(this->unk94, 0);
                this->unkFC = 0;
                this->unk44 = 0xB;
            }
        } else {
            this->methods->slot30(this, 0xB);
            this->unk44 = 0xA;
        }
    } else if (this->unk44 == 0xB) {
        if (this->unkFC == 0x64) {
            this->methods->slot30(this, 0xC);
        } else {
            this->unk94->methods->slotCC(this->unk94, -0x64, 0);
        }
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed. Two early guard clauses (`if
(!cond) return;`) followed by a two-level dispatch on `this->unk44`'s
current small-integer state (`0xC` vs `0xB`), each level with its own
nested condition -- reads as an ordinary state machine tick function, one
state transition per call.

New vtable slot discovered: `Unk94Methods::slotCC` (`void (*)(Unk94Obj
*self, s32 arg1, s32 arg2)`), called as `slotCC(this->unk94, -0x64, 0)`.
Return value unused at this, its only known call site, so `void` is a safe
default per the project's usual caveat. Added to `include/Entity.h`,
carved out of the `pad000[0x100]` gap at the FRONT of `Unk94Methods` (the
struct previously started its first named slot at `+0x100`; this pushes a
named slot as low as `+0xCC`, with `slot100` unchanged at `+0x100`). This
also reuses the already-typed `Unk94Methods::slot100` (`s32 (*)(Unk94Obj
*self)`) and `slot130` (`void (*)(Unk94Obj*, s32)`) with no changes.

### Proposed learning

None -- clean state-machine dispatch, no residue, no register-allocation
surprises. Notable only for being the third distinct slot discovered on
`Unk94Methods` in this unit (after `slot1A0` from `Entity__MoodCue46`),
confirming `Unk94Obj` carries a substantial vtable of its own worth
resolving incrementally as more Entity functions touch it.

## Naming

`Entity__MoodCue47` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 47, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity.c`'s own header comment); which dream object owns the row is not.

## Proposed field names

**Head, round 76: APPLIED** as `Entity::moodState` by type scope (definition first; the compiler listed 152 accessors across Entity, Entity_b..g; all fixed; whole-image oracle and check-nonmatching green).

| field | proposed | tier | evidence |
| --- | --- | --- | --- |
| `Entity::unk44` (+0x44) | `moodState` | B | Read/written by all 7 Entity units (`Entity.c`, `Entity_b..g`, 173 whole-word hits), so this is a PROPOSAL, not a rename. Every accessor in this unit treats it as a small internal phase code, distinct from `moodTimer` (per-tick counter, reset alongside a phase change) and `moodIndex` (which row of `gEntityMoodHandlerTable` this entity dispatches through). `Entity__MoodCue47` is the clearest single example: `unk44` starts 0, becomes 0xC, then 0xB, then 0xA in sequence, each transition gated on `moodTimer` thresholds and gating what the rest of the function does -- a textbook small FSM register, not a flag or a count. `moodIndex`/`moodTimer` are already-established names in the same struct, so `moodState` extends that family rather than inventing a new one. Posted to `tools/broadcast.sh` for the head/other Entity-unit runners to weigh in before any unit applies it. |

Not proposing `unk80`/`unk84`: both are read here as `moodTimer`-scaling
constants (a divisor/multiplier derived from the mood row) but the existing
header comment already flags `unk84` as carrying a SECOND, unrelated
loop-counter meaning in `Entity.c` (`Entity__MoodCue91`/`Entity__MoodCue92`), so a
single name would misdescribe one of the two uses -- exactly the ambiguity
CLAUDE.md's field-ownership rule exists to keep out of a shared header.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Named: `ENTITY_EFFECT_EVENT_VIDEO`, `ENTITY_EFFECT_END_DREAM` (evidence on each definition: EntityEffect and ENTITY_STATE_DONE in include/Entity.h, SOUND_CUE_STOP in include/SoundCueSet.h). clearTickCallbacks' bool clearLook is `false`. Byte-identical (whole image green).
