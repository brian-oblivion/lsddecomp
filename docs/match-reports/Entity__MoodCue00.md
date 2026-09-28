# Entity__MoodCue00

> Renamed from `func_8005E160` on 2026-09-23 (tools/rename.py). Address 0x8005e160.

**Unit:** Entity · **Size:** 153 words · **Status:** MATCHED (153/153
words, whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. The largest
mood-dispatch handler reached so far, with a genuinely redundant-looking
range test that turned out to be exactly what retail compiled:

1. If `out->unk4 == 0` and `this->unk94->methods->slot200(this->unk94) == 5`,
   sets `this->unk44 = 0x64`.
2. `out->unk10 = this->methods->slot148(this);` (the usual opener).
3. If `this->unk44 == 0`: conditionally sets `out->unk1C`/`out->unk20` when
   `out->unk4 % 10 == 0`, then a three-way `this->unkFC` dispatch
   (`== 0x960` sets `unkFC = -1`; `< 0x4B0` calls `slotC4(this, 0x32, 0)`;
   else `slotC4(this, -0x32, 0)`).
4. Else (`this->unk44 != 0`), a four-way `this->unkFC` dispatch:
   - `< 0xFA` (250): the SAME `out->unk4 % 10` check as step 3, THEN its own
     inner two-way split (`< 0x64` -> `slotC4(this, 0x32, 0)`; else
     `slotBC(this, TRANSLATE_Y_PLUS64_Z_MINUS64)`).
   - `== 0xFA`: `slot130(this)`, `out->unk1C = -2`.
   - `>= 0x105 && < 0x238`: `slotC4(this, -0x32, 0)` then
     `slot44(this, 1, sRotationYawMinus120)`.
   - `>= 0x239`: `slot44(this, 1, ROTATION_X50_YMINUS120_Z30)`.

## The "redundant" range test that isn't reachable-code noise

Inside the `< 0xFA` branch of step 4, retail's own bytes re-test `this->unkFC
< 0xFA` a SECOND time (as the `else if` between the `< 0x64` and `slotBC`
cases) even though that fact is already established by the OUTER branch
guarding the whole block. This looked, on first read, like it might be
dead/unreachable code the compiler left in — but it's not: GCC 2.6.3 does
not eliminate a source-level redundant re-test across intervening code at
`-O2` in this pipeline, so the retest is a direct transcription of an
`if (unkFC < 0xFA) { ... if (unkFC < 0x64) {...} else if (unkFC < 0xFA)
{...} ... }` shape in the ORIGINAL source. Writing the C with the same
apparently-pointless second test reproduced retail's bytes on the first
attempt; collapsing it to a plain `else` (which is logically equivalent)
would very likely have cost a residue, though that alternative wasn't
needed since the literal transcription matched immediately.

## New symbols

- `Unk94Methods::slot200`, `s32 (*)(Unk94Obj *self)` — a new, far-out slot in
  the same `Unk94Obj` vtable `Entity__MoodCue01.md`/`Entity__IsTargetInRange.md`
  established; padded out to `+0x200` with no other slots resolved in
  between (nothing else in this unit reaches that far into the table yet).
- Three more `D_8008xxxx` opaque data-row externs: `TRANSLATE_Y_PLUS64_Z_MINUS64`,
  `sRotationYawMinus120`, `ROTATION_X50_YMINUS120_Z30`.

## Final C

```c
void Entity__MoodCue00(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        if (this->unk94->methods->slot200(this->unk94) == 5) {
            this->unk44 = 0x64;
        }
    }
    out->unk10 = this->methods->slot148(this);
    if (this->unk44 == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->unkFC == 0x960) {
            this->unkFC = -1;
        } else if (this->unkFC < 0x4B0) {
            this->methods->slotC4(this, 0x32, 0);
        } else {
            this->methods->slotC4(this, -0x32, 0);
        }
    } else if (this->unkFC < 0xFA) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->unkFC < 0x64) {
            this->methods->slotC4(this, 0x32, 0);
        } else if (this->unkFC < 0xFA) {
            this->methods->slotBC(this, TRANSLATE_Y_PLUS64_Z_MINUS64);
        }
    } else if (this->unkFC == 0xFA) {
        this->methods->slot130(this);
        out->unk1C = -2;
    } else if (this->unkFC >= 0x105 && this->unkFC < 0x238) {
        this->methods->slotC4(this, -0x32, 0);
        this->methods->slot44(this, 1, sRotationYawMinus120);
    } else if (this->unkFC >= 0x239) {
        this->methods->slot44(this, 1, ROTATION_X50_YMINUS120_Z30);
    }
}
```

The `>= 0x105 && < 0x238` test compiles directly to retail's unsigned
range-check idiom (`(unsigned)(unkFC - 0x105) < 0x133`) with no manual
reconstruction needed — GCC 2.6.3 performs that transform on its own for a
two-constant `&&` range test.

## Attempt log

Matched on the first attempt.

## Proposed learning

**A source-level re-test of a condition already established by an
enclosing `if` is not evidence of an authoring mistake or of unreachable
code — GCC 2.6.3 compiles it literally, byte for byte, rather than proving
it always-true and eliding it.** When a disassembly shows the same `slti`
constant tested twice with no intervening code that could invalidate the
first result, the straightforward reading (transcribe both tests as
written) is correct and should be tried BEFORE reaching for a `goto`,
`switch`, or collapsed `else` — collapsing it is tempting because it reads
as "cleaner" C, but it changes the source shape retail actually has.

## Naming

`Entity__MoodCue00` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E160`.

`gEntityMoodHandlerTable` row 0. Body: on tick 0, if the target's slot +0x200 returns 5, it sets `unk44 = 100`. It sets the attenuation. With `unk44 == 0` it requests voice 0 tone 5 (pitch -2) every 10th tick and moves along local z (slot +0xC4) +0x32 below moodTimer 0x4B0 and -0x32 from there, resetting moodTimer to -1 at 0x960, so it paces back and forth. With `unk44 != 0` it runs a timeline: below 250 the same tone plus a +0x32 move (below 100) or a `TRANSLATE_Y_PLUS64_Z_MINUS64` step; at 250 slot +0x130 and voice 0 stop (-2); from 0x105 to 0x237 a -0x32 move and `updateRotation(1, sRotationYawMinus120)`; from 0x239 `updateRotation(1, ROTATION_X50_YMINUS120_Z30)`.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Proposed field names

Slots of `EntityMethods` (include/Entity.h). The compiler lists accessors in Entity_b through Entity_g for each of them, so they are proposals only:

| slot | proposed | tier | evidence |
| --- | --- | --- | --- |
| `slot44` (+0x44) | `updateRotation` | B | occupant `func_8001CEB4` (`tools/classtable.py gEntityMethods`), the same function the head named `SceneNodeMethods::updateRotation` in round 70. Every data argument here is a {num, den} degree triple (ROTATION_YAW_*), flag 1 = set, 0 = add |
| `slot48` (+0x48) | `updateScale` | B | occupant `func_8001D008`, round 70's `SceneNodeMethods::updateScale`; arguments SCALE_HALF / SCALE_DOUBLE |

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

`getDreamColor(...) == 5` is `DREAM_COLOR_PINK` (DreamSys.h's DreamColors, the slot's declared return type). `program = -2` is `SOUND_CUE_STOP`. `state = 100` is this handler's own non-zero phase (banner convention). A comment explains `moodTimer = -1` (Entity__TickSoundCue counts it back to 0). Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs and `state` phases (hex remains only for masks). Byte-identical (whole image green).
