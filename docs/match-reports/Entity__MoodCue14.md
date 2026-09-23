# Entity__MoodCue14

> Renamed from `func_8005EC98` on 2026-09-23 (tools/rename.py). Address 0x8005ec98.

**Unit:** Entity_b · **Size:** 30 words · **Status:** MATCHED (30/30 words,
whole-image build verified byte-exact)

## What it does

Another `gEntityMoodHandlerTable` mood-dispatch handler, `(Entity *this,
EntityMoodHandlerArg *out)`. Sets `out->unk10` from `slot148`, conditionally
sets `out->unk1C = 0xD` when `this->unk84 == 0xA`, then unconditionally calls
`this->methods->slotC4(this, -0xA, 0)`.

## New field: `Entity::unk84`

Read here as a plain word compared against a literal (`0xA`); also read by
`Entity__MoodCue07` (still `INCLUDE_ASM`) where it's compared against `this->unk80
/ 2` instead of a literal. Both readers treat it as a full `s32`.

## Final C

```c
void Entity__MoodCue14(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == 0xA) {
        out->unk1C = 0xD;
    }
    this->methods->slotC4(this, -0xA, 0);
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new — this is the cleanest instance yet of the `slot148`/conditional-
`unk1C`/`slotC4` mood-handler shape already established by
`Entity__MoodCue05`/`Entity__MoodCue10`/`Entity__MoodCue09` in this unit.

## Naming

`Entity__MoodCue14` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005EC98`.

`gEntityMoodHandlerTable` row 14. Body: sets the attenuation, requests voice 0 tone 0xD when `unk84 == 10`, and moves -0xA along local z every tick.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `func_8002CD08` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.
