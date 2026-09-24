# Entity__MoodCue10

> Renamed from `func_8005E7A8` on 2026-09-23 (tools/rename.py). Address 0x8005e7a8.

**Unit:** Entity_b · **Size:** 20 words · **Status:** MATCHED (20/20 words,
whole-image build verified byte-exact)

## What it does

Another `gEntityMoodHandlerTable` mood-dispatch table entry (see `Entity__MoodCue05`'s report
for the table). Takes `this` and an `EntityMoodHandlerArg *out`:
unconditionally clears `out->unk10`, then, only if `out->unk4 == 0`, sets
`out->unk1C`/`out->unk30`/`out->unk44` all to `0xB`. Finally, unconditionally,
calls `this->methods->slotC4(this, -0x1E, 0)`.

`slotC4` is a shared BasicClass-inherited slot: confirmed with
`tools/classtable.py` that BOTH this unit's own vtable (`ENTITY_METHODS`) and
Class65650's vtable (`gClass65650Methods`, `include/code_55dd4.h`) hold the identical
function (`BaseObjO__func_5748c`) at `+0xC4`, and `code_55dd4.h`'s own
`Class65650Methods.slotC4` already documents the exact same call shape
(`slotC4(self, -0x1E, 0)`, from `func_80066150`) — so this is not a
coincidence, it is the shared ancestor's method, reached the same way in two
unrelated classes.

## Final C

```c
void Entity__MoodCue10(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0xB;
        out->unk30 = 0xB;
        out->unk44 = 0xB;
    }
    this->methods->slotC4(this, -0x1E, 0);
}
```

## Attempt log

Matched on the first attempt — the `out->unk10 = 0;` unconditional store
ahead of the gated block was read directly off the disassembly's delay-slot
placement (the branch's delay slot performs the store regardless of which way
the branch goes), so no reshaping was needed here (unlike `Entity__MoodCue05`,
which needed a correction for the analogous shape).

## Proposed learning

Cross-referencing `tools/classtable.py`'s output for a suspicious 3-argument
vtable call against `include/code_55dd4.h`'s already-documented
`Class65650Methods` slots is a fast way to confirm a shared-ancestor slot's
signature without deriving it from scratch — the same physical function
occupying the same offset in two different classes' tables is strong
corroboration, not just a coincidence to note.

## Naming

`Entity__MoodCue10` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E7A8`.

`gEntityMoodHandlerTable` row 10. Body: attenuation 0. On tick 0 it requests tone 0xB on all three voices. Every tick it moves -0x1E along local z.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.
