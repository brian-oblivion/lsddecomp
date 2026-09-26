# Entity__MoodCue91

> Renamed from `func_80063ED4` on 2026-09-25 (tools/rename.py). Address 0x80063ed4.

**Unit:** Entity_f · **Size:** 105 words · **Status:** MATCHED (105/105 words)

## What it does

On `this->unkFC == 0`: calls `Entity__GetOrCreateUnk100(this, NULL, NULL, (void*)5,
0)` and, on a non-NULL result, conditionally `slotBC(this, TRANSLATE_Y_MINUS256)`
and unconditionally `this->unk100->methods->slotD4(this->unk100,
this->unk50, 0, 0)`. On `this->unkFC != 0`: if `this->unk84 == 0`, runs a
`do { this->unk88 = this->methods->slot134(this, this->unk88, 0);
this->unk84 += 1; } while (this->unk84 < 0x18);` loop. Independently
(regardless of which branch above fired), `if (this->unk84 >= 0x19) {
slotC4(this,-0x14,0); this->unk94->methods->slot130(this->unk94,1); }`.
Then a further `this->unkFC == 0x32` / `== 0xC` dispatch, and an
unconditional `slot48(this, 1, D_80089DE4)`.

## Derivation

The `this->unk84 >= 0x19` check is NOT nested inside either the
`unkFC==0`/`unkFC!=0` arms — it is a SEPARATE top-level `if` reached from
BOTH, confirmed by the disassembly: the `unkFC==0` path's tail jumps
directly into the `>=0x19` check code (skipping the loop's own guard
entirely, since that path never touches `unk84`), while the `unkFC!=0`
path falls through to the identical check after the loop. Writing this as
nested-inside-else would have made the `unkFC==0` path skip the `>=0x19`
check, which is wrong. Same "independent top-level `if` after a reload,
not an `else` branch" idiom already established in this project for
functions with a similar shared-tail shape (see e.g. Entity_d's
`Entity__MoodCue58` report).

The `slot134` loop is byte-identical in shape to `Entity__MoodCue92`'s own
(this unit, later in ROM order) — both establish the new
`EntityMethods::slot134` slot and `Entity::unk88` field; this function is
first in ROM order, so both are added here.

## Header additions (`include/Entity.h`, additive only)

- `EntityMethods::slot134` — new slot at `+0x134`, splitting the existing
  `pad134[0x144-0x134]` gap.
- `Entity::unk88` — new `s32` field at `+0x88`, splitting the existing
  `pad88[0x94-0x88]` gap.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue91` | B | `gEntityMoodHandlerTable` row 91 |

Why `MoodCue91`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 91 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: Tick 0: gets/creates `unk100`, `addVec14(TRANSLATE_Y_MINUS256)` on a coin flip, `unk100->slotD4(unk50, 0, 0)`; later, if the TOD frame is 0, fast-forwards 24 frames with `applyTodFrame`; past frame 24 `slotC4(-0x14, 0)` and `target->slot130(1)`; `notifyParents(0xA)` at 50, voice-0 tone 21 at 12; `updateScale(1, D_80089DE4)` every tick.

### Data constant named (round 79)

| old | new | tier | bytes |
| --- | --- | --- | --- |
| `D_80089D90` | `TRANSLATE_Y_MINUS256` | A | three s32 `(0, -256, 0)`, the format of `TRANSLATE_Y_MINUS512`/`TRANSLATE_Y_MINUS64`; passed to `addVec14` like the other `TRANSLATE_*` tables |

`D_80089DE4` (`(4,5, 6,5, 5,5)` as s16 pairs, a non-uniform 4/5, 6/5, 1
scale) is left unnamed, as Entity_g's header comment already decided for the
same symbol: no precedent for naming a non-uniform, non-unit-fraction scale.

### Fields renamed (round 79, applied, compiler-listed accessors all in Entity_f)

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `EntityMethods::slot134` (+0x134) | `applyTodFrame` | A | `tools/classtable.py gEntityMethods` +0x134 = `Class65650__ApplyTodFrame`; `code_55dd4.h` already calls the same slot `applyTodFrame` in Class65650's own method table. Here: `todFramePtr = applyTodFrame(this, todFramePtr, 0)` 24 times, i.e. fast-forward 24 TOD frames |
| `Entity::unk88` (+0x88) | `todFramePtr` | A | Class65650's own +0x88 is `todFramePtr` (`code_55dd4.h`; `Class65650__SetTod` writes it, `Class65650__Tick` stores `applyTodFrame`'s return in it), and Entity inherits Class65650's layout (`Entity__Entity` runs Class65650's ctor). The only accessor, this loop, uses it exactly that way |

Types were left as they were (`s32`), not corrected to Class65650's `u8 *`;
no offset or size moved. Both oracles green after each.

## Track 4 (2026-09-26, round 87, echo)

`this->unk100` is a `Class6E99C *` (include/Class6E99C.h); the slot
call through its +0x0D4 is now `startFadeDown` (Class6E99C__StartFadeDown), with `companion2`, an
`s32` in Entity.h, cast `(BasicClass *)` as the fade's source (Class6E99C's
configure adds it as a child; no code). Image byte-identical.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
