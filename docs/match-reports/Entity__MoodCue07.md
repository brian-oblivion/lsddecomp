# Entity__MoodCue07

> Renamed from `func_8005E4D0` on 2026-09-23 (tools/rename.py). Address 0x8005e4d0.

**Unit:** Entity_b · **Size:** 113 words · **Status:** MATCHED (113/113
words, whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. A larger member of this
unit's mood-dispatch handler family:

1. `out->unk10 = this->methods->slot148(this);` (the usual opener).
2. If `this->unk84 == this->unk80 / 2`, sets four `out` fields
   (`unk1C=0x7`, `unk20=-0x2`, `unk30=0x3`, `unk34=-0x2`).
3. If `out->unk4 % 90 < 3`, sets `out->unk44 = 0x6` and `out->unk48 = -0x1`.
4. Dispatches on `this->unkFC`:
   - `>= 0x79` (121): `this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);` then
     `this->methods->slotC4(this, -0x140, 0);`
   - `< 0x79` and (`>= 0x38` (56) or `Entity__IsNearTarget(this, &this->unk14->x, 1,
     1) != 0`): `this->methods->slotBC(this, TRANSLATE_Y_MINUS64);`
   - `< 0x38` and `Entity__IsNearTarget(...) == 0`, sub-dispatch on `this->unkFC`
     again: `>= 0xA` (10) calls `Class6B5CC__FaceTarget(...)` then
     `this->methods->slotC4(this, -0x100, 0)`; `< 0xA` calls only
     `Class6B5CC__FaceTarget(...)`.

The magic-multiply constant `0xB60B60B7` at shift 6, reconstructed by
retail's own multiply-back sequence (`*3`, `*15` via `<<4` minus itself,
`*2` = `*90`), is division by 90 — writing `out->unk4 % 90` reproduces it
exactly with no manual constant derivation needed.

## New fields

- `EntityMethods::slot44`, `void (*)(Entity *self, s32 arg1, void *arg2)` —
  filled the last gap in the `0x40`..`0x48` slot run (`slot40`/`slot48`
  already existed either side of it).
- `EntityMoodHandlerArg::unk20`/`unk34`/`unk48` — each paired one word after
  an already-known field (`unk1C`/`unk30`/`unk44` respectively), splitting
  the existing padding runs.
- `ROTATION_YAW_PLUS2` — a fourth `D_8008xxxx` opaque data row, same convention as
  `SCALE_HALF`/`SCALE_DOUBLE`/`TRANSLATE_Y_MINUS64` already declared at the top of this
  file.

## Final C

```c
void Entity__MoodCue07(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == this->unk80 / 2) {
        out->unk1C = 0x7;
        out->unk20 = -0x2;
        out->unk30 = 0x3;
        out->unk34 = -0x2;
    }
    if (out->unk4 % 90 < 3) {
        out->unk44 = 0x6;
        out->unk48 = -0x1;
    }
    if (this->unkFC >= 0x79) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
        this->methods->slotC4(this, -0x140, 0);
    } else if (this->unkFC >= 0x38 ||
               Entity__IsNearTarget(this, &this->unk14->x, 1, 1) != 0) {
        this->methods->slotBC(this, TRANSLATE_Y_MINUS64);
    } else if (this->unkFC >= 0xA) {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        this->methods->slotC4(this, -0x100, 0);
    } else {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    }
}
```

Note the duplicated `Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);` call in the
last two arms — retail's own bytes call it identically in both, one arm just
has an extra `slotC4` call afterward. Not a shared/hoisted call: written as
two literal statements, matching retail's own (redundant-looking but
byte-real) duplication.

## Attempt log

Two attempts. First attempt wrote the top-level dispatch as `if (unkFC <
0x79) { <0x79 body } else { slot44/slotC4 }` (the more "natural" reading
order, `<0x79` case first) — 59/113, diverging right at that branch. Reading
the raw disassembly showed retail places the `>= 0x79` body as the
PHYSICAL FALL-THROUGH (no jump) and reaches the `< 0x79` body via an
explicit forward branch — the same "GCC lays the WRITTEN condition's true
branch as fall-through" shape from `Entity__MoodCue13.md` in this same round.
Swapping to test `>= 0x79` FIRST (as the primary `if`, with the `< 0x79`
logic as the `else`) matched immediately, with every inner branch unchanged.

## Proposed learning

A second confirming instance of `Entity__MoodCue13.md`'s finding, now definitely
a pattern rather than a one-off: **when a residue is "right content, wrong
physical position, otherwise byte-identical," try testing the OPPOSITE
condition as the primary `if`** — GCC 2.6.3 consistently lays the written
condition's true-branch body as the fall-through and the `else` as a jump
target, so which comparison is written first controls which body sits where
in the instruction stream, even though both spellings are semantically
identical C.

## Naming

`Entity__MoodCue07` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E4D0`.

`gEntityMoodHandlerTable` row 7. Body: sets the attenuation. When `unk84 == unk80 / 2` it requests voice 0 tone 7 and voice 1 tone 3 (both pitch -2). On ticks where `tick % 90 < 3` it requests voice 2 tone 6 (pitch -1). Then a moodTimer timeline: from 0x79 it turns by `ROTATION_YAW_PLUS2` (relative) and moves -0x140; from 0x38, or when near the target, it steps `TRANSLATE_Y_MINUS64`; from 10 it faces the target and moves -0x100; before that it only faces the target.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

Reading the body with the proposed `SoundCueSet` field names: `unk4` = tick (0 on the first callback), `unk10` = attenuation (from `getProximityRatio`), `unk1C`/`unk30`/`unk44` = tone request for voice 0/1/2, `unk20`/`unk34`/`unk48` = that voice's pitch offset.

## Proposed field names

`EntityMoodHandlerArg` is Entity.h's local view of `SoundCueSet` (src/code_179d8_e.c). `ServiceSoundCueSet` hands the set itself to the callback, and every offset below lines up with that struct. Evidence comes from `ServiceSoundCueSet`'s body (asm/nonmatchings/code_179d8_l/ServiceSoundCueSet.s). Each tick it writes -1/0/0x7F/0x40 into each voice slot's +0x4/+0x8/+0xC/+0x10 (stride 0x14 from +0x18) and zeroes set+0x10. Then it calls the callback. If set+0x10 >= 0, for each slot whose request is >= 0 it stops the old voice (channel vt+0x84, `VabStreamObj__StopVoice`), calls `VabStreamObj__SetPitchOffset` (vt+0x9C) with slot+0x8, and plays `VabStreamObj__PlayTone` (vt+0x80) with `request << 4` and two volumes, each reduced by `vol / set->unk14(=10) * set+0x10`. A request of -2 stops the voice. Finally it increments set+0x4. The compiler lists accessors of every one of these fields in Entity_b through Entity_g, so they are proposals only:

| field | proposed | tier | evidence |
| --- | --- | --- | --- |
| `unk4` (+0x04) | `tick` | A | zeroed by InitSoundCueSet, incremented once per ServiceSoundCueSet pass; handlers test `== 0` for the first tick, and Entity__MoodCue13 writes -1 to restart |
| `unk10` (+0x10) | `attenuation` | A | zeroed per tick, then each volume -= vol/10 * this; < 0 skips all playback that tick; handlers store `getProximityRatio` (0..proximityDivisor, -1 beyond threshold) or 0 |
| `unk1C` / `unk30` / `unk44` | `voice0Tone` / `voice1Tone` / `voice2Tone` | A | slot +0x4 request: -1 none (per-tick reset), -2 stop, else `PlayTone(tone << 4)` |
| `unk20` / `unk34` / `unk48` | `voice0Pitch` / `voice1Pitch` / `voice2Pitch` | B | slot +0x8, forwarded to `VabStreamObj__SetPitchOffset`; reset to 0 per tick |

Better still, and track 4's call: replace `EntityMoodHandlerArg` with one shared `SoundCueSet`/`SoundCueSlot` type (its three voices are `slots[3]` at +0x18, with `index` at +0x0 of each).

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
