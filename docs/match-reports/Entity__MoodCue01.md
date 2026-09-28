# Entity__MoodCue01

> Renamed from `func_8005E3C4` on 2026-09-23 (tools/rename.py). Address 0x8005e3c4.

**Unit:** Entity · **Size:** 47 words · **Status:** MATCHED (47/47 words,
whole-image build verified byte-exact)

## What it does

One of this unit's `gEntityMoodHandlerTable` mood-dispatch handlers (same family as
`Entity__MoodCue05`/`Entity__MoodCue10`/`Entity__MoodCue15`), taking `(Entity *this,
EntityMoodHandlerArg *out)`. Unconditionally zeroes `out->unk10`, then — only
when `out->unk4 == 0` — sets three `out` fields to `0x14` and fires a call
through a SECOND, DIFFERENT object's vtable: `this->unk94->methods->slot130`,
not `this->methods->slot130`. Unconditionally calls the still-uncarved
`SceneNode__FaceTarget`, then `this->methods->slotC4`, then conditionally
`this->methods->slot30` when `this->unkFC == 0x1E`.

The `this->unk94->methods->slot130(this->unk94, 1)` call is the interesting
part: it resolves a genuine two-argument call through `Unk94Obj`'s OWN vtable
(new struct added to `include/Entity.h` this round, see below) — a different
table from `Entity`'s own `EntityMethods`, even though both happen to have
something at offset `+0x130`. Conflating them would have forced
`EntityMethods::slot130` (already typed self-only, matched via
`Entity__StopSoundCue` in `src/world/Entity.c`) to grow a spurious second argument and
broken that OTHER unit's already-matched function.

## New struct: `Unk94Obj`/`Unk94Methods` (`include/Entity.h`)

`Entity::unk94` was previously `void *`, described only as "a pointer to SOME
object, real type unconfirmed" (known from `Entity__UpdateTargetProximity`'s call into the
still-uncarved `SceneNode__FaceTarget`, which dereferences it at `+0xC`/`+0x14`).
This function's own disassembly resolves two more facts about it:

- `lw $v0, 0(a0)` / `lw $v0, 0x130($v0)` / `jalr $v0` (with `a0` = the loaded
  `this->unk94`) is the class-framework method-call idiom from
  `docs/research/class-framework.md` — so offset `+0x00` of whatever
  `this->unk94` points at is ITS OWN method-table pointer.
- The call sets `$a1` explicitly (`ori $a1, $zero, 0x1`) before the `jalr` —
  per this round's "MIPS o32 fills argument registers left to right" learning,
  that is positive evidence of a real second argument, not leftover garbage.

Since `SceneNode__FaceTarget`'s body dereferences this object at `+0xC` as a pointer,
and `Entity::unk0C` (`this`'s OWN `+0xC`) is a plain `s32` flag, `unk94` is
provably NOT another `Entity` — despite `Entity__IsTargetInRange` (attempted the same
round, see its stall report) also reading `+0x14` off it as an `EntityPos *`,
the SAME convention `Entity::unk14` uses. Modeled as its own minimal type:

```c
struct Unk94Methods {
    u8 pad000[0x130];
    void (*slot130)(Unk94Obj *self, s32 arg1); /* called by Entity__MoodCue01 */
};

struct Unk94Obj {
    Unk94Methods *methods; /* +0x00 */
    u8 pad04[0x14 - 0x04];
    EntityPos *unk14;        /* +0x14, read by Entity__IsTargetInRange */
};
```

`Entity::unk94` retyped from `void *` to `Unk94Obj *`.

## Final C

```c
void Entity__MoodCue01(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0x14;
        out->unk30 = 0x14;
        out->unk44 = 0x14;
        this->unk94->methods->slot130(this->unk94, 1);
    }
    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
    this->methods->slotC4(this, -0x5A, 0);
    if (this->unkFC == 0x1E) {
        this->methods->slot30(this, 0xA);
    }
}
```

`out->unk10 = 0;` sits OUTSIDE the `if` because it is in the branch's delay
slot in the disassembly (executes on every path) — the "default value, then
conditionally overwritten" idiom from `docs/DECOMPILATION_LEARNINGS.md`,
except here the default write is a plain top-level statement rather than
folded into the branch itself, since the other three writes it precedes
(`unk1C`/`unk30`/`unk44`) are genuinely conditional.

## Attempt log

Matched on the first attempt, once `Unk94Obj`/`Unk94Methods` existed and
`Entity::unk94` was retyped.

## Proposed learning

**Two different objects' vtables can share a numeric slot offset by
coincidence, and conflating them corrupts an ALREADY-MATCHED function in a
different unit.** `this->unk94->methods->slot130` (2 args, this function) and
`this->methods->slot130` (1 arg, `Entity`'s own, matched via `Entity__StopSoundCue`
in `src/world/Entity.c`) are unrelated functions that only share the offset
`+0x130` because they live in different tables. Before typing a vtable call
through a field whose OWN class isn't pinned down, check whether the same
offset is already spoken for on `this`'s own table — and if the two call
sites disagree on arity, that is itself evidence they are different tables,
not evidence one of them is wrong.

## Naming

`Entity__MoodCue01` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E3C4`.

`gEntityMoodHandlerTable` row 1. Body: attenuation 0. On tick 0 it requests tone 0x14 on all three voices and calls the target's slot +0x130 with 1. Every tick it faces the target and moves -0x5A along local z. At moodTimer 30 it calls `notifyParents(this, 0xA)`.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

`notifyParents(this, 0xA)` is `ENTITY_EFFECT_LINK_STAGE`. Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs and `state` phases (hex remains only for masks). Byte-identical (whole image green).
