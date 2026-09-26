# Entity__MoodCue12

> Renamed from `func_8005EA94` on 2026-09-23 (tools/rename.py). Address 0x8005ea94.

**Unit:** Entity_b · **Size:** 72 words · **Status:** MATCHED (72/72 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. A state-machine step for `this->unk44` (the same
small state code this unit's mood-dispatch handlers write literals into —
see `Entity__MoodCue13.md`):

1. If `this->unkFC == 0` and a coin flip (`rand() & 1 == 0`) lands, sets
   `this->unk44 = 0xB`.
2. If `this->unk14->y < 0x7D0` (2000), fires the still-uncarved
   `Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0)`.
3. Then, depending on `this->unk44`:
   - `== 0xB`: calls `this->methods->slot144(this, this->unk94)`; if the
     result is `< 0xA00` (2560), calls `this->unk4C->methods->slot138(this->unk4C,
     1, 1)`, sets `this->unkFC = 1`, and advances the state to `0xC`.
   - `== 0xC`: increments `this->unkFC`, and if the value it held BEFORE the
     increment was `0x12C` (300), calls `this->methods->slot30(this, 0xC)`.

## New field: `Entity::unk4C` retyped, and `Unk4CObj`/`Unk4CMethods` added

`Entity::unk4C` was modeled as a plain `s32` (only known write:
`this->unk4C = 0;` in `Entity__DetachFromParent`, `src/Entity.c`) — which type-checks
identically against a NULL pointer, so retyping it doesn't touch that
already-matched function. This function dereferences it at `+0x00` as a
method-table pointer (the same class-framework idiom used everywhere else in
this unit) and calls its own `+0x138` slot with two extra literal-`1`
arguments:

```c
struct Unk4CMethods {
    u8 pad000[0x138];
    void (*slot138)(Unk4CObj *self, s32 arg1, s32 arg2);
};

struct Unk4CObj {
    Unk4CMethods *methods; /* +0x00 */
};
```

## `EntityMethods::slot144` — the retyping that unblocked BOTH this function and `Entity__IsTargetInRange`

This function's OWN call to `slot144` is what settled a stall from the
previous round (see `Entity__IsTargetInRange.md`, now matched). Retail sets `$a1`
explicitly right before the `jalr`:

```
lw   $v0, 0x0($s0)
lw   $a1, 0x94($s0)     ; this->unk94 -- nothing overwrites $a1 before the jalr
lw   $v0, 0x144($v0)
jalr $v0
 move $a0, $s0
```

— proof, per `docs/DECOMPILATION_LEARNINGS.md`'s "value in an argument
register live at the next call IS an argument" rule, that `slot144` takes a
second parameter. `Entity__IsTargetInRange`'s own call to the SAME slot looked
1-argument (a plain `nop` delay slot) only because that function already had
`this->unk94` resident in `$a1` from earlier in its own body — no fresh load
needed. Fixing the signature here is what let `Entity__IsTargetInRange` finally match
too; see that report for the full account.

## Final C

```c
void Entity__MoodCue12(Entity *this) {
    s32 y;
    s32 result;
    s32 oldFC;

    if (this->unkFC == 0) {
        if ((rand() & 1) == 0) {
            this->unk44 = 0xB;
        }
    }
    y = this->unk14->y;
    if (y < 0x7D0) {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    }
    if (this->unk44 == 0xB) {
        result = this->methods->slot144(this, this->unk94);
        if (result < 0xA00) {
            this->unk4C->methods->slot138(this->unk4C, 1, 1);
            this->unkFC = 1;
            this->unk44 = 0xC;
        }
    } else if (this->unk44 == 0xC) {
        oldFC = this->unkFC;
        this->unkFC = oldFC + 1;
        if (oldFC == 0x12C) {
            this->methods->slot30(this, 0xC);
        }
    }
}
```

## Attempt log

Matched on the first attempt, once `Unk4CObj`/`Unk4CMethods` existed and
`slot144` was retyped to two arguments (both landed in the same working
session as this function, see `Entity__IsTargetInRange.md`).

## Proposed learning

Already captured in full in `Entity__IsTargetInRange.md`'s "Round 8" section: cross-
check every caller of a shared vtable slot before accepting a 1-argument
signature on the strength of one call site's unremarkable-looking bytes.
This function was the SECOND caller that exposed the gap the first caller's
own bytes couldn't.

## Naming

`Entity__MoodCue12` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005EA94`.

`gEntityMoodHandlerTable` row 12. Body (takes only `this`): on moodTimer 0 a coin flip may set phase `unk44 = 0xB`. It faces the target while its own y is below 0x7D0. In phase 0xB, once `distanceToRegion` to the target is below 0xA00, it calls `unk4C`'s slot +0x138(1, 1), sets moodTimer to 1 and enters phase 0xC. In phase 0xC it advances moodTimer itself and calls `notifyParents(this, 0xC)` at 300.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
