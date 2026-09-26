# Entity__MoodCue05

> Renamed from `func_8005E480` on 2026-09-23 (tools/rename.py). Address 0x8005e480.

**Unit:** Entity_b · **Size:** 20 words · **Status:** MATCHED (20/20 words,
whole-image build verified byte-exact)

## What it does

One entry (function pointer + 3 data words per 16-byte row) of the
`this->moodIndex`-selected event-dispatch table `gEntityMoodHandlerTable`
(`asm/data/79528.data.s`, immediately after the `gEntityMoodTable` mood-row family;
not itself named/carved this round). Takes `this` and an output/state buffer
`out` (`EntityMoodHandlerArg`, a new opaque struct this round — see "Proposed
learning"): stores `this->methods->slot148(this)`'s result into `out->unk10`
UNCONDITIONALLY, then, only if `out->unk4 == 0`, also sets `out->unk1C = 0x17`.

`slot148` holds `Entity__GetProximityRatio` (still `addiu_at`-blocked in `Entity.c`, see
its own stub report), typed `s32 (*)(Entity *self)` here purely from this call
site's own register usage (a1/a2 not used, `$v0` consumed as a stored value).

## Final C

```c
void Entity__MoodCue05(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x17;
    }
}
```

## Attempt log

First attempt wrote the vtable call's result into a local `s32 result;` and
conditionally copied it into `out->unk10` alongside the `out->unk1C` store,
producing one spurious extra `nop` in the branch delay slot (retail folds the
`out->unk10 = result` store itself into the `bnez` branch's delay slot,
making it unconditional — this call's residue is a second instance of the
"unconditional default write, conditional extra write" idiom, see
`docs/DECOMPILATION_LEARNINGS.md`). Moving the store out of the `if` and
directly onto the call expression closed it on the second attempt.

## Proposed learning

**`EntityMoodHandlerArg`** (see `include/Entity.h`) is the second argument to
every entry of the `gEntityMoodHandlerTable` table (confirmed so far by
`Entity__MoodCue05`/`Entity__MoodCue10`/`Entity__MoodCue15`) — an opaque state/result
buffer, not an `Entity`. Whoever carves the remaining table entries
(`Entity__MoodCue07`, `Entity__MoodCue09`, `Entity__MoodCue11`, `Entity__MoodCue12`,
`Entity__MoodCue13`, `Entity__MoodCue14`, `Entity__MoodCue16` — all still `INCLUDE_ASM` in
this unit) should check whether they share this same second-parameter type
before inventing a new one.

## Naming

`Entity__MoodCue05` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E480`.

`gEntityMoodHandlerTable` row 5. Body: sets the attenuation from `getProximityRatio` and requests voice 0 tone 0x17 on tick 0. The smallest non-trivial callback.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
