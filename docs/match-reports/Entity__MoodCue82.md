# Entity__MoodCue82

> Renamed from `func_800634A8` on 2026-09-25 (tools/rename.py). Address 0x800634a8.

**Unit:** Entity · **Size:** 143 words · **Status:** MATCHED (143/143 words)

## What it does

Mood-dispatch handler (`Entity *this, EntityMoodHandlerArg *out`). On
`this->unk44==0 && this->unkFC==0`, rolls a `%3`: on a hit, fires
`slot16C`/`slotC4(-0x5000,0)` and discards a bare `rand()` call; on a miss,
sets `this->unk44` to `0xB` or `0xC` via a `rand()&1` ternary. Then, on
`unk44==0xC`, calls `SceneNode__FaceTarget` and drives `out`/`slotC4`/`slot30` off
`unkFC` range checks; on `unk44==0xB`, calls `slot130` and, if
`slot144(this,this->unk94) < 0x200`, rolls another `%3` to pick between
`slot16C`/`slot160`.

## Derivation

First function of the unit, and the one that surfaced this round's
recurring residue: **`if (cond) { big-or-first-listed } else { small }`
does not reliably reproduce retail's branch polarity when the two
branches aren't a small-guard/large-body shape** — plain functional
correctness of the condition is not enough; the compiled TEST direction is
an independent degree of freedom. Two instances in this one function:

1. `if (rand() % 3 == 0) { slot16C/slotC4/rand } else { unk44 = ternary }`
   compiled with the branch test inverted relative to retail (confirmed
   via `asm-differ`, not guessed) — retail's `beq` sends the *zero*
   remainder to the (textually second, further-away) slot16C block and
   falls through to the ternary; my first draft's structurally identical
   C put the zero-remainder arm textually first and got the opposite
   polarity. Fix: swap which arm is the `if` vs `else` (write the
   `!= 0` test with the ternary first, `== 0`'s effect in the `else`) —
   not a code content change, purely which arm is textually first.
2. `if (rand() % 3 == 0) { slot16C } else { slot160 }`, same fix (swap to
   `!= 0` with `slot160` first).

Both fixes were purely "which arm goes first, with the condition negated
to match" — no other code content changed. Confirmed with `asm-differ`,
not assumed: after each swap the target/current columns lined up exactly.

## Proposed learning

**When a two-armed `if`/`else` compiles with the right VALUES but the
wrong branch polarity (`asm-differ` shows `beq`↔`bne`/`beqz`↔`bnez` at an
otherwise-matching test, or a `~>` jump whose target flips), try
swapping which arm is written first with the condition negated before
suspecting anything else.** This is not the same as the existing
"write a small early exit as an inverted guard clause" idiom (that one is
about arm SIZE); here both arms were single-statement-scale and still hit
this, so **size is not the discriminator — try the swap whenever a
straightforward `if`/`else` mismatches only in polarity.** Confirmed twice
in one function this round, and again in `Entity__MoodCue85`'s
`rand() % 5` ternary in the same unit (see that report) — cheap enough
(a rebuild is under a
second) to just try before writing up a residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue82` | B | `gEntityMoodHandlerTable` row 82 |

Why `MoodCue82`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 82 (base 0x80089EB0, stride 0x10; the row's
first word), read from `disk/SLPS_015.56` directly rather than inferred from
address order (rounds 76-77 measured that row order does not track code
address). Nothing else references it. `Entity__StartSoundCue` hands the row's
handler to `InitSoundCueSet`, and `ServiceSoundCueSet` calls it once per tick
as `callback(owner, set)`, so `out` is the `SoundCueSet` (`EntityMoodHandlerArg`
is Entity.h's local view; field readings in `Entity__MoodCue07.md`
`## Proposed field names`: `unk4` tick, `unk10` attenuation, `unk1C`/`unk30`/`unk44`
voice 0/1/2 tone request (-2 = stop), `unk20`/`unk34`/`unk48` pitch offset).
Tier B, same as every sibling `Entity__MoodCueNN` (Entity..Entity_g): the
row mapping is a fact of the binary, which dream object or state a row is
for is not established. Row kept decimal so names sort in table order.

What it does, in the unit's current field names: State machine on `moodState` 0/0xB/0xC: at tick 0 picks 0xB or 0xC (2/3) or stops the cue and calls `slotC4(-0x5000, 0)`; 0xC faces the target, requests tones 18/3 at full volume at `moodTimer` 20 and calls `target->slot130(1)`, `slotC4(-0x28, 0)` after 20, `notifyParents(0xA)` at 40; 0xB calls `slot130` (StopTod, see Proposed field names in `Entity__MoodCue93.md`) and, within 0x200 of the target, deactivates (2/3) or stops the cue.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 95, bravo)

### Constants

- `notifyParents(this, 0xA)` is `ENTITY_EFFECT_LINK_STAGE` (enum EntityEffect, include/Entity.h: DreamSys__InstanceEffectsOnJournal's case 10 links to the stage `getLinkStage` names; Entity_g spells the same call this way)
- `state` 0xB/0xC are this handler's own phases, now 11/12; `moveLocalZ(-0x5000)` is -20480, `distanceToPeer < 0x200` is 512.
- Every other literal went to decimal (tick counts, TOD frames, distances, VAB programs; no masks): they are this handler's tuning, named by nothing else.

### Unit history moved from the Entity_f.c banner

The banner was rewritten as documentation of what the file holds. The history it carried, verbatim (the unit's range, how the row mapping was read from retail, and round 93's TodActor field and slot renames, detailed in `Entity__MoodCue93.md`), and the old forward-declaration comment:

```c
/* Entity_f -- one of the Entity class's split units (Entity_c..Entity_g
 * hold its 97-function remainder after Entity/Entity_b), 0x800634A8..
 * 0x80064618, fully matched.
 *
 * 15 of the 17 functions are `gEntityMoodHandlerTable` callbacks
 * (Entity.h), `Entity__MoodCueNN` for the row whose `handler` word holds
 * their address: rows 82-96, consecutive and in address order here, read
 * from disk/SLPS_015.56 (base 0x80089EB0, stride 0x10). `Entity__MoodCue93`
 * also occupies row 107, named for its lower row as in Entity_c/_e/_g.
 * ServiceSoundCueSet calls each once per tick with the entity's SoundCueSet
 * (`SoundCueSet`); they request tones and step the entity's pose
 * and TOD animation on moodTimer / TOD-frame thresholds.
 *
 * The other two, `SetCueTones7_7_7` and `SetCueTones18_3_3`, are private
 * helpers of Entity__MoodCue85/86 that take only the SoundCueSet and write
 * a fixed three-voice tone request into it.
 *
 * Entity derives from TodActor (code_55dd4.h), whose TOD fields and slots
 * it inherits at the same offsets: this round renamed the two only this unit
 * touches (`todFramePtr` +0x88, `applyTodFrame` +0x134) and proposed the
 * shared ones (setTod/playTod/stopTod, todIndex/todFrame, companion2 --
 * now Actor's `ticker` -- and moodDuration -> todFrameCount) in
 * Entity__MoodCue93.md; the head applied them by type scope at merge.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

/* Forward declarations: both are defined later in this file (in ROM
 * order), but Entity__MoodCue85 and Entity__MoodCue86 call them before their own
 * definitions appear -- same convention as Entity_d.c's own forward calls. */
void SetCueTones7_7_7(SoundCueSet *out);
void SetCueTones18_3_3(SoundCueSet *out);
```
