# Entity__MoodCue13

> Renamed from `func_8005EBB4` on 2026-09-23 (tools/rename.py). Address 0x8005ebb4.

**Unit:** Entity · **Size:** 57 words · **Status:** MATCHED (57/57 words,
whole-image build verified byte-exact)

## What it does

Another `gEntityMoodHandlerTable` mood-dispatch handler, `(Entity *this,
EntityMoodHandlerArg *out)`. Sets `out->unk10` from `slot148`, then branches
on `out->unk4`: if zero, sets `out->unk1C = 0xC` and increments
`this->unk44`; otherwise (nonzero), clamps `out->unk4` to `-1` once it grows
past `this->unk80 - 1`. Afterward, unconditionally: if `this->unk44 == 0x24`,
rolls `rand() % 3 == 0` and calls `this->methods->slot30(this, 0xB)` on a hit.

## Final C

```c
void Entity__MoodCue13(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0xC;
        this->unk44++;
    } else if (out->unk4 >= this->unk80 - 1) {
        out->unk4 = -1;
    }
    if (this->unk44 == 0x24) {
        if (rand() % 3 == 0) {
            this->methods->slot30(this, 0xB);
        }
    }
}
```

`this->unk80` reuses the field established the same round in
`Entity__MoodCue09.md`. `rand() % 3` reproduces retail's magic-multiply
sign-corrected remainder exactly, per the `x % N` idiom already confirmed in
`docs/DECOMPILATION_LEARNINGS.md`.

## Attempt log

Two attempts. The first attempt wrote the `if`/`else` with the branches
SWAPPED (`if (out->unk4 != 0) { clamp } else { unk1C/unk44 }`) — same
semantics, but retail lays the physical code out with the `out->unk4 == 0`
body as the fall-through and the `>= this->unk80 - 1` body reached by an
explicit `bnez`/jump-back. Reading the raw disassembly's branch DIRECTION
(`bnez $v1, <clamp-block>` — branches to the clamp block when `out->unk4` is
NONZERO, falls through to the `unk1C`/`unk44++` block when zero) showed the
retail source tests `out->unk4 == 0` FIRST, not `!= 0` first; swapping which
condition is written first in the `if`/`else` matched immediately, with no
other change needed (word length was already correct on attempt 1 — this was
a pure physical-layout swap, not a missing/extra instruction).

## Proposed learning

**For an `if`/`else` where GCC lays the "then" body as the fall-through and
the "else" body as a jump target, the WRITTEN CONDITION determines which
body is which** — swapping to `if (!cond) A else B` (equivalent to `if (cond)
B else A`) changes nothing semantically but flips which physical block is
the fall-through. When a residue is "right content, wrong position, same
register set, same instruction count," check whether the retail branch
TESTS THE OPPOSITE of what was written, not just whether the branch target
label matches.

## Naming

`Entity__MoodCue13` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005EBB4`.

`gEntityMoodHandlerTable` row 13. Body: sets the attenuation. On tick 0 it requests voice 0 tone 0xC and increments `unk44`. At tick `unk80 - 1` it sets the set's tick back to -1, which restarts the loop. When `unk44` reaches 36, a 1-in-3 roll calls `notifyParents(this, 0xB)`.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

`notifyParents(this, 0xB)` is `ENTITY_EFFECT_EVENT_VIDEO`. A comment explains `out->tick = -1` (SoundCueSet.h: the callback may set -1 to restart the count). Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs and `state` phases (hex remains only for masks). Byte-identical (whole image green).
