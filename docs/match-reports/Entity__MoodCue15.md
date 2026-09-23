# Entity__MoodCue15

> Renamed from `func_8005ED10` on 2026-09-23 (tools/rename.py). Address 0x8005ed10.

**Unit:** Entity_b · **Size:** 8 words · **Status:** MATCHED (8/8 words,
whole-image build verified byte-exact)

## What it does

Smallest `gEntityMoodHandlerTable` mood-dispatch table entry (see `Entity__MoodCue05`'s
report). No vtable call at all: if `this->unkFC == 0`, sets `out->unk10 = 0`
and `out->unk1C = 0xF`.

## Final C

```c
void Entity__MoodCue15(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        out->unk10 = 0;
        out->unk1C = 0xF;
    }
}
```

## Attempt log

Matched on the first attempt — exactly as predicted by the sizing note (a
16-line body with no calls).

## Proposed learning

None new.

## Naming

`Entity__MoodCue15` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005ED10`.

`gEntityMoodHandlerTable` row 15. Body: on moodTimer 0 only, attenuation 0 and voice 0 tone 0xF.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `func_8002CD08` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.
